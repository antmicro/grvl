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

#include <grvl/Font.h>
#include <grvl/Format.h>

#include <array>
#include <stack>
#include <stdint.h>
#include <string>
#include <vector>

namespace grvl {

    template <typename T>
    struct Vec2 {
        T x;
        T y;
    };

    using Vec2f = Vec2<float>;
    using Vec2i = Vec2<int>;

    class Image;
    class ContentManager;
    class ImageContent;

    /// Represents object used to draw graphics.
    class Painter {
    public:
        Painter()
            : backgroundColor(0)
            , XSize(0)
            , YSize(0)
            , contentManager(0)
            , is_rotated(false)
        {
        }

        virtual ~Painter();

        void SetDisplaySize(int32_t x, int32_t y);

        /// Creates the framebuffers that will be used by the painter, or uses the ones provided
        virtual void CreateFramebuffersCollection(Format fmt, void* framebuffer_1 = nullptr, void* framebuffer_2 = nullptr) = 0;
        virtual void InitFramebuffersCollection() = 0;

        void SetContentManager(ContentManager* cm);
        ContentManager* GetContentManager();

        constexpr uint32_t GetXSize() const { return XSize; }
        constexpr uint32_t GetYSize() const { return YSize; }

        struct DrawingBound {
            int32_t startX { 0 };
            int32_t startY { 0 };
            int32_t endX { 0 };
            int32_t endY { 0 };

            int32_t Width() const { return endX - startX; }
            int32_t Height() const { return endY - startY; }
            bool operator==(const DrawingBound& r) const { return (startX == r.startX) && (startY == r.startY) && (endX == r.endX) && (endY == r.endY); }
            bool operator!=(const DrawingBound& r) const { return !(*this == r); }
        };

        // Request frames (used by GIFs, scrolling etc)
        void RequestNextFrame() { nextFrameRequested = true; }
        bool IsNextFrameRequested() const { return nextFrameRequested; }
        void ClearNextFrameRequest() { nextFrameRequested = false; }

        void ResetDrawingBounds();
        void PushDrawingBound(int32_t startX, int32_t startY, int32_t endX, int32_t endY);
        void PopDrawingBound();

        enum class CircleQuarter : unsigned int {
            TOP_LEFT = 0,
            TOP_RIGHT = 1,
            BOTTOM_RIGHT = 2,
            BOTTOM_LEFT = 3
        };

        virtual void BeginFrame() = 0;

        /// Draw filled circle of the given radius and color with its center at (x, y)
        virtual void FillCircle(int16_t x, int16_t y, int16_t radius, uint32_t color) const = 0;

        /// Draw circle outline of the given radius and color with its center at (x, y)
        virtual void DrawCircle(int16_t x, int16_t y, int16_t radius, uint32_t color) const = 0;

        /// Draw arc outline such that the circle the angle belong to has its center at (x, y)
        virtual void DrawArc(int16_t x, int16_t y, int16_t radius, float startAngleDeg, float endAngleDeg, uint32_t color) const = 0;

        /// Draw arc outline of specific width such that the circle the angle belongs to has its center at (x, y)
        virtual void DrawArc(int32_t x, int32_t y, int32_t radius, int32_t startAngleDeg, int32_t endAngleDeg, int32_t width, uint32_t startColor, uint32_t endColor) const = 0;

        /// Draw filled arc, akin to a "pizza-slice", such that the circle the angle belongs to has its center at (x, y)
        virtual void FillArc(int32_t x, int32_t y, int32_t radius, float startAngleDeg, float endAngleDeg, uint32_t color) const = 0;

        /// Draw 1/4 of a filled circle
        virtual void FillQuarterCircle(int32_t x, int32_t y, int32_t radius, CircleQuarter quarter, uint32_t color) const = 0;

        /// Draw outline of an ellipse
        virtual void DrawEllipse(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const = 0;

        /// Draw filled ellipse
        virtual void FillEllipse(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const = 0;

        /// Draw the given ImageContent, the (x, y) point the the image's lower-left corner
        virtual void DrawImage(int16_t x, int16_t y, const ImageContent* image, uint32_t frame = 0) const = 0;

        /// Draw 1-pixel wide line between two points
        virtual void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) const = 0;

        /// Draw 1-pixel wide vertical line
        virtual void DrawVLine(int32_t x, int32_t y, int32_t length, uint32_t color) const = 0;

        /// Draw 1-pixel wide horizontal line
        virtual void DrawHLine(int32_t x, int32_t y, int32_t length, uint32_t color) const = 0;

        /// Draw 1-pixel wide vertical line with a smooth color gradient across it
        virtual void DrawGradientVLine(int16_t x1, int16_t y1, int16_t y2, uint32_t startingColor, uint32_t endingColor, float startingValue) const = 0;

        /// Draw a solid-color rectangle
        virtual void FillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const = 0;

        /// Draw a semi-transparent screen overlay
        virtual void ShadowBuffer(uint8_t alpha) = 0;

        // Called at the end of frame, TODO rename to EndFrame()?
        virtual void SwapBuffers() = 0;

        /// Draw a solid-color rectangle with round edges
        virtual void FillRoundRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, float radius) const;

