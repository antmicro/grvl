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

#include "KeyboardKey.h"

#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    Component* KeyboardKey::Clone() const
    {
        return new KeyboardKey(*this);
    }

    KeyboardKey* KeyboardKey::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        KeyboardKey* result = new KeyboardKey();

        result->InitFromXML(xmlElement);

        result->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", "#ffffffff"));
        result->SetActiveTextColor(XMLSupport::ParseColor(xmlElement, "activeTextColor", result->GetTextColor()));

        result->SetSecondaryText(XMLSupport::GetAttributeOrDefault(xmlElement, "secondaryText", ""));
        result->SetSecondaryTextFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "secondaryTextFont", "normal")));
        result->SetSecondaryTextColor(XMLSupport::ParseColor(xmlElement, "secondaryTextColor", "#ffffffff"));
        result->SetActiveSecondaryTextColor(XMLSupport::ParseColor(xmlElement, "activeSecondaryTextColor", result->GetTextColor()));

        result->SetTextTopOffset(XMLSupport::GetAttributeOrDefault(xmlElement, "text_top_offset", (uint32_t)0));

        result->SetOnClickEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onClick"))));

        const char* tempChar = xmlElement->Attribute("image");
        if(tempChar) {
            Image buttonImg(NULL, 0, 0, 0);
            man->BindImageContentToImage(tempChar, &buttonImg);
            result->SetImage(buttonImg);
            int32_t imgX = XMLSupport::GetAttributeOrDefault(xmlElement, "image_x", (uint32_t)-1);
            int32_t imgY = XMLSupport::GetAttributeOrDefault(xmlElement, "image_y", (uint32_t)-1);
            if(imgX == -1 || imgY == -1) {
                result->SetImageCentered(true);
                imgX = result->GetWidth() / 2 - buttonImg.GetWidth() / 2;
                imgY = result->GetHeight() / 2 - buttonImg.GetHeight() / 2;
            }
            result->SetImagePosition(imgX, imgY);
        }

        result->SetOnLongPressEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPress"))));
        result->SetOnLongPressRepeatEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPressRepeat"))));

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

    void KeyboardKey::SetSecondaryTextFont(Font const* font)
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
        if (secondaryText.empty()) {
            return;
        }

        currentTextValueIndex = (currentTextValueIndex + 1) % 2;
    }

    void KeyboardKey::DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        const char* primaryKeyValue = Text.c_str();
        const char* secondaryKeyValue = secondaryText.c_str();
        if (currentTextValueIndex == 1) {
            std::swap(primaryKeyValue, secondaryKeyValue);
        }

        uint32_t CurrentPrimaryTextColor = GetTextColor();
        if(primaryKeyValue && ButtonFont != 0) {
            uint16_t TextSize = ButtonFont->GetWidth(primaryKeyValue);
            uint16_t BeginX = (RenderWidth / 2) - (TextSize / 2);
            float TempButtonHeightMultiplier = secondaryKeyValue ? 0.66 : 0.5;
            uint16_t BeginY = RenderHeight * TempButtonHeightMultiplier - (ButtonFont->GetHeight() / 2) + TextTopOffset;

            painter.DisplayBoundedAntialiasedString(
                ButtonFont,
                RenderX + BeginX,
                RenderY + BeginY,
                RenderX,
                RenderY,
                RenderWidth,
                RenderHeight,
                primaryKeyValue,
                CurrentPrimaryTextColor);
        }

        uint32_t CurrentSecondaryTextColor = Painter::InterpolateColors(0xFF191A1C, 0xFFECEDEE, 0.4f);
        if(secondaryKeyValue && secondaryTextFont) {
            uint16_t TextSize = secondaryTextFont->GetWidth(secondaryKeyValue);
            uint16_t BeginX = (RenderWidth / 2) - (TextSize / 2);
            uint16_t BeginY = (RenderHeight / 4) - (secondaryTextFont->GetHeight() / 2) + TextTopOffset;

            painter.DisplayBoundedAntialiasedString(
                secondaryTextFont,
                RenderX + BeginX,
                RenderY + BeginY,
                RenderX,
                RenderY,
                RenderWidth,
                RenderHeight,
                secondaryKeyValue,
                CurrentSecondaryTextColor);
        }
    }

} /* namespace grvl */
