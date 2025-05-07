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

#include "ImageContent.h"
#include "grvl.h"
#include "JPEGHelper.h"
#include "Misc.h"
#include "Painter.h"
#include "PngHelper.h"

// #define USE_PICOPNG
#ifdef USE_PICOPNG
#include "picopng.h"
#endif

namespace grvl {

    static void DitherRgb888ToRgb565InPlaceFloydSteinberg(unsigned char* src, int w, int h);
    static void DitherRgb888ToRgb565InPlaceIndexed(unsigned char* src, int w, int h);

    void ImageContent::Init(uint8_t* data, uint32_t dataLength, uint32_t colorFormat, uint32_t width, uint32_t height,
                            uint16_t numberOfFrames, bool alpha, DitherMode dither, ImageType imageType)
    {
        this->PLTE = 0;
        this->PLTELength = 0;
        // Check if Color Lookup Table is supported by hardware
        bool cltSupport = grvl::Callbacks()->dma_operation_clt != nullptr;
        if(imageType == IMAGE_TYPE_JPEG) {
            GetJPEGInfo(data, (int)dataLength, &width, &height);
            colorFormat = COLOR_FORMAT_RGB888;
            int rawImageSize = width * height * PixelFormatToBPP(colorFormat);
            uint8_t* rawData = (uint8_t*)grvl::Callbacks()->malloc(rawImageSize);
            GetJPEGData(rawData, data, (int)dataLength);
            data = rawData;
        } else if(imageType == IMAGE_TYPE_PNG) {
            grvl::Log("[!] Loading PNG");

            png_info_t pngInfo;
            GetPngInfo(&pngInfo, data, dataLength, cltSupport);
            static constexpr auto CT_ARGB = 6;
            static constexpr auto CT_AL = 4;
            static constexpr auto CT_RGB = 2;
            static constexpr auto CT_INDEXED = 3;

            switch(pngInfo.color_type) {
                case CT_ARGB:
                    colorFormat = COLOR_FORMAT_ARGB8888;
                    alpha = true;
                    break;
                case CT_AL:
                    colorFormat = COLOR_FORMAT_AL88; // Grayscale + alpha
                    alpha = true;
                    break;
                case CT_RGB:
                    colorFormat = COLOR_FORMAT_RGB888;
                    alpha = false;
                    break;
                case CT_INDEXED: {
                    alpha = pngInfo.has_alpha;
                    if(cltSupport) { // if PLTE supported
                        colorFormat = COLOR_FORMAT_L8; // PLTE
                        this->PLTELength = pngInfo.plte_length;
                        this->PLTE = (uintptr_t)grvl::Callbacks()->malloc(this->PLTELength * 3);

                        // Transfer colors - swapping the red and the blue channel.
                        uint8_t* pltein = (uint8_t*)pngInfo.plte;
                        uint8_t* plteout = (uint8_t*)this->PLTE;
                        for(unsigned char i = 0; i < this->PLTELength; i++) {
                            plteout[i * 3 + 0] = pltein[i * 3 + 2];
                            plteout[i * 3 + 1] = pltein[i * 3 + 1];
                            plteout[i * 3 + 2] = pltein[i * 3 + 0];
                        }
                    } else {
                        colorFormat = alpha ? COLOR_FORMAT_ARGB8888 : COLOR_FORMAT_RGB888;
                    }
                    break;
                }
                default:
                    grvl::Log("[WARNING] ImageContent: color type %d not supported.", pngInfo.color_type);
            }

            grvl::Log("[WARNING] ImageContent: Loading image %d x %d of type %d (%s).", pngInfo.width, pngInfo.height, pngInfo.color_type, (pngInfo.color_type == 6) ? "ARGB8888" : ((pngInfo.color_type == 4) ? "AL88" : ((pngInfo.color_type == 2) ? "RGB888" : ((pngInfo.color_type == 3) ? "L8/PLTE" : "Unknown"))));

            height = pngInfo.height;
            width = pngInfo.width;

#ifdef USE_PICOPNG
            colorFormat = COLOR_FORMAT_ARGB8888; // TODO: force ARGB8888 for now.
#endif

            int rawImageSize = width * height * PixelFormatToBPP(colorFormat);
            uint8_t* rawData = (uint8_t*)grvl::Callbacks()->malloc(rawImageSize);

#ifdef USE_PICOPNG
            decodePNG(rawData, (unsigned long)width, (unsigned long)height, data, (unsigned long)dataLength, true);
#else

            // TODO: temporarily check CRC 2 times.
            int ok = 0;
            int not_ok = 0;
            unsigned int last_crc = 0xFFFFFFFF;
            int i = 0;
            while(ok < 1) {
                i++;
                // grvl::Log("before GetPngData");
                GetPngData(rawData, data, dataLength, alpha, cltSupport); // libpng
                // grvl::Log("after GetPngData");

                uint16_t crc = gen_crc16(rawData, rawImageSize);
                if((last_crc == crc) || (last_crc == 0xFFFFFFFF)) {
                    last_crc = crc;
                    ok++;
                    not_ok = 0;
                    continue;
                }
                ok = 0;
                not_ok++;
                if(not_ok > 3) {
                    last_crc = crc;
                    not_ok = 0;
                }
                grvl::Log("[WARNING] ImageContent: iteration: %d CRC out 0x%x (size: %d) in 0x%x (size: %d) last_crc: 0x%04X", i, crc, rawImageSize, gen_crc16(data, dataLength), dataLength, last_crc);
            }
            grvl::Log("[INFO] ImageContent: Both CRC checks passed, image %dx%d, crc was 0x%04X", width, height, last_crc);

#endif

            if(dither != DITHER_NONE) {
                if(alpha) {
                    grvl::Log("[WARNING] ImageContent: dither + alpha not supported yet!");
                } else {
                    switch(dither) {
                        case DITHER_FLOYD_STEINBERG:
                            DitherRgb888ToRgb565InPlaceFloydSteinberg(rawData, width, height);
                            break;
                        case DITHER_INDEXED:
                            DitherRgb888ToRgb565InPlaceIndexed(rawData, width, height);
                            break;
                        default:
                            grvl::Log("[WARNING] ImageContent: unsupported dithering algorithm");
                            break;
                    }
                }
            }

            data = rawData;
        }

        this->data = data;
        this->lines = height;
        this->height = height;
        this->pixelsPerLine = width;
        this->width = width / numberOfFrames;
        this->numberOfFrames = numberOfFrames;
        this->imageType = imageType;
        this->colorFormat = colorFormat;
        this->rotated = false;
        this->hasAlpha = alpha;

        if(width % numberOfFrames) {
            grvl::Log("[ERROR] ImageContent: width (%d) should be divisible by number of frames (%d)!", width, numberOfFrames);
        }
    }

