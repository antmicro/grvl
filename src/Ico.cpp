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

#include "Ico.h"

namespace grvl {

    Ico::Ico(int32_t charCode, Font* IcoFont) : IcoChar(charCode), IcoFont(IcoFont)
    {
        if(IcoFont) {
            AdjustSize();
        }
    }

    int32_t Ico::GetIcoChar() const
    {
        return IcoChar;
    }

    void Ico::SetIcoChar(int32_t icoChar)
    {
        IcoChar = icoChar;
        if(IcoFont) {
            AdjustSize();
        }
    }

    Font const* Ico::GetIcoFont() const
    {
        return IcoFont;
    }

    void Ico::SetIcoFont(Font const* icoFont)
    {
        IcoFont = icoFont;
        if(IcoChar) {
            AdjustSize();
        }
    }

    void Ico::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible) {
            return;
        }

        if(IcoChar != -1 && IcoFont != NULL) {
            if (BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
                painter.FillRoundRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor, BorderArcRadius);
            } else {
                painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor);
            }

            uint16_t BeginX = X + (Width / 2) - (IcoFont->GetCharWidth((uint32_t)IcoChar) / 2);
            uint16_t BeginY = Y + (Height / 2) - (IcoFont->GetFontHeight() / 2);

            painter.DisplayAntialiasedChar(IcoFont, BeginX + ParentRenderX, BeginY + ParentRenderY, IcoChar, ForegroundColor);

            DrawBorderIfNecessary(painter, ParentRenderX + X, ParentRenderY + Y, Width, Height);
        }
    }

    void Ico::AdjustSize()
    {
        int32_t tWidth = IcoFont->GetCharWidth(IcoChar);
        if(tWidth > Width) {
            Width = tWidth;
        }
        int32_t tHeight = IcoFont->GetHeight();
        if(tHeight > Height) {
            Height = tHeight;
        }
    }

} /* namespace grvl */
