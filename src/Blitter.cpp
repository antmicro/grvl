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

namespace grvl {

    uint32_t FormatToDma2d(Format format) {
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

    /*
     * Old API compatibility
     */

    void UseOldDmaBlitClt(
        uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
        uint32_t NumberOfLine, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
        Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor, uintptr_t backCLT, uintptr_t frontCTL)
    {
        uint32_t dma2d_inPixelFormat = FormatToDma2d(inPixelFormat);
        uint32_t dma2d_backgroundPixelFormat = FormatToDma2d(backgroundPixelFormat);
        uint32_t dma2d_outPixelFormat = FormatToDma2d(outPixelFormat);
        grvl::Callbacks()->dma_operation_clt(inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLine, inOffset, backgroundOffset, outOffset, dma2d_inPixelFormat, dma2d_backgroundPixelFormat, dma2d_outPixelFormat, frontColor, backCLT, frontCTL);
    }

    void UseOldDmaBlit(
        uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t PixelsPerLine,
        uint32_t NumberOfLine, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
        Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor)
    {
        uint32_t dma2d_inPixelFormat = FormatToDma2d(inPixelFormat);
        uint32_t dma2d_backgroundPixelFormat = FormatToDma2d(backgroundPixelFormat);
        uint32_t dma2d_outPixelFormat = FormatToDma2d(outPixelFormat);
        grvl::Callbacks()->dma_operation(inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLine, inOffset, backgroundOffset, outOffset, dma2d_inPixelFormat, dma2d_backgroundPixelFormat, dma2d_outPixelFormat, frontColor);
    }

    void UseOldDmaFill(uintptr_t dst, uint32_t xs, uint32_t ys, uint32_t offset, uint32_t color_index, Format pixel_format)
    {
        uint32_t dma2d_pixel_format = FormatToDma2d(pixel_format);
        grvl::Callbacks()->dma_fill(dst, xs, ys, offset, color_index, dma2d_pixel_format);
    }

    /*
     * Fallback Blitter
     */

    using BakedFillFunc = void (*) (uint8_t* dst, uint32_t color, uint32_t columns, uint32_t rows, uint32_t offset);
    using BakedBlitFunc = void (*) (uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff, uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL);

    // Based on the ST documentation
    uint32_t Blend(uint32_t bcol, uint32_t icol)
    {
        uint8_t alphai = (icol >> 24) & 0xFF;
        uint8_t alphab = (bcol >> 24) & 0xFF;

        uint8_t alphamulti = ((alphai * alphab) / 255);
        uint8_t alphar = alphai + alphab - alphamulti;
        if(alphar == 0) {
            return 0;
        }

        uint32_t result;
        uint8_t* resultb = (uint8_t*)&result;
        uint8_t* icolb = (uint8_t*)&icol;
        uint8_t* bcolb = (uint8_t*)&bcol;

        for(int i = 0; i < 3; i++) {
            resultb[i] = ((icolb[i] * alphai) + (bcolb[i] * alphab) - (bcolb[i] * alphamulti)) / (alphar);
        }

        resultb[3] = alphar;

        return result;
    }

    uint32_t LookupClt(uint8_t* mem, Format format, uint8_t* clt)
    {
        uint32_t color = 0;
        uint16_t cltOffset = 0;
        if(format == Format::AL44) {
            color |= ((mem[0] & 0xf0) * 0x11) << 24;
            cltOffset = (mem[0] & 0x0f) * 0x11;
        } else if(format == Format::AL88) {
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

    template <size_t stride, typename T>
    static uint32_t FastConvertPixel(const T* data, Format input, Format output)
    {
        uint32_t color = 0;
        memcpy(&color, data, stride);
        return ConvertColorFormat(color, input, output);
    }

    template <size_t stride>
    static void PixelFormatConvert(uintptr_t imem, uintptr_t bmem, uintptr_t omem, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCLT)
    {
        uint32_t icol = 0;
        uint32_t bcol = 0;
        uint32_t ocol = 0;

        if(GetFormatUsesColorLookup(ifmt)) {
            icol = LookupClt((uint8_t*)imem, ifmt, (uint8_t*)frontCLT);
        } else {
            icol = FastConvertPixel<stride>((const unsigned char*)imem, ifmt, Format::ARGB8888);
            if((ifmt == Format::A8) || (ifmt == Format::AXXX8888)) {
                icol = (icol & 0xff000000) | (font_color & 0x00ffffff);
            }
        }

        if(bmem != 0) {
            if(GetFormatUsesColorLookup(bfmt)) {
                bcol = LookupClt((uint8_t*)bmem, bfmt, (uint8_t*)backCLT);
            } else {
                bcol = FastConvertPixel<stride>((const unsigned char*)bmem, bfmt, Format::ARGB8888);
            }
            ocol = Blend(bcol, icol);
        } else {
            ocol = icol;
        }

        ocol = FastConvertPixel<stride>(&ocol, Format::ARGB8888, ofmt);
        memcpy((void*)omem, &ocol, stride);
    }

    template <size_t ostride>
    static void FastBlitPixel(uintptr_t omem, uintptr_t imem, uintptr_t bmem, uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL)
    {
        const uint32_t istride = GetFormatStride(ifmt);
        const uint32_t bstride = bmem == 0 ? 0 : GetFormatStride(bfmt);

        for (uint32_t y = 0; y < rows; y++) {
            for (uint32_t x = 0; x < columns; x++) {
                PixelFormatConvert<ostride>(imem, bmem, omem, ifmt, bfmt, ofmt, font_color, backCLT, frontCTL);
                omem += ostride;
                imem += istride;
                bmem += bstride;
            }

            omem += ooff * ostride;
            imem += ioff * istride;
            bmem += boff * bstride;
        }
    }

    void FallbackBlitClt(uintptr_t imem, uintptr_t bmem, uintptr_t omem,
        uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color, uintptr_t backCLT, uintptr_t frontCTL)
    {
        const uint32_t ostride = GetFormatStride(ofmt);

        const BakedBlitFunc func[5] = {
            FastBlitPixel<0>, // this is a dummy case
            FastBlitPixel<1>,
            FastBlitPixel<2>,
            FastBlitPixel<3>,
            FastBlitPixel<4>
        };

        // bake stride into the function so that memcpy call can be optimized away
        func[ostride](omem, imem, bmem, columns, rows, ioff, boff, ooff, ifmt, bfmt, ofmt, font_color, backCLT, frontCTL);
    }

    void FallbackBlit(uintptr_t imem, uintptr_t bmem, uintptr_t omem,
        uint32_t columns, uint32_t rows, uint32_t ioff, uint32_t boff,
        uint32_t ooff, Format ifmt, Format bfmt, Format ofmt, uint32_t font_color)
    {
        FallbackBlitClt(imem, bmem, omem, columns, rows, ioff, boff, ooff, ifmt, bfmt, ofmt, font_color, 0, 0);
    }

    /*
     * Fallback Filler
     */

    template <size_t stride>
    static void FastFillPixel(uint8_t* dst, uint32_t color, uint32_t columns, uint32_t rows, uint32_t offset)
    {
        for (uint32_t y = 0; y < rows; y++) {
            for (uint32_t x = 0; x < columns; x++) {
                memcpy(dst, &color, stride);
                dst += stride;
            }

            dst += offset * stride;
        }
    }

    void FallbackFill(uintptr_t dst, uint32_t columns, uint32_t rows, uint32_t offset, uint32_t color_index, Format pixel_format)
    {
        const uint32_t color = ConvertColorFormat(color_index, Format::ARGB8888, pixel_format);
        const uint32_t stride = GetFormatStride(pixel_format);

        const BakedFillFunc func[5] = {
            FastFillPixel<0>, // this is a dummy case
            FastFillPixel<1>,
            FastFillPixel<2>,
            FastFillPixel<3>,
            FastFillPixel<4>
        };

        // bake stride into the function so that memcpy call can be optimized away
        func[stride]((uint8_t*) dst, color, columns, rows, offset);
    }

}
