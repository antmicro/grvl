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

#include <grvl/Blitter.h>
#include <grvl/Misc.h>
#include <grvl/ImageContent.h>
#include <grvl/grvl.h>

// unless stated otherwise enable default blitter
#ifndef __ZEPHYR__
#ifndef CONFIG_GRVL_ENABLE_DEFAULT_BLITTER
#define CONFIG_GRVL_ENABLE_DEFAULT_BLITTER 1
#endif
#endif

namespace grvl {

    uint32_t FormatToDma2d(Format format)
    {
        if (format == Format::ARGB8888) return 0x00000000;
        if (format == Format::RGB888) return 0x00000001;
        if (format == Format::RGB565) return 0x00000002;
        if (format == Format::ARGB1555) return 0x00000003;
        if (format == Format::ARGB4444) return 0x00000004;
        if (format == Format::L8) return 0x00000005;
        if (format == Format::AL44) return 0x00000006;
        if (format == Format::AL88) return 0x00000007;
        if (format == Format::A8) return 0x00000009;
        if (format == Format::ARGB6666) return 0x0000000B;
        if (format == Format::AXXX8888) return 0x00000100;

        return 0;
    }

    // Based on the ST documentation
    uint32_t Blend(uint32_t bcol, uint32_t icol)
    {
        Color fc = DecomposeColorFormat(icol, Format::ARGB8888);
        Color bc = DecomposeColorFormat(bcol, Format::ARGB8888);

        uint8_t am = ((fc.a * bc.a) / 255);
        uint8_t ar = fc.a + bc.a - am;

        // Checking for zero here is much faster than checking if `fc.a == 0`
        if (ar == 0) {
            return 0;
        }

        fc.r = ((fc.r * fc.a) + (bc.r * bc.a) - (bc.r * am)) / ar;
        fc.g = ((fc.g * fc.a) + (bc.g * bc.a) - (bc.g * am)) / ar;
        fc.b = ((fc.b * fc.a) + (bc.b * bc.a) - (bc.b * am)) / ar;
        fc.a = ar;

        return fc.pack(Format::ARGB8888);
    }

    void UseBlitAsBlitClt(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL)
    {
        // we ignore those and use color index directly as brightness
        // this is fine in most cases (e.g. when we load the image from file)
        (void) backCLT;
        (void) frontCTL;

        grvl::Callbacks()->blit(imem, bmem, omem, columns, rows, ioff, boff, ooff, ifmt, bfmt, ofmt, font_color);
    }

#if CONFIG_GRVL_ENABLE_DEFAULT_BLITTER

    /*
     * Default Blitter
     */

    using BakedFillFunc = void (*) (uint8_t* dst, uint32_t color, uint32_t columns, uint32_t rows, uint32_t offset);
    using BakedBlitFunc = void (*) (uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL);

    template <Format format>
    static uint32_t LookupClt(uint8_t* mem, uint8_t* clt)
    {
        uint32_t color = 0;
        uint16_t cltOffset = 0;

        if constexpr(format == Format::AL44) {
            color |= ((mem[0] & 0xf0) * 0x11) << 24;
            cltOffset = (mem[0] & 0x0f) * 0x11;
        } else if constexpr(format == Format::AL88) {
            // Little endian, so alpha is actually after luminance
            color |= mem[1] << 24;
            cltOffset = mem[0];
        } else {
            // We make sure this function will only ever be called for CLT formats
            color |= 0xff000000;
            cltOffset = mem[0];
        }

        // Do greyscale if no palette is passed
        if(clt == nullptr) {
            color |= cltOffset * 0x010101;
        } else {
            cltOffset *= 3;
            color |= clt[cltOffset + 2] << 16 | clt[cltOffset + 1] << 8 | clt[cltOffset + 0];
        }
        return color;
    }

    template <size_t stride, Format input, Format output, typename T>
    static uint32_t FastConvertPixel(const T* data)
    {
        if constexpr (stride == 0) {
            return 0;
        }

        uint32_t color = 0;
        memcpy(&color, data, stride);
        return ConvertColorFormat(color, input, output);
    }

    template <bool transparency, Format ifmt, Format bfmt, Format ofmt>
    static void PixelFormatConvert(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCLT)
    {
        constexpr size_t istride = GetFormatStride(ifmt);
        constexpr size_t bstride = transparency ? GetFormatStride(bfmt) : 0;
        constexpr size_t ostride = GetFormatStride(ofmt);

        uint32_t icol = 0;
        uint32_t bcol = 0;
        uint32_t ocol = 0;

        // dummy case
        if constexpr (istride == 0 || ostride == 0) {
            return;
        }

        if(GetFormatUsesColorLookup(ifmt)) {
            icol = LookupClt<ifmt>((uint8_t*)imem, (uint8_t*)frontCLT);
        } else {
            icol = FastConvertPixel<istride, ifmt, Format::ARGB8888>((const unsigned char*)imem);
            if((ifmt == Format::A8) || (ifmt == Format::AXXX8888)) {
                icol = (icol & 0xff000000) | (font_color & 0x00ffffff);
            }
        }

        if constexpr (transparency) {
            if(GetFormatUsesColorLookup(bfmt)) {
                bcol = LookupClt<bfmt>((uint8_t*)bmem, (uint8_t*)backCLT);
            } else {
                bcol = FastConvertPixel<bstride, bfmt, Format::ARGB8888>((const unsigned char*)bmem);
            }
            ocol = Blend(bcol, icol);
        } else {
            ocol = icol;
        }

        ocol = FastConvertPixel<sizeof(ocol), Format::ARGB8888, ofmt>(&ocol);
        memcpy((void*)omem, &ocol, ostride);
    }

