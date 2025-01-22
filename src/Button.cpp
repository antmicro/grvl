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

#include "Button.h"
#include "Component.h"
#include "grvl.h"
#include "Manager.h"
#include "XMLSupport.h"
#include "JSEngine.h"
#include "JSObject.h"
#include "JSObjectBuilder.h"

namespace grvl {

    Component* Button::Clone() const
    {
        return new Button(*this);
    }

    void Button::SetIcoFont(Font const* font)
    {
        IcoFont = font;
    }

    void Button::ClearIcoFont()
    {
        IcoFont = NULL;
    }

    Font const* Button::GetIcoFont()
    {
        return IcoFont;
    }

    void Button::SetIcoChar(int16_t textIco)
    {
        IcoChar = textIco;
    }

    void Button::ClearIcoChar()
    {
        IcoChar = -1;
    }

    Button* Button::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        Button* result = new Button();

        result->InitFromXML(xmlElement);
        static constexpr uint32_t noIco = 0xFFFFFFFF;
        result->SetIcoChar(XMLSupport::GetAttributeOrDefault(xmlElement, "icoChar", noIco));
        result->SetIcoFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "icoFont", "normal")));

        result->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", "#ffffffff"));
        result->SetActiveTextColor(XMLSupport::ParseColor(xmlElement, "activeTextColor", result->GetTextColor()));
        result->SetIcoColor(XMLSupport::ParseColor(xmlElement, "icoColor", result->GetTextColor()));
        result->SetActiveIcoColor(XMLSupport::ParseColor(xmlElement, "activeIcoColor", result->GetIcoColor()));

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

    void Button::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        DrawBackgroundItems(painter, RenderX, RenderY, Width, Height);
        DrawText(painter, RenderX, RenderY, Width, Height);

    }

    void Button::DrawBackgroundItems(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        uint32_t TempBackgroundColor = COLOR_ARGB8888_TRANSPARENT;
        uint32_t TempIcoColor = COLOR_ARGB8888_TRANSPARENT;

        if(State == On || State == Pressed || State == OnAndSelected || isFocused) {
            TempBackgroundColor = ActiveBackgroundColor;
            TempIcoColor = ActiveIcoColor;
        } else if(State == Off || State == Released || State == OffAndSelected) {
            TempBackgroundColor = BackgroundColor;
            TempIcoColor = ActiveIcoColor;
        } else if(State == Off || State == Released || State == OffAndSelected) {
            TempBackgroundColor = BackgroundColor;
            TempIcoColor = IcoColor;
        }

        if(TempBackgroundColor > 0 && TempBackgroundColor & 0xFF000000) {
            if (BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
                painter.FillRoundRectangle(RenderX, RenderY, RenderWidth, RenderHeight, TempBackgroundColor, BorderArcRadius);
            } else {
                painter.FillRectangle(RenderX, RenderY, RenderWidth, RenderHeight, TempBackgroundColor);
            }
        }

        DrawBorderIfNecessary(painter, RenderX, RenderY, RenderWidth, RenderHeight);

        if(!ButtonImage.IsEmpty()) {
            ButtonImage.Draw(painter, RenderX, RenderY);
        }
    }

    void Button::DrawText(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        uint32_t TempTextColor = COLOR_ARGB8888_TRANSPARENT;
        uint32_t TempIcoColor = COLOR_ARGB8888_TRANSPARENT;

        if(State == On || State == Pressed || State == OnAndSelected || isFocused) {
            TempTextColor = ActiveTextColor;
            TempIcoColor = ActiveIcoColor;
        } else if(State == Off || State == Released || State == OffAndSelected) {
            TempTextColor = TextColor;
            TempIcoColor = IcoColor;
        }

        uint32_t TextLen = Text.length();
        if(TextLen > 0 && ButtonFont != 0) {
            uint16_t TextSize = ButtonFont->GetWidth(Text.c_str());
            uint16_t BeginX = (Width / 2) - (TextSize / 2);
            uint16_t BeginY = (Height / 2) - (ButtonFont->GetHeight() / 2) + TextTopOffset;

            if(IcoChar != -1 && IcoFont != 0) {
                BeginY += Height / 3;
            }

            painter.DisplayBoundedAntialiasedString(ButtonFont, ParentRenderX + BeginX,
                                                    ParentRenderY + BeginY,
                                                    ParentRenderX, ParentRenderY,
                                                    Width,
                                                    Height,
                                                    Text.c_str(), TempTextColor);
        }

        if(IcoChar != -1 && IcoFont != 0) {
            static constexpr auto charPositionOffsetScale = 5;
            uint16_t BeginX = (Width / 2) - (IcoFont->GetCharWidth((uint32_t)IcoChar) / 2);
            uint16_t BeginY = (Height / 2) - (IcoFont->GetHeight() / 2);

            if(TextLen > 0 && ButtonFont != 0) {
                BeginY -= Height / charPositionOffsetScale;
            }

            painter.DisplayAntialiasedCharInBound(IcoFont, BeginX + ParentRenderX,
                                                  BeginY + ParentRenderY,
                                                  ParentRenderX, ParentRenderY,
                                                  Width,
                                                  Height,
                                                  IcoChar, TempIcoColor);
        }
    }

    void Button::SetTextColor(uint32_t color)
    {
        TextColor = color;
    }

    void Button::SetActiveTextColor(uint32_t color)
    {
        ActiveTextColor = color;
    }

    void Button::SetImagePosition(int32_t x, int32_t y)
    {
        if(!ButtonImage.IsEmpty()) {
            ButtonImage.SetPosition(x, y);
        }
    }

    void Button::SetTextTopOffset(int32_t value)
    {
        TextTopOffset = value;
    }

    uint32_t Button::GetTextColor()
    {
        return ForegroundColor;
    }

    uint32_t Button::GetActiveTextColor()
    {
        return ActiveForegroundColor;
    }

    int32_t Button::GetTextTopOffset() const
    {
        return TextTopOffset;
    }

    void Button::SetIcoColor(uint32_t color)
    {
        IcoColor = color;
    }

    void Button::SetActiveIcoColor(uint32_t color)
    {
        ActiveIcoColor = color;
    }

    uint32_t Button::GetIcoColor() const
    {
        return IcoColor;
    }

    uint32_t Button::GetActiveIcoColor() const
    {
        return ActiveIcoColor;
    }

    void Button::SetImageCentered(bool isCentered)
    {
        imageCentered = isCentered;
    }

    void Button::SetSize(int32_t width, int32_t height)
    {
        if(Width != width || Height != height) {
            Width = width;
            Height = height;
            if(imageCentered) {
                ButtonImage.SetPosition(width / 2 - ButtonImage.GetWidth() / 2, height / 2 - ButtonImage.GetHeight() / 2);
            }
        }
    }

    void Button::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        AbstractButton::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("icoColor", Button::JSGetIcoColorWrapper, Button::JSSetIcoColorWrapper);
        jsObjectBuilder.AddProperty("activeIcoColor", Button::JSGetActiveIcoColorWrapper, Button::JSSetActiveIcoColorWrapper);
    }

} /* namespace grvl */
