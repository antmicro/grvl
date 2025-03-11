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

#ifndef GRVL_PANEL_H_
#define GRVL_PANEL_H_

#include "Container.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Represents panel widget.
    ///
    /// Panel is a fixed top part of a screen that displays
    /// widgets that should be always visible.
    ///
    /// XML parameters:
    /// * id                      - identifier of a widget
    /// * height                  - widget height in pixels (default: 30)
    ///
    /// * backgroundColor         - background color (default: transparent)
    ///
    /// @remark
    /// XML node describing this widget can contain child nodes
    /// with components of type:
    /// * guiClock
    /// * image
    /// * textView
    /// * button
    /// * switch
    /// * scrollBar
    /// * listItem
    /// * progressBar
    ///
    class Panel : public Container {

    public:
        Panel() = default;

        Panel(int32_t x, int32_t y, int32_t width, int32_t height)
            : Container(x, y, width, height)
        {
        }

        Panel(const Panel& other) = default;
        Panel& operator=(const Panel& other) = default;

        Component* Clone() const override;

        static Panel* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;
    };

} /* namespace grvl */

#endif /* GRVL_PANEL_H_ */
