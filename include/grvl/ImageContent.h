// Copyright 2014-2026 Antmicro <antmicro.com>
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

#ifndef GRVL_IMAGECONTENT_H_
#define GRVL_IMAGECONTENT_H_

#include <grvl/Definitions.h>
#include <grvl/File.h>
#include <grvl/Format.h>
#include <grvl/Misc.h>
#include <grvl/Painter.h>
#include <grvl/grvl.h>

#include <optional>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

namespace grvl {

    class ImageContent {
    public:
        ImageContent(const char* path, std::optional<Format> format = std::nullopt);
        ImageContent(uint8_t* pixels, int width, int height, int frames, Format format = Format::ARGB8888, uint32_t frameDelay = 100);
        ImageContent(int32_t width, int32_t height, int32_t frames = 1, Format format = Format::ARGB8888);
        ImageContent(const ImageContent& content);
        ImageContent& operator=(const ImageContent& other);

        ~ImageContent();

        constexpr uint8_t* GetData()
        {
            return data;
        }

        constexpr const uint8_t* GetData() const
        {
            return data;
        }

        constexpr uint8_t* GetFrameData(uint32_t frame)
        {
            return data + GetFrameDataLength() * frame;
        }

        constexpr const uint8_t* GetFrameData(uint32_t frame) const
        {
            return data + GetFrameDataLength() * frame;
        }

        constexpr uint32_t GetDataLength() const
        {
            return GetWidth() * GetHeight() * GetNumberOfFrames() * GetBytesPerPixel();
        }

        constexpr uint32_t GetFrameDataLength() const
        {
            return GetWidth() * GetHeight() * GetBytesPerPixel();
        }

        constexpr uint16_t GetNumberOfFrames() const
        {
            return frames;
        }

        constexpr uint32_t GetWidth() const
        {
            return width;
        }

        constexpr uint32_t GetHeight() const
        {
            return height;
        }

        constexpr uint32_t GetBytesPerPixel() const
        {
            return GetFormatStride(format);
        }

        constexpr bool HasAlphaChannel() const
        {
            return GetFormatAlphaChannel(format);
        }

        constexpr bool IsRotated() const
        {
            return rotated;
        }

        constexpr bool IsEmpty() const
        {
            return !data;
        }

        constexpr Format GetColorFormat() const
        {
            return format;
        }

        constexpr uint32_t GetNumberOfLines() const
        {
            return rotated ? width : height;
        }

        constexpr uint32_t GetPixelsPerLine() const
        {
            return rotated ? height : width;
        }

        constexpr uintptr_t GetColorPalette() const
        {
            return 0;
        }

        constexpr void* At(int x, int y, int frame = 0)
        {
            return static_cast<void*>(GetFrameData(frame) + GetBytesPerPixel() * (GetPixelsPerLine() * y + x));
        }

        constexpr const void* At(int x, int y, int frame = 0) const
        {
            return static_cast<const void*>(GetFrameData(frame) + GetBytesPerPixel() * (GetPixelsPerLine() * y + x));
        }

        uint32_t ReadPixel(int x, int y, int frame) const;
        void WritePixel(int x, int y, int frame, uint32_t color);

        void Blit(int x, int y, int target_frame, const ImageContent* content, int source_frame);
        void Fill(uint8_t value, int frame = 0);

        ImageContent* Resize(int width, int height) const;
        void Transcode(Format format);
        void Rotate90();

        bool IsAnimated() const
        {
            return frames > 1 && frameDurations.size() == static_cast<size_t>(frames);
        }

        uint32_t GetFrameDuration(uint32_t frame) const
        {
            if(frame >= frameDurations.size()) {
                return 0;
            }
            return frameDurations[frame];
        }

        const std::vector<uint32_t>& GetFrameDurations() const
        {
            return frameDurations;
        }

        // Take ownership of the underlying buffer away from ImageContent,
        // the ImageContent gets reset afte rthis operation.
        [[nodiscard]] constexpr void* TakeData()
        {
            uint8_t* tmp = GetData();
            width = 0;
            height = 0;
            frames = 0;
            rotated = false;
            data = nullptr;
            return tmp;
        }

    private:
        uint8_t* data = nullptr;
        int32_t width = 0;
        int32_t height = 0;
        uint32_t frames = 0;
        Format format = Format::ARGB8888;
        std::vector<uint32_t> frameDurations;
        bool rotated = false;
    };

    /// Convert the pointed to pixel to a color, in the specific output format
    uint32_t ConvertPixel(const uint8_t* data, Format input, Format output);

    /// Convert the pointed to pixel to a color, in the specific output format
    void ConvertPixel(const uint8_t* in, uint8_t* out, Format input, Format output);

} /* namespace grvl */

#endif /* GRVL_IMAGECONTENT_H_ */
