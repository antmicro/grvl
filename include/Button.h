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

#ifndef GRVL_BUTTON_H_
#define GRVL_BUTTON_H_

#include "AbstractButton.h"
#include "Font.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Represents rectangle button.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * text                    - caption text to display on the widget (default: "")
    /// * font                    - caption text font (default: "normal")
    /// * textColor               - caption text color (default: black)
    /// * activeTextColor         - caption text color while pressed (default: textColor)
    ///
    /// * icoChar                 - single-character icon for the button (default: none)
    /// * icoFont                 - single-character icon font (default: "normal")
    /// * icoColor                - single-character icon color (default: textColor)
    /// * activeIcoColor          - single-character icon color while pressed (default: textColor)
    ///
    /// * backgroundColor         - button background color (default: transparent)
    /// * activeBackgroundColor   - button background color while pressed (default: backgroundColor)
    ///
    /// * frameColor              - button frame color (default: transparent)
    /// * text_top_offset         - caption text top position offset
    ///
    /// * image                   - identifier of image content to display
    /// * image_x                 - image position on x axis in pixels (setting to -1 will center the image)
    /// * image_y                 - image position on y axis in pixels (setting to -1 will center the image)
    ///
    /// XML events:
    /// * onClick                 - event invoked when touch is released, but only when it has not left widget boundaries
    ///                             since pressing and long press was not reported
    /// * onPress                 - event invoked when touch is detected within widget boundaries
    /// * onRelease               - event invoked when touch is released within widget boundaries
    ///                             or when it leaves the boundaries
    /// * onLongPress             - event invoked when the widget is pressed for longer than a second
    /// * onLongPressRepeat       - event invoked periodically (every half a second) while the widget is being pressed
    ///
    class Button : public AbstractButton {
    public:
        Button()
            : AbstractButton()
            , IcoColor(COLOR_ARGB8888_BLACK)
            , ActiveIcoColor(COLOR_ARGB8888_BLACK)
            , IcoChar(-1)
            , TextTopOffset(0)
            , IcoFont(NULL)
            , imageCentered(false)
        {
            BackgroundColor = COLOR_ARGB8888_LIGHTGRAY;
        }

        Button(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractButton(x, y, width, height)
            , IcoColor(COLOR_ARGB8888_BLACK)
            , ActiveIcoColor(COLOR_ARGB8888_BLACK)
            , IcoChar(-1)
            , TextTopOffset(0)
            , IcoFont(NULL)
            , imageCentered(false)
        {
            BackgroundColor = COLOR_ARGB8888_LIGHTGRAY;
        }

        virtual ~Button();

        void SetTextColor(uint32_t color);
        void SetIcoColor(uint32_t color);
        void SetActiveTextColor(uint32_t color);
        void SetActiveIcoColor(uint32_t color);
        void SetIcoFont(Font const* font);
        void SetIcoChar(int16_t textIco);
        void SetImagePosition(int32_t x, int32_t y);
        void SetTextTopOffset(int32_t value);
        void SetImageCentered(bool isCentered);
        virtual void SetSize(int32_t width, int32_t height);

        void ClearIcoFont();
        void ClearIcoChar();

        uint32_t GetTextColor();
        uint32_t GetIcoColor() const;
        uint32_t GetActiveTextColor();
        uint32_t GetActiveIcoColor() const;
        Font const* GetIcoFont();

        static Button* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

        virtual void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder);

        GENERATE_DUK_UNSIGNED_INT_GETTER(Button, IcoColor, GetIcoColor)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Button, IcoColor, SetIcoColor)

        GENERATE_DUK_UNSIGNED_INT_GETTER(Button, ActiveIcoColor, GetActiveIcoColor)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Button, ActiveIcoColor, SetActiveIcoColor)
        
    protected:
        uint32_t IcoColor, ActiveIcoColor;
        int16_t IcoChar;
        int32_t TextTopOffset;
        Font const* IcoFont;
        bool imageCentered;
    };

} /* namespace grvl */

#endif /* GRVL_BUTTON_H_ */
