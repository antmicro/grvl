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

#include <cstdint>
#include <grvl/JSObjectBuilder.h>
#include <grvl/Manager.h>
#include <grvl/component/Component.h>
#include <grvl/component/Label.h>

#include <algorithm>
#include <string_view>
#include <vector>

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

    void Label::SetWrapping(bool wrapping)
    {
        Wrapping = wrapping;
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
        result->SetWrapping(xml.Attribute("wrapping").ElseStyle().Parse<bool>().GetOr(false));

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

        painter.PushDrawingBound(RenderX, RenderY, RenderX + Width, RenderY + Height);

        if(BackgroundColor & alpha) {
            painter.FillRectangle(RenderX, RenderY, Width, Height, BackgroundColor);
        }

        if(TextFont) {
            std::vector<std::string> lines = SplitIntoLines(Text);

            const int32_t lineHeight = TextFont->GetFontHeight();
            const int32_t lineGap = lineHeight;
            const int32_t lineSpacing = lineHeight + lineGap;
            int32_t beginY = (Height + lineHeight - (static_cast<int32_t>(lines.size()) - 1) * lineSpacing) / 2;

            for(const std::string& line : lines) {
                const int32_t textWidth = TextFont->GetWidth(line.c_str());
                int32_t beginX = 0;

                switch(TextHorizontalAlignment) {
                    case HorizontalAlignment::Left:
                        beginX = 3;
                        break;
                    case HorizontalAlignment::Right:
                        beginX = Width - textWidth - 3;
                        break;
                    case HorizontalAlignment::Center:
                    default:
                        beginX = Width / 2 - textWidth / 2;
                        break;
                }

                painter.DisplayAntialiasedString(
                    TextFont, RenderX + beginX, RenderY + beginY, line.c_str(), TextColor);
                beginY += lineSpacing;
            }
        }

        DrawBorderIfNecessary(painter, ParentRenderX + X, ParentRenderY + Y, Width, Height);

        painter.PopDrawingBound();
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

        int32_t max = 0;
        auto lines = SplitIntoLines(Text);
        for(auto line : lines) {
            int32_t line_width = TextFont->GetWidth(line.c_str());
            if(line_width > max) {
                max = line_width;
            }
        }

        return max;
    }

    void Label::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("text", Label::JSGetTextWrapper, Label::JSSetTextWrapper);
        jsObjectBuilder.AddProperty("textColor", Label::JSGetTextColorWrapper, Label::JSSetTextColorWrapper);
        jsObjectBuilder.AddProperty("textWidth", Label::JSGetTextWidthWrapper);
        jsObjectBuilder.AddProperty("wrapping", Label::JSGetWrappingWrapper, Label::JSSetWrappingWrapper);
    }

    std::vector<std::string> Label::SplitIntoLines(std::string_view text) const
    {
        std::vector<std::string> lines;
        const int32_t maxWidth = std::max(0, Width - 6);

        auto isWhitespace = [](char c) {
            return c == ' ' || c == '\t';
        };

        size_t paragraphStart = 0;

        while(paragraphStart <= text.size()) {
            const size_t paragraphEnd = text.find('\n', paragraphStart);
            const std::string_view paragraph = text.substr(paragraphStart, paragraphEnd - paragraphStart);

            std::string line;
            size_t pos = 0;

            while(pos < paragraph.size()) {
                if(isWhitespace(paragraph[pos])) {
                    const char c = paragraph[pos++];

                    std::string candidate = line + c;

                    if(Wrapping && !line.empty() && TextFont->GetWidth(candidate.c_str()) > maxWidth) {
                        lines.push_back(std::move(line));
                        line = c;
                    } else {
                        line += c;
                    }

                    continue;
                }

                const size_t start = pos;

                while(pos < paragraph.size() && !isWhitespace(paragraph[pos])) {
                    ++pos;
                }

                const std::string word(paragraph.substr(start, pos - start));

                if(Wrapping && !line.empty() && TextFont->GetWidth((line + word).c_str()) > maxWidth) {
                    lines.push_back(std::move(line));
                    line = word;
                } else {
                    line += word;
                }
            }

            lines.push_back(std::move(line));

            if(paragraphEnd == std::string_view::npos) {
                break;
            }

            paragraphStart = paragraphEnd + 1;
        }

        return lines;
    }
} /* namespace grvl */
