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

#ifndef GRVL_CUSTOMVIEW_H_
#define GRVL_CUSTOMVIEW_H_

#include "AbstractView.h"
#include "Painter.h"
#include "stl.h"

namespace grvl {

    /// Represents a screen type supporting any element placement except for listItems and gridRows.
    class CustomView : public AbstractView {
    public:
        CustomView()
            : AbstractView()
        {
        }

        CustomView(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractView(x, y, width, height)
        {
        }

        virtual ~CustomView();

        static CustomView* BuildFromXML(XMLElement* xmlElement);

        void AddElement(Component* element);
        virtual Component* GetElement(uint32_t index);
        virtual Component* GetElement(const char* id);
        virtual Component* GetLastElement();

        virtual Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY,
                                                  int32_t modificator = 0);

        virtual void CheckPlacement();

        virtual void OnPress();
        virtual void OnRelease();

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);
        virtual void PrepareToOpen();
        virtual void PrepareToClose();

    protected:
        vector<Component*> Elements;
    };

} /* namespace grvl */

#endif /* GRVL_CUSTOMVIEW_H_ */
