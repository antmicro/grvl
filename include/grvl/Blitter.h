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

    void UseBlitAsBlitClt(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL);

    uint32_t Blend(uint32_t bcol, uint32_t icol);
    uint32_t LookupClt(uint8_t* mem, Format format, uint8_t* clt);

    void FallbackBlitClt(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL);

    void FallbackBlit(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color);

    void FallbackFill(uintptr_t dst, uint32_t columns, uint32_t rows, uint32_t offset, uint32_t color_index, Format pixel_format);

}

#endif
