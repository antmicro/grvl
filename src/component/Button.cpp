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

#include <grvl/component/Button.h>
#include <grvl/component/Component.h>
#include <grvl/JSEngine.h>
#include <grvl/JSObject.h>
#include <grvl/JSObjectBuilder.h>
#include <grvl/Manager.h>
#include <grvl/XMLSupport.h>
#include <grvl/grvl.h>

namespace grvl {
    ButtonContentLayoutMode ParseButtonContentLayoutOrDefault(XMLElement* xmlElement, const char* attributeName, ButtonContentLayoutMode defaultValue)
    {
        const char* value = nullptr;
        if(!XMLSupport::TryGetAttribute(xmlElement, attributeName, &value)) {
            return defaultValue;
        }

        if(strcmp(value, "Inline") == 0) {
            return ButtonContentLayoutMode::Inline;
        }

        if(strcmp(value, "Overlay") == 0) {
            return ButtonContentLayoutMode::Overlay;
        }

        return defaultValue;
    }


    Component* Button::Clone() const
    {
        return new Button(*this);
    }

    void Button::SetIcoFont(Font* font)
    {
        IcoFont = font;
    }

    void Button::ClearIcoFont()
    {
        IcoFont = NULL;
    }

    Font* Button::GetIcoFont()
    {
        return IcoFont;
    }

    HorizontalAlignment Button::GetContentAlignment() const
    {
        return ContentAlignment;
    }

    ButtonContentLayoutMode Button::GetContentLayoutMode() const
    {
        return ContentLayoutMode;
    }

    int32_t Button::GetImageTextGap() const
    {
        return ImageTextGap;
    }

    int32_t Button::GetHorizontalPadding() const
    {
        return HorizontalPadding;
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

        result->SetContentAlignment(XMLSupport::ParseAlignmentOrDefault(xmlElement, "alignment", HorizontalAlignment::Center));
        result->SetContentLayoutMode(ParseButtonContentLayoutOrDefault(xmlElement, "contentLayout", ButtonContentLayoutMode::Overlay));
        if (result->ButtonFont) {
            result->SetImageTextGap(XMLSupport::GetAttributeOrDefault(xmlElement, "imageTextGap", result->ButtonFont->GetCharWidth(' ')));
        }
        result->SetHorizontalPadding(XMLSupport::GetAttributeOrDefault(xmlElement, "padding", 0));

        return result;
    }

    void Button::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        // Consider calling this only when layout changes
        CalculateContentLayout();

        painter.PushDrawingBoundsStackElement(RenderX, RenderY, RenderX + Width, RenderY + Height);

        DrawBackgroundItems(painter, RenderX, RenderY, Width, Height);
        DrawText(painter, RenderX, RenderY, Width, Height);

        painter.PopDrawingBoundsStackElement();
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

        if(layout.hasImage) {
            const int32_t oldImageX = ButtonImage.GetX();
            const int32_t oldImageY = ButtonImage.GetY();

            if (ContentLayoutMode == ButtonContentLayoutMode::Inline) {
                ButtonImage.SetPosition(layout.imageX, layout.imageY);
            }
            ButtonImage.Draw(painter, RenderX, RenderY);
            if (ContentLayoutMode == ButtonContentLayoutMode::Inline) {
                ButtonImage.SetPosition(oldImageX, oldImageY);
            }
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

        if(layout.hasText) {
            painter.DisplayBoundedAntialiasedString(ButtonFont, ParentRenderX + layout.textX,
                                                    ParentRenderY + layout.textY,
                                                    ParentRenderX, ParentRenderY,
                                                    Width,
                                                    Height,
                                                    Text.c_str(), TempTextColor);
        }

        if(layout.hasIco) {
            painter.DisplayAntialiasedCharInBound(IcoFont, ParentRenderX + layout.icoX,
                                                  ParentRenderY + layout.icoY,
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

    void Button::SetContentAlignment(HorizontalAlignment alignment)
    {
        ContentAlignment = alignment;
    }

    void Button::SetContentLayoutMode(ButtonContentLayoutMode mode)
    {
        ContentLayoutMode = mode;
    }

    void Button::SetImageTextGap(int32_t gap)
    {
        ImageTextGap = gap;
    }

    void Button::SetHorizontalPadding(int32_t padding)
    {
        HorizontalPadding = padding;
    }

    void Button::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        AbstractButton::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("icoColor", Button::JSGetIcoColorWrapper, Button::JSSetIcoColorWrapper);
        jsObjectBuilder.AddProperty("activeIcoColor", Button::JSGetActiveIcoColorWrapper, Button::JSSetActiveIcoColorWrapper);
    }

    void Button::CalculateContentLayout()
    {
        layout.hasImage = !ButtonImage.IsEmpty();
        layout.hasText = ButtonFont != nullptr && !Text.empty();
        layout.hasIco = (IcoChar != -1) && (IcoFont != nullptr);

        const int32_t imageWidth = layout.hasImage ? ButtonImage.GetWidth() : 0;
        const int32_t imageHeight = (layout.hasImage ? ButtonImage.GetHeight() : 0) + TextTopOffset;
        const int32_t textWidth = layout.hasText ? ButtonFont->GetWidth(Text.c_str()) : 0;
        const int32_t textHeight = (layout.hasText ? ButtonFont->GetFontHeight() : 0) + TextTopOffset;
        const int32_t icoWidth = layout.hasIco ? IcoFont->GetCharWidth((uint32_t)IcoChar) : 0;
        const int32_t icoHeight = layout.hasIco ? IcoFont->GetFontHeight() : 0;

        const auto alignedX = [this](int32_t contentWidth) {
            switch(ContentAlignment) {
                case HorizontalAlignment::Left:
                    return HorizontalPadding;
                case HorizontalAlignment::Right:
                    return Width - contentWidth - HorizontalPadding;
                case HorizontalAlignment::Center:
                default:
                    return (Width - contentWidth) / 2;
            }
        };

        if (layout.hasImage && layout.hasText && ContentLayoutMode == ButtonContentLayoutMode::Inline) {
            const int32_t contentWidth = imageWidth + ImageTextGap + textWidth;
            const int32_t contentX = alignedX(contentWidth);

            layout.imageX = contentX;
            layout.textX = contentX + imageWidth + ImageTextGap;
        } else {
            layout.imageX = alignedX(imageWidth);
            layout.textX = alignedX(textWidth);
        }

        if (layout.hasImage) {
            layout.imageY = (Height - imageHeight) / 2;
        }
        if (layout.hasText) {
            layout.textY = (Height + textHeight) / 2;
        }
        if (layout.hasIco) {
            const int32_t icoYOffset = Height / 3;
            layout.imageY += icoYOffset;
            layout.textY += icoYOffset;

            layout.icoX = alignedX(icoWidth);
            layout.icoY = (Height + IcoFont->GetFontHeight()) / 2;
            if (layout.hasText || (layout.hasImage && ContentLayoutMode == ButtonContentLayoutMode::Inline)) {
                static constexpr auto charPositionOffsetScale = 5;
                layout.icoY -= Height / charPositionOffsetScale;
            }
        }
    }

} /* namespace grvl */
