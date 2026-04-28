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

#ifndef GRVL_FORMAT_H_
#define GRVL_FORMAT_H_

#include <cstdint>

namespace grvl {

    enum struct Format {
        ARGB8888 = 0,
        RGB888 = 1,
        RGB565 = 2,
        ARGB1555 = 3,
        ARGB4444 = 4,
        L8 = 5,
        AL44 = 6,
        AL88 = 7,
        A8 = 8,
        ARGB6666 = 9,
        AXXX8888 = 10,
    };

    struct FormatInfo {
        int stride;
        int channels;
        bool alpha;
        const char* name;
    };

    constexpr inline FormatInfo format_descriptors[] = {
        {4, 4, true, "ARGB8888"},
        {3, 3, false, "RGB888"},
        {2, 3, false, "RGB565"},
        {2, 4, true, "ARGB1555"},
        {2, 4, true, "ARGB4444"},
        {1, 1, false, "L8"},
        {1, 2, true, "AL44"},
        {2, 2, true, "AL88"},
        {1, 1, true, "A8"},
        {3, 4, true, "ARGB6666"},
        {4, 1, true, "AXXX8888"},
    };

    // Get format information object
    constexpr const FormatInfo& GetFormatInfo(Format format)
    {
        return format_descriptors[static_cast<int>(format)];
    }

    // Get size of one pixel in bytes
    constexpr int GetFormatStride(Format format)
    {
        return GetFormatInfo(format).stride;
    }

    // Get number of separate color channels
    constexpr int GetFormatChannelCount(Format format)
    {
        return GetFormatInfo(format).channels;
    }

    // Check if this format has an Alpha (transparency) channel
    constexpr bool GetFormatAlphaChannel(Format format)
    {
        return GetFormatInfo(format).alpha;
    }

    // Convert the format enum to a human readable name
    constexpr const char* GetFormatName(Format format)
    {
        return GetFormatInfo(format).name;
    }

    // Convert color from one format to another
    constexpr uint32_t ConvertColorFormat(uint32_t color, Format input, Format output)
    {
        if (input == output) {
            return color;
        }

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        // first we decompose the color using the input format
        // then recombine it using output format, this avoids having to write a full-graph of conversions

        switch (input) {

            case Format::ARGB8888:
                r = (color & 0x00ff0000) >> 16;
                g = (color & 0x0000ff00) >> 8;
                b = (color & 0x000000ff) >> 0;
                a = (color & 0xff000000) >> 24;
                break;

            case Format::RGB888:
                r = (color & 0x00ff0000) >> 16;
                g = (color & 0x0000ff00) >> 8;
                b = (color & 0x000000ff) >> 0;
                break;

            case Format::RGB565:
                r = ((color & 0b11111'000000'00000) >> 11) << 3;
                g = ((color & 0b00000'111111'00000) >> 5) << 2;
                b = ((color & 0b00000'000000'11111) >> 0) << 3;
                r |= (r >> 5);
                g |= (g >> 6);
                b |= (b >> 5);
                break;

            case Format::ARGB1555:
                r = ((color & 0b0'11111'00000'00000) >> 10) << 3;
                g = ((color & 0b0'00000'11111'00000) >> 5) << 3;
                b = ((color & 0b0'00000'00000'11111) >> 0) << 3;
                a = (color & 0b1'00000'00000'00000) ? 255 : 0;
                r |= (r >> 5);
                g |= (g >> 5);
                b |= (b >> 5);
                break;

            case Format::ARGB4444:
                r = (color & 0x0f00) >> 4;
                g = (color & 0x00f0) >> 0;
                b = (color & 0x000f) << 4;
                a = (color & 0xf000) >> 8;
                r |= (r >> 4);
                g |= (g >> 4);
                b |= (b >> 4);
                a |= (a >> 4);
                break;

            case Format::L8:
                r = (color & 0xff);
                g = r;
                b = r;
                break;

            case Format::AL44:
                r = (color & 0x0f) << 4;
                a = (color & 0xf0);
                r |= (r >> 4);
                a |= (a >> 4);
                g = r;
                b = r;
                break;

            case Format::AL88:
                r = (color & 0x00ff);
                a = (color & 0xff00) >> 8;
                g = r;
                b = r;
                break;

            case Format::A8:
                a = (color & 0xff);
                break;

            case Format::ARGB6666:
                r = ((color & 0b000000'111111'000000'000000) >> (2*6)) << 2;
                g = ((color & 0b000000'000000'111111'000000) >> (1*6)) << 2;
                b = ((color & 0b000000'000000'000000'111111) >> (0*6)) << 2;
                a = ((color & 0b111111'000000'000000'000000) >> (3*6)) << 2;
                r |= (r >> 6);
                g |= (g >> 6);
                b |= (b >> 6);
                a |= (a >> 6);
                break;

            case Format::AXXX8888:
                a = (color & 0xff000000) >> 24;
                break;

        }

        switch (output) {

            case Format::ARGB8888: return (a << 24) | (r << 16) | (g << 8) | (b);
            case Format::RGB888: return (r << 16) | (g << 8) | (b);
            case Format::RGB565: return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            case Format::ARGB1555: return ((a >> 7) << 15) | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
            case Format::ARGB4444: return ((a >> 4) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4);
            case Format::L8: return r;
            case Format::AL44: return (r >> 4) | (a & 0xf0);
            case Format::AL88: return r | (a << 8);
            case Format::A8: return a;
            case Format::ARGB6666: return ((a >> 2) << 18) | ((r >> 2) << 12) | ((g >> 2) << 6) | (b >> 2);
            case Format::AXXX8888: return a << 24;

        }

        return 0;
    }

}

#endif
