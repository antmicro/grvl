// Copyright 2014-2024 Antmicro <antmicro.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "Container.h"
#include "Manager.h"
#include "XMLSupport.h"
#include "JSEngine.h"
#include <cassert>

namespace grvl {

    Container::~Container()
    {
        deleteContainerComponents();
    }

    void Container::deleteContainerComponents()
    {
        vector<Component*>::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }
    }

    Container::Container(const Container& other)
        : Component(other)
        , childDropped(false)
        , BackgroundImage(other.BackgroundImage)
        , lastActiveChild(NULL)
        , isSelection{other.isSelection}
    {
        ID = ID.append(std::to_string(uniqueID));
        copyComponents(other.Elements);
    }

    Container& Container::operator=(const Container& Obj)
    {
        if(this != &Obj) {
            Component::operator=(Obj);
            ID = ID.append(std::to_string(uniqueID));
            childDropped = Obj.childDropped;
            BackgroundImage = Obj.BackgroundImage;
            lastActiveChild = Obj.lastActiveChild;
            isSelection = Obj.isSelection;
            deleteContainerComponents();
            copyComponents(Obj.Elements);
        }
        return *this;
    }

    void Container::copyComponents(const std::vector<Component*>& other)
    {
        for (std::size_t index = 0; index < other.size(); ++index) {
            AddElement(other[index]->Clone());
        }
    }

    void Container::SetBackgroundImage(Image* image)
    {
        BackgroundImage = image;
    }

    void Container::SetAsSelection(bool value)
    {
        isSelection = value;
    }

    bool Container::SetCurrentlySelectedItem(const char* elementId)
    {
        for(auto& currentElement : Elements) {
            if(Container* container = dynamic_cast<Container*>(currentElement)) {
                if(container->SetCurrentlySelectedItem(elementId)) {
                    SetCurrentlySelectedComponent(currentElement);
                    return true;
                }
            }

            if(strcmp(currentElement->GetID(), elementId) == 0) {
                SetCurrentlySelectedComponent(currentElement);
                return true;
            }
        }

        return false;
    }

    void Container::SetCurrentlySelectedComponent(Component* component)
    {
        if (component == lastActiveChild) {
            return;
        }

        if (IsSelection())
        {
            ClearSelection();
            component->SetIsFocused(true);
        }
        lastActiveChild = component;
    }

    void Container::ClearSelection()
    {
        if (isSelection && lastActiveChild) {
            lastActiveChild->SetIsFocused(false);
            lastActiveChild = nullptr;
        }
    }

    void Container::SetIsFocused(bool value)
    {
        Component::SetIsFocused(value);

        if (!IsFocused())
        {
            ClearSelection();
        }
    }

    void Container::AddElement(Component* el)
    {
        assert(el && "Cannot pass null object");
        el->SetParentID(GetID());
        Elements.emplace_back(el);
    }

    void Container::RemoveElement(const char* elementId)
    {
        Component* foundComponent{nullptr};
        for (int index = 0; index < Elements.size(); ++index) {
            if (strcmp(Elements[index]->GetID(), elementId) == 0) {
                foundComponent = Elements[index];
                Elements.erase(Elements.begin() + index);
                delete foundComponent;
                return;
            }
        }
    }

    vector<Component*>& Container::GetElements()
    {
        return Elements;
    }

    Component* Container::GetElement(const char* SearchedComponentID)
    {
        for(auto& CurrentElement : Elements) {
            if(Component* component = TryToGetElementFromChildContainer(CurrentElement, SearchedComponentID)) {
                return component;
            }

            if(strcmp(CurrentElement->GetID(), SearchedComponentID) == 0) {
                return CurrentElement;
            }
        }
        return nullptr;
    }

    Component* Container::GetElementByIndex(int index)
    {
        assert(index < Elements.size());

        return Elements[index];
    }

    Component* Container::TryToGetElementFromChildContainer(Component* possible_container, const char* searched_component_id)
    {
        if (Container* container = dynamic_cast<Container*>(possible_container)) {
            return container->GetElement(searched_component_id);
        }

        return nullptr;
    }

    bool Container::HasElement(const char* ComponentID)
    {
        return GetElement(ComponentID) != nullptr;
    }

    void Container::CheckPlacement()
    {
        static constexpr auto bufferSize = 100;
        char elid[bufferSize];
        char pid[bufferSize];

        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(!Component::CheckSizeValidity(Elements[i])) {
                snprintf(elid, bufferSize, "%s", Elements[i]->GetID());
                snprintf(pid, bufferSize, "%s", this->GetID());
                if(strcmp(elid, "") == 0) {
                    sprintf(elid, "(unnamed)");
                }
                if(strcmp(pid, "") == 0) {
                    sprintf(pid, "(unnamed)");
                }
                grvl::Log("Element \"%s\" inside \"%s\" misplaced", elid, pid);
            }
        }
    }

    void Container::InitFromXML(tinyxml2::XMLElement* xmlElement)
    {
        Component::InitFromXML(xmlElement);

        SetAsSelection(XMLSupport::GetAttributeOrDefault(xmlElement, "selection", false));
    }

    Touch::TouchResponse Container::ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator)
    {
        if(tp.GetState() == Touch::Pressed) { // init state, find child
            ClearSelection();
            touchActive = false;
            childDropped = false;

            if(IsTouchPointInObject(tp.GetCurrentX() - ParentX, tp.GetCurrentY() - ParentY)) { // Container selected
                for(int32_t i = Elements.size() - 1; i >= 0; i--) {
                    if(Elements[i]->IsVisible() && Touch::TouchHandled == Elements[i]->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator)) { // Trigger onPress
                        SetCurrentlySelectedComponent(Elements[i]);
                        touchActive = true;
                        break;
                    }
                }
            } else {
                return Touch::TouchNotApplicable;
            }
        }

        if(touchActive) {
            if(childDropped || lastActiveChild == NULL) { // Process data
                if(tp.GetState() == Touch::Released) {
                    touchActive = false;
                    return Touch::TouchReleased;
                }

                return Touch::TouchHandled;
            }  // Push data

            if(tp.GetState() != Touch::Pressed) { // Not to trigger onPress twice.
                return lastActiveChild->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator);
            }

            if (tp.GetState() == Touch::Moving) {
                return Touch::LongTouchHandled;
            }

            return Touch::TouchHandled;
        }
        return Touch::TouchNotApplicable;
    }

    void Container::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AttachMemberFunction("GetElementById", Container::JSGetElementByIdWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("GetElementByIndex", Container::JSGetElementByIndexWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("HasElement", Container::JSHasElementWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("AddElement", Container::JSAddElementWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("RemoveElement", Container::JSRemoveElementWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("SetCurrentlySelectedItem", Container::JSSetCurrentlySelectedItemWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("GetNumberOfComponents", Container::JSGetNumberOfComponentsWrapper, 0);
    }

    duk_ret_t Container::JSGetElementByIdWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        Container* container = static_cast<Container*>(duk_to_pointer(ctx, -1));
        if (!container) {
            return 0;
        }

        const char* componentName = duk_to_string(ctx, 0);
        if (Component* foundComponent = container->GetElement(componentName)) {
            JSEngine::PushComponentAsJSObjectOntoStack(foundComponent);
            return 1;
        }

        return 0;
    }

    duk_ret_t Container::JSGetElementByIndexWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        Container* container = static_cast<Container*>(duk_to_pointer(ctx, -1));
        if (!container) {
            return 0;
        }

        int index = duk_to_int(ctx, 0);
        JSEngine::PushComponentAsJSObjectOntoStack(container->GetElementByIndex(index));
        return 1;
    }

    duk_ret_t Container::JSHasElementWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        Container* container = static_cast<Container*>(duk_to_pointer(ctx, -1));
        if (!container) {
            return 0;
        }

        const char* componentName = duk_to_string(ctx, 0);
        duk_push_boolean(ctx, container->HasElement(componentName));

        return 1;
    }

    duk_ret_t Container::JSAddElementWrapper(duk_context* ctx)
    {
        duk_get_prop_string(ctx, 0, JSObject::C_OBJECT_POINTER_KEY);
        Component* componentToAdd = static_cast<Component*>(duk_to_pointer(ctx, -1));

        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        Container* container = static_cast<Container*>(duk_to_pointer(ctx, -1));
        if (!container || !componentToAdd) {
            return 0;
        }

        container->AddElement(componentToAdd);

        return 0;
    }

    duk_ret_t Container::JSRemoveElementWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        AbstractView* container = static_cast<AbstractView*>(duk_to_pointer(ctx, -1));
        if (!container) {
            return 0;
        }

        const char* componentName = duk_to_string(ctx, 0);
        container->RemoveElement(componentName);

        return 0;
    }

    duk_ret_t Container::JSSetCurrentlySelectedItemWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        AbstractView* container = static_cast<AbstractView*>(duk_to_pointer(ctx, -1));
        if (!container) {
            return 0;
        }

        const char* componentName = duk_to_string(ctx, 0);
        container->SetCurrentlySelectedItem(componentName);

        return 0;
    }

    duk_ret_t Container::JSGetNumberOfComponentsWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        Container* container = static_cast<Container*>(duk_to_pointer(ctx, -1));
        if (!container) {
            return 0;
        }

        duk_push_int(ctx, container->GetElements().size());

        return 1;
    }

} /* namespace grvl */
