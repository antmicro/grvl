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
    /// * font                    - caption text (default: none)
    /// * textColor               - caption text color (default: black)
    /// * activeTextColor         - caption text color when pressed (default: textColor)
    /// * backgroundColor         - background color (default: transparent)
    /// * frameColor              - widget frame color (default: transparent)
    /// * selectedFrameColor      - widget frame color when pressed (default: frameColor)
    /// * switchColor             - switching element color (default: transparent)
    /// * activeSwitchColor       - switching element color when pressed (default: switchColor)
    /// * image                   - identifier of image content to use instead of switching element
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
            , SwitchColor(COLOR_ARGB8888_GRAY)
            , ActiveSwitchColor(COLOR_ARGB8888_LIGHTBLUE)
            , TextColor(COLOR_ARGB8888_BLACK)
            , ActiveTextColor(COLOR_ARGB8888_BLACK)
            , FrameColor(COLOR_ARGB8888_BLACK)
            , SelectedFrameColor(COLOR_ARGB8888_BLACK)
            , switchState(false)
            , previousSwitchState(false)
            , onSwitchON()
            , onSwitchOFF()
            , TextWidth(0)
        {
            BackgroundColor = COLOR_ARGB8888_LIGHTGRAY;
        }

        SwitchButton(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractButton(x, y, width, height)
            , SwitchColor(COLOR_ARGB8888_GRAY)
            , ActiveSwitchColor(COLOR_ARGB8888_LIGHTBLUE)
            , TextColor(COLOR_ARGB8888_BLACK)
            , ActiveTextColor(COLOR_ARGB8888_BLACK)
            , FrameColor(COLOR_ARGB8888_BLACK)
            , SelectedFrameColor(COLOR_ARGB8888_BLACK)
            , switchState(false)
            , previousSwitchState(false)
            , onSwitchON()
            , onSwitchOFF()
            , TextWidth(0)
        {
            BackgroundColor = COLOR_ARGB8888_LIGHTGRAY;
        }

        virtual ~SwitchButton();

        virtual void OnPress();
        virtual void OnRelease();
        virtual void OnClick();

        void SetSwitchColor(uint32_t color);
        void SetActiveSwitchColor(uint32_t color);
        void SetTextColor(uint32_t color);
        void SetActiveTextColor(uint32_t color);
        void SetFrameColor(uint32_t color);
        void SetSelectedFrameColor(uint32_t color);
        void SetSwitchState(bool state);

        uint32_t GetSwitchColor() const;
        uint32_t GetActiveSwitchColor() const;
        uint32_t GetTextColor() const;
        uint32_t GetActiveTextColor() const;
        uint32_t GetFrameColor() const;
        uint32_t GetSelectedFrameColor() const;
        bool GetSwitchState() const;

        void SetOnSwitchONEvent(const Event& event);
        void SetOnSwitchOFFEvent(const Event& event);

        static SwitchButton* BuildFromXML(XMLElement* xmlElement);

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);

    protected:
        uint32_t SwitchColor, ActiveSwitchColor, TextColor, ActiveTextColor, FrameColor, SelectedFrameColor;
        bool switchState, previousSwitchState;
        Event onSwitchON, onSwitchOFF;
        int32_t TextWidth;

        virtual Touch::TouchResponse ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY);
    };

} /* namespace grvl */

#endif /* GRVL_SWITCHBUTTON_H_ */
