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

namespace grvl {

    Container::~Container()
    {
        vector<Component*>::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }
    }

    Container& Container::operator=(const Container& Obj)
    {
        if(this != &Obj) {
            Component::operator=(Obj);
            Elements = Obj.Elements;
            childDropped = Obj.childDropped;
            BackgroundImage = Obj.BackgroundImage;
            activeChild = Obj.activeChild;
        }
        return *this;
    }

    void Container::SetBackgroundImage(Image* image)
    {
        BackgroundImage = image;
    }

    void Container::AddElement(Component* el)
    {
        Elements.push_back(el);
    }

    vector<Component*>& Container::GetElements()
    {
        return Elements;
    }

    Component* Container::GetElement(const char* id)
    {
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(strcmp(Elements[i]->GetID(), id) == 0) {
                return Elements[i];
            }

            if(CustomView* screen = dynamic_cast<CustomView*>(Elements[i])) {
                return screen->GetElement(id);
            }

            if(VerticalScrollView* screen = dynamic_cast<VerticalScrollView*>(Elements[i])) {
                return screen->GetElement(id);
            }
        }
        return NULL;
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
    }

    Touch::TouchResponse Container::ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator)
    {
        if(tp.GetState() == Touch::Pressed) { // init state, find child
            touchActive = false;
            childDropped = false;

            bool ownCheck = IsTouchPointInObject(tp.GetCurrentX() - ParentX, tp.GetCurrentY() - ParentY);

            if(ownCheck) { // Container selected
                touchActive = true;
                for(int32_t i = Elements.size() - 1; i >= 0; i--) {
                    if(Touch::TouchHandled == Elements[i]->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator)) { // Trigger onPress
                        activeChild = Elements[i];
                        break;
                    }
                }
            } else {
                return Touch::TouchNotApplicable;
            }
        }

        if(touchActive) {
            if(childDropped || activeChild == NULL) { // Process data
                if(tp.GetState() == Touch::Released) {
                    return Touch::TouchReleased;
                    touchActive = false;
                }
                return Touch::TouchHandled;
            }  // Push data
            if(tp.GetState() != Touch::Pressed) { // Not to trigger onPress twice.
                Touch::TouchResponse childResponse = activeChild->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator);
                if(childResponse == Touch::TouchReleased || childResponse == Touch::TouchNotApplicable) { // Drop
                    childDropped = true;
                    activeChild = NULL;
                }
            }
            return Touch::TouchHandled;
        }
        return Touch::TouchNotApplicable;
        ;
    }

    void Container::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AttachMemberFunction("GetElementById", Panel::JSGetElementByIdWrapper, 1);
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

} /* namespace grvl */