    ImageContent::ImageContent(const ImageContent& content)
        : numberOfFrames(content.numberOfFrames)
        , width(content.width)
        , height(content.height)
        , lines(content.lines)
        , pixelsPerLine(content.pixelsPerLine)
        , imageType(content.imageType)
        , colorFormat(content.colorFormat)
        , rotated(false)
        , hasAlpha(false)
        , PLTE(0)
        , PLTELength(0)
    {
        if((imageType == IMAGE_TYPE_PNG) || (imageType == IMAGE_TYPE_JPEG)) {
            // TODO: why it is like this? should it not be the other way round,
            //       that we only copy if that was a RAW? otherwise this is
            //       already a copy we're copying...
            //       also in general - what does this constructor do? a copy of ImageContent? why?
            uint32_t dataLength = content.GetDataLength();
            data = (uint8_t*)grvl::Callbacks()->malloc(dataLength);
            memcpy(data, content.data, dataLength);
        } else {
            data = content.data;
        }
    }

    ImageContent& ImageContent::operator=(const ImageContent& other)
    {
        ImageContent copy(other);

        grvl::Callbacks()->free(data);
        data = copy.data;
        numberOfFrames = copy.numberOfFrames;
        width = copy.width;
        height = copy.height;
        imageType = copy.imageType;
        colorFormat = copy.colorFormat;
        lines = copy.lines;
        pixelsPerLine = copy.pixelsPerLine;
        hasAlpha = copy.hasAlpha;
        return *this;
    }

    uint32_t ImageContent::XYToOffset(uint32_t x, uint32_t y, uint32_t byteCount, uint32_t bytesPerPixel,
                                      uint32_t wholeImageWidth)
    {
        return (y * wholeImageWidth + x) * bytesPerPixel + byteCount;
    }

    void ImageContent::Rotate90()
    {
        if(this->rotated) {
            return;
        }

        uint32_t x, y, i, f;
        uint8_t value;

        uint8_t* bufferCopy = (uint8_t*)grvl::Callbacks()->malloc(GetDataLength());
        uint32_t bytesPerPixel = GetBytesPerPixel();
        uint32_t wholeImageWidth = width * numberOfFrames;

        memcpy(bufferCopy, data, GetDataLength());

        for(f = 0; f < numberOfFrames; f++) {
            for(x = 0; x < width; x++) {
                for(y = 0; y < height; y++) {
                    for(i = 0; i < bytesPerPixel; i++) {
                        value = bufferCopy[XYToOffset(f * width + x, y, i, bytesPerPixel, wholeImageWidth)];
                        data[XYToOffset(f * height + y, width - x - 1, i, bytesPerPixel, height * numberOfFrames)] = value;
                    }
                }
            }
        }

        grvl::Callbacks()->free(bufferCopy);

        this->lines = this->width;
        this->pixelsPerLine = this->height * this->numberOfFrames;
        this->rotated = true;
    }

