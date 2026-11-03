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

#include <grvl/Blitter.h>
#include <grvl/ContentManager.h>
#include <grvl/ImageContent.h>
#include <grvl/Misc.h>
#include <grvl/Painter.h>
#include <grvl/component/Image.h>
#include <grvl/container/AbstractView.h>

#include <cassert>
#include <cmath>
#include <string>

// NOLINTBEGIN

namespace grvl {

    Painter::~Painter() { }

    void Painter::DrawString(Font* font, int16_t x, int16_t y, const std::string& text, uint32_t color, uint32_t background) const
    {
        DisplayAntialiasedString(font, x, y, text.c_str(), color, background);
    }

    void Painter::DisplayAntialiasedString(Font* font, int16_t x, int16_t y, const char* text, uint32_t color, uint32_t background) const
    {
        InnerDisplayAntialiasedString(font, x, y, text, color, false, CurrentDrawingBoundsStartX(), CurrentDrawingBoundsStartY(), CurrentDrawingBoundsWidth(), CurrentDrawingBoundsHeight(), background);
    }

    void Painter::DisplayBoundedAntialiasedString(Font* font, int16_t x, int16_t y, int16_t ParentX, int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, const char* text, uint32_t color, uint32_t background) const
    {
        const int32_t textWidth = font->GetWidth(text);
        const int32_t textHeight = font->GetFontHeight();

        if(x >= CurrentDrawingBoundsEndX() || y >= CurrentDrawingBoundsEndY() || x + textWidth < CurrentDrawingBoundsStartX() || y + textHeight < CurrentDrawingBoundsStartY()) {
            return;
        }

        InnerDisplayAntialiasedString(font, x, y, text, color, true, ParentX, ParentY, ParentWidth, ParentHeight, background);
    }

    void Painter::InnerDisplayAntialiasedString(Font* font, int16_t x, int16_t y, const char* text, uint32_t color, bool bound, int16_t ParentX,
                                                int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t background) const
    {
        uint32_t prev_unicode = 0;

        while(*text) {

            Unicode unicode = ParseUnicodeCodepoint(text);
            text += unicode.length;

            Glyph glyph = font->GetGlyph(unicode.code);

            if(prev_unicode) {
                x += font->GetKerning(prev_unicode, unicode.code);
            }

            if(bound) {
                DisplayAntialiasedCharInBound(font, x, y, ParentX, ParentY, ParentWidth, ParentHeight, unicode.code, color, background);
            } else {
                DisplayAntialiasedChar(font, x, y, unicode.code, color, background);
            }

            x += glyph.advance;

            if(unicode.code != ' ') {
                prev_unicode = unicode.code;
            }
        }
    }

    // TODO remove?
    void Painter::DisplayAntialiasedChar(Font* font, uint16_t x, uint16_t y, uint32_t index, uint32_t color, uint32_t background) const
    {
        DrawAntialiasedChar(font, x, y, index, color, background);
    }

    void Painter::DrawAntialiasedChar(Font* font, int16_t x, int16_t y, uint32_t index, uint32_t color, uint32_t background) const
    {
        DrawAntialiasedCharInBound(font, x, y, CurrentDrawingBoundsStartX(), CurrentDrawingBoundsStartY(), CurrentDrawingBoundsWidth(), CurrentDrawingBoundsHeight(), index, color, background);
    }

    // TODO remove?
    void Painter::DisplayAntialiasedCharInBound(Font* font, int16_t x, int16_t y, int16_t ParentX, int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t index, uint32_t color, uint32_t background) const
    {
        DrawAntialiasedCharInBound(font, x, y, ParentX, ParentY, ParentWidth, ParentHeight, index, color, background);
    }

