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

#include "Painter.h"
#include "AbstractView.h"
#include "ContentManager.h"
#include "Image.h"
#include "ImageContent.h"
#include "Misc.h"
#include "stl.h"

// NOLINTBEGIN

#define max(x, y) (x < y ? y : x)

namespace grvl {

    static uint32_t ConvertColor(uint32_t color, uint32_t inputPixelFormat, uint32_t outputPixelFormat)
    {
        uint8_t color1 = 0;
        uint8_t color2 = 0;
        uint8_t color3 = 0;
        uint8_t alpha = 0;
        if(inputPixelFormat == outputPixelFormat) {
            return color;
        }

        switch(inputPixelFormat) {
            case COLOR_FORMAT_ARGB8888:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB4444: // ARGB8888 to ARGB4444
                        color1 = (color & 0x00ff0000) >> 20;
                        color2 = (color & 0x0000ff00) >> 12;
                        color3 = (color & 0x000000ff) >> 4;
                        alpha = (color & 0xff000000) >> 28;
                        return ((alpha << 12) | (color1 << 8) | (color2 << 4) | (color3));

                    case COLOR_FORMAT_ARGB6666: // ARGB8888 to ARGB6666
                        color1 = (color & 0x00ff0000) >> 18;
                        color2 = (color & 0x0000ff00) >> 10;
                        color3 = (color & 0x000000ff) >> 2;
                        alpha = (color & 0xff000000) >> 26;
                        return ((alpha << 18) | (color1 << 12) | (color2 << 6) | (color3));

                    case COLOR_FORMAT_RGB565: // ARGB8888 to RGB565
                        color1 = (color & 0x00ff0000) >> 19;
                        color2 = (color & 0x0000ff00) >> 10;
                        color3 = (color & 0x000000ff) >> 3;
                        return ((color1 << 11) | (color2 << 5) | (color3));

                    case COLOR_FORMAT_RGB888: // ARGB8888 to RGB888
                        return (color & 0x00FFFFFF);
                }
                break;

            case COLOR_FORMAT_AL88:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB4444: // AL88 to ARGB4444
                        color1 = (color & 0xff) >> 4;
                        color2 = color1;
                        color3 = color1;
                        alpha = (color & 0xff00) >> 12;
                        return ((alpha << 12) | (color1 << 8) | (color2 << 4) | (color3));

                    case COLOR_FORMAT_RGB565: // AL88 to RGB565
                        color1 = (color & 0xff) >> 3;
                        color2 = (color & 0xff) >> 2;
                        color3 = (color & 0xff) >> 3;
                        return ((color1 << 8) | (color2 << 4) | (color3));

                    case COLOR_FORMAT_RGB888: // AL88 to RGB888
                        color1 = color & 0xff;
                        color2 = color1;
                        color3 = color1;
                        return ((color1 << 16) | (color2 << 8) | (color3));

                    case COLOR_FORMAT_ARGB8888: // AL88 to ARGB8888
                        color1 = color & 0xff;
                        alpha = (color & 0xff00) >> 8;
                        return ((alpha << 24) | (color1 << 16) | (color1 << 8) | (color1));
                }
                break;

            case COLOR_FORMAT_L8:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB4444: // L8 to ARGB4444
                        color1 = (color & 0xff) >> 4;
                        alpha = 0xf;
                        return ((alpha << 12) | (color1 << 8) | (color1 << 4) | (color1));

                    case COLOR_FORMAT_RGB565: // L8 to RGB565
                        color1 = (color & 0xff) >> 3;
                        color2 = (color & 0xff) >> 2;
                        return ((color1 << 8) | (color2 << 4) | (color1));

                    case COLOR_FORMAT_RGB888: // L8 to RGB888
                        color1 = color & 0xff;
                        return ((color1 << 16) | (color1 << 8) | (color1));

