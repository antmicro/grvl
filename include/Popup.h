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

#ifndef GRVL_POPUP_H_
#define GRVL_POPUP_H_

#include "Container.h"
#include "Label.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Pop-up window widget.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    ///
    /// * backgroundColor         - background color (default: white)
    ///
    /// @remark
    /// XML node describing this widget can contain child nodes
    /// with components of type:
    /// * image
    /// * textView
    /// * message
    /// * button
    /// * switch
    /// * scrollBar
    /// * listItem
    /// * progressBar
    class Popup : public Container {
    public:
        Popup() = default;

        Popup(int32_t x, int32_t y, int32_t width, int32_t height)
            : Container(x, y, width, height)
        {
        }

        Popup(const Popup& Obj)
            : Container(Obj)
            , Message(new Label(*Obj.Message))
        {
        }

        Popup& operator=(const Popup& Obj);

        virtual ~Popup();

        void SetMessage(const char* message);
        void SetMessagePointer(Label* message);
        void SetTimestamp(uint64_t currentTimestamp);

        Label* GetMessagePointer();
        uint64_t GetTimetamp() const;

        static Popup* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

    protected:
        Label* Message{nullptr};
        uint64_t PopupTimestamp{0};
    };

} /* namespace grvl */

#endif /* GRVL_POPUP_H_ */
