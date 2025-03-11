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

#ifndef GRVL_SWITCHBUTTON_H_
#define GRVL_SWITCHBUTTON_H_

#include "AbstractButton.h"
#include "Event.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"
#include "JSObjectBuilder.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Represents switch (flip-flop) button.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// XML events:
    /// * onClick                 - event invoked when touch is released, but only when it has not left the boundaries
    ///                             of widget since pressing and long press was not reported
    /// * onPress                 - event invoked when touch is detected in the boundaries of the widget
    /// * onRelease               - event invoked when touch is released in the boundaries of the widget
    /// * onLongPress             - event invoked when the widget is pressed for longer than a second
    /// * onLongPressRepeat       - event invoked periodically (every half a second) while the widget is being pressed
    /// * onSwitchON              - event invoked when switch button is switched to ON mode
    /// * onSwitchOFF             - event invoked when switch button is switched to OFF mode
    ///
    class SwitchButton : public AbstractButton {
    public:
        SwitchButton()
            : AbstractButton()
            , switchState(false)
            , previousSwitchState(false)
            , onSwitchON()
            , onSwitchOFF()
        {
            BackgroundColor = COLOR_ARGB8888_LIGHTGRAY;
        }

        SwitchButton(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractButton(x, y, width, height)
            , switchState(false)
            , previousSwitchState(false)
            , onSwitchON()
            , onSwitchOFF()
        {
            BackgroundColor = COLOR_ARGB8888_LIGHTGRAY;
        }

        virtual void OnPress();
        virtual void OnRelease();
        virtual void OnClick();

        void SetSwitchState(bool state);
        bool GetSwitchState() const;

        void SetOnSwitchONEvent(const Event& event);
        void SetOnSwitchOFFEvent(const Event& event);
        void SetStateIndicatorWidth(uint32_t value);
        void SetStateIndicatorHeight(uint32_t value);
        void SetStateIndicatorArcRadius(uint32_t value);

        static SwitchButton* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;

        GENERATE_DUK_BOOLEAN_GETTER(SwitchButton, SwitchState, GetSwitchState)

    protected:
        virtual void DrawActiveState(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY, int32_t RenderWidth, int32_t RenderHeight);
        virtual void DrawInactiveState(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY, int32_t RenderWidth, int32_t RenderHeight);

        bool switchState{false};
        bool previousSwitchState{false};
        Event onSwitchON{};
        Event onSwitchOFF{};

        uint32_t stateIndicatorWidth{0};
        uint32_t stateIndicatorHeight{0};
        uint32_t stateIndicatorArcRadius{0};

        virtual Touch::TouchResponse ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY);
    };

} /* namespace grvl */

#endif /* GRVL_SWITCHBUTTON_H_ */
