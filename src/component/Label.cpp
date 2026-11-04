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

#include <grvl/JSObjectBuilder.h>
#include <grvl/Manager.h>
#include <grvl/component/Component.h>
#include <grvl/component/Label.h>

namespace grvl {

    void Label::SetTextColor(uint32_t color)
    {
        TextColor = color;
    }

    Component* Label::Clone() const
    {
        return new Label(*this);
    }

    void Label::SetText(const char* text)
    {
        Text = std::string(text);
    }

    void Label::SetHorizontalAlignment(HorizontalAlignment alignment)
    {
        TextHorizontalAlignment = alignment;
    }

    HorizontalAlignment Label::GetMode()
    {
        return TextHorizontalAlignment;
    }

    Label* Label::BuildFromXML(XmlElement xml)
    {
        Manager& man = Manager::GetInstance();
        Label* result = new Label();
        result->InitFromXML(xml);

        result->SetTextColor(xml.Attribute("textColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_WHITE));
        result->SetText(xml.Attribute("text").ElseStyle().GetOr(""));

        result->SetTextFont(man.GetFontPointer(xml.Attribute("font").ElseStyle().GetOr("normal")));
        result->SetHorizontalAlignment(xml.Attribute("alignment").ElseStyle().ParseEnum(HORIZONTAL_ALIGNMENT_ENUM).GetOr(HorizontalAlignment::Center));

        return result;
    }

    void Label::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        static constexpr auto alpha = 0xff000000;

        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        painter.PushDrawingBoundsStackElement(RenderX, RenderY, RenderX + Width, RenderY + Height);

        if(BackgroundColor & alpha) {
            painter.FillRectangle(RenderX, RenderY, Width, Height, BackgroundColor);
        }
        if(!TextFont)
            return;
        int32_t TextWidth = TextFont->GetWidth(Text.c_str());
        uint16_t BeginX = 0;
        uint16_t BeginY = (Height / 2) + (TextFont->GetFontHeight() / 2);
        switch(TextHorizontalAlignment) {
            case HorizontalAlignment::Left:
                BeginX = 3;
                break;
            case HorizontalAlignment::Right:
                BeginX = Width - TextWidth - 3;
                break;
            case HorizontalAlignment::Center:
            default:
                BeginX = (Width / 2) - (TextWidth / 2);
                break;
        }

        painter.DisplayAntialiasedString(TextFont, RenderX + BeginX, RenderY + BeginY, Text.c_str(), TextColor);

        DrawBorderIfNecessary(painter, ParentRenderX + X, ParentRenderY + Y, Width, Height);

        painter.PopDrawingBoundsStackElement();
    }

    void Label::SetTextFont(Font* font)
    {
        TextFont = font;
    }

    Font* Label::GetTextFont()
    {
        return TextFont;
    }

    const char* Label::GetText()
    {
        return Text.c_str();
    }

    int32_t Label::GetTextWidth()
    {
        if(!TextFont)
            return 0;

        return TextFont->GetWidth(Text.c_str());
    }

    void Label::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("text", Label::JSGetTextWrapper, Label::JSSetTextWrapper);
        jsObjectBuilder.AddProperty("textColor", Label::JSGetTextColorWrapper, Label::JSSetTextColorWrapper);
        jsObjectBuilder.AddProperty("textWidth", Label::JSGetTextWidthWrapper);
    }

} /* namespace grvl */
