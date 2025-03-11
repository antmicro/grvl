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

#ifndef GRVL_GRIDVIEW_H_
#define GRVL_GRIDVIEW_H_

#include "GridRow.h"
#include "Manager.h"
#include "Painter.h"
#include "VerticalScrollView.h"
#include "stl.h"
#include "tinyxml2.h"

namespace grvl {
    class Manager;

    /// Grid view widget.
    ///
    /// This is a layout widget which organizes
    /// the screen by displaying inner widgets
    /// on a grid.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * scrollingEnabled        - indicates if scrolling is enabled (default: false)
    /// * overscrollEnabled       - indicates if scrolling beyond top/bottom is possible (default: false)
    /// * overscrollColor         - overscrolled area color (default: light gray)
    /// * overscrollHeight        - overscrolled area height in pixels (default: 50)
    /// * scrollIndicatorColor    - scroll indicator color (default: transparent)
    ///
    /// * elementWidth            - grid element width in pixels (default: 0)
    /// * elementHeight           - grid element height in pixel (default: 0)
    /// * verticalOffset          - vertical offset of grid elements (default: 0)
    ///
    /// * globalPanelVisible      - indicates if global panel is visible (default: true)
    /// * backgroundColor         - widget background color (default: transparent)
    /// * collection              - collection of elements
    ///
    /// XML events:
    /// * onSlideLeft             - event invoked when widget is scrolled to the left
    /// * onSlideRight            - event invoked when widget is scrolled to the right
    ///
    /// @remark
    /// XML node describing this widget can contain child nodes
    /// with components of type:
    /// * gridRow
    /// * panel (as a header)
    ///
    class GridView : public VerticalScrollView {

    public:
        GridView() = default;

        GridView(int32_t x, int32_t y, int32_t width, int32_t height)
            : VerticalScrollView(x, y, width, height)
        {
        }

        GridView(const GridView& other) = default;
        GridView& operator=(const GridView& other) = default;

        void SetGridParameters(uint32_t width, uint32_t height, uint32_t verticalOffset);
        void AddElement(Component* item);
        Component* Clone() const override;

        virtual void SetBackgroundColor(uint32_t color);

        virtual void PrepareToOpen() { }
        virtual void PrepareToClose() { }

        static GridView* BuildFromXML(XMLElement* xmlElement);

    protected:
        uint32_t ElementWidth{0};
        uint32_t ElementHeight{0};
        uint32_t VerticalOffset{0};
    };

} /* namespace grvl */

#endif /* GRVL_GRIDVIEW_H_ */