        /// Draw an outline of a solid-color rectangle
        virtual void DrawRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) const;

        /// Draw an outline of a solid-color rectangle with round edges
        virtual void DrawRoundRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, float radius) const;

        void DrawString(Font* font, int16_t x, int16_t y, const std::string& text, uint32_t color, uint32_t background = 0) const;
        void DisplayAntialiasedString(Font* font, int16_t x, int16_t y, const char* text, uint32_t color, uint32_t background = 0) const;
        void DisplayBoundedAntialiasedString(Font* font, int16_t x, int16_t y, int16_t ParentX, int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, const char* text, uint32_t color, uint32_t background = 0) const;
        void DisplayAntialiasedChar(Font* font, uint16_t x, uint16_t y, uint32_t index, uint32_t color, uint32_t background = 0) const;
        void DrawAntialiasedChar(Font* font, int16_t x, int16_t y, uint32_t index, uint32_t color, uint32_t background = 0) const;
        void DisplayAntialiasedCharInBound(Font* font, int16_t x, int16_t y, int16_t ParentX, int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t index, uint32_t color, uint32_t background = 0) const;

        static uint32_t InterpolateColors(uint32_t first, uint32_t second, float t);
        bool IsRotated() const;
        void SetBackgroundColor(uint32_t color);
        void SetRotation(bool rotate90);
        uint32_t GetBackgroundColor() const;

        void FillMemory(void* memory, int32_t width, int32_t height, uint32_t color, Format format = Format::ARGB8888);

        [[deprecated("This will become private")]] void DrawAntialiasedCharInBound(Font* font, int16_t x, int16_t y, int16_t ParentX, int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t index, uint32_t color, uint32_t background = 0) const;

        [[deprecated("For Removal")]] int32_t GetDisplayWidth() const;

        [[deprecated("For Removal")]] int32_t GetDisplayHeight() const;

        [[deprecated("For Removal")]] uint8_t GetSwapperValue() const; // temporary

        [[deprecated("For Removal")]] uintptr_t GetBuffer(int id) const;

        [[deprecated("For Removal")]] void SetActiveBuffer(uint32_t activeBuffer);

        [[deprecated("For Removal")]] void AddBackgroundBlock(int32_t y_position, int32_t height, uint32_t backgroundColor);

        virtual std::string GetDebugString() const = 0;

    protected:
        void InnerDisplayAntialiasedString(Font* Font, int16_t x, int16_t y, const char* text, uint32_t color, bool bounded, int16_t ParentX,
                                           int16_t ParentY, int16_t ParentWidth, int16_t ParentHeight, uint32_t background = 0) const;

        virtual void DrawGlyph(Glyph& glyph, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst, int32_t width, int32_t height, uint32_t fontColor) const = 0;

        // callbacks
        virtual void OnSizeChanged(int width, int height);

        const DrawingBound& GetCurrentBound() const { return bounds.top(); }

        int32_t CurrentDrawingBoundsStartX() const { return GetCurrentBound().startX; }
        int32_t CurrentDrawingBoundsStartY() const { return GetCurrentBound().startY; }
        int32_t CurrentDrawingBoundsEndX() const { return GetCurrentBound().endX; }
        int32_t CurrentDrawingBoundsEndY() const { return GetCurrentBound().endY; }
        int32_t CurrentDrawingBoundsWidth() const { return GetCurrentBound().Width(); }
        int32_t CurrentDrawingBoundsHeight() const { return GetCurrentBound().Height(); }

        constexpr float ToRadians(float eulerAngles) const
        {
            return eulerAngles * 3.14159f / 180.0f;
        }

        // TODO rename width/height
        uint32_t XSize, YSize;

    private:
        std::stack<DrawingBound> bounds {};
        ContentManager* contentManager;
        uint32_t backgroundColor;
        bool is_rotated;
        bool nextFrameRequested { false };
    };

} /* namespace grvl */

#endif /* GRVL_PAINTER_H_ */
