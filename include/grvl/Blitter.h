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

#ifndef GRVL_BLITTER_H_
#define GRVL_BLITTER_H_

#include <grvl/Format.h>

namespace grvl {

    uint32_t FormatToDma2d(Format format);

    [[deprecated]]
    void UseOldDmaBlitClt(
        uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
        uint32_t NumberOfLine, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
        Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor, uintptr_t backCLT, uintptr_t frontCTL);

    [[deprecated]]
    void UseOldDmaBlit(
        uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
        uint32_t NumberOfLine, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
        Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor);

    [[deprecated]]
    void UseOldDmaFill(uintptr_t dst, uint32_t xs, uint32_t ys, uint32_t offset, uint32_t color_index, Format pixel_format);

    uint32_t Blend(uint32_t bcol, uint32_t icol);
    uint32_t LookupClt(uint8_t* mem, Format format, uint8_t* clt);

    void FallbackBlitClt(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
        uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset,
        uint32_t outOffset, Format inColor, Format backgroundColor, Format outColor, uint32_t fontColor, uintptr_t backCLT, uintptr_t frontCTL);

    void FallbackBlit(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
        uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset,
        uint32_t outOffset, Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor);

    void FallbackFill(uintptr_t dst, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t offset, uint32_t color_index, Format pixel_format);

}

#endif
