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

#include "Label.h"
#include "Component.h"
#include "Manager.h"
#include "XMLSupport.h"
#include "JSObject.h"
#include "JSObjectBuilder.h"

namespace grvl {

    Label::~Label()
    {
    }

    void Label::SetText(const char* text)
    {
        Text = string(text);
    }

    void Label::SetHorizontalAlignment(TextHorizontalAlignment alignment)
    {
        HorizontalAlignment = alignment;
    }

    void Label::SetFrameColor(uint32_t color)
    {
        FrameColor = color;
    }

    Label::TextHorizontalAlignment Label::GetMode()
    {
        return HorizontalAlignment;
    }

    uint32_t Label::GetFrameColor() const
    {
        return FrameColor;
    }

    Label* Label::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        Label* result = new Label();
        result->InitFromXML(xmlElement);

        result->SetText(XMLSupport::GetAttributeOrDefault(xmlElement, "text", ""));

        const auto* fontName = XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal");
        const auto* font = man->GetFontPointer(fontName);
        if(!font){
            grvl::Log("[WARNING] Setting invalid label font %s, the label isn't going to be drawn", fontName);
        }
        result->SetTextFont(font);

        result->SetFrameColor(XMLSupport::ParseColor(xmlElement, "frameColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));

        result->SetHorizontalAlignment(
            XMLSupport::ParseAlignmentOrDefault(xmlElement, "alignment", Label::Center));

        return result;
    }

    void Label::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        static constexpr auto alpha = 0xff000000;

        if(!Visible) {
            return;
        }

        // Background
        if(BackgroundColor && alpha > 0) { // TODO: probably "&" was intended?
            painter.FillRectangle(ParentX + X, ParentY + Y, Width, Height, BackgroundColor);
        }
        if(!TextFont) return;
        int32_t TextWidth = TextFont->GetWidth(Text.c_str());
        uint16_t BeginX = 0;
        uint16_t BeginY = Y + (Height / 2) - (TextFont->GetHeight() / 2);
        switch(HorizontalAlignment) {
            case Left:
                BeginX = X + 3;
                break;
            case Right:
                BeginX = X + Width - TextWidth - 3;
                break;
            case Center:
            default:
                BeginX = X + (Width / 2) - (TextWidth / 2);
                break;
        }

        painter.DisplayAntialiasedString(TextFont, ParentX + BeginX, ParentY + BeginY, Text.c_str(), ForegroundColor);

        // Frame
        if(FrameColor && alpha > 0) {
            painter.DrawRectangle(ParentX + X, ParentY + Y, Width, Height, FrameColor);
        }
    }

    void Label::SetTextFont(Font const* font)
    {
        TextFont = font;
    }

    Font const* Label::GetTextFont()
    {
        return TextFont;
    }

    Label& Label::operator=(const Label& Obj)
    {
        if(this != &Obj) {
            Component::operator=(Obj);
            Text = Obj.Text;
            FrameColor = Obj.FrameColor;
            HorizontalAlignment = Obj.HorizontalAlignment;
            TextFont = Obj.TextFont;
        }
        return *this;
    }

    const char* Label::GetText()
    {
        return Text.c_str();
    }

    void Label::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("text", Label::JSGetTextWrapper, Label::JSSetTextWrapper);
    }

} /* namespace grvl */