    // TODO fixup locals
    void Painter::DrawAntialiasedCharInBound(Font* Font, int16_t x, int16_t y,
                                             int16_t ParentX, int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight,
                                             uint32_t Index, uint32_t color, uint32_t background) const
    {
        if(ParentWidth <= 0 || ParentHeight <= 0) {
            return;
        }

        Glyph glyph = Font->GetGlyph(Index);
        if(glyph.bitmap == nullptr || glyph.width <= 0 || glyph.height <= 0) {
            return;
        }

        const int32_t glyphX = static_cast<int32_t>(x) + glyph.xoff;
        const int32_t glyphY = static_cast<int32_t>(y) + glyph.yoff;
        const int32_t glyphRight = glyphX + glyph.width;
        const int32_t glyphBottom = glyphY + glyph.height;

        const int32_t boundLeft = ParentX;
        const int32_t boundTop = ParentY;
        const int32_t boundRight = static_cast<int32_t>(ParentX) + ParentWidth;
        const int32_t boundBottom = static_cast<int32_t>(ParentY) + ParentHeight;

        const int32_t clipLeft = std::max(glyphX, boundLeft);
        const int32_t clipTop = std::max(glyphY, boundTop);
        const int32_t clipRight = std::min(glyphRight, boundRight);
        const int32_t clipBottom = std::min(glyphBottom, boundBottom);

        if(clipLeft >= clipRight || clipTop >= clipBottom) {
            return;
        }

        const int32_t srcX = clipLeft - glyphX;
        const int32_t srcY = clipTop - glyphY;
        const int32_t clippedWidth = clipRight - clipLeft;
        const int32_t clippedHeight = clipBottom - clipTop;

        if(background != 0) {
            FillRectangle(clipLeft - 1, clipTop - 1, clippedWidth + 2, clippedHeight + 2, background);
        }

        DrawGlyph(glyph, srcX, srcY, clipLeft, clipTop, clippedWidth, clippedHeight, color);
    }

    void Painter::DrawRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const
    {
        if(IsColorTransparent(color)) {
            return;
        }

        if(x >= CurrentDrawingBoundsEndX() || y >= CurrentDrawingBoundsEndY()) {
            return;
        }

        if(x < CurrentDrawingBoundsStartX()) {
            width = std::max(0, static_cast<int32_t>(width) - CurrentDrawingBoundsStartX() + x);
            x = CurrentDrawingBoundsStartX();
        }

        if(y < CurrentDrawingBoundsStartY()) {
            height = std::max(0, static_cast<int32_t>(height) - CurrentDrawingBoundsStartY() + y);
            y = CurrentDrawingBoundsStartY();
        }

        if(width <= 0 || height <= 0) {
            return;
        }
        DrawHLine(x, y, width - 1, color);
        DrawHLine(x, (y + height - 1), width - 1, color);
        DrawVLine(x, y, height, color);
        DrawVLine((x + width - 1), y, height, color);
    }

