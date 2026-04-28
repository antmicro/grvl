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
#include <grvl/Misc.h>
#include <grvl/Painter.h>
#include <grvl/grvl.h>
#include <grvl/Format.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace grvl {

    class ImageContent {
    public:

        ImageContent(const char* path, Format format = Format::ARGB8888);
        ImageContent(uint8_t* pixels, int width, int height, int frames, Format format = Format::ARGB8888);
        ImageContent(const ImageContent& content);
        ImageContent& operator=(const ImageContent& other);

        ~ImageContent();

        uint8_t* GetData()
        {
            return data;
        }

        const uint8_t* GetData() const
        {
            return data;
        }

        uint32_t GetDataLength() const
        {
            return GetWidth() * GetHeight() * GetNumberOfFrames() * GetBytesPerPixel();
        }

        uint16_t GetNumberOfFrames() const
        {
            return frames;
        }

        uint32_t GetWidth() const
        {
            return width;
        }

        uint32_t GetHeight() const
        {
            return height;
        }

        uint32_t GetBytesPerPixel() const
        {
            return GetFormatStride(format);
        }

        bool HasAlphaChannel() const
        {
            return GetFormatAlphaChannel(format);
        }

        bool IsRotated() const
        {
            return rotated;
        }

        bool IsEmpty() const
        {
            return !data;
        }

        Format GetColorFormat() const
        {
            return format;
        }

        uint32_t GetNumberOfLines() const
        {
            return rotated ? width : height;
        }

        uint32_t GetPixelsPerLine() const
        {
            return rotated ? height * frames : width;
        }

        void Transcode(Format format);
        void Rotate90();

        [[deprecated("For removal! ImageContent::FromPNG() call can be removed!")]]
        constexpr static const char* FromPNG(const char* path) {
            return path;
        }

        [[deprecated("For removal! ImageContent::FromJPEG() call can be removed!")]]
        constexpr static const char* FromJPEG(const char* path) {
            return path;
        }

    private:

        uint8_t* data;
        int32_t width, height, frames;
        Format format;
        bool rotated = false;

    };

    /// Convert the pointed to pixel to a color, in the specific output format
    uint32_t ConvertPixel(const uint8_t* data, Format input, Format output);

    /// Convert the pointed to pixel to a color, in the specific output format
    void ConvertPixel(const uint8_t* in, uint8_t* out, Format input, Format output);

} /* namespace grvl */

#endif /* GRVL_IMAGECONTENT_H_ */
