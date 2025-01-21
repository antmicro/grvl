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

#include "Container.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

namespace grvl {

    /// Widget representing single row of a grid displayed by GridRow.
    /// The widget can contain buttons only.
    class GridRow : public Container {
    public:
        GridRow()
            : Container()
            , ElementWidth(0)
            , childDropped(false)
            , ignoreTouchModificator(false)
            , lastActiveChild(NULL)
        {
        }

        GridRow(int32_t x, int32_t y, int32_t width, int32_t height)
            : Container(x, y, width, height)
            , ElementWidth(0)
            , childDropped(false)
            , ignoreTouchModificator(false)
            , lastActiveChild(NULL)
        {
        }

        void AddElement(Component* item) override;

        virtual void SetSize(int32_t width, int32_t height);

        virtual void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY);

        static GridRow* BuildFromXML(XMLElement* xmlElement);

        void SetElementWidth(int32_t elementWidth);
        void SetHorizontalOffset(int32_t horizontalOffset);

    protected:
        int32_t ElementWidth;
        int32_t HorizontalOffset{0};
        bool childDropped, ignoreTouchModificator;
        Component* lastActiveChild;

        void ReorderElements();
    };

} /* namespace grvl */

#endif /* GRVL_GRIDROW_H_ */