    ImageContent::~ImageContent()
    {
        if((imageType == IMAGE_TYPE_PNG) || (imageType == IMAGE_TYPE_JPEG)) {
            grvl::Callbacks()->free(data);
        }
        if(PLTE) {
            grvl::Callbacks()->free((void*)PLTE);
        }
    }

    static const uint8_t rgb565_matrix[64] = {
        0, 4, 1, 5, 0, 4, 1, 5,
        6, 2, 7, 3, 6, 2, 7, 3,
        1, 5, 0, 4, 1, 5, 0, 4,
        7, 3, 6, 2, 7, 3, 6, 2,
        0, 4, 1, 5, 0, 4, 1, 5,
        6, 2, 7, 3, 6, 2, 7, 3,
        1, 5, 0, 4, 1, 5, 0, 4,
        7, 3, 6, 2, 7, 3, 6, 2
    };

/* Dithering by individual subpixel */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

    // indexed dithering 888->565
    static void DitherRgb888ToRgb565InPlaceIndexed(unsigned char* src, int w, int h)
    {
        int i = 0;
        for(int y = 0; y < h; y++) {
            for(int x = 0; x < w; x++) {
                uint8_t tresshold_id = ((y & 7) << 3) + (x & 7);
                src[i] = MIN(src[i] + rgb565_matrix[tresshold_id], 0xff) & 0xF8;
                src[i + 1] = MIN(src[i + 1] + rgb565_matrix[tresshold_id], 0xff) & 0xFC;
                src[i + 2] = MIN(src[i + 2] + rgb565_matrix[tresshold_id], 0xff) & 0xF8;
                i += 3;
            }
        }
    }

    // Floyd-Steinberg dithering 888->565
    static void DitherRgb888ToRgb565InPlaceFloydSteinberg(unsigned char* src, int w, int h)
    {
#define compute_new_color(a, b, c) (((a) + ((b) * (c))) > 255) ? 255 : (unsigned char)((a) + ((b) * (c)))
#define GET_INDEX(x, y) (((y)*w + (x)) * 3)
        int ww, hh;
        int idx, gidx;
        for(hh = 0; hh < h; hh++) {
            for(ww = 0; ww < w; ww++) {
                gidx = GET_INDEX(ww, hh);
                if(ww < (w - 1)) {
                    idx = GET_INDEX(ww + 1, hh);
                    src[idx] = compute_new_color(src[idx], 0.4375, src[gidx] & 7);
                    src[idx + 1] = compute_new_color(src[idx + 1], 0.4375, src[gidx + 1] & 3);
                    src[idx + 2] = compute_new_color(src[idx + 2], 0.4375, src[gidx + 2] & 7);
                }
                if(hh < (h - 1)) {
                    idx = GET_INDEX(ww, hh + 1);
                    src[idx] = compute_new_color(src[idx], 0.1875, src[gidx] & 7);
                    src[idx + 1] = compute_new_color(src[idx + 1], 0.1875, src[gidx + 1] & 3);
                    src[idx + 2] = compute_new_color(src[idx + 2], 0.1875, src[gidx + 2] & 7);
                    if(ww > 0) {
                        idx = GET_INDEX(ww - 1, hh + 1);
                        src[idx] = compute_new_color(src[idx], 0.3125, src[gidx] & 7);
                        src[idx + 1] = compute_new_color(src[idx + 1], 0.3125, src[gidx + 1] & 3);
                        src[idx + 2] = compute_new_color(src[idx + 2], 0.3125, src[gidx + 2] & 7);
                    }
                    if(ww < (w - 1)) {
                        idx = GET_INDEX(ww + 1, hh + 1);
                        src[idx] = compute_new_color(src[idx], 0.0625, src[gidx] & 7);
                        src[idx + 1] = compute_new_color(src[idx + 1], 0.0625, src[gidx + 1] & 3);
                        src[idx + 2] = compute_new_color(src[idx + 2], 0.0625, src[gidx + 2] & 7);
                    }
                }
                src[gidx] &= 0xF8;
                src[gidx + 1] &= 0xFC;
                src[gidx + 2] &= 0xF8;
            }
        }
    }

} /* namespace grvl */
// NOLINTEND
