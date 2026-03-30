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

#pragma once

enum struct Format {
    ARGB8888 = 0x00000000,
    RGB888 = 0x00000001,
    RGB565 = 0x00000002,
    ARGB1555 = 0x00000003,
    ARGB4444 = 0x00000004,
    L8 = 0x00000005,
    AL44 = 0x00000006,
    AL88 = 0x00000007,
    A8 = 0x00000009,
    ARGB6666 = 0x0000000B,
    AXXX8888 = 0x00000100,
};

// Get size of one pixel in bytes
constexpr int GetFormatStride(Format format) {
    if (format == Format::ARGB8888) return 4;
    if (format == Format::RGB888) return 3;
    if (format == Format::RGB565) return 2;
    if (format == Format::ARGB1555) return 2;
    if (format == Format::ARGB4444) return 2;
    if (format == Format::L8) return 1;
    if (format == Format::AL44) return 1;
    if (format == Format::AL88) return 2;
    if (format == Format::A8) return 1;
    if (format == Format::ARGB6666) return 3;
    if (format == Format::AXXX8888) return 4;

    return 0;
}

// Get number of separate color channels
constexpr int GetFormatChannelCount(Format format) {
    if (format == Format::ARGB8888) return 4;
    if (format == Format::RGB888) return 3;
    if (format == Format::RGB565) return 3;
    if (format == Format::ARGB1555) return 4;
    if (format == Format::ARGB4444) return 4;
    if (format == Format::L8) return 1;
    if (format == Format::AL44) return 2;
    if (format == Format::AL88) return 2;
    if (format == Format::A8) return 1;
    if (format == Format::ARGB6666) return 4;
    if (format == Format::AXXX8888) return 4;

    return 0;
}

// Check if this format has an Alpha (transparency) channel
constexpr bool GetFormatAlphaChannel(Format format) {
    if (format == Format::ARGB8888) return true;
    if (format == Format::RGB888) return false;
    if (format == Format::RGB565) return false;
    if (format == Format::ARGB1555) return true;
    if (format == Format::ARGB4444) return true;
    if (format == Format::L8) return false;
    if (format == Format::AL44) return true;
    if (format == Format::AL88) return true;
    if (format == Format::A8) return true;
    if (format == Format::ARGB6666) return true;
    if (format == Format::AXXX8888) return true;

    return false;
}

// Colors
#define COLOR_ARGB8888_LIGHTBLUE 0xFF8080FF
#define COLOR_ARGB8888_BLUE 0xFF0000FF
#define COLOR_ARGB8888_DARKBLUE 0xFF000080

#define COLOR_ARGB8888_LIGHTGREEN 0xFF80FF80
#define COLOR_ARGB8888_GREEN 0xFF00FF00
#define COLOR_ARGB8888_DARKGREEN 0xFF008000

#define COLOR_ARGB8888_LIGHTRED 0xFFFF8080
#define COLOR_ARGB8888_RED 0xFFFF0000
#define COLOR_ARGB8888_DARKRED 0xFF800000

#define COLOR_ARGB8888_LIGHTCYAN 0xFF80FFFF
#define COLOR_ARGB8888_CYAN 0xFF00FFFF
#define COLOR_ARGB8888_DARKCYAN 0xFF008080

#define COLOR_ARGB8888_LIGHTMAGENTA 0xFFFF80FF
#define COLOR_ARGB8888_MAGENTA 0xFFFF00FF
#define COLOR_ARGB8888_DARKMAGENTA 0xFF800080

#define COLOR_ARGB8888_LIGHTYELLOW 0xFFFFFF80
#define COLOR_ARGB8888_YELLOW 0xFFFFFF00
#define COLOR_ARGB8888_DARKYELLOW 0xFF808000

#define COLOR_ARGB8888_BROWN 0xFFA52A2A
#define COLOR_ARGB8888_ORANGE 0xFFFFA500

#define COLOR_ARGB8888_WHITE 0xFFFFFFFF
#define COLOR_ARGB8888_LIGHTGRAY 0xFFD3D3D3
#define COLOR_ARGB8888_GRAY 0xFF808080
#define COLOR_ARGB8888_DARKGRAY 0xFF404040
#define COLOR_ARGB8888_BLACK 0xFF000000

#define COLOR_ARGB8888_TRANSPARENT 0x00000000