    template <bool transparency, Format ifmt, Format bfmt, Format ofmt>
    struct FastBlitPixel_tibo {

        void operator() (uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL) const
        {

            constexpr size_t istride = GetFormatStride(ifmt);
            constexpr size_t bstride = transparency ? GetFormatStride(bfmt) : 0;
            constexpr size_t ostride = GetFormatStride(ofmt);

            for (uint32_t y = 0; y < rows; y++) {
                for (uint32_t x = 0; x < columns; x++) {
                    PixelFormatConvert<transparency, ifmt, bfmt, ofmt>(imem, bmem, omem, font_color, backCLT, frontCTL);
                    omem += ostride;
                    imem += istride;
                    bmem += bstride;
                }

                omem += ooff * ostride;
                imem += ioff * istride;
                bmem += boff * bstride;
            }

        };

    };

    template <bool transparency, Format ifmt, Format bfmt>
    struct FastBlitPixel_tib {

        template <Format T>
        using Next = FastBlitPixel_tibo<transparency, ifmt, bfmt, T>;

        void operator() (uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL) const
        {
            static_format_lookup<Next>(
                ofmt,
                omem, imem, bmem, columns, rows, ioff, boff, ooff, font_color, backCLT, frontCTL
            );

        };

    };

    template <bool transparency, Format ifmt>
    struct FastBlitPixel_ti {

        template <Format T>
        using Next = FastBlitPixel_tib<transparency, ifmt, T>;

        void operator() (uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL) const
        {
            static_format_lookup<Next>(
                bfmt,
                omem, imem, bmem, columns, rows, ioff, boff, ooff, ofmt, font_color, backCLT, frontCTL
            );

        };

    };

    template <bool transparency>
    struct FastBlitPixel_t {

        template <Format T>
        using Next = FastBlitPixel_ti<transparency, T>;

        static void call(uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL)
        {
            static_format_lookup<Next>(
                ifmt,
                omem, imem, bmem, columns, rows, ioff, boff, ooff, bfmt, ofmt, font_color, backCLT, frontCTL
            );
        }

    };

    static void DefaultBlitClt(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL)
    {
        const bool blend = GetFormatAlphaChannel(ifmt) && (bmem != 0);

        BakedBlitFunc blit = blend
            ? FastBlitPixel_t<true>::call
            : FastBlitPixel_t<false>::call;

        // we will now jump though a chain of functions lookups in a effort to bake strides into the functions at compile time
        // so that memcpy call can be optimized away instead of making a call to glibc
        blit(omem, imem, bmem, columns, rows, ioff, boff, ooff, ifmt, bfmt, ofmt, font_color, backCLT, frontCTL);
    }

    void DefaultBlit(uintptr_t imem, uintptr_t bmem, uintptr_t omem,
        uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color)
    {
        DefaultBlitClt(imem, bmem, omem, columns, rows, ioff, boff, ooff, ifmt, bfmt, ofmt, font_color, 0, 0);
    }

    /*
     * Default Filler
     */

    template <size_t stride>
    static void FastFillPixel(uint8_t* dst, uint32_t color, uint32_t columns, uint32_t rows, uint32_t offset)
    {
        uint8_t bytes[stride * columns];
        uint8_t* ptr = bytes;

        for (uint32_t x = 0; x < columns; x++) {
            memcpy(ptr, &color, stride);
            ptr += stride;
        }

        for (uint32_t y = 0; y < rows; y++) {
            memcpy(dst, bytes, stride * columns);
            dst += (columns + offset) * stride;
        }
    }


    static void DefaultFill(uintptr_t dst, uint32_t columns, uint32_t rows, uint32_t offset, uint32_t color_index, Format pixel_format)
    {
        const uint32_t color = ConvertColorFormat(color_index, Format::ARGB8888, pixel_format);
        const uint32_t stride = GetFormatStride(pixel_format);

        const BakedFillFunc func[4] = {
            FastFillPixel<1>,
            FastFillPixel<2>,
            FastFillPixel<3>,
            FastFillPixel<4>
        };

        func[stride - 1]((uint8_t*) dst, color, columns, rows, offset);
    }

#endif

    /*
     * Fallback
     */

    static void FallbackFill(uintptr_t dst, uint32_t columns, uint32_t rows, uint32_t offset, uint32_t color_index, Format fmt) {}
    static void FallbackBlit(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,  uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t fcol) {}
    static void FallbackBlitClt(uintptr_t imem, uintptr_t bmem, uintptr_t omem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t bctl, uintptr_t fctl) {}

    /*
     * Getters
     */

    DmaFillFunction GetFillFunction()
    {
#if CONFIG_GRVL_ENABLE_DEFAULT_BLITTER
        return DefaultFill;
#else
        return FallbackFill;
#endif
    }

    DmaBlitFunction GetBlitFunction()
    {
#if CONFIG_GRVL_ENABLE_DEFAULT_BLITTER
        return DefaultBlit;
#else
        return FallbackBlit;
#endif
    }

    DmaBlitCltFunction GetBlitCltFunction()
    {
#if CONFIG_GRVL_ENABLE_DEFAULT_BLITTER
        return DefaultBlitClt;
#else
        return FallbackBlitClt;
#endif
    }

}
