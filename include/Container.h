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

#ifndef GRVL_CONTAINER_H_
#define GRVL_CONTAINER_H_

#include "Component.h"
#include "Image.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

namespace grvl {
    class Manager;

    /// Represents base container class for panel and pop-up window.
    class Container : public Component {
    public:
        Container() = default;

        Container(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
        {
        }

        Container(const Container& other);
        Container& operator=(const Container& other);

        virtual ~Container();

        virtual void CheckPlacement();
        void SetBackgroundImage(Image* image);

        vector<Component*>& GetElements();
        Component* GetElement(const char* id);
        Component* GetElementByIndex(int index);
        bool HasElement(const char* ComponentID);
        virtual void AddElement(Component* el);
        virtual void RemoveElement(const char* elementId);

        void SetIsFocused(bool value) override;
        bool IsSelection() const { return isSelection; }
        virtual void SetAsSelection(bool value);
        virtual bool SetCurrentlySelectedItem(const char* elementId);
        virtual void SetCurrentlySelectedComponent(Component* component);
        virtual void ClearSelection();

        Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator = 0) override;

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;
        static duk_ret_t JSGetElementByIdWrapper(duk_context* ctx);
        static duk_ret_t JSGetElementByIndexWrapper(duk_context* ctx);
        static duk_ret_t JSHasElementWrapper(duk_context* ctx);
        static duk_ret_t JSAddElementWrapper(duk_context* ctx);
        static duk_ret_t JSRemoveElementWrapper(duk_context* ctx);
        static duk_ret_t JSSetCurrentlySelectedItemWrapper(duk_context* ctx);
        static duk_ret_t JSGetNumberOfComponentsWrapper(duk_context* ctx);

    protected:
        std::vector<Component*> Elements;
        Image* BackgroundImage;
        Component* lastActiveChild;
        bool childDropped;

        bool isSelection{false};

        virtual void InitFromXML(XMLElement* xmlElement);

        virtual Component* TryToGetElementFromChildContainer(Component* possible_container, const char* searched_component_id);

        virtual void copyComponents(const std::vector<Component*>& other);
        virtual void deleteContainerComponents();
    };

} /* namespace grvl */

#endif /* GRVL_CONTAINER_H_ */
