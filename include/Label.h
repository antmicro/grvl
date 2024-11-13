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

#ifndef GRVL_LABEL_H_
#define GRVL_LABEL_H_

#include "Component.h"
#include "Font.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

#include <duktape.h>

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Widget displaying static text.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible (default: true)
    ///
    /// * text                    - text to display on the label (default: none)
    /// * font                    - text font (default: normal)
    /// * textColor               - text color (default: black)
    /// * backgroundColor         - widget background color (default: transparent)
    /// * frameColor              - widget frame color (default: transparent)
    /// * alignment               - horizontal alignment of the text (default: center)
    ///
    class Label : public Component {
    public:
        enum TextHorizontalAlignment {
            Left,
            Right,
            Center
        };

    protected:
        string Text;
        uint32_t FrameColor;
        TextHorizontalAlignment HorizontalAlignment;
        Font const* TextFont;

    public:
        Label()
            : Component()
            , Text("")
            , FrameColor(COLOR_ARGB8888_TRANSPARENT)
            , HorizontalAlignment(Left)
            , TextFont(NULL)
        {
        }

        Label(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
            , Text("")
            , FrameColor(COLOR_ARGB8888_TRANSPARENT)
            , HorizontalAlignment(Left)
            , TextFont(NULL)
        {
        }

        Label(int32_t x, int32_t y, int32_t width, int32_t height, const char* text, TextHorizontalAlignment alignment)
            : Component(x, y, width, height)
            , Text(text)
            , FrameColor(COLOR_ARGB8888_TRANSPARENT)
            , HorizontalAlignment(alignment)
            , TextFont(NULL)
        {
        }

        Label(const Label& Obj)
            : Component(Obj)
            , Text(Obj.Text)
            , FrameColor(Obj.FrameColor)
            , HorizontalAlignment(Obj.HorizontalAlignment)
            , TextFont(Obj.TextFont)
        {
        }

        virtual ~Label();

        Label& operator=(const Label& Obj);

        void SetText(const char* text);
        void SetHorizontalAlignment(TextHorizontalAlignment alignment);
        void SetFrameColor(uint32_t color);
        void SetTextFont(Font const* font);

        /// @return Label's text.
        const char* GetText();

        TextHorizontalAlignment GetMode();
        uint32_t GetFrameColor() const;
        Font const* GetTextFont();

        static Label* BuildFromXML(XMLElement* xmlElement);

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;

        GENERATE_DUK_STRING_GETTER(Label, Text, GetText)
        GENERATE_DUK_STRING_SETTER(Label, Text, SetText)
    };
} /* namespace grvl */

#endif /* GRVL_LABEL_H_ */
