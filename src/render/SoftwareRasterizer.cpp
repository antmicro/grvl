// Copyright 2026 Antmicro <antmicro.com>
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

#include <grvl/render/SoftwareRasterizer.h>

#include <grvl/Blitter.h>
#include <grvl/ImageContent.h>
#include <grvl/Misc.h>
#include <grvl/component/Image.h>
#include <grvl/container/AbstractView.h>

#include <cassert>
#include <cmath>
#include <string>

namespace grvl {

    void SoftwareRasterizer::BeginFrame()
    {
        // do nothing
    }

    void SoftwareRasterizer::FillCircle(int16_t Xpos, int16_t Ypos, int16_t Radius, uint32_t color) const
    {
        if(Radius == 0) {
            return;
        }

        float middleX = Xpos + Radius;
        float middleY = Ypos + Radius;

        float radiusWithOffset = Radius;
        const int numOfSteps = Radius * 2;
        float angleDiff = ToRadians(90.0f / numOfSteps);
        float currentAngle = ToRadians(90.0f);
        float currentX = 0;
        float currentY = 0;

        do {
            // TODO: maybe try with some lookup table with 90 angle values
            float currentXOffset = radiusWithOffset * std::cos(currentAngle);
            float currentYOffset = radiusWithOffset * std::sin(currentAngle);

            currentX = middleX - currentXOffset;
            currentY = middleY - currentYOffset;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentX = middleX + currentXOffset - 1;
            currentY = middleY - currentYOffset;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentX = middleX + currentXOffset - 1;
            currentY = middleY + currentYOffset - 1;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentX = middleX - currentXOffset;
            currentY = middleY + currentYOffset - 1;
            DrawAntialiasedPixel(currentX, currentY, color);

            DrawVLine(std::ceil(middleX - currentXOffset), std::ceil(middleY - currentYOffset), std::floor(currentYOffset * 2), color);
            DrawVLine(std::floor(std::max(0.0f, middleX + currentXOffset - 1)), std::ceil(middleY - currentYOffset), std::floor(currentYOffset * 2), color);

            currentAngle -= angleDiff;
        } while(currentAngle > 0);
    }

    void SoftwareRasterizer::DrawCircle(int16_t Xpos, int16_t Ypos, int16_t Radius, uint32_t color) const
    {
        if(Radius == 0) {
            return;
        }

        float middleX = Xpos + Radius;
        float middleY = Ypos + Radius;

        float radiusWithOffset = Radius;
        const int numOfSteps = Radius * 2;
        float angleDiff = ToRadians(90.0f / numOfSteps);
        float currentAngle = ToRadians(90.0f);
        float currentX = 0;
        float currentY = 0;

        do {
            // TODO: maybe try with some lookup table with 90 angle values
            float currentXOffset = radiusWithOffset * std::cos(currentAngle);
            float currentYOffset = radiusWithOffset * std::sin(currentAngle);

            currentX = middleX - currentXOffset;
            currentY = middleY - currentYOffset;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentX = middleX + currentXOffset - 1;
            currentY = middleY - currentYOffset;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentX = middleX + currentXOffset - 1;
            currentY = middleY + currentYOffset - 1;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentX = middleX - currentXOffset;
            currentY = middleY + currentYOffset - 1;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentAngle -= angleDiff;
        } while(currentAngle > 0);
    }

    // Source: http://joshbeam.com/articles/triangle_rasterization/
    void SoftwareRasterizer::DrawSpan(int x1, int x2, uint32_t color, int y) const
    {
        if(x1 > x2) {
            int xt = x1;
            x1 = x2;
            x2 = xt;
        }

        int xdiff = x2 - x1;
        if(xdiff == 0) {
            return;
        }

        DrawHLine(x1, y, xdiff, color);
    }

    void SoftwareRasterizer::DrawSpansBetweenEdges(const Edge& e1, const Edge& e2) const
    {
        // calculate difference between the y coordinates
        // of the first edge and return if 0
        float e1ydiff = (float)(e1.Y2 - e1.Y1);
        if(e1ydiff == 0.0f) {
            return;
        }

        // calculate difference between the y coordinates
        // of the second edge and return if 0
        float e2ydiff = (float)(e2.Y2 - e2.Y1);
        if(e2ydiff == 0.0f) {
            return;
        }

        float e1xdiff = (float)(e1.X2 - e1.X1);
        float e2xdiff = (float)(e2.X2 - e2.X1);

        // calculate factors to use for interpolation
        // with the edges and the step values to increase
        // them by after drawing each span
        float factor1 = (float)(e2.Y1 - e1.Y1) / e1ydiff;
        float factorStep1 = 1.0f / e1ydiff;
        float factor2 = 0.0f;
        float factorStep2 = 1.0f / e2ydiff;

        // loop through the lines between the edges and draw spans
        for(int y = e2.Y1; y < e2.Y2; y++) {
            // create and draw span
            DrawSpan(e1.X1 + (int)(e1xdiff * factor1 + 0.5f), e2.X1 + (int)(e2xdiff * factor2 + 0.5f), e1.Color, y);

            // increase factors
            factor1 += factorStep1;
            factor2 += factorStep2;
        }
    }

