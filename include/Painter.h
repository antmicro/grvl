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

#ifndef GRVL_PAINTER_H_
#define GRVL_PAINTER_H_

#include "Definitions.h"
#include "Font.h"
#include "stl.h"
#include <stdint.h>

namespace grvl {
    class Image;
    class ContentManager;
    class ImageContent;

    typedef struct {
        uintptr_t data;
        uint32_t pixel_format;
    } layer_t;

    typedef struct {
        int32_t y_position;
        int32_t height;
    } background_block;

    void FallbackDmaOperation(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
                              uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset,
                              uint32_t outOffset, uint32_t inColor, uint32_t backgroundColor, uint32_t outColor, uint32_t fontColor, uintptr_t backCLT, uintptr_t frontCTL);

    void FallbackDmaFill(uintptr_t dst, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t offset,
                         uint32_t color_index, uint32_t pixel_format);

    /// Represents object used to draw graphics.
    class Painter {
    public:
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

        Painter()
            : backgroundColor(0)
            , XSize(0)
            , YSize(0)
            , VisibleBuffer(0)
            , ActiveBuffer(0)
            , BackgroundImage(0)
            , contentManager(0)
            , is_rotated(false)
            , shadowImage(0)
        {
        }

        virtual ~Painter();

        void SetDisplaySize(int32_t x, int32_t y);
        void CreateFramebuffersCollection(uint8_t BufferBPP, uint8_t* framebuffer = NULL);
        void InitFramebuffersCollection();
        void SetBackgroundImage(Image* image);
        void SetBackgroundImage(const string& resource);

        void SetContentManager(ContentManager* cm);
        ContentManager* GetContentManager();

        uint32_t GetXSize() const;
        uint32_t GetYSize() const;
        uint8_t GetSwapperValue() const; // temporary

        uintptr_t GetBuffer(int id) const;
        uintptr_t GetVisibleBuffer() const;
        uintptr_t GetActiveBuffer() const;

        void SetActiveBuffer(uint32_t activeBuffer);

        void FlipBuffers();
        void FlipSynchronizeBuffers();

        void FillCircle(int16_t Xpos, int16_t Ypos, int16_t Radius, uint32_t color) const;
        void DrawCircle(int16_t Xpos, int16_t Ypos, int16_t Radius, uint32_t color) const;

        void FillArc(int32_t Xpos, int32_t Ypos, int32_t startAngle, int32_t endAngle, int32_t radius, int32_t width,
                     uint32_t startColor, uint32_t endColor) const;

        void DrawEllipse(int32_t Xpos, int32_t Ypos, int32_t XRadius, int32_t YRadius, uint32_t color) const;
        void FillEllipse(int32_t Xpos, int32_t Ypos, int32_t XRadius, int32_t YRadius, uint32_t color) const;

        void FillTriangle(int16_t x1, int16_t x2, int16_t x3, int16_t y1, int16_t y2, int16_t y3, uint32_t color) const;

        void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const;
        void DrawAntialiasedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const;
        void DrawVLine(int32_t Xpos, int32_t Ypos, uint16_t Length, uint32_t text_color) const;
        void DrawPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const;
        void BlendPixel(uint32_t Xpos, uint32_t Ypos, uint32_t RGB_Code) const;
        uint32_t ReadPixel(uint32_t Xpos, uint32_t Ypos) const;

        void SetLayerAddress(uint32_t display);

        void DmaOperation(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                          uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                          uint32_t inPixelFormat, uint32_t backgroundPixelFormat, uint32_t outPixelFormat,
                          uint32_t frontColor = 0) const;

        void DmaOperationCLT(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
                             uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                             uint32_t inPixelFormat, uint32_t backgroundPixelFormat, uint32_t outPixelFormat, uintptr_t backCLT, uintptr_t frontCLT) const;

        void DmaFill(uintptr_t outputMem, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t outOffset,
                     uint32_t color_index, uint32_t pixel_format) const;

        void DmaMove(uintptr_t fb_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                     int32_t width, int32_t height, uint32_t src_pixel_format, uint32_t dst_pixel_format) const;

