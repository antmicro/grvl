// Copyright 2014-2025 Antmicro <antmicro.com>
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

#include <grvl/Manager.h>
#include <grvl/component/KeyboardKey.h>

namespace grvl {

    Component* KeyboardKey::Clone() const
    {
        return new KeyboardKey(*this);
    }

    KeyboardKey* KeyboardKey::BuildFromXML(XmlElement xml)
    {
        Manager& man = Manager::GetInstance();
        KeyboardKey* result = new KeyboardKey();

        result->InitFromXML(xml);

        result->SetTextColor(xml.Attribute("textColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_WHITE));
        result->SetActiveTextColor(xml.Attribute("activeTextColor").ElseStyle().ParseColor().GetOr(result->GetTextColor()));

        const std::string secondaryText = xml.Attribute("secondaryText").ElseStyle().GetOr("");
        const std::string secondaryTextFont = xml.Attribute("secondaryTextFont").ElseStyle().GetOr("normal");

        result->SetSecondaryText(secondaryText.c_str());
        result->SetSecondaryTextFont(man.GetFontPointer(secondaryTextFont.c_str()));

        result->SetSecondaryTextColor(xml.Attribute("secondaryTextColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_WHITE));
        result->SetActiveSecondaryTextColor(xml.Attribute("activeSecondaryTextColor").ElseStyle().ParseColor().GetOr(result->GetSecondaryTextColor()));

        result->SetTextTopOffset(xml.Attribute("text_top_offset").ElseStyle().Parse<uint32_t>().GetOr(0));

        result->SetOnClickEvent(man.GetOrCreateCallback(xml.Attribute("onClick").ParseCallback().GetOr({})));

        if(auto image = xml.Attribute("image").ElseStyle()) {
            Image buttonImg {};
            man.BindImageContentToImage(image.Get(), &buttonImg);
            result->SetImage(buttonImg);
            int32_t imgX = xml.Attribute("image_x").ElseStyle().Parse<int32_t>().GetOr(-1);
            int32_t imgY = xml.Attribute("image_y").ElseStyle().Parse<int32_t>().GetOr(-1);

            if(imgX == -1 || imgY == -1) {
                result->SetImageCentered(true);
                imgX = result->GetWidth() / 2 - buttonImg.GetWidth() / 2;
                imgY = result->GetHeight() / 2 - buttonImg.GetHeight() / 2;
            }
            result->SetImagePosition(imgX, imgY);
        }

        result->SetOnLongPressEvent(man.GetOrCreateCallback(xml.Attribute("onLongPress").ParseCallback().GetOr({})));
        result->SetOnLongPressRepeatEvent(man.GetOrCreateCallback(xml.Attribute("onLongPressRepeat").ParseCallback().GetOr({})));

        return result;
    }

    void KeyboardKey::SetParentKeyboard(Keyboard* keyboard)
    {
        parentKeyboard = keyboard;
    }

    void KeyboardKey::SetSecondaryText(const char* text)
    {
        secondaryText = text;
    }

    void KeyboardKey::SetSecondaryTextFont(Font* font)
    {
        secondaryTextFont = font;
    }

    void KeyboardKey::SetSecondaryTextColor(uint32_t color)
    {
        secondaryTextColor = color;
    }

    void KeyboardKey::SetActiveSecondaryTextColor(uint32_t color)
    {
        activeSecondaryTextColor = color;
    }

    const char* KeyboardKey::GetSecondaryText()
    {
        return secondaryText.c_str();
    }

    void KeyboardKey::SwitchKeyValue()
    {
        if(secondaryText.empty()) {
            return;
        }

        currentTextValueIndex = (currentTextValueIndex + 1) % 2;
    }

    void KeyboardKey::DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        const char* primaryKeyValue = Text.c_str();
        const char* secondaryKeyValue = secondaryText.c_str();
        if(currentTextValueIndex == 1) {
            std::swap(primaryKeyValue, secondaryKeyValue);
        }

        uint32_t curPrimaryTextColor = COLOR_ARGB8888_TRANSPARENT;
        uint32_t curSecondaryTextColor = COLOR_ARGB8888_TRANSPARENT;
        if(State == On || State == Pressed || State == OnAndSelected || isFocused) {
            curPrimaryTextColor = GetActiveTextColor();
            curSecondaryTextColor = GetActiveSecondaryTextColor();
        } else if(State == Off || State == Released || State == OffAndSelected) {
            curPrimaryTextColor = GetTextColor();
            curSecondaryTextColor = GetSecondaryTextColor();
        }

        if(primaryKeyValue && ButtonFont != 0) {
            uint16_t TextSize = ButtonFont->GetWidth(primaryKeyValue);
            uint16_t BeginX = (RenderWidth / 2) - (TextSize / 2);
            float TempButtonHeightMultiplier = secondaryKeyValue ? 0.66 : 0.5;
            uint16_t BeginY = RenderHeight * TempButtonHeightMultiplier + (ButtonFont->GetFontHeight() / 2) + TextTopOffset;

            painter.DisplayBoundedAntialiasedString(
                ButtonFont,
                RenderX + BeginX,
                RenderY + BeginY,
                RenderX,
                RenderY,
                RenderWidth,
                RenderHeight,
                primaryKeyValue,
                curPrimaryTextColor);
        }

        if(secondaryKeyValue && secondaryTextFont) {
            uint16_t TextSize = secondaryTextFont->GetWidth(secondaryKeyValue);
            uint16_t BeginX = (RenderWidth / 2) - (TextSize / 2);
            uint16_t BeginY = (RenderHeight / 4) + (secondaryTextFont->GetFontHeight() / 2) + TextTopOffset;

            painter.DisplayBoundedAntialiasedString(
                secondaryTextFont,
                RenderX + BeginX,
                RenderY + BeginY,
                RenderX,
                RenderY,
                RenderWidth,
                RenderHeight,
                secondaryKeyValue,
                curSecondaryTextColor);
        }
    }

} /* namespace grvl */