    void SoftwareRasterizer::FillTriangle(int16_t x1, int16_t x2, int16_t x3, int16_t y1, int16_t y2, int16_t y3,
                                          uint32_t color) const
    {

        Edge edges[3] = { Edge(x1, y1, x2, y2, color), Edge(x2, y2, x3, y3, color), Edge(x3, y3, x1, y1, color) };

        int maxLength = 0;
        int longEdge = 0;

        // find edge with the greatest length in the y axis
        for(int i = 0; i < 3; i++) {
            int length = edges[i].Y2 - edges[i].Y1;
            if(length > maxLength) {
                maxLength = length;
                longEdge = i;
            }
        }

        int shortEdge1 = (longEdge + 1) % 3;
        int shortEdge2 = (longEdge + 2) % 3;

        DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge1]);
        DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge2]);
    }

    // Source http://www.codeproject.com/Articles/13360/Antialiasing-Wu-Algorithm
    void SoftwareRasterizer::DrawAntialiasedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const
    {
        unsigned short IntensityShift, ErrorAdj, ErrorAcc;
        unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
        short DeltaX, DeltaY, Temp, XDir;

        /* Make sure the line runs top to bottom */
        if(y1 > y2) {
            Temp = y1;
            y1 = y2;
            y2 = Temp;
            Temp = x1;
            x1 = x2;
            x2 = Temp;
        }

        DrawPixel(x1, y1, color);

        if((DeltaX = x2 - x1) >= 0) {
            XDir = 1;
        } else {
            XDir = -1;
            DeltaX = -DeltaX; /* make DeltaX positive */
        }

        if((DeltaY = y2 - y1) == 0) {
            /* Horizontal line */
            while(DeltaX-- != 0) {
                x1 += XDir;
                DrawPixel(x1, y1, color);
            }
            return;
        }

        if(DeltaX == 0) {
            /* Vertical line */
            do {
                y1++;
                DrawPixel(x1, y1, color);
            } while(--DeltaY != 0);
            return;
        }
        if(DeltaX == DeltaY) {
            /* Diagonal line */
            do {
                x1 += XDir;
                y1++;
                DrawPixel(x1, y1, color);
            } while(--DeltaY != 0);
            return;
        }

        ErrorAcc = 0; /* initialize the line error accumulator to 0 */
        IntensityShift = 8;
        WeightingComplementMask = 255;

        /* Is this an X-major or Y-major line? */
        if(DeltaY > DeltaX) {
            ErrorAdj = ((unsigned long)DeltaX << 16) / (unsigned long)DeltaY;
            /* Draw all pixels other than the first and last */
            while(--DeltaY) {
                ErrorAccTemp = ErrorAcc; /* remember current accumulated error */
                ErrorAcc += ErrorAdj; /* calculate error for next pixel */
                if(ErrorAcc <= ErrorAccTemp) {
                    /* The error accumulator turned over, so advance the X coord */
                    x1 += XDir;
                }
                y1++; /* Y-major, so always advance Y */
                Weighting = ErrorAcc >> IntensityShift;
                BlendPixel(x1 + XDir, y1, (color & 0x00FFFFFF) | Weighting << 24);
                BlendPixel(x1, y1, (color & 0x00FFFFFF) | ((Weighting ^ WeightingComplementMask) << 24));
            }
        } else {
            ErrorAdj = ((unsigned long)DeltaY << 16) / (unsigned long)DeltaX;
            /* Draw all pixels other than the first and last */
            while(--DeltaX) {
                ErrorAccTemp = ErrorAcc; /* remember current accumulated error */
                ErrorAcc += ErrorAdj; /* calculate error for next pixel */
                if(ErrorAcc <= ErrorAccTemp) {
                    /* The error accumulator turned over, so advance the Y coord */
                    y1++;
                }
                x1 += XDir; /* X-major, so always advance X */
                Weighting = ErrorAcc >> IntensityShift;
                BlendPixel(x1, y1 + 1, (color & 0x00FFFFFF) | Weighting << 24);
                BlendPixel(x1, y1, (color & 0x00FFFFFF) | ((Weighting ^ WeightingComplementMask) << 24));
            }
        }

        DrawPixel(x2, y2, color);
    }

    void SoftwareRasterizer::DrawImage(int16_t x, int16_t y, const ImageContent* image, uint32_t frame) const
    {
        if(image == nullptr || image->IsEmpty()) {
            return;
        }

        uintptr_t address = reinterpret_cast<uintptr_t>(image->GetFrameData(frame));

        DmaMoveImage(
            address, GetBackBuffer(), 0, 0, x, y, image->GetWidth(), image->GetHeight(),
            image->GetPixelsPerLine(), image->GetNumberOfLines(), 0, 1,
            image->GetColorFormat(), GetPixelFormat(), image->HasAlphaChannel(), image->GetColorPalette());
    }

    void SoftwareRasterizer::DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const
    {
        int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0, den = 0, num = 0,
                numadd = 0, numpixels = 0, curpixel = 0;

        deltax = abs(x2 - x1); /* The difference between the x's */
        deltay = abs(y2 - y1); /* The difference between the y's */
        x = x1; /* Start x off at the first pixel */
        y = y1; /* Start y off at the first pixel */

        if(x2 >= x1) { /* The x-values are increasing */
            xinc1 = 1;
            xinc2 = 1;
        } else { /* The x-values are decreasing */
            xinc1 = -1;
            xinc2 = -1;
        }

        if(y2 >= y1) { /* The y-values are increasing */
            yinc1 = 1;
            yinc2 = 1;
        } else { /* The y-values are decreasing */
            yinc1 = -1;
            yinc2 = -1;
        }

        if(deltax >= deltay) { /* There is at least one x-value for every y-value */
            xinc1 = 0; /* Don't change the x when numerator >= denominator */
            yinc2 = 0; /* Don't change the y for every iteration */
            den = deltax;
            num = deltax / 2;
            numadd = deltay;
            numpixels = deltax; /* There are more x-values than y-values */
        } else { /* There is at least one y-value for every x-value */
            xinc2 = 0; /* Don't change the x for every iteration */
            yinc1 = 0; /* Don't change the y when numerator >= denominator */
            den = deltay;
            num = deltay / 2;
            numadd = deltax;
            numpixels = deltay; /* There are more y-values than x-values */
        }

        for(curpixel = 0; curpixel <= numpixels; curpixel++) {
            DrawPixel(x, y, color); /* Draw the current pixel */
            num += numadd; /* Increase the numerator by the top of the fraction */
            if(num >= den) { /* Check if numerator >= denominator */
                num -= den; /* Calculate the new numerator value */
                x += xinc1; /* Change the x as appropriate */
                y += yinc1; /* Change the y as appropriate */
            }
            x += xinc2; /* Change the x as appropriate */
            y += yinc2; /* Change the y as appropriate */
        }
    }

    void SoftwareRasterizer::DrawGradientVLine(int16_t x1, int16_t y1, int16_t y2, uint32_t startingColor, uint32_t endingColor, float startingValue) const
    {
        if(y1 >= YSize || x1 < 0 || x1 >= XSize) {
            return;
        }

        auto finalEndingY = std::min(y2, static_cast<int16_t>(YSize));

        int16_t length = finalEndingY - y1;
        if(length <= 0) {
            return;
        }

        uintptr_t ax = 0;
        uintptr_t ptr = GetBackBuffer();
        uint32_t bytes = GetBytesPerPixel();
        Format pixelFormat = GetPixelFormat();

        float currentGradientValue = startingValue;
        float gradientDelta = (1.0f - currentGradientValue) / length;
        for(int i = 0; i < length; ++i) {
            uint32_t interpolatedColor = InterpolateColors(startingColor, endingColor, currentGradientValue);
            DrawPixel(x1, y1 + i, interpolatedColor);
            currentGradientValue = std::min(1.0f, currentGradientValue + gradientDelta);
        }
    }

    void SoftwareRasterizer::FillArc(int32_t x, int32_t y, int32_t radius, float startAngleDeg, float endAngleDeg, uint32_t color) const
    {
        Log(ERROR, "FillArc is not implemented for SoftwareRasterizer");
    }

    void SoftwareRasterizer::DrawEllipse(int32_t Xpos, int32_t Ypos, int32_t XRadius, int32_t YRadius, uint32_t color) const
    {
        if(IsColorTransparent(color)) {
            return;
        }

        int x = 0, y = -YRadius, err = 2 - 2 * XRadius, e2;
        float K = 0, rad1 = 0, rad2 = 0;

        rad1 = XRadius;
        rad2 = YRadius;

        K = (float)(rad2 / rad1);

        do {
            DrawPixel((Xpos - (uint16_t)(x / K)), (Ypos + y), color);
            DrawPixel((Xpos + (uint16_t)(x / K)), (Ypos + y), color);
            DrawPixel((Xpos + (uint16_t)(x / K)), (Ypos - y), color);
            DrawPixel((Xpos - (uint16_t)(x / K)), (Ypos - y), color);

            e2 = err;
            if(e2 <= x) {
                err += ++x * 2 + 1;
                if(-y == x && e2 <= y) {
                    e2 = 0;
                }
            }
            if(e2 > y) {
                err += ++y * 2 + 1;
            }
        } while(y <= 0);
    }

    void SoftwareRasterizer::FillEllipse(int32_t Xpos, int32_t Ypos, int32_t XRadius, int32_t YRadius, uint32_t color) const
    {
        if(IsColorTransparent(color)) {
            return;
        }

        int x = 0, y = -YRadius, err = 2 - 2 * XRadius, e2;
        float K = 0, rad1 = 0, rad2 = 0;

        rad1 = XRadius;
        rad2 = YRadius;

        K = (float)(rad2 / rad1);

        do {
            DrawHLine((Xpos - (uint16_t)(x / K)), (Ypos + y), (2 * (uint16_t)(x / K) + 1), color);
            DrawHLine((Xpos - (uint16_t)(x / K)), (Ypos - y), (2 * (uint16_t)(x / K) + 1), color);

            e2 = err;
            if(e2 <= x) {
                err += ++x * 2 + 1;
                if(-y == x && e2 <= y) {
                    e2 = 0;
                }
            }
            if(e2 > y) {
                err += ++y * 2 + 1;
            }
        } while(y <= 0);
    }

    void SoftwareRasterizer::ShadowBuffer(uint8_t alpha)
    {
        static ImageContent* shadow = nullptr;

        // Why is this done like this and what is it?
        // ShadowBuffer() is a method that applies a half-transparent gray tint to the screen,
        // this is used as a background effect for when a popup is on the screen. You may wonder why DmaFill is not used
        // here - that is because DmaFill doesn't support transparency, it's just a glorified memcpy(). So this is
        // implemented by allocating a (XSize x 1) image (once) and then on each invocation of ShadowBuffer() memseting it
        // with our alpha value and then DmaBlit'ing that image multiple times until the whole screen is covered.
        // In the past there was an alternative implemention that instead performed a DmaBlit from a invalid address (nullptr)
        // into the target while tinting the image, obv this will never work in a memory protected environment.
        // As not all applications use popups we only allocate the shadow image once we need it.

        if(shadow == nullptr) {
            const int size = XSize * 4;
            uint8_t* buffer = (uint8_t*)malloc(size);

            shadow = new ImageContent(buffer, XSize, 1, 1, Format::ARGB8888);

            if(IsRotated() && !shadow->IsRotated()) {
                shadow->Rotate90();
            }

            Log(INFO, "Allocated %d bytes for the shadow buffer", size);
        }

        uint32_t color = static_cast<uint32_t>(alpha) << 24;
        memset(shadow->GetData(), alpha, shadow->GetWidth() * 4);

        for(uint32_t i = 0; i < YSize; i++) {
            DmaMoveShadow((uintptr_t)shadow->GetData(), GetBackBuffer(), 0, i, shadow->GetWidth(), 1, GetPixelFormat(), color);
        }
    }

    void SoftwareRasterizer::DrawPixel(uint32_t x, uint32_t y, uint32_t color) const
    {
        if(x >= CurrentDrawingBoundsEndX() || x < CurrentDrawingBoundsStartX()) {
            return;
        }

        if(y >= CurrentDrawingBoundsEndY() || y < CurrentDrawingBoundsStartY()) {
            return;
        }

        uint64_t offset = (y * GetXSize() + x) * GetBytesPerPixel();
        uint8_t* ptr = ((uint8_t*)GetBackBuffer()) + offset;

        ConvertPixel((const uint8_t*)&color, ptr, Format::ARGB8888, GetPixelFormat());
    }

    void SoftwareRasterizer::DrawAntialiasedPixel(float Xpos, float Ypos, uint32_t RGB_Code) const
    {
        if(Xpos >= CurrentDrawingBoundsEndX() || Xpos < CurrentDrawingBoundsStartX() || Ypos >= CurrentDrawingBoundsEndY() || Ypos < CurrentDrawingBoundsStartY()) {
            return;
        }

        float EndX = Clamp(Xpos + 1.0f, 0.0f, static_cast<float>(GetXSize()));
        float EndY = Clamp(Ypos + 1.0f, 0.0f, static_cast<float>(GetYSize()));

        uint32_t startingXPos = static_cast<uint32_t>(std::floor(Xpos));
        uint32_t startingYPos = static_cast<uint32_t>(std::floor(Ypos));
        uint32_t endingXPos = static_cast<uint32_t>(std::floor(EndX));
        uint32_t endingYPos = static_cast<uint32_t>(std::floor(EndY));

        uint32_t pixelRGB = ReadPixel(startingXPos, startingYPos);
        float XPart = 1.0f - (Xpos - static_cast<float>(startingXPos));
        float YPart = 1.0f - (Ypos - static_cast<float>(startingYPos));
        DrawPixel(startingXPos, startingYPos, InterpolateColors(pixelRGB, RGB_Code, XPart * YPart));

        pixelRGB = ReadPixel(startingXPos, endingYPos);
        XPart = 1.0f - (Xpos - static_cast<float>(startingXPos));
        YPart = 1.0f - (endingYPos - static_cast<float>(Ypos));
        DrawPixel(startingXPos, endingYPos, InterpolateColors(pixelRGB, RGB_Code, XPart * YPart));

        pixelRGB = ReadPixel(endingXPos, startingYPos);
        XPart = 1.0f - (endingXPos - static_cast<float>(Xpos));
        YPart = 1.0f - (Ypos - static_cast<float>(startingYPos));
        DrawPixel(endingXPos, startingYPos, InterpolateColors(pixelRGB, RGB_Code, XPart * YPart));

        pixelRGB = ReadPixel(endingXPos, endingYPos);
        XPart = 1.0f - (endingXPos - static_cast<float>(Xpos));
        YPart = 1.0f - (endingYPos - static_cast<float>(Ypos));
        DrawPixel(endingXPos, endingYPos, InterpolateColors(pixelRGB, RGB_Code, XPart * YPart));
    }

    void SoftwareRasterizer::BlendPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const
    {
        uint32_t oldColor = ReadPixel(Xpos, Ypos);
        uint32_t newColor = Blend(oldColor, RGB_Code);
        DrawPixel(Xpos, Ypos, newColor);
    }

    uint32_t SoftwareRasterizer::ReadPixel(uint32_t x, uint32_t y) const
    {
        if(x > GetXSize() || y > GetYSize()) {
            return COLOR_ARGB8888_BLACK;
        }

        uint64_t offset = (y * GetXSize() + x) * GetBytesPerPixel();
        uint8_t* ptr = ((uint8_t*)GetBackBuffer()) + offset;

        return ConvertPixel(ptr, GetPixelFormat(), Format::ARGB8888);
    }

    void SoftwareRasterizer::DrawArc(int16_t x, int16_t y, int16_t radius, float startAngle, float endAngle, uint32_t color) const
    {
        if(IsColorTransparent(color)) {
            return;
        }

        if(radius == 0) {
            return;
        }

        float middleX = x + radius;
        float middleY = y + radius;

        float angleDiffInRadians = ToRadians((endAngle - startAngle) / (radius * 4));
        float currentAngleInRadians = ToRadians(startAngle);
        float endAngleInRadians = ToRadians(endAngle);
        float currentX = 0;
        float currentY = 0;

        do {
            // TODO: maybe try with some lookup table with 90 angle values
            float currentXOffset = radius * std::cos(currentAngleInRadians);
            float currentYOffset = radius * std::sin(currentAngleInRadians);

            currentX = middleX - currentXOffset;
            currentY = middleY - currentYOffset;
            DrawAntialiasedPixel(currentX, currentY, color);

            currentAngleInRadians += angleDiffInRadians;
        } while(currentAngleInRadians < endAngleInRadians);
    }

    void SoftwareRasterizer::FillQuarterCircle(int32_t posXi, int32_t posYi, int32_t radius,
                                               CircleQuarter circleQuarter, uint32_t color) const
    {
        if(radius == 0) {
            return;
        }

        // Consider that the center of the circle is at the top-left corner of a pixel.
        const float posX = posXi - 0.5f;
        const float posY = posYi - 0.5f;

        int16_t startY = 0;
        switch(circleQuarter) {
            case CircleQuarter::TOP_LEFT:
            case CircleQuarter::TOP_RIGHT:
                startY = posYi - radius;
                break;
            case CircleQuarter::BOTTOM_LEFT:
            case CircleQuarter::BOTTOM_RIGHT:
                startY = posYi;
                break;
            default:
                return;
        }

        // The idea is to have two radiuses: inner and outer (radius-0.5 and radius+0.5).
        // The inner circle we fill without antialiasing, using DrawHLine and the area between
        // an the inner and outer circle is filled pixel-by-pixel, blending with the pixel that was
        // already there. Blending factor is derived from the distance from the inner circle.

        for(int32_t curY = startY; curY <= startY + radius; ++curY) {
            if((float)radius * radius - (curY - posY) * (curY - posY) < 0.0f) {
                continue;
            }
            const float tmpInner = std::sqrt((radius - 0.5f) * (radius - 0.5f) - (curY - posY) * (curY - posY));
            const float tmpOuter = std::sqrt((radius + 0.5f) * (radius + 0.5f) - (curY - posY) * (curY - posY));

            float innerLeftX;
            float innerRightX;
            float outerLeftX;
            float outerRightX;
            if(circleQuarter == CircleQuarter::TOP_LEFT || circleQuarter == CircleQuarter::BOTTOM_LEFT) {
                innerLeftX = -tmpInner + posX;
                innerRightX = posX;

                outerLeftX = -tmpOuter + posX;
                outerRightX = innerLeftX;
            } else {
                innerLeftX = posX;
                innerRightX = tmpInner + posX;

                outerLeftX = innerRightX;
                outerRightX = tmpOuter + posX;
            }

            const int32_t outerLeftXi = outerLeftX;
            const int32_t outerRightXi = std::ceil(outerRightX);
            for(int32_t curX = outerLeftXi; curX <= outerRightXi; ++curX) {
                const float dist = std::sqrt((curX - posX) * (curX - posX) + (curY - posY) * (curY - posY))
                    - radius;

                if(dist >= -0.5f && dist <= 0.5f) {
                    DrawPixel(curX, curY, InterpolateColors(color, ReadPixel(curX, curY), dist + 0.5f));
                }
            }

            DrawHLine(
                std::ceil(innerLeftX), curY,
                std::floor(innerRightX) - std::ceil(innerLeftX) + 1.0f,
                color);
        }
    }

    void SoftwareRasterizer::DrawArc(int32_t x, int32_t y, int32_t radius, int32_t startAngle, int32_t endAngle, int32_t width, uint32_t startColor, uint32_t endColor) const
    {
        if(startAngle == endAngle) {
            return;
        }

        const int steps = 80; // TODO: adjust, or auto-adjustment
        float radians = 0;
        float t = startAngle;
        Vec2f points[steps * 2 + 3];
        float angleDiff = AngleDiff(startAngle, endAngle);

        // Compute points
        for(int i = 0; i < steps * 2 + 2; i += 2) {
            radians = (t - 90) * 3.1415f / 180.0f;

            // Store all points - for future purposes
            points[i].x = (radius - width) * cos(radians) + x;
            points[i].y = (radius - width) * sin(radians) + y;

            points[i + 1].x = radius * cos(radians) + x;
            points[i + 1].y = radius * sin(radians) + y;

            t += (float)(ConstrainAngle(angleDiff)) / steps;
        }

        float alphaDiff = (float)(((int32_t)(endColor & 0xFF000000) >> 24) - ((int32_t)(startColor & 0xFF000000) >> 24)) / (steps * 2);
        float redDiff = (float)(((int32_t)(endColor & 0x00FF0000) >> 16) - ((int32_t)(startColor & 0x00FF0000) >> 16)) / (steps * 2);
        float greenDiff = (float)(((int32_t)(endColor & 0x0000FF00) >> 8) - ((int32_t)(startColor & 0x0000FF00) >> 8)) / (steps * 2);
        float blueDiff = (float)(((int32_t)(endColor & 0x000000FF) >> 0) - ((int32_t)(startColor & 0x000000FF) >> 0)) / (steps * 2);

        float alpha = (((uint32_t)startColor & 0xFF000000) >> 24);
        float red = (((uint32_t)startColor & 0x00FF0000) >> 16);
        float green = (((uint32_t)startColor & 0x0000FF00) >> 8);
        float blue = (((uint32_t)startColor & 0x000000FF) >> 0);

        for(int i = 2; i < steps * 2 + 2; i++) {
            uint32_t currentColor = (uint32_t)(alpha += alphaDiff) << 24 | (uint32_t)(red += redDiff) << 16
                | (uint32_t)(green += greenDiff) << 8 | (uint32_t)(blue += blueDiff);
            FillTriangle(
                points[i - 2].x + 0.5f, points[i - 1].x + 0.5f, points[i].x + 0.5f, points[i - 2].y + 0.5f,
                points[i - 1].y + 0.5f, points[i].y + 0.5f, currentColor);
        }
    }

    void SoftwareRasterizer::DrawHLine(int32_t Xpos, int32_t Ypos, int32_t Length, uint32_t text_color) const
    {
        if(IsColorTransparent(text_color)) {
            return;
        }

        if(Ypos >= CurrentDrawingBoundsEndY() || Ypos < CurrentDrawingBoundsStartY() || Xpos >= CurrentDrawingBoundsEndX()) {
            return;
        }

        if(Xpos + Length >= CurrentDrawingBoundsEndX()) {
            Length = CurrentDrawingBoundsEndX() - Xpos - 1;
        }

        if(Xpos < CurrentDrawingBoundsStartX()) {
            Length = Length - (CurrentDrawingBoundsStartX() - Xpos);
            Xpos = CurrentDrawingBoundsStartX();
        }

        if(Length <= 0) {
            return;
        }

        uintptr_t ax = 0;
        uintptr_t ptr = GetBackBuffer();
        uint32_t bytes = GetBytesPerPixel();
        Format pixel_format = GetPixelFormat();
        if(!IsRotated()) {
            ax = ptr + bytes * (GetXSize() * Ypos + Xpos);
            DmaFill(ax, Length, 1, 0, text_color, pixel_format);
        } else {
            ax = ptr + bytes * (((GetXSize() - Xpos - Length) * GetYSize()) + Ypos);
            DmaFill(ax, 1, Length, (GetYSize() - 1), text_color, pixel_format);
        }
    }

    void SoftwareRasterizer::DrawVLine(int32_t Xpos, int32_t Ypos, int32_t Length, uint32_t text_color) const
    {
        if(IsColorTransparent(text_color)) {
            return;
        }

        if(Xpos < CurrentDrawingBoundsStartX() || Xpos >= CurrentDrawingBoundsEndX() || Ypos >= CurrentDrawingBoundsEndY()) {
            return;
        }

        if(Ypos + Length >= CurrentDrawingBoundsEndY()) {
            Length = CurrentDrawingBoundsEndY() - Ypos - 1;
        }

        if(Ypos < CurrentDrawingBoundsStartY()) {
            Length -= CurrentDrawingBoundsStartY() - Ypos;
            Ypos = CurrentDrawingBoundsStartY();
        }

        if(Length <= 0) {
            return;
        }

        uintptr_t ax = 0;
        uintptr_t ptr = GetBackBuffer();
        uint32_t bytes = GetBytesPerPixel();
        Format pixel_format = GetPixelFormat();
        if(!IsRotated()) {
            /* Get the line address */
            ax = ptr + bytes * (GetXSize() * Ypos + Xpos);

            /* Write line */
            DmaFill(ax, 1, Length, (GetXSize() - 1), text_color, pixel_format);
        } else {
            /* Get the line address */
            ax = ptr + bytes * (((GetXSize() - Xpos - 1) * GetYSize()) + Ypos);

            /* Write line */
            DmaFill(ax, Length, 1, 0, text_color, pixel_format);
        }
    }

    void SoftwareRasterizer::FillRectangle(int32_t Xpos, int32_t Ypos, int32_t Width, int32_t Height, uint32_t text_color) const
    {
        if(IsColorTransparent(text_color)) {
            return;
        }

        if(Xpos >= CurrentDrawingBoundsEndX() || Ypos >= CurrentDrawingBoundsEndY()) {
            return;
        }

        if(Xpos < CurrentDrawingBoundsStartX()) {
            Width = Width - (CurrentDrawingBoundsStartX() - Xpos);
            Xpos = CurrentDrawingBoundsStartX();
        }

        if(Ypos < CurrentDrawingBoundsStartY()) {
            Height = Height - (CurrentDrawingBoundsStartY() - Ypos);
            Ypos = CurrentDrawingBoundsStartY();
        }

        if(Xpos + Width >= CurrentDrawingBoundsEndX()) {
            Width = CurrentDrawingBoundsEndX() - Xpos;
        }

        if(Ypos + Height >= CurrentDrawingBoundsEndY()) {
            Height = CurrentDrawingBoundsEndY() - Ypos;
        }

        if(Width <= 0 || Height <= 0) {
            return;
        }

        uintptr_t ax = 0;
        uint32_t bytes = GetBytesPerPixel();
        uintptr_t ptr = GetBackBuffer();
        Format pixel_format = GetPixelFormat();
        if(!IsRotated()) {
            /* Get the rectangle start address */
            ax = ptr + bytes * (GetXSize() * Ypos + Xpos);
            /* Fill the rectangle */
            DmaFill(ax, Width, Height, (GetXSize() - Width), text_color, pixel_format);
        } else {
            /* Get the rectangle start address */
            ax = ptr + bytes * ((GetXSize() - Xpos - Width) * (GetYSize()) + Ypos);
            /* Fill the rectangle */
            DmaFill(ax, Height, Width, (GetYSize() - Height), text_color, pixel_format);
        }
    }

    void SoftwareRasterizer::DmaOperation(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                                          uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                                          Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor) const
    {
        if(PixelsPerLine == 0 || NumberOfLines == 0) {
            return;
        }

        grvl::Callbacks()->blit(
            inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
            inPixelFormat, backgroundPixelFormat, outPixelFormat, frontColor);
    }

    void SoftwareRasterizer::DmaOperationCLT(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                                             uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                                             Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uintptr_t backCLT, uintptr_t frontCLT) const
    {
        if(PixelsPerLine == 0 || NumberOfLines == 0) {
            return;
        }

        bool usesClt = GetFormatUsesColorLookup(inPixelFormat) || GetFormatUsesColorLookup(backgroundPixelFormat);

        if(usesClt) {

            // We use a null `backCLT`/`frontCLT` to indicate greyscale, but the user callback may not follow the same convention,
            // so we explicitly specify a greyscale palette. The internal fallback operation supports null-as-greyscale
            if(backCLT == 0) {
                backCLT = (uintptr_t)greyscaleCltPalette;
            }

            if(frontCLT == 0) {
                frontCLT = (uintptr_t)greyscaleCltPalette;
            }

            grvl::Callbacks()->blit_clt(
                inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
                inPixelFormat, backgroundPixelFormat, outPixelFormat, 0, backCLT, frontCLT);

            return;
        }

        grvl::Callbacks()->blit(
            inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
            inPixelFormat, backgroundPixelFormat, outPixelFormat, 0);
    }

    void SoftwareRasterizer::DmaFill(uintptr_t outputMem, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t outOffset,
                                     uint32_t color_index, Format pixel_format) const
    {
        return grvl::Callbacks()->fill(outputMem, PixelsPerLine, NumberOfLines, outOffset, color_index, pixel_format);
    }

    Format SoftwareRasterizer::GetPixelFormat() const
    {
        return pixel_format;
    }

    uint32_t SoftwareRasterizer::GetBytesPerPixel() const
    {
        return GetFormatStride(GetPixelFormat());
    }

    void SoftwareRasterizer::DmaMove(uintptr_t fb_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                                     int32_t width, int32_t height, Format src_pixel_format, Format dst_pixel_format) const
    {
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

        if(IsRotated()) {
            inputMem = fb_src + GetFormatStride(src_pixel_format) * (y_lcd_size * (x_lcd_size - x_src - width) + y_src);
            outputMem = fb_dst + GetFormatStride(dst_pixel_format) * (y_lcd_size * (x_lcd_size - x_dst - width) + y_dst);
            NumberOfLines = width;
            PixelsPerLine = height;
            inOffset = y_lcd_size - height;
            outOffset = y_lcd_size - height;
        } else {
            inputMem = fb_src + GetFormatStride(src_pixel_format) * ((x_lcd_size * (y_src)) + x_src);
            outputMem = fb_dst + GetFormatStride(dst_pixel_format) * ((x_lcd_size * (y_dst)) + x_dst);
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = x_lcd_size - width;
            outOffset = x_lcd_size - width;
        }
        DmaOperation(
            inputMem, 0, outputMem, PixelsPerLine, NumberOfLines, inOffset, 0, outOffset, src_pixel_format, Format::ARGB8888,
            dst_pixel_format);
    }

    void SoftwareRasterizer::DmaMoveImage(uintptr_t img_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst,
                                          int32_t y_dst, int32_t width, int32_t height, int32_t totalImageWidth, int32_t totalImageHeight,
                                          uint32_t activeFrame, uint32_t frames, Format inPixelFormat, Format outPixelFormat, bool hasAlpha, uintptr_t imageCLT) const
    {

        int inBytes = GetFormatStride(inPixelFormat);
        int outBytes = GetFormatStride(outPixelFormat);
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

        /* safety checks so that image is in drawing bounds */
        if(y_dst < CurrentDrawingBoundsStartY()) {
            auto offset = std::abs(y_dst - CurrentDrawingBoundsStartY());
            y_dst = CurrentDrawingBoundsStartY();
            y_src += offset;
            height -= offset;
        }

        if(y_dst + height > CurrentDrawingBoundsEndY()) {
            height -= y_dst + height - CurrentDrawingBoundsEndY();
        }

        if(x_dst < CurrentDrawingBoundsStartX()) {
            auto offset = std::abs(x_dst - CurrentDrawingBoundsStartX());
            x_dst = CurrentDrawingBoundsStartX();
            x_src += offset;
            width -= offset;
        }

        if(x_dst + width > CurrentDrawingBoundsEndX()) {
            width -= x_dst + width - CurrentDrawingBoundsEndX();
        }

        if(width <= 0 || height <= 0) {
            return;
        }

        if(IsRotated()) {
            uint32_t frameWidth = totalImageWidth / frames;
            inputMem = img_src + inBytes * ((totalImageWidth * (totalImageHeight - width - x_src) + y_src + (activeFrame * frameWidth)));
            outputMem = fb_dst + outBytes * (y_lcd_size * (x_lcd_size - x_dst - width) + y_dst);
            NumberOfLines = width - x_src;
            PixelsPerLine = height;
            inOffset = totalImageWidth - height;
            outOffset = y_lcd_size - height;
        } else {
            const size_t frameSize = static_cast<size_t>(totalImageWidth) * static_cast<size_t>(totalImageHeight);
            const size_t frameOffset = static_cast<size_t>(activeFrame) * frameSize;
            const size_t sourceOffset = frameOffset + static_cast<size_t>(totalImageWidth) * y_src + x_src;
            inputMem = img_src + inBytes * sourceOffset;
            outputMem = fb_dst + outBytes * ((x_lcd_size * (y_dst)) + x_dst);
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = totalImageWidth - width;
            outOffset = x_lcd_size - width;
        }

        if(hasAlpha) {
            DmaOperationCLT(
                inputMem, outputMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, outOffset, outOffset,
                inPixelFormat, outPixelFormat, outPixelFormat, 0, imageCLT);
        } else {
            DmaOperationCLT(
                inputMem, 0, outputMem, PixelsPerLine, NumberOfLines, inOffset, 0, outOffset,
                inPixelFormat, Format::ARGB8888, outPixelFormat, 0, imageCLT);
        }
    }

    void SoftwareRasterizer::DmaMoveShadow(uintptr_t img_src, uintptr_t fb_dst, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height,
                                           Format outPixelFormat, uint32_t color) const
    {

        int outBytes = GetFormatStride(outPixelFormat);
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

        /* safety checks so that image is in drawing bounds */
        if(y_dst < CurrentDrawingBoundsStartY()) {
            auto offset = std::abs(y_dst - CurrentDrawingBoundsStartY());
            y_dst = CurrentDrawingBoundsStartY();
            height -= offset;
        }

        if(y_dst + height >= CurrentDrawingBoundsEndY()) {
            height -= y_dst + height - CurrentDrawingBoundsEndY();
        }

        if(x_dst < CurrentDrawingBoundsStartX()) {
            auto offset = std::abs(x_dst - CurrentDrawingBoundsStartX());
            x_dst = CurrentDrawingBoundsStartX();
            width -= offset;
        }

        if(x_dst + width >= CurrentDrawingBoundsEndX()) {
            width -= x_dst + width - CurrentDrawingBoundsEndX();
        }

        if(width <= 0 || height <= 0) {
            return;
        }

        if(IsRotated()) {
            inputMem = img_src;
            outputMem = fb_dst + outBytes * (y_lcd_size * (x_lcd_size - x_dst - width) + y_dst);
            NumberOfLines = width;
            PixelsPerLine = height;
            inOffset = 0;
            outOffset = y_lcd_size - height;
        } else {
            inputMem = img_src;
            outputMem = fb_dst + outBytes * ((x_lcd_size * (y_dst)) + x_dst);
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = 0;
            outOffset = 0;
        }

        DmaOperation(
            inputMem, outputMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, outOffset, outOffset,
            Format::ARGB8888, outPixelFormat, outPixelFormat, color);
    }

    void SoftwareRasterizer::DrawGlyph(Glyph& glyph, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height, uint32_t text_color) const
    {
        Format outPixelFormat = GetPixelFormat();
        int inBytes = GetFormatStride(Format::A8);
        int outBytes = GetFormatStride(outPixelFormat);
        uint32_t x_lcd_size = CurrentDrawingBoundsEndX();
        uint32_t y_lcd_size = CurrentDrawingBoundsEndY();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

        /* safety checks so that font is in buffer bounds */
        if(y_dst < CurrentDrawingBoundsStartY()) {
            auto offset = std::abs(y_dst - CurrentDrawingBoundsStartY());
            y_dst = CurrentDrawingBoundsStartY();
            y_src += offset;
            height -= offset;
        }

        if(y_dst + height >= y_lcd_size) {
            height -= y_dst + height - y_lcd_size;
        }

        if(x_dst < CurrentDrawingBoundsStartX()) {
            auto offset = std::abs(x_dst - CurrentDrawingBoundsStartX());
            x_dst = CurrentDrawingBoundsStartX();
            x_src += offset;
            width -= offset;
        }

        if(x_dst + width >= x_lcd_size) {
            width -= x_dst + width - x_lcd_size;
        }

        if(width <= 0 || height <= 0) {
            return;
        }

        if(IsRotated()) {
            return;
        } else {
            inputMem = reinterpret_cast<uintptr_t>(glyph.bitmap) + (uint32_t)(inBytes * ((glyph.width * y_src) + x_src));
            outputMem = GetBackBuffer() + (uint32_t)(outBytes * ((GetXSize() * (y_dst)) + x_dst));
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = glyph.width - width;
            outOffset = GetXSize() - width;
        }

        DmaOperation(inputMem, outputMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, outOffset, outOffset,
                     Format::A8, outPixelFormat, outPixelFormat, text_color);
    }

    void SoftwareRasterizer::CreateFramebuffersCollection(Format fmt, void* framebuffer_1, void* framebuffer_2)
    {
        this->pixel_format = fmt;
        const size_t bytes = GetFormatStride(fmt) * XSize * YSize;

        if(framebuffer_1 == nullptr) {
            framebuffer_1 = malloc(bytes);
            Log(INFO, "Allocated framebuffer #1 of size %u bytes", bytes);
        }

        if(framebuffer_2 == nullptr) {
            framebuffer_2 = malloc(bytes);
            Log(INFO, "Allocated framebuffer #2 of size %u bytes", bytes);
        }

        framebuffers[0] = (uintptr_t)framebuffer_1;
        framebuffers[1] = (uintptr_t)framebuffer_2;
    }

    void SoftwareRasterizer::InitFramebuffersCollection()
    {
        const Format format = GetPixelFormat();

        FillMemory((void*)GetBackBuffer(), XSize, YSize, COLOR_ARGB8888_BLACK, format);
        FillMemory((void*)GetFrontBuffer(), XSize, YSize, COLOR_ARGB8888_BLACK, format);
    }

    uintptr_t SoftwareRasterizer::GetFrontBuffer() const
    {
        return (uintptr_t)framebuffers[swapper ? 1 : 0];
    }

    uintptr_t SoftwareRasterizer::GetBackBuffer() const
    {
        return (uintptr_t)framebuffers[swapper ? 0 : 1];
    }

    void SoftwareRasterizer::SwapBuffers()
    {
        if(grvl::Callbacks()->wait_for_vsync) {
            grvl::Callbacks()->wait_for_vsync();
        }

        swapper = !swapper;
        grvl::Callbacks()->set_layer_pointer(GetFrontBuffer());

        if(grvl::Callbacks()->flipping_completed) {
            grvl::Callbacks()->flipping_completed();
        }
    }

    std::string SoftwareRasterizer::GetDebugString() const
    {
        return "Software Renderer";
    }

}
