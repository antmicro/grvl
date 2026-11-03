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

#pragma once

#include <grvl/Painter.h>

namespace grvl {

    /// Uses software rasterization and DMA callbacks
    class SoftwareRasterizer final : public Painter {
    public:
        void BeginFrame() override;

        void CreateFramebuffersCollection(Format fmt, void* framebuffer_1 = nullptr, void* framebuffer_2 = nullptr) override;
        void InitFramebuffersCollection() override;

        void FillCircle(int16_t x, int16_t y, int16_t radius, uint32_t color) const override;
        void DrawCircle(int16_t x, int16_t y, int16_t radius, uint32_t color) const override;
        void DrawArc(int16_t x, int16_t y, int16_t radius, float startAngleDeg, float endAngleDeg, uint32_t color) const override;
        void FillQuarterCircle(int32_t x, int32_t y, int32_t radius, CircleQuarter circleQuarter, uint32_t color) const override;
        void DrawArc(int32_t x, int32_t y, int32_t radius, int32_t startAngleDeg, int32_t endAngleDeg, int32_t width, uint32_t startColor, uint32_t endColor) const override;
        void FillArc(int32_t x, int32_t y, int32_t radius, float startAngleDeg, float endAngleDeg, uint32_t color) const override;
        void DrawEllipse(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const override;
        void FillEllipse(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const override;
        void DrawImage(int16_t x, int16_t y, const ImageContent* image, uint32_t frame = 0) const override;
        void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const override;
        void DrawVLine(int32_t Xpos, int32_t Ypos, int32_t Length, uint32_t text_color) const override;
        void DrawGradientVLine(int16_t x1, int16_t y1, int16_t y2, uint32_t startingColor, uint32_t endingColor, float startingValue) const override;
        void DrawHLine(int32_t x, int32_t y, int32_t length, uint32_t color) const override;
        void FillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const override;
        void ShadowBuffer(uint8_t alpha) override;

        void SwapBuffers() override;
        std::string GetDebugString() const override;

    private:
        void DrawAntialiasedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const;

        void DrawAntialiasedPixel(float x, float y, uint32_t color) const;
        void FillTriangle(int16_t x1, int16_t x2, int16_t x3, int16_t y1, int16_t y2, int16_t y3, uint32_t color) const;
        void DrawGlyph(Glyph& glyph, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height, uint32_t fontColor) const override;

        class Edge {
        public:
            uint32_t Color;
            int X1, Y1, X2, Y2;

            Edge(int x1, int y1, int x2, int y2, const uint32_t color)
            {
                if(y1 < y2) {
                    X1 = x1;
                    Y1 = y1;
                    X2 = x2;
                    Y2 = y2;
                } else {
                    X1 = x2;
                    Y1 = y2;
                    X2 = x1;
                    Y2 = y1;
                }
                Color = color;
            }
        };

        void DrawSpansBetweenEdges(const Edge& e1, const Edge& e2) const;

        void DrawPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const;
        void BlendPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const;
        uint32_t ReadPixel(uint32_t Xpos, uint32_t Ypos) const;

        uintptr_t GetFrontBuffer() const;
        uintptr_t GetBackBuffer() const;

        void DmaOperation(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                          uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                          Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat,
                          uint32_t frontColor = 0) const;

        void DmaOperationCLT(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                             uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                             Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uintptr_t backCLT, uintptr_t frontCLT) const;

        void DmaFill(uintptr_t outputMem, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t outOffset,
                     uint32_t color_index, Format pixel_format) const;

        void DmaMove(uintptr_t fb_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                     int32_t width, int32_t height, Format src_pixel_format, Format dst_pixel_format) const;

        void DmaMoveImage(uintptr_t img_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                          int32_t width, int32_t height, int32_t totalImageWidth, int32_t totalImageHeight, uint32_t activeFrame,
                          uint32_t frames, Format inPixelFormat, Format outPixelFormat, bool hasAlpha, uintptr_t imageCLT = 0) const;

        void DmaMoveShadow(uintptr_t img_src, uintptr_t fb_dst, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height,
                           Format outPixelFormat, uint32_t color) const;

        Format GetPixelFormat() const;
        uint32_t GetBytesPerPixel() const;

        void DrawSpan(int x1, int x2, uint32_t color, int y) const;

        Format pixel_format = Format::ARGB8888;
        bool swapper = true;
        uintptr_t framebuffers[2];
    };

}
