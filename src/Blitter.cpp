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
        } else if(format == Format::L8) {
            color |= 0xff000000;
            cltOffset = mem[0];
        } else {
            // We make sure this function will only ever be called for CLT formats
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

    void PixelFormatConvert(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, Format inputPixelFormat, Format backgroundPixelFormat, Format outputPixelFormat, uint32_t fontColor, uintptr_t backCLT, uintptr_t frontCLT)
    {
        uint32_t inputColor = 0;
        uint32_t backgroundColor = 0;
        uint32_t outputColor = 0;

        if(PixelFormatIsCLT(inputPixelFormat)) {
            inputColor = LookupClt((uint8_t*)inputMem, inputPixelFormat, (uint8_t*)frontCLT);
        } else {
            inputColor = ConvertPixel((const unsigned char*)inputMem, inputPixelFormat, Format::ARGB8888);
            if((inputPixelFormat == Format::A8) || (inputPixelFormat == Format::AXXX8888)) {
                inputColor = (inputColor & 0xff000000) | (fontColor & 0x00ffffff);
            }
        }

        if(backgroundMem != 0) {
            if(PixelFormatIsCLT(backgroundPixelFormat)) {
                inputColor = LookupClt((uint8_t*)backgroundMem, backgroundPixelFormat, (uint8_t*)backCLT);
            } else {
                backgroundColor = ConvertPixel((const unsigned char*)backgroundMem, backgroundPixelFormat, Format::ARGB8888);
            }
            outputColor = Blend(backgroundColor, inputColor);
        } else {
            outputColor = inputColor;
        }

        outputColor = ConvertPixel((const unsigned char*)&outputColor, Format::ARGB8888, outputPixelFormat);
        memcpy((void*)outputMem, (void*)&outputColor, GetFormatStride(outputPixelFormat));
    }

    void FallbackBlitClt(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
        uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset,
        uint32_t outOffset, Format inColor, Format backgroundColor, Format outColor, uint32_t fontColor, uintptr_t backCLT, uintptr_t frontCTL)
    {
        uintptr_t omem = outputMem;
        uintptr_t imem = inputMem;
        uintptr_t bmem = backgroundMem;
        unsigned int y, x;
        uint32_t omemBPP = GetFormatStride(outColor);
        uint32_t imemBPP = GetFormatStride(inColor);
        uint32_t bmemBPP = backgroundMem == 0 ? 0 : GetFormatStride(backgroundColor);

        for(y = 0; y < NumberOfLines; y++) {
            for(x = 0; x < PixelsPerLine; x++) {
                PixelFormatConvert(imem, bmem, omem, inColor, backgroundColor, outColor, fontColor, backCLT, frontCTL);
                omem += omemBPP;
                imem += imemBPP;
                bmem += bmemBPP;
            }
            omem += outOffset * omemBPP;
            imem += inOffset * imemBPP;
            bmem += backgroundOffset * bmemBPP;
        }
    }

    void FallbackBlit(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem,
        uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t inOffset, uint32_t backgroundOffset,
        uint32_t outOffset, Format inPixelFormat, Format backgroundPixelFormat, Format outPixelFormat, uint32_t frontColor)
    {
        FallbackBlitClt(inputMem, backgroundMem, outputMem, PixelsPerLine, NumberOfLines, inOffset, backgroundOffset, outOffset, inPixelFormat, backgroundPixelFormat, outPixelFormat, frontColor, 0, 0);
    }

    void FallbackFill(uintptr_t dst, uint32_t PixelsPerLine, uint32_t NumberOfLines, uint32_t offset, uint32_t color_index, Format pixel_format) {
        unsigned int x, y;
        uint32_t color = ConvertColorFormat(color_index, Format::ARGB8888, pixel_format);
        uint32_t dstBPP = GetFormatStride(pixel_format);

        for(y = 0; y < NumberOfLines; y++) {
            for(x = 0; x < PixelsPerLine; x++) {
                memcpy((void*)dst, (void*)&color, dstBPP);
                dst += dstBPP;
            }
            dst += offset * dstBPP;
        }
    }

}