                    case COLOR_FORMAT_ARGB8888: // L8 to ARGB8888
                        color1 = color & 0xff;
                        alpha = 0xff;
                        return ((alpha << 24) | (color1 << 16) | (color1 << 8) | (color1));
                }
                break;

            case COLOR_FORMAT_RGB888:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB4444: // RGB888 to ARGB4444
                        color1 = (color & 0x00ff0000) >> 20;
                        color2 = (color & 0x0000ff00) >> 12;
                        color3 = (color & 0x000000ff) >> 4;
                        return (0xF000 | (color1 << 8) | (color2 << 4) | (color3));

                    case COLOR_FORMAT_RGB565: // RGB888 to RGB565
                        color1 = (color & 0x00ff0000) >> 19;
                        color2 = (color & 0x0000ff00) >> 10;
                        color3 = (color & 0x000000ff) >> 3;
                        return ((color1 << 11) | (color2 << 5) | (color3));

                    case COLOR_FORMAT_ARGB8888: // RGB888 to ARGB8888
                        return ((color & 0x00FFFFFF) | 0xFF000000);
                }
                break;

            case COLOR_FORMAT_ARGB4444:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB8888: // ARGB4444 to ARGB8888
                        color1 = (color & 0x0f00) >> 8;
                        color2 = (color & 0x00f0) >> 4;
                        color3 = (color & 0x000f) >> 0;
                        alpha = (color & 0xf000) >> 12;
                        return (uint32_t)((alpha << 28 | alpha << 24) | (color1 << 20 | color1 << 16) | (color2 << 12 | color2 << 8)
                                          | (color3 << 4 | color3));
                }
                break;

            case COLOR_FORMAT_ARGB6666:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB8888: // ARGB6666 to ARGB8888
                        color1 = (color >> 12) & 0x3f;
                        color2 = (color >> 6)  & 0x3f;
                        color3 = (color >> 0)  & 0x3f;
                        alpha =  (color >> 18) & 0x3f;
                        return (uint32_t)((alpha << 26 | alpha << 24) | (color1 << 18 | color1 << 16) | (color2 << 10 | color2 << 8) /* temporary */
                                          | (color3 << 2 | color3));
                }
                break;

            case COLOR_FORMAT_RGB565:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB8888: // RGB565 to RGB8888
                        color1 = (color & 0x1f << 11) >> 11;
                        color2 = (color & 0x3f << 5) >> 5;
                        color3 = (color & 0x1f);
                        color1 = (color1 * 527 + 23) >> 6;
                        color2 = (color2 * 259 + 33) >> 6;
                        color3 = (color3 * 527 + 23) >> 6;
                        return (uint32_t)((color1 << 16) | (color2 << 8) | (color3) | 0xff000000);
                }
                break;
            case COLOR_FORMAT_AXXX8888:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB8888: // AXXX8888 to ARGB8888
                        return color & 0xFF000000;
                }
            case COLOR_FORMAT_A8:
                switch(outputPixelFormat) {
                    case COLOR_FORMAT_ARGB8888: // A8 to ARGB8888
                        return (color & 0xFF) << 24;
                }
        }
        return 0;
    }

    static uint32_t ConvertPixel(unsigned char* data, uint32_t inputPixelFormat, uint32_t outputPixelFormat)
    {
        uint32_t color;

        // Fixme should not read 4 bytes if pixel format is different than ARGB8888
        memcpy(&color, data, 4);
        return ConvertColor(color, inputPixelFormat, outputPixelFormat);
    }

    // Based on the ST documentation
    static uint32_t Blend(uint32_t bcol, uint32_t icol)
    {
        uint8_t alphai = (icol >> 24) & 0xFF;
        uint8_t alphab = (bcol >> 24) & 0xFF;

        uint8_t alphamulti = ((alphai * alphab) / 255);
        uint8_t alphar = alphai + alphab - alphamulti;
        if(alphar == 0) {
            return 0;
        }

        uint32_t result;
        uint8_t* resultb = (uint8_t*)&result;
        uint8_t* icolb = (uint8_t*)&icol;
        uint8_t* bcolb = (uint8_t*)&bcol;

        for(int i = 0; i < 3; i++) {
            resultb[i] = ((icolb[i] * alphai) + (bcolb[i] * alphab) - (bcolb[i] * alphamulti)) / (alphar);
        }

        resultb[3] = alphar;

        return result;
    }

    static void PixelFormatConvert(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
                                   uint32_t inputPixelFormat, uint32_t backgroundPixelFormat, uint32_t outputPixelFormat, uint32_t fontColor, uintptr_t backCLT, uintptr_t frontCLT)
    {
        uint32_t inputColor, backgroundColor, outputColor;

        if((inputPixelFormat == COLOR_FORMAT_A8) || (inputPixelFormat == COLOR_FORMAT_AXXX8888)) {
            inputColor = ConvertPixel((unsigned char*)inputMem, inputPixelFormat, COLOR_FORMAT_ARGB8888);
            inputColor = (inputColor & 0xff000000) | (fontColor & 0x00ffffff);
        } else if(inputPixelFormat == COLOR_FORMAT_L8) {
            uint8_t* inclr = (uint8_t*)frontCLT;
            uint8_t imageData = *(uint8_t*)inputMem * 3;
            inputColor = 0xFF000000 | inclr[imageData + 2] << 16 | inclr[imageData + 1] << 8 | inclr[imageData + 0];
        } else {
            inputColor = ConvertPixel((unsigned char*)inputMem, inputPixelFormat, COLOR_FORMAT_ARGB8888);
        }

        if(backgroundMem != 0) {
            if(backgroundPixelFormat == COLOR_FORMAT_L8) {
                uint8_t* bckclr = (uint8_t*)backCLT;
                uint8_t backgroundData = *(uint8_t*)backgroundMem * 3;
                backgroundColor = 0xFF000000 | ((bckclr[backgroundData + 2] << 16) | (bckclr[backgroundData + 1] << 8) | (bckclr[backgroundData + 0]));
            } else {
                backgroundColor = ConvertPixel((unsigned char*)backgroundMem, backgroundPixelFormat, COLOR_FORMAT_ARGB8888);
            }
            outputColor = Blend(backgroundColor, inputColor);
        } else {
            outputColor = inputColor;
        }

        outputColor = ConvertPixel((unsigned char*)&outputColor, COLOR_FORMAT_ARGB8888, outputPixelFormat);
        memcpy((void*)outputMem, (void*)&outputColor, PixelFormatToBPP(outputPixelFormat));
    }

    void FallbackDmaOperation(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
                              uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset,
                              uint32_t outOffset, uint32_t inColor, uint32_t backgroundColor, uint32_t outColor, uint32_t fontColor, uintptr_t backCLT, uintptr_t frontCTL)
    {
        uintptr_t omem = outputMem;
        uintptr_t imem = inputMem;
        uintptr_t bmem = backgroundMem;
        unsigned int y, x;
        uint32_t omemBPP = PixelFormatToBPP(outColor);
        uint32_t imemBPP = PixelFormatToBPP(inColor);
        uint32_t bmemBPP = backgroundMem == 0 ? 0 : PixelFormatToBPP(backgroundColor);

        for(y = 0; y < NumberOfLines; y++) {
            for(x = 0; x < PixelsPerLine; x++) {
                PixelFormatConvert(imem, bmem, omem, inColor, backgroundColor, outColor, fontColor, backCLT, frontCTL);
                omem += omemBPP;
                imem += imemBPP;
                bmem += bmemBPP;
            }
            omem += outOffset * omemBPP;
            imem += inOffset * imemBPP;
            bmem += backgroundOffset * bmemBPP;
        }
    }

    void FallbackDmaFill(uintptr_t dst, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t offset,
                         uint32_t color_index, uint32_t pixel_format)
    {
        unsigned int x, y;
        uint32_t color = ConvertColor(color_index, COLOR_FORMAT_ARGB8888, pixel_format);
        uint32_t dstBPP = PixelFormatToBPP(pixel_format);
        for(y = 0; y < NumberOfLines; y++) {
            for(x = 0; x < PixelsPerLine; x++) {
                memcpy((void*)dst, (void*)&color, dstBPP);
                dst += dstBPP;
            }
            dst += offset * dstBPP;
        }
    }

    static bool HasTransparency(uint32_t color)
    {
        return !((color & 0xff000000) == 0xff000000);
    }

    Painter::~Painter()
    {
    }

    void Painter::SetDisplaySize(int32_t x, int32_t y)
    {
        XSize = x;
        YSize = y;
    }

    // Note: this was originally commented out
    void Painter::ShadowBuffer(uint8_t number, uint32_t color)
    {
        memset((void*)shadowImage->GetData(), color >> 24, shadowImage->GetWidth() * 4);

        for(uint32_t i = 0; i < YSize; i++) {
            DmaMoveShadow((uintptr_t)shadowImage->GetData(), GetVisibleBuffer(), 0, i, shadowImage->GetWidth(),
                          1, GetDisplayPixelFormat(), color);
        }
    }

    // Note: Code below causes segfault, is copying from addr 0x0 intended?

    // void Painter::ShadowBuffer(uint8_t number, uint32_t color) {
    //     Painter::DmaOperation(0, GetVisibleBuffer(), GetVisibleBuffer(), XSize, YSize, 0, 0, 0, COLOR_FORMAT_ARGB8888, GetActiveBufferPixelFormat(), GetActiveBufferPixelFormat(), color);
    // }

    void Painter::DrawPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const
    {
        uintptr_t ptr = GetActiveBuffer();
        if(Xpos > GetXSize()) {
            return;
        }
        if(Ypos > GetYSize()) {
            return;
        }
        if(GetActiveBufferBytesPerPixel() == 4) {
            uint32_t* pixels = (uint32_t*)ptr;
            pixels[Ypos * GetXSize() + Xpos] = RGB_Code;
        } else {
            uint16_t* pixels = (uint16_t*)ptr;
            uint16_t col = ConvertColor(RGB_Code, COLOR_FORMAT_ARGB8888, GetActiveBufferPixelFormat()) & 0xFFFF;
            pixels[Ypos * GetXSize() + Xpos] = col;
        }
    }

    void Painter::BlendPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const
    {
        uint32_t oldColor = ReadPixel(Xpos, Ypos);
        uint32_t newColor = Blend(oldColor, RGB_Code);
        DrawPixel(Xpos, Ypos, newColor);
    }

    uint32_t Painter::ReadPixel(uint32_t Xpos, uint32_t Ypos) const
    {
        uintptr_t ptr = GetActiveBuffer();
        if(GetActiveBufferBytesPerPixel() == 4) {
            uint32_t* pixels = (uint32_t*)ptr;
            return pixels[Ypos * GetXSize() + Xpos];
        } else {
            uint16_t* pixels = (uint16_t*)ptr;
            return ConvertColor(pixels[Ypos * GetXSize() + Xpos], GetActiveBufferPixelFormat(), COLOR_FORMAT_ARGB8888);
        }
    }

    void Painter::FillCircle(int16_t Xpos, int16_t Ypos, int16_t Radius, uint32_t color) const
    {
        if(Radius == 0) {
            return;
        }
        int32_t D; /* Decision Variable */
        uint32_t CurX; /* Current X Value */
        uint32_t CurY; /* Current Y Value */

        D = 3 - (Radius << 1);

        CurX = 0;
        CurY = Radius;
        while(CurX <= CurY) {
            if(CurY > 0) {
                DrawHLine(Xpos - CurY, Ypos + CurX, 2 * CurY, color);
                DrawHLine(Xpos - CurY, Ypos - CurX, 2 * CurY, color);
            }

            if(CurX > 0) {
                DrawHLine(Xpos - CurX, Ypos - CurY, 2 * CurX, color);
                DrawHLine(Xpos - CurX, Ypos + CurY, 2 * CurX, color);
            }

            if(D < 0) {
                D += (CurX << 2) + 6;
            } else {
                D += ((CurX - CurY) << 2) + 10;
                CurY--;
            }
            CurX++;
        }
        DrawCircle(Xpos, Ypos, Radius, color);
    }

    void Painter::DrawCircle(int16_t Xpos, int16_t Ypos, int16_t Radius, uint32_t color) const
    {
        if(Radius == 0) {
            return;
        }
        int32_t D; /* Decision Variable */
        uint32_t CurX; /* Current X Value */
        uint32_t CurY; /* Current Y Value */

        D = 3 - (Radius << 1);
        CurX = 0;
        CurY = Radius;
        while(CurX <= CurY) {
            DrawPixel((Xpos + CurX), (Ypos - CurY), color);
            DrawPixel((Xpos - CurX), (Ypos - CurY), color);
            DrawPixel((Xpos + CurY), (Ypos - CurX), color);
            DrawPixel((Xpos - CurY), (Ypos - CurX), color);
            DrawPixel((Xpos + CurX), (Ypos + CurY), color);
            DrawPixel((Xpos - CurX), (Ypos + CurY), color);
            DrawPixel((Xpos + CurY), (Ypos + CurX), color);
            DrawPixel((Xpos - CurY), (Ypos + CurX), color);

            if(D < 0) {
                D += (CurX << 2) + 6;
            } else {
                D += ((CurX - CurY) << 2) + 10;
                CurY--;
            }
            CurX++;
        }
    }

    // Source: http://joshbeam.com/articles/triangle_rasterization/
    void Painter::DrawSpan(int x1, int x2, uint32_t color, int y) const
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

    void Painter::DrawSpansBetweenEdges(const Edge& e1, const Edge& e2) const
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
            DrawSpan(e1.X1 + (int)(e1xdiff * factor1 + 0.5), e2.X1 + (int)(e2xdiff * factor2 + 0.5), e1.Color, y);

            // increase factors
            factor1 += factorStep1;
            factor2 += factorStep2;
        }
    }

    void Painter::FillTriangle(int16_t x1, int16_t x2, int16_t x3, int16_t y1, int16_t y2, int16_t y3,
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
    void Painter::DrawAntialiasedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const
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

    void Painter::DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const
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

    void Painter::DrawEllipse(int32_t Xpos, int32_t Ypos, int32_t XRadius, int32_t YRadius, uint32_t color) const
    {
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

    void Painter::FillEllipse(int32_t Xpos, int32_t Ypos, int32_t XRadius, int32_t YRadius, uint32_t color) const
    {
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

    struct fPoint {
        float x;
        float y;
    };
    struct iPoint {
        int32_t x;
        int32_t y;
    };

    void Painter::FillArc(int32_t Xpos, int32_t Ypos, int32_t startAngle, int32_t endAngle, int32_t radius, int32_t width,
                          uint32_t startColor, uint32_t endColor) const
    {
        if(startAngle == endAngle) {
            return;
        }

        const int steps = 80; // TODO: adjust, or auto-adjustment
        float radians = 0;
        float t = startAngle;
        fPoint points[steps * 2 + 3];
        float angleDiff = AngleDiff(startAngle, endAngle);

        // Compute points
        for(int i = 0; i < steps * 2 + 2; i += 2) {
            radians = (t - 90) * 3.1415 / 180.0;

            // Store all points - for future purposes
            points[i].x = (radius - width) * cos(radians) + Xpos;
            points[i].y = (radius - width) * sin(radians) + Ypos;

            points[i + 1].x = radius * cos(radians) + Xpos;
            points[i + 1].y = radius * sin(radians) + Ypos;

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
                points[i - 2].x + 0.5, points[i - 1].x + 0.5, points[i].x + 0.5, points[i - 2].y + 0.5,
                points[i - 1].y + 0.5, points[i].y + 0.5, currentColor);
        }
    }

    void Painter::DrawHLine(int32_t Xpos, int32_t Ypos, uint16_t Length, uint32_t text_color) const
    {
        if(Length == 0) {
            return;
        }
        uintptr_t ax = 0;
        uintptr_t ptr = GetActiveBuffer();
        uint32_t bytes = GetActiveBufferBytesPerPixel();
        uint32_t pixel_format = GetActiveBufferPixelFormat();
        // sanity checks
        if(Ypos > (int32_t)GetYSize()) {
            return;
        }
        if(Ypos < 0) {
            return;
        }
        if(Xpos < 0) {
            Length = Length + Xpos;
            Xpos = 0;
        }
        if(((uint32_t)Xpos + Length) > GetXSize()) {
            Length = GetXSize() - Xpos;
        }

        if(!IsRotated()) {
            ax = ptr + bytes * (GetXSize() * Ypos + Xpos);
            DmaFill(ax, Length, 1, 0, text_color, pixel_format);
        } else {
            ax = ptr + bytes * (((GetXSize() - Xpos - Length) * GetYSize()) + Ypos);
            DmaFill(ax, 1, Length, (GetYSize() - 1), text_color, pixel_format);
        }
    }

    void Painter::DrawVLine(int32_t Xpos, int32_t Ypos, uint16_t Length, uint32_t text_color) const
    {
        uintptr_t ax = 0;
        uintptr_t ptr = GetActiveBuffer();
        uint32_t bytes = GetActiveBufferBytesPerPixel();
        uint32_t pixel_format = GetActiveBufferPixelFormat();
        if(Xpos < 0) {
            return;
        }
        if(Xpos > (int32_t)GetXSize()) {
            return;
        }
        if(Ypos < 0) {
            Length = Length + Ypos;
            Ypos = 0;
        }
        if(((uint32_t)Ypos + Length) > GetYSize()) {
            Length = GetYSize() - Ypos;
        }
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

    void Painter::FillRectangle(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t text_color) const
    {
        if(Width == 0 || Height == 0) {
            return;
        }
        uintptr_t ax = 0;
        uint32_t bytes = GetActiveBufferBytesPerPixel();
        uintptr_t ptr = GetActiveBuffer();
        uint32_t pixel_format = GetActiveBufferPixelFormat();
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

    void Painter::FillMemory(uintptr_t memory, uint32_t width, uint32_t height, uint32_t text_color, uint32_t colorFormat)
    {
        if(width == 0 || height == 0) {
            return;
        }
        DmaFill(memory, width, height, 0, text_color, colorFormat);
    }

    void Painter::DrawRectangle(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t text_color) const
    {
        if(Width == 0 || Height == 0) {
            return;
        }
        DrawHLine(Xpos, Ypos, Width - 1, text_color);
        DrawHLine(Xpos, (Ypos + Height - 1), Width - 1, text_color);
        DrawVLine(Xpos, Ypos, Height, text_color);
        DrawVLine((Xpos + Width - 1), Ypos, Height, text_color);
    }

    void Painter::DmaOperation(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                               uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                               uint32_t inPixelFormat, uint32_t backgroundPixelFormat, uint32_t outPixelFormat, uint32_t frontColor) const
    {
        if(PixelsPerLine == 0 || NumberOfLines == 0) {
            return;
        }
        if(!grvl::Callbacks()->dma_operation) {
            FallbackDmaOperation(
                inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
                inPixelFormat, backgroundPixelFormat, outPixelFormat, frontColor, 0, 0);
            return;
        } else {
            grvl::Callbacks()->dma_operation(
                inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
                inPixelFormat, backgroundPixelFormat, outPixelFormat, frontColor);
        }
    }

    void Painter::DmaOperationCLT(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                                  uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                                  uint32_t inPixelFormat, uint32_t backgroundPixelFormat, uint32_t outPixelFormat, uintptr_t backCLT, uintptr_t frontCLT) const
    {
        if(PixelsPerLine == 0 || NumberOfLines == 0) {
            return;
        }

        if(grvl::Callbacks()->dma_operation_clt) {
            grvl::Callbacks()->dma_operation_clt(
                inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
                inPixelFormat, backgroundPixelFormat, outPixelFormat, 0, backCLT, frontCLT);
        } else if(grvl::Callbacks()->dma_operation) {
            grvl::Callbacks()->dma_operation(
                inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
                inPixelFormat, backgroundPixelFormat, outPixelFormat, 0);
        } else { // TODO Add support for CLT to FallBackDMA
            FallbackDmaOperation(
                inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset,
                inPixelFormat, backgroundPixelFormat, outPixelFormat, 0, backCLT, frontCLT);
        }
    }

    void Painter::DmaFill(uintptr_t outputMem, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t outOffset,
                          uint32_t color_index, uint32_t pixel_format) const
    {
        if(!grvl::Callbacks()->dma_fill) {
            return FallbackDmaFill(outputMem, PixelsPerLine, NumberOfLines, outOffset, color_index, pixel_format);
        } else {
            return grvl::Callbacks()->dma_fill(
                outputMem, PixelsPerLine, NumberOfLines, outOffset, color_index, pixel_format);
        }
    }

    uint32_t Painter::GetActiveBufferPixelFormat() const
    {
        return backLayerPointers[ActiveBuffer].pixel_format;
    }

    uint32_t Painter::GetActiveBufferBytesPerPixel() const
    {
        return PixelFormatToBPP(backLayerPointers[ActiveBuffer].pixel_format);
    }

    uint32_t Painter::GetPixelFormat() const
    {
        return backLayerPointers[0].pixel_format;
    }

    uint32_t Painter::GetBytesPerPixel() const
    {
        return PixelFormatToBPP(backLayerPointers[0].pixel_format);
    }

    uint32_t Painter::GetDisplayPixelFormat() const
    {
        return backLayerPointers[2].pixel_format;
    }

    uint32_t Painter::GetDisplayBytesPerPixel() const
    {
        return PixelFormatToBPP(backLayerPointers[2].pixel_format);
    }
    void Painter::DmaMove(uintptr_t fb_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                          int32_t width, int32_t height, uint32_t src_pixel_format, uint32_t dst_pixel_format) const
    {
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

        if(IsRotated()) {
            inputMem = fb_src + PixelFormatToBPP(src_pixel_format) * (y_lcd_size * (x_lcd_size - x_src - width) + y_src);
            outputMem = fb_dst + PixelFormatToBPP(dst_pixel_format) * (y_lcd_size * (x_lcd_size - x_dst - width) + y_dst);
            NumberOfLines = width;
            PixelsPerLine = height;
            inOffset = y_lcd_size - height;
            outOffset = y_lcd_size - height;
        } else {
            inputMem = fb_src + PixelFormatToBPP(src_pixel_format) * ((x_lcd_size * (y_src)) + x_src);
            outputMem = fb_dst + PixelFormatToBPP(dst_pixel_format) * ((x_lcd_size * (y_dst)) + x_dst);
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = x_lcd_size - width;
            outOffset = x_lcd_size - width;
        }
        DmaOperation(
            inputMem, 0, outputMem, PixelsPerLine, NumberOfLines, inOffset, 0, outOffset, src_pixel_format, 0,
            dst_pixel_format);
    }

    void Painter::DmaMoveImage(uintptr_t img_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst,
                               int32_t y_dst, int32_t width, int32_t height, int32_t totalImageWidth, int32_t totalImageHeight,
                               uint32_t activeFrame, uint32_t frames, uint32_t inPixelFormat, uint32_t outPixelFormat, bool hasAlpha, uintptr_t imageCLT) const
    {

        int inBytes = PixelFormatToBPP(inPixelFormat);
        int outBytes = PixelFormatToBPP(outPixelFormat);
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0, frameWidth = 0;

        if(IsRotated()) {
            frameWidth = totalImageWidth / frames;
            inputMem = img_src + inBytes * ((totalImageWidth * (totalImageHeight - width - x_src) + y_src + (activeFrame * frameWidth)));
            outputMem = fb_dst + outBytes * (y_lcd_size * (x_lcd_size - x_dst - width) + y_dst);
            NumberOfLines = width - x_src;
            PixelsPerLine = height;
            inOffset = totalImageWidth - height;
            outOffset = y_lcd_size - height;
        } else {
            frameWidth = totalImageWidth / frames;
            inputMem = img_src + inBytes * ((totalImageWidth * y_src) + x_src + (frameWidth * activeFrame));
            outputMem = fb_dst + outBytes * ((x_lcd_size * (y_dst)) + x_dst);
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = totalImageWidth - width + (x_src + frameWidth - width);
            outOffset = x_lcd_size - width;
        }

        if(hasAlpha) {
            DmaOperationCLT(
                inputMem, outputMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, outOffset, outOffset,
                inPixelFormat, outPixelFormat, outPixelFormat, 0, imageCLT);
        } else {
            DmaOperationCLT(
                inputMem, 0, outputMem, PixelsPerLine, NumberOfLines, inOffset, 0, outOffset, inPixelFormat, 0,
                outPixelFormat, 0, imageCLT);
        }
    }

    void Painter::DmaMoveShadow(uintptr_t img_src, uintptr_t fb_dst, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height,
                                uint32_t outPixelFormat, uint32_t color) const
    {

        int outBytes = PixelFormatToBPP(outPixelFormat);
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

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
            COLOR_FORMAT_ARGB8888, outPixelFormat, outPixelFormat, color);
    }

    void Painter::DmaMoveFont(uintptr_t font_src, uintptr_t font_dst, int32_t x_src, int32_t y_src, int32_t x_dst,
                              int32_t y_dst, int32_t width, int32_t height, int32_t fontWidth, int32_t fontHeight, uint32_t outPixelFormat,
                              uint32_t fontColor) const
    {
        int inBytes = PixelFormatToBPP(COLOR_FORMAT_A8);
        int outBytes = PixelFormatToBPP(outPixelFormat);
        uint32_t x_lcd_size = GetXSize();
        uint32_t y_lcd_size = GetYSize();
        uintptr_t inputMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0;

        /* safety checks so that font is in buffer bounds */
        if(y_dst < 0){
            auto offset = std::abs(y_dst);
            y_dst = 0;
            y_src += offset;
            height -= offset;
        }

        if(y_dst + height >= y_lcd_size){
            height -= y_dst + height - y_lcd_size;
        }

        if(x_dst < 0){
            auto offset = std::abs(x_dst);
            x_dst = 0;
            x_src += offset;
            width -= offset;
        }

        if(x_dst + width >= x_lcd_size){
            width -= x_dst + width - x_lcd_size;
        }

        if(IsRotated()) {
            return;
        } else {
            inputMem = font_src + (uint32_t)(inBytes * ((fontWidth * y_src) + x_src));
            outputMem = font_dst + (uint32_t)(outBytes * ((x_lcd_size * (y_dst)) + x_dst));
            NumberOfLines = height;
            PixelsPerLine = width;
            inOffset = fontWidth - width + x_src;
            outOffset = x_lcd_size - width;
        }

        DmaOperation(inputMem, outputMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, outOffset, outOffset,
                             COLOR_FORMAT_A8, outPixelFormat, outPixelFormat, fontColor);
    }

    void Painter::DmaTransferToFramebuffer(int32_t y_position, int32_t height, bool with_background, bool inPlace)
    {

        uint8_t backFramebufferPixelFormat = GetPixelFormat();
        uint8_t backFramebufferBPP = GetBytesPerPixel();
        uint8_t displayFramebufferPixelFormat = GetDisplayPixelFormat();
        uint8_t displayFramebufferBPP = GetDisplayBytesPerPixel();
        uint8_t backgroundPixelFormat = BackgroundImage->GetContentColorFormat();
        uint8_t backgroundBPP = BackgroundImage->GetContentBytesPerPixel();

        uintptr_t inputMem = 0, backMem = 0, outputMem = 0;
        uint32_t NumberOfLines = 0, PixelsPerLine = 0, inOffset = 0, outOffset = 0, backOffset = 0;

        if(IsRotated()) {
            inputMem = GetActiveBuffer() + y_position * backFramebufferBPP;

            if(with_background) {
                backMem = (uintptr_t)BackgroundImage->GetContentData() + y_position * backgroundBPP;
                if((y_position + height) > BackgroundImage->GetHeight()) {
                    grvl::Log("[ERROR] Background is smaller than the surface!");
                }
            }

            outputMem = GetVisibleBuffer() + y_position * displayFramebufferBPP;
            NumberOfLines = GetDisplayWidth();
            PixelsPerLine = height;
            inOffset = (GetDisplayHeight() - height) * backFramebufferBPP;
            backOffset = BackgroundImage->GetHeight() - height;
            outOffset = (GetDisplayHeight() - height) * displayFramebufferBPP;

        } else {
            inputMem = GetActiveBuffer() + (y_position * GetDisplayWidth() * backFramebufferBPP);
            if(with_background) {
                backMem = (uintptr_t)BackgroundImage->GetContentData()
                    + (y_position * BackgroundImage->GetWidth() * backgroundBPP);
                if((y_position + height) > BackgroundImage->GetHeight()) {
                    grvl::Log("[ERROR] Background is smaller than the surface!");
                }
            } else {
                backMem = 0;
            }
            outputMem = GetVisibleBuffer() + (y_position * GetDisplayWidth() * displayFramebufferBPP);
            NumberOfLines = height;
            PixelsPerLine = GetDisplayWidth();
            inOffset = 0;
            backOffset = GetDisplayWidth() - BackgroundImage->GetWidth();
            outOffset = 0;
        }

        if(with_background && !BackgroundImage->IsEmpty() && backgroundPixelFormat == COLOR_FORMAT_L8) {
            DmaOperationCLT(
                inputMem, backMem, inPlace ? inputMem : outputMem, PixelsPerLine, NumberOfLines, inOffset, backOffset, outOffset,
                backFramebufferPixelFormat, backgroundPixelFormat, displayFramebufferPixelFormat, BackgroundImage->GetContent()->GetPLTE(), 0);
        } else {
            DmaOperation(
                inputMem, backMem, inPlace ? inputMem : outputMem, PixelsPerLine, NumberOfLines, inOffset, backOffset, outOffset,
                backFramebufferPixelFormat, backgroundPixelFormat, displayFramebufferPixelFormat, 0);
        }
    }

    void Painter::CreateFramebuffersCollection(uint8_t BufferBPP, uint8_t* framebuffer)
    {
        if (framebuffer) {
            backLayerPointers[0].data = (uintptr_t)framebuffer;
        }
        else {
            backLayerPointers[0].data = (uintptr_t)grvl::Callbacks()->malloc(BufferBPP * XSize * YSize * 4);
        }

        backLayerPointers[1].data = backLayerPointers[0].data + BufferBPP * XSize * YSize;
        backLayerPointers[2].data = backLayerPointers[1].data + BufferBPP * XSize * YSize;
        backLayerPointers[3].data = backLayerPointers[2].data + BufferBPP * XSize * YSize;

        switch(BufferBPP) {
            case 2:
            {
                backLayerPointers[0].pixel_format = COLOR_FORMAT_ARGB4444;
                backLayerPointers[1].pixel_format = backLayerPointers[0].pixel_format;
                backLayerPointers[2].pixel_format = COLOR_FORMAT_RGB565;
                backLayerPointers[3].pixel_format = COLOR_FORMAT_RGB565;
                break;
            }
            case 3:
            {
                backLayerPointers[0].pixel_format = COLOR_FORMAT_ARGB4444;
                backLayerPointers[1].pixel_format = backLayerPointers[0].pixel_format;
                backLayerPointers[2].pixel_format = COLOR_FORMAT_RGB888;
                backLayerPointers[3].pixel_format = COLOR_FORMAT_RGB888;
                break;
            }
            case 4:
            {
                backLayerPointers[0].pixel_format = COLOR_FORMAT_ARGB8888;
                backLayerPointers[1].pixel_format = backLayerPointers[0].pixel_format;
                backLayerPointers[2].pixel_format = COLOR_FORMAT_ARGB8888;
                backLayerPointers[3].pixel_format = COLOR_FORMAT_ARGB8888;
                break;
            }
        }

        // Prepare shadow image
        uint8_t* imgContent = (uint8_t*)grvl::Callbacks()->malloc(XSize * /*1*/ 4);
        shadowImage = new ImageContent(ImageContent::FromRAW(imgContent, XSize, 1, 1, /*COLOR_FORMAT_A8*/ COLOR_FORMAT_RGB888));
        if(IsRotated()) {
            if(!shadowImage->IsRotated()) {
                shadowImage->Rotate90();
            }
        }
    }

    void Painter::InitFramebuffersCollection()
    {
        FillMemory(backLayerPointers[0].data, XSize, YSize * 2, COLOR_ARGB8888_BLACK, backLayerPointers[0].pixel_format);
        FillMemory(backLayerPointers[2].data, XSize, YSize * 2, COLOR_ARGB8888_BLACK, backLayerPointers[2].pixel_format);
    }

    uint32_t Painter::GetXSize() const
    {
        return XSize;
    }

    uint32_t Painter::GetYSize() const
    {
        return YSize;
    }

    int32_t Painter::GetDisplayWidth() const
    {
        return XSize;
    }

    int32_t Painter::GetDisplayHeight() const
    {
        return YSize;
    }

    void Painter::SetLayerAddress(uint32_t display)
    {
        grvl::Callbacks()->set_layer_pointer(backLayerPointers[display].data);
    }

    void Painter::AddBackgroundBlock(int32_t y_position, int32_t height, uint32_t backgroundColor)
    {
        if(!HasTransparency(backgroundColor) || BackgroundImage->IsEmpty()) {
            return;
        }

        background_block bblock;
        bblock.y_position = y_position;
        bblock.height = height;
        // y_position is equal to ParentY + Y; both of them could be defined as < 0 by mistake.
        // In this case blocks with y_position < 0 are ignored.
        if(height > 0 && (y_position + height) <= (int32_t)GetYSize()) {
            bblocks.push_back(bblock);
        }
    }

    static int findNextBlock(const Painter::background_block_vector& vec, int transferPosition)
    {
        int nextValue = INT_LEAST16_MAX;
        int index = -1;

        for(uint32_t i = 0; i < vec.size(); i++) {
            if(vec[i].y_position >= transferPosition && vec[i].y_position < nextValue) {
                nextValue = vec[i].y_position;
                index = i;
            }
        }

        return index;
    }

    void Painter::MergeBuffers(bool inPlace)
    {
        if(bblocks.size() == 0 || BackgroundImage->IsEmpty()) {
            DmaTransferToFramebuffer(0, GetYSize(), false, inPlace);
        } else {
            int currentTransferPosition = 0, i = 0;
            // Transfer blocks
            while(1) {
                i = findNextBlock(bblocks, currentTransferPosition);
                if(i == -1) {
                    break;
                }

                if(currentTransferPosition < bblocks[i].y_position) { // Transfer without background
                    DmaTransferToFramebuffer(
                        currentTransferPosition, bblocks[i].y_position - currentTransferPosition, false, inPlace);
                    currentTransferPosition = bblocks[i].y_position;
                } else if(currentTransferPosition > bblocks[i].y_position) {
                    continue;
                }

                // Detect block continuity
                int mergedBlockBeginY = bblocks[i].y_position, mergedBlockHeight = bblocks[i].height, j = 0;

                while(1) { // Check if another block can be merged
                    j = findNextBlock(bblocks, currentTransferPosition + mergedBlockHeight);
                    if(j > -1 && bblocks[j].y_position <= mergedBlockBeginY + mergedBlockHeight) {
                        mergedBlockHeight = max(mergedBlockHeight,
                                                bblocks[j].y_position + bblocks[j].height - mergedBlockBeginY);
                    } else {
                        break;
                    }
                }
                DmaTransferToFramebuffer(mergedBlockBeginY, mergedBlockHeight, true, inPlace);
                currentTransferPosition += mergedBlockHeight;
            }
            if(currentTransferPosition < (int32_t)GetYSize()) {
                DmaTransferToFramebuffer(currentTransferPosition, GetYSize() - currentTransferPosition, false, inPlace);
            }
            bblocks.clear();
        }
    }

    void Painter::FlipBuffers()
    {
        SetLayerAddress(VisibleBuffer ? 2 : 3);
        VisibleBuffer = VisibleBuffer ? 0 : 1;
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
        return VisibleBuffer;
    }

    void Painter::FlipSynchronizeBuffers()
    {
        if(grvl::Callbacks()->wait_for_vsync) {
            grvl::Callbacks()->wait_for_vsync();
        }
        FlipBuffers();
        if(grvl::Callbacks()->flipping_completed) {
            grvl::Callbacks()->flipping_completed();
        }
    }

    void Painter::SetBackgroundImage(const string& resource)
    {
        // TODO:
        contentManager->BindImageContentToImage(resource, BackgroundImage);
    }

    void Painter::SetBackgroundImage(Image* image)
    {
        if(image) {
            BackgroundImage = image;
        }
    }

    uintptr_t Painter::GetBuffer(int id) const
    {
        if(id > 3) {
            return 0;
        }
        return backLayerPointers[id].data;
    }

    uintptr_t Painter::GetActiveBuffer() const
    {
        return backLayerPointers[ActiveBuffer].data;
    }

    uintptr_t Painter::GetVisibleBuffer() const
    {
        return backLayerPointers[VisibleBuffer ? 2 : 3].data;
    }

    void Painter::SetActiveBuffer(uint32_t activeBuffer)
    {
        if(activeBuffer < 4) {
            ActiveBuffer = activeBuffer;
        }
    }

    void Painter::DisplayAntialiasedString(const Font* Font, int16_t Xpos, int16_t Ypos, const char* Text,
                                           uint32_t text_color) const
    {
        InnerDisplayAntialiasedString(Font, Xpos, Ypos, Text, text_color, false, 0, 0, 0, 0);
    }

    void Painter::DisplayBoundedAntialiasedString(const Font* Font, int16_t Xpos, int16_t Ypos, int16_t ParentX,
                                                  int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, const char* Text, uint32_t text_color) const
    {
        InnerDisplayAntialiasedString(Font, Xpos, Ypos, Text, text_color, true, ParentX, ParentY, ParentWidth, ParentHeight);
    }

    void Painter::InnerDisplayAntialiasedString(const Font* Font, int16_t Xpos, int16_t Ypos, const char* Text, uint32_t text_color, bool bound, int16_t ParentX,
                                                int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight) const
    {
        uint32_t previousCharCode = 0; // Save char code placed before space
        while(*Text != 0) {
            uint32_t charCode = 0, nextCharCode = 0;
            int length = 1;
            Font::unicode_character uchar, next_uchar;

            /* Display one character on LCD */
            if(*Text != 0x20) { // Not a space
                if((unsigned char)(*Text) > 0x7F) { // Unicode
                    uchar = Font::GetUnicodeCharacter(Text);
                    length = uchar.length;
                    charCode = uchar.code;
                    if((*(Text + length)) != 0) {
                        next_uchar = Font::GetUnicodeCharacter(Text + length);
                        nextCharCode = next_uchar.code;
                    }
                } else { // UTF8
                    charCode = *Text;
                    if((*(Text + length)) != 0) {
                        if((*(Text + length)) <= 0x7f) {
                            nextCharCode = *(Text + length);
                        } else { // Next character is a unicode character
                            next_uchar = Font::GetUnicodeCharacter(Text + length);
                            nextCharCode = next_uchar.code;
                        }
                    }
                }

                if(bound) {
                    DisplayAntialiasedCharInBound(
                        Font, Xpos, Ypos, ParentX, ParentY, ParentWidth, ParentHeight, charCode, text_color);
                } else {
                    DisplayAntialiasedChar(Font, Xpos, Ypos, charCode, text_color);
                }
                Xpos += Font->GetCharWidth(charCode);
                if(nextCharCode) {
                    Xpos += Font->GetKerning(charCode, nextCharCode);
                }
            } else { // space
                charCode = *Text;

                if((*(Text + length)) != 0) {
                    if((*(Text + length)) <= 0x7f) {
                        nextCharCode = *(Text + length);
                    } else { // Next character is a unicode character
                        next_uchar = Font::GetUnicodeCharacter(Text + length);
                        nextCharCode = next_uchar.code;
                    }
                }

                Xpos += Font->GetSpaceLength();
                if(nextCharCode && previousCharCode) {
                    Xpos += Font->GetKerning(previousCharCode, nextCharCode);
                }
            }

            Text += length;

            if(nextCharCode == 0x20) { // Save char code placed before space
                previousCharCode = charCode;
            }
        }
    }

    void Painter::DisplayAntialiasedChar(const Font* Font, uint16_t Xpos, uint16_t Ypos, uint32_t Index,
                                         uint32_t text_color) const
    {
        DrawAntialiasedChar(Font, Xpos, Ypos, Index, text_color);
    }

    void Painter::DrawAntialiasedChar(const Font* Font, int16_t Xpos, int16_t Ypos, uint32_t Index,
                                      uint32_t text_color) const
    {
        DrawAntialiasedCharInBound(Font, Xpos, Ypos, 0, 0, GetXSize(), GetYSize(), Index, text_color);
    }

    void Painter::DrawAntialiasedCharInBound(const Font* Font, int16_t Xpos, int16_t Ypos, int16_t ParentX,
                                             int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t Index,
                                             uint32_t text_color) const
    {
        uint32_t* c = Font->GetCharData(Index);
        if(c == NULL) {
            return; // Character doesn't exists
        }
        uint16_t height, width;
        int32_t TopOffset = 0, BottomOffset = 0;

        if(ParentHeight == 0) {
            return;
        }

        height = Font->GetHeight();
        width = Font->GetCharWidth(Index);

        int myY = Ypos - ParentY;

        if(ParentHeight < 0) {
            TopOffset = -ParentHeight;
            TopOffset = myY - TopOffset;
            if(TopOffset > 0) {
                TopOffset = 0;
            } else {
                TopOffset = -TopOffset;
            }
        } else if(ParentHeight <= myY + height) { // Bottom offset
            BottomOffset = height - (ParentHeight - myY);
        }

        if(TopOffset + BottomOffset >= height) {
            return;
        }

        DmaMoveFont(
            (uintptr_t)c, GetActiveBuffer(), 0, TopOffset, Xpos, Ypos + TopOffset, (int32_t)width,
            (int32_t)height - (BottomOffset + TopOffset), (int32_t)width, (int32_t)height,
            GetActiveBufferPixelFormat(), text_color);
    }

    void Painter::DisplayAntialiasedCharInBound(const Font* Font, int16_t Xpos, int16_t Ypos, int16_t ParentX,
                                                int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t index, uint32_t text_color) const
    {
        DrawAntialiasedCharInBound(
            Font, Xpos, Ypos, ParentX, ParentY, ParentWidth, ParentHeight, index, text_color);
    }

    void Painter::SetContentManager(ContentManager* cm)
    {
        contentManager = cm;
    }

    ContentManager* Painter::GetContentManager()
    {
        return contentManager;
    }

} /* namespace grvl */

// NOLINTEND
