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

#ifndef GRVL_GRIDROW_H_
#define GRVL_GRIDROW_H_

#include "AbstractButton.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

namespace grvl {

    /// Widget representing single row of a grid displayed by GridRow.
    /// The widget can contain buttons only.
    class GridRow : public AbstractButton {
    public:
        GridRow()
            : AbstractButton()
            , ElementWidth(0)
            , childDropped(false)
            , ignoreTouchModificator(false)
            , activeChild(NULL)
        {
        }

        GridRow(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractButton(x, y, width, height)
            , ElementWidth(0)
            , childDropped(false)
            , ignoreTouchModificator(false)
            , activeChild(NULL)
        {
        }

        virtual ~GridRow();

        virtual void AddElement(Component* item);

        virtual void SetSize(int32_t width, int32_t height);

        virtual Component* GetElement(const char* id);
        virtual Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator = 0);

        virtual void CheckPlacement();

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

        static GridRow* BuildFromXML(XMLElement* xmlElement);

        void SetElementWidth(int32_t elementWidth);

    protected:
        vector<Component*> Elements;
        int32_t ElementWidth;
        bool childDropped, ignoreTouchModificator;
        Component* activeChild;

        void ReorderElements();
    };

} /* namespace grvl */

#endif /* GRVL_GRIDROW_H_ */
