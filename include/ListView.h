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

#ifndef GRVL_LISTVIEW_H_
#define GRVL_LISTVIEW_H_

#include "ListItem.h"
#include "Painter.h"
#include "VerticalScrollView.h"
#include "stl.h"

namespace grvl {

    /// Vertical list view widget.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * overscrollEnabled       - indicates if scrolling beyond top/bottom is possible (default: false)
    /// * overscrollColor         - overscrolled area color (default: light gray)
    /// * overscrollHeight        - overscrolled area height in pixels (default: 50)
    /// * scrollIndicatorColor    - scroll indicator color (default: transparent)
    ///
    /// * splitLineColor          - color of the line drawn between list elements (default: transparent)
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
    /// * listItem
    /// * panel (as a header)
    ///
    class ListView : public VerticalScrollView {
    public:
        ListView()
            : VerticalScrollView()
        {
            SplitLineColor = COLOR_ARGB8888_BLACK;
            overscrollBarEnabled = true;
        }

        ListView(int32_t x, int32_t y, int32_t width, int32_t height)
            : VerticalScrollView(x, y, width, height)
        {
            SplitLineColor = COLOR_ARGB8888_BLACK;
            overscrollBarEnabled = true;
        }

        void AddElement(Component* component) override;
        void RemoveElement(const char* elementId) override;

        void SetVerticalGap(float value);

        void Refresh();

        // XML operations
        void ClearList();
        bool AddToList(Manager* man, string& listContent);

        static ListView* BuildFromXML(XMLElement* xmlElement);

    private:
        uint32_t verticalGap{0};
    };

} /* namespace grvl */

#endif /* GRVL_LISTVIEW_H_ */