    void Painter::FillRoundRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, float radius) const
    {
        if(IsColorTransparent(color)) {
            return;
        }

        if(radius > width / 2.0f || radius > height / 2.0f) {
            return;
        }

        if(x >= CurrentDrawingBoundsEndX() || y >= CurrentDrawingBoundsEndY()) {
            return;
        }

        if(width <= 0 || height <= 0) {
            return;
        }

        FillQuarterCircle(x + width - radius, y + radius, radius, CircleQuarter::TOP_RIGHT, color);
        FillQuarterCircle(x + width - radius, y + height - radius, radius, CircleQuarter::BOTTOM_RIGHT, color);
        FillQuarterCircle(x + radius, y + height - radius, radius, CircleQuarter::BOTTOM_LEFT, color);
        FillQuarterCircle(x + radius, y + radius, radius, CircleQuarter::TOP_LEFT, color);

        FillRectangle(x + radius, y, width - radius * 2, height, color);
        FillRectangle(x, y + radius, width, height - radius * 2, color);
    }

    void Painter::DrawRoundRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, float radius) const
    {
        if(radius > width / 2.0f || radius > height / 2.0f) {
            return;
        }

        if(x >= CurrentDrawingBoundsEndX() || y >= CurrentDrawingBoundsEndY()) {
            return;
        }

        if(width <= 0 || height <= 0) {
            return;
        }

        float DoubleBorderArcRadius = radius * 2;

        DrawHLine(x + radius, y, width - DoubleBorderArcRadius, color);
        DrawHLine(x + radius, y + height - 1, width - DoubleBorderArcRadius, color);
        DrawVLine(x, y + radius, height - DoubleBorderArcRadius, color);
        DrawVLine((x + width - 1), y + radius, height - DoubleBorderArcRadius, color);

        DrawArc(x + width - DoubleBorderArcRadius - 1, y, radius, 90.0f, 180.0f, color);
        DrawArc(x + width - DoubleBorderArcRadius - 1, y + height - DoubleBorderArcRadius - 1, radius, 180.0f, 270.0f, color);
        DrawArc(x, y + height - DoubleBorderArcRadius - 1, radius, 270.0f, 360.0f, color);
        DrawArc(x, y, radius, 0.0f, 90.0f, color);
    }

    void Painter::SetDisplaySize(int32_t w, int32_t h)
    {
        bool changed = false;

        if(XSize != w || YSize != h) {
            XSize = w;
            YSize = h;
            changed = true;
        }

        ResetDrawingBounds();

        if(changed) {
            OnSizeChanged(w, h);
        }
    }

    void Painter::FillMemory(void* memory, int32_t width, int32_t height, uint32_t color, Format format)
    {
        if(width <= 0 || height <= 0) {
            return;
        }

        grvl::Callbacks()->fill((uintptr_t)memory, width, height, 0, color, format);
    }

    uint32_t Painter::InterpolateColors(uint32_t first, uint32_t second, float t)
    {
        uint32_t interpolatedAlpha = ((0xFF000000 & second) >> 24) * t + ((0xFF000000 & first) >> 24) * (1.0f - t);
        uint32_t interpolatedRed = ((0xFF0000 & second) >> 16) * t + ((0xFF0000 & first) >> 16) * (1.0f - t);
        uint32_t interpolatedGreen = ((0xFF00 & second) >> 8) * t + ((0xFF00 & first) >> 8) * (1.0f - t);
        uint32_t interpolatedBlue = (0xFF & second) * t + (0xFF & first) * (1.0f - t);

        uint32_t interpolatedColor { 0 };
        interpolatedColor |= interpolatedAlpha << 24;
        interpolatedColor |= interpolatedRed << 16;
        interpolatedColor |= interpolatedGreen << 8;
        interpolatedColor |= interpolatedBlue;

        return interpolatedColor;
    }

    int32_t Painter::GetDisplayWidth() const
    {
        return XSize;
    }

    int32_t Painter::GetDisplayHeight() const
    {
        return YSize;
    }

    void Painter::AddBackgroundBlock(int32_t y_position, int32_t height, uint32_t backgroundColor)
    {
        // this function is now redundant, it was only needed for background image handling
    }

    bool Painter::IsRotated() const
    {
        return is_rotated;
    }

    void Painter::SetRotation(bool rotate90)
    {
        is_rotated = rotate90;
    }

    void Painter::SetBackgroundColor(uint32_t color)
    {
        backgroundColor = color;
    }

    uint32_t Painter::GetBackgroundColor() const
    {
        return backgroundColor;
    }

    uint8_t Painter::GetSwapperValue() const
    {
        return 0;
    }

    uintptr_t Painter::GetBuffer(int id) const
    {
        return 0;
    }

    void Painter::SetActiveBuffer(uint32_t activeBuffer)
    {
        Log(ERROR, "Deprecated Painter::SetActiveBuffer() called!");
    }

    void Painter::SetContentManager(ContentManager* cm)
    {
        contentManager = cm;
    }

    ContentManager* Painter::GetContentManager()
    {
        return contentManager;
    }

    void Painter::PushDrawingBound(int32_t startX, int32_t startY, int32_t endX, int32_t endY)
    {
        if(!bounds.empty()) {
            startX = std::max(CurrentDrawingBoundsStartX(), startX);
            startY = std::max(CurrentDrawingBoundsStartY(), startY);
            endX = std::min(CurrentDrawingBoundsEndX(), endX);
            endY = std::min(CurrentDrawingBoundsEndY(), endY);
        }

        DrawingBound bound {};
        bound.startX = startX;
        bound.startY = startY;
        bound.endX = endX;
        bound.endY = endY;

        bounds.push(bound);
    }

    void Painter::PopDrawingBound()
    {
        if(!bounds.empty()) {
            const DrawingBound prev = bounds.top();
            bounds.pop();

            if(bounds.empty()) {
                ResetDrawingBounds();
            }
        }
    }

    void Painter::ResetDrawingBounds()
    {
        while(!bounds.empty()) {
            bounds.pop();
        }

        PushDrawingBound(0, 0, XSize, YSize);
    }

    void Painter::OnSizeChanged(int width, int height)
    {
        // Called when the display size changes
    }

} /* namespace grvl */

// NOLINTEND
