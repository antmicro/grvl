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
        Container()
            : Component()
            , childDropped(false)
            , BackgroundImage(NULL)
            , activeChild(NULL)
        {
        }

        Container(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
            , childDropped(false)
            , BackgroundImage(NULL)
            , activeChild(NULL)
        {
        }

        Container(const Container& Obj)
            : Component(Obj)
            , childDropped(false)
            , BackgroundImage(Obj.BackgroundImage)
            , activeChild(NULL)
        {
        }

        virtual ~Container();

        Container& operator=(const Container& Obj);

        virtual void CheckPlacement();
        vector<Component*>& GetElements();
        Component* GetElement(const char* id);
        virtual void AddElement(Component* el);
        void SetBackgroundImage(Image* image);

        virtual Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY,
                                                  int32_t modificator = 0);

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth,
                          int32_t ParentHeight)
            = 0;

        virtual void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder);
        static duk_ret_t JSGetElementByIdWrapper(duk_context* ctx);

    protected:
        vector<Component*> Elements;
        bool childDropped;
        Image* BackgroundImage;
        Component* activeChild;

        virtual void InitFromXML(XMLElement* xmlElement);
    };

} /* namespace grvl */

#endif /* GRVL_CONTAINER_H_ */
