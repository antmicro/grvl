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

// NOLINTBEGIN

#include <grvl/ImageContent.h>
#include <grvl/grvl.h>
#include <grvl/Misc.h>
#include <grvl/Painter.h>
#include <grvl/Endian.h>

// save on binary size
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace grvl {

    static uint32_t XYToOffset(uint32_t x, uint32_t y, uint32_t byteCount, uint32_t bytesPerPixel, uint32_t wholeImageWidth)
    {
        return (y * wholeImageWidth + x) * bytesPerPixel + byteCount;
    }

    ImageContent::ImageContent(const char* path, Format format)
    {
        Format image_format = Format::ARGB8888;

        if (format == Format::L8 || format == Format::AL88) {
            image_format = format;
        }

        int32_t file_channels;
        const int channels = GetFormatChannelCount(image_format);
        this->data = stbi_load(path, &width, &height, &file_channels, channels);

        if (!this->data) {
            grvl::Log("[ERROR] Failed to load image %s: %s", path, stbi_failure_reason());
            this->width = 0;
            this->height = 0;
            this->frames = 0;
            this->format = format;
            return;
        }

        this->frames = 1;
        this->format = image_format;

        // grvl and STB use a different channel order, we swap them here
        if (image_format == Format::ARGB8888) {
            uint32_t* pixels = reinterpret_cast<uint32_t*>(this->data);
            const uint32_t length = width * height * frames;

            for (uint32_t i = 0; i < length; i ++) {
                uint32_t& pixel = pixels[i];

#if GRVL_BIG_ENDIAN
                // Move A to the front (RGBA -> ARGB)
                uint8_t a = pixel & 0xff;
                pixel = (pixel >> 8) | (a << 24);
#else
                // Swap R and B (ABGR -> ARGB)
                uint8_t r = pixel & 0xff;
                uint8_t b = (pixel >> 16) & 0xff;
                pixel = (pixel & 0xff00ff00) | r << 16 | b;
#endif
            }
        }

        // if the format could not have been loaded directly we perform transcoding
        Transcode(format);

        grvl::Log("[INFO] Loaded %dx%d image %s as %s", width, height, path, GetFormatName(format));
    }

    ImageContent::ImageContent(uint8_t* pixels, int width, int height, int frames, Format format)
    {
        this->data = pixels;
        this->width = width;
        this->height = height;
        this->frames = frames;
        this->format = format;
    }

    ImageContent::ImageContent(int32_t width, int32_t height, int32_t frames, Format format)
    {
        this->width = width;
        this->height = height;
        this->frames = frames;
        this->format = format;
        this->data = static_cast<uint8_t*>(malloc(this->GetDataLength()));
    }

    ImageContent::ImageContent(const ImageContent& other)
    {
        const auto size = other.GetDataLength();
        data = static_cast<uint8_t*>(malloc(size));
        if (data && other.data) {
            memcpy(data, other.data, size);
        }

        width = other.width;
        height = other.height;
        frames = other.frames;
        format = other.format;
        rotated = other.rotated;
    }

    ImageContent& ImageContent::operator=(const ImageContent& other)
    {
        if (this == &other) {
            return *this;
        }

        const auto size = other.GetDataLength();
        if (data) free(data);
        data = static_cast<uint8_t*>(malloc(size));
        if (data && other.data) {
            memcpy(data, other.data, size);
        }

        width = other.width;
        height = other.height;
        frames = other.frames;
        format = other.format;
        rotated = other.rotated;

        return *this;
    }

    ImageContent::~ImageContent()
    {
        if (data) {
            free(data);
            data = nullptr;
        }
    }

    void ImageContent::Transcode(Format target)
    {
        if (format == target) {
            return;
        }

        const int input_stride = GetBytesPerPixel();
        const int output_stride = GetFormatStride(target);

        const uint32_t input_size = GetDataLength();
        const uint32_t output_size = output_stride * width * height * frames;

        const uint8_t* input_buffer = data;
        uint8_t* output_buffer = static_cast<uint8_t*>(malloc(output_size));

        for (uint32_t j = 0, i = 0; i < input_size; i += input_stride) {
            ConvertPixel(input_buffer + i, output_buffer + j, format, target);
            j += output_stride;
        }

        // update object
        free(this->data);
        this->data = output_buffer;
        this->format = target;
    }

    void ImageContent::Rotate90()
    {
        if(this->rotated) {
            return;
        }

        uint8_t* bufferCopy = static_cast<uint8_t*>(malloc(GetDataLength()));
        uint32_t bytesPerPixel = GetBytesPerPixel();
        uint32_t wholeImageWidth = width * frames;

        memcpy(bufferCopy, data, GetDataLength());

        for(uint32_t f = 0; f < frames; f++) {
            for(uint32_t y = 0; y < height; y++) {
                for(uint32_t x = 0; x < width; x++) {
                    for(uint32_t i = 0; i < bytesPerPixel; i++) {
                        uint8_t value = bufferCopy[XYToOffset(f * width + x, y, i, bytesPerPixel, wholeImageWidth)];
                        data[XYToOffset(f * height + y, width - x - 1, i, bytesPerPixel, height * frames)] = value;
                    }
                }
            }
        }

        free(bufferCopy);
        this->rotated = true;
    }

    uint32_t ConvertPixel(const uint8_t* data, Format input, Format output)
    {
        uint32_t color = 0;
        memcpy(&color, data, GetFormatStride(input));
        return ConvertColorFormat(color, input, output);
    }

    void ConvertPixel(const uint8_t* in, uint8_t* out, Format input, Format output)
    {
        uint32_t color = ConvertPixel(in, input, output);
        memcpy(out, &color, GetFormatStride(output));
    }

} /* namespace grvl */
// NOLINTEND
