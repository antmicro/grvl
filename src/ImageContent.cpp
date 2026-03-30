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

// NOLINTBEGIN

#include <grvl/ImageContent.h>
#include <grvl/grvl.h>
#include <grvl/Misc.h>
#include <grvl/Painter.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace grvl {

    static uint32_t XYToOffset(uint32_t x, uint32_t y, uint32_t byteCount, uint32_t bytesPerPixel, uint32_t wholeImageWidth)
    {
        return (y * wholeImageWidth + x) * bytesPerPixel + byteCount;
    }

    ImageContent::ImageContent(const char* path, Format format)
    {

        int channels = GetFormatChannelCount(format);
        int stride = GetFormatStride(format);

        // we only support loading images where one byte is used per channel
        if ((channels != stride) || (stride == 0)) {
            data = nullptr;
            return;
        }

        int32_t file_channels = 4;

        this->data = stbi_load(path, &width, &height, &file_channels, channels);
        this->frames = 1;
        this->format = format;

    }

    ImageContent::ImageContent(uint8_t* pixels, int width, int height, int frames, Format format)
    {
        this->data = pixels;
        this->width = width;
        this->height = height;
        this->frames = frames;
        this->format = format;
    }

    ImageContent::ImageContent(const ImageContent& other)
    {
        uint8_t* copied = (uint8_t*) malloc(other.width * other.height * 4);
        width = other.width;
        height = other.height;
        frames = other.frames;
        format = other.format;
        rotated = other.rotated;
        data = copied;
    }

    ImageContent& ImageContent::operator=(const ImageContent& other)
    {
        uint8_t* copied = (uint8_t*) malloc(other.width * other.height * 4);
        this->~ImageContent();

        width = other.width;
        height = other.height;
        frames = other.frames;
        format = other.format;
        rotated = other.rotated;
        data = copied;

        return *this;
    }

    ImageContent::~ImageContent()
    {
        if (data) {
            free(data);
            data = nullptr;
        }
    }

    void ImageContent::Rotate90()
    {
        if(this->rotated) {
            return;
        }

        uint8_t* bufferCopy = (uint8_t*) malloc(GetDataLength());
        uint32_t bytesPerPixel = GetBytesPerPixel();
        uint32_t wholeImageWidth = width * frames;

        memcpy(bufferCopy, data, GetDataLength());

        for(uint32_t f = 0; f < frames; f++) {
            for(uint32_t x = 0; x < width; x++) {
                for(uint32_t y = 0; y < height; y++) {
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

} /* namespace grvl */
// NOLINTEND
