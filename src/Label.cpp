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
        Text = string(text);
    }

    void Label::SetHorizontalAlignment(TextHorizontalAlignment alignment)
    {
        HorizontalAlignment = alignment;
    }

    Label::TextHorizontalAlignment Label::GetMode()
    {
        return HorizontalAlignment;
    }

    Label* Label::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        Label* result = new Label();
        result->InitFromXML(xmlElement);

        static const char* defaultTextColor = "0xFFFFFFFF";
        result->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", defaultTextColor));

        result->SetText(XMLSupport::GetAttributeOrDefault(xmlElement, "text", ""));

        const auto* fontName = XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal");
        const auto* font = man->GetFontPointer(fontName);
        if(!font){
            grvl::Log("[WARNING] Setting invalid label font %s, the label isn't going to be drawn", fontName);
        }
        result->SetTextFont(font);

        result->SetHorizontalAlignment(
            XMLSupport::ParseAlignmentOrDefault(xmlElement, "alignment", Label::Center));

        return result;
    }

    void Label::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        static constexpr auto alpha = 0xff000000;

        if (!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        if(BackgroundColor & alpha) {
            painter.FillRectangle(RenderX, RenderY, Width, Height, BackgroundColor);
        }
        if(!TextFont) return;
        int32_t TextWidth = TextFont->GetWidth(Text.c_str());
        uint16_t BeginX = 0;
        uint16_t BeginY = (Height / 2) - (TextFont->GetHeight() / 2);
        switch(HorizontalAlignment) {
            case Left:
                BeginX = 3;
                break;
            case Right:
                BeginX = Width - TextWidth - 3;
                break;
            case Center:
            default:
                BeginX = (Width / 2) - (TextWidth / 2);
                break;
        }

        painter.DisplayAntialiasedString(TextFont, RenderX + BeginX, RenderY + BeginY, Text.c_str(), TextColor);

        DrawBorderIfNecessary(painter, ParentRenderX + X, ParentRenderY + Y, Width, Height);
    }

    void Label::SetTextFont(Font const* font)
    {
        TextFont = font;
    }

    Font const* Label::GetTextFont()
    {
        return TextFont;
    }

    const char* Label::GetText()
    {
        return Text.c_str();
    }

    void Label::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("text", Label::JSGetTextWrapper, Label::JSSetTextWrapper);
        jsObjectBuilder.AddProperty("textColor", Label::JSGetTextColorWrapper, Label::JSSetTextColorWrapper);
    }

} /* namespace grvl */