        void DmaMoveImage(uintptr_t img_src, uintptr_t fb_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                          int32_t width, int32_t height, int32_t totalImageWidth, int32_t totalImageHeight, uint32_t activeFrame,
                          uint32_t frames, uint32_t inPixelFormat, uint32_t outPixelFormat, bool hasAlpha, uintptr_t imageCLT = 0) const;

        void DmaMoveFont(uintptr_t font_src, uintptr_t font_dst, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst,
                         int32_t width, int32_t height, int32_t fontWidth, int32_t fontHeight, uint32_t outPixelFormat,
                         uint32_t fontColor) const;

        void DmaMoveShadow(uintptr_t img_src, uintptr_t fb_dst, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height,
                           uint32_t outPixelFormat, uint32_t color) const;

        bool IsRotated() const;

        void DrawHLine(int32_t Xpos, int32_t Ypos, uint16_t Length, uint32_t text_color) const;

        void DisplayAntialiasedString(const Font* Font, int16_t Xpos, int16_t Ypos, const char* Text,
                                      uint32_t text_color) const;
        void DisplayAntialiasedChar(const Font* Font, uint16_t Xpos, uint16_t Ypos, uint32_t Index,
                                    uint32_t text_color) const;
        void DrawAntialiasedChar(const Font* Font, int16_t Xpos, int16_t Ypos, uint32_t Index,
                                 uint32_t text_color) const;
        void DrawAntialiasedCharInBound(const Font* Font, int16_t Xpos, int16_t Ypos, int16_t ParentX, int16_t ParentY,
                                        int16_t ParentWidth, int16_t ParentHeight, uint32_t Index, uint32_t text_color) const;
        void DisplayAntialiasedCharInBound(const Font* Font, int16_t Xpos, int16_t Ypos, int16_t ParentX,
                                           int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t index, uint32_t text_color) const;
        void DisplayBoundedAntialiasedString(const Font* Font, int16_t Xpos, int16_t Ypos, int16_t ParentX,
                                             int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, const char* Text, uint32_t text_color) const;
        uint32_t GetPixelFormat() const;
        uint32_t GetBytesPerPixel() const;
        uint32_t GetDisplayPixelFormat() const;
        uint32_t GetDisplayBytesPerPixel() const;
        uint32_t GetActiveBufferPixelFormat() const;
        uint32_t GetActiveBufferBytesPerPixel() const;
        void SetBackgroundColor(uint32_t color);
        void SetRotation(bool rotate90);
        int32_t GetDisplayWidth() const;
        int32_t GetDisplayHeight() const;
        uint32_t GetBackgroundColor() const;
        void FillRectangle(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t text_color) const;
        void DrawRectangle(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t text_color) const;

        void FillMemory(uintptr_t memory, uint32_t width, uint32_t height, uint32_t text_color, uint32_t colorFormat = COLOR_FORMAT_ARGB8888);

        // Background blocks
        typedef vector<background_block> background_block_vector;
        background_block_vector bblocks;
        void AddBackgroundBlock(int32_t y_position, int32_t height, uint32_t backgroundColor);
        void DmaTransferToFramebuffer(int32_t y_position, int32_t height, bool with_background, bool inPlace = false);
        void MergeBuffers(bool inPlace = false);
        void ShadowBuffer(uint8_t number, uint32_t color);

    protected:
        layer_t backLayerPointers[4];
        uint32_t backgroundColor;
        uint32_t XSize, YSize;
        uint8_t VisibleBuffer, ActiveBuffer;
        Image* BackgroundImage;
        ContentManager* contentManager;
        bool is_rotated;
        ImageContent* shadowImage;
        void DrawSpansBetweenEdges(const Edge& e1, const Edge& e2) const;
        void DrawSpan(int x1, int x2, uint32_t color, int y) const;

    private:
        void InnerDisplayAntialiasedString(const Font* Font, int16_t Xpos, int16_t Ypos, const char* Text, uint32_t text_color, bool bounded, int16_t ParentX,
                                           int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight) const;
    };

} /* namespace grvl */

#endif /* GRVL_PAINTER_H_ */
