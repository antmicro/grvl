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

#ifndef GRVL_IMAGECONTENT_H_
#define GRVL_IMAGECONTENT_H_

#include "Definitions.h"
#include "File.h"
#include "grvl.h"
#include "Misc.h"
#include "Painter.h"
#include "crc16.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace grvl {

    enum ImageType {
        IMAGE_TYPE_RAW,
        IMAGE_TYPE_PNG,
        IMAGE_TYPE_JPEG
    };
    enum DitherMode {
        DITHER_NONE,
        DITHER_FLOYD_STEINBERG,
        DITHER_INDEXED
    };

    /// Represents content of an image loaded into memory.
    class ImageContent {
    public:
        struct FromPNG {
            FromPNG(uint8_t* data, uint32_t dataLength, uint16_t numberOfFrames = 1, bool alpha = true,
                    DitherMode dither = DITHER_NONE)
                : data(data)
                , dataLength(dataLength)
                , numberOfFrames(numberOfFrames)
                , alpha(alpha)
                , dither(dither)
                , initialized(true)
            {
            }

            FromPNG(const char* fpath, uint16_t numberOfFrames = 1, bool alpha = true, DitherMode dither = DITHER_NONE)
                : data(NULL)
                , dataLength(0)
                , numberOfFrames(numberOfFrames)
                , alpha(alpha)
                , dither(dither)
                , initialized(false)
            {
                File f(fpath);
                int32_t fileSize = f.GetSize();
                if(fileSize == -1 || !f.HasExtension("png")) {
                    grvl::Log("[ERROR] GUIImageContent: Could not load PNG image from file %s", fpath);
                    return;
                }

                dataLength = fileSize;
                data = (uint8_t*)grvl::Callbacks()->malloc(dataLength);
                initialized = f.ReadToBuffer(data);
            }

            uint8_t* data;
            uint32_t dataLength;
            uint16_t numberOfFrames;
            bool alpha;
            DitherMode dither;

            bool initialized;
        };

        struct FromRAW : FromPNG {
            FromRAW(uint8_t* data, uint32_t width, uint32_t height, uint16_t numberOfFrames, uint32_t colorFormat = COLOR_FORMAT_ARGB1555)
                : FromPNG(data, 0, numberOfFrames, false)
                , width(width)
                , height(height)
                , colorFormat(colorFormat)
            {
            }

            FromRAW(const char* fpath, uint32_t width, uint32_t height, uint16_t numberOfFrames, uint32_t colorFormat = COLOR_FORMAT_ARGB1555)
                : FromPNG(NULL, 0, numberOfFrames, false, DITHER_NONE)
                , width(width)
                , height(height)
                , colorFormat(
                      colorFormat)
            {
                initialized = false;
                File f(fpath);
                int32_t fileSize = f.GetSize();
                if(fileSize == -1 || !f.HasExtension("raw")) {
                    grvl::Log("[ERROR] GUIImageContent: Could not load RAW image from file %s", fpath);
                    return;
                }

                dataLength = fileSize;
                data = (uint8_t*)grvl::Callbacks()->malloc(dataLength);
                initialized = f.ReadToBuffer(data);
            }

            uint32_t width;
            uint32_t height;
            uint32_t colorFormat;
        };

        struct FromJPEG : FromPNG {
            FromJPEG(uint8_t* data, uint16_t numberOfFrames)
                : FromPNG(data, 0, numberOfFrames, false)
                , width(0)
                , height(0)
                , colorFormat(COLOR_FORMAT_RGB888)
            {
            }

            FromJPEG(const char* fpath, uint16_t numberOfFrames)
                : FromPNG(NULL, 0, numberOfFrames, false, DITHER_NONE)
                , width(0)
                , height(0)
                , colorFormat(
                      COLOR_FORMAT_RGB888)
            {
                initialized = false;
                File f(fpath);
                int32_t fileSize = f.GetSize();
                if(fileSize == -1 || !f.HasExtension("jpg")) {
                    grvl::Log("[ERROR] GUIImageContent: Could not load JPG image from file %s", fpath);
                    return;
                }

                dataLength = fileSize;
                data = (uint8_t*)grvl::Callbacks()->malloc(dataLength);
                initialized = f.ReadToBuffer(data);
            }

            uint32_t width;
            uint32_t height;
            uint32_t colorFormat;
        };

        ImageContent(const FromPNG& fromPNG)
        {
            if(fromPNG.initialized) {
                Init(fromPNG.data, fromPNG.dataLength, 0, 0, 0, fromPNG.numberOfFrames, fromPNG.alpha, fromPNG.dither,
                     IMAGE_TYPE_PNG);
                if (!File::noFS) {
                    grvl::Callbacks()->free(fromPNG.data);
                }
            } else {
                data = NULL;
            }
        };

        ImageContent(const FromRAW& fromRAW)
        {
            if(fromRAW.initialized) {
                Init(fromRAW.data, fromRAW.dataLength, fromRAW.colorFormat, fromRAW.width, fromRAW.height,
                     fromRAW.numberOfFrames, false, DITHER_NONE, IMAGE_TYPE_RAW);
            } else {
                data = NULL;
            }
        };

        ImageContent(const FromJPEG& fromJPEG)
        {
            if(fromJPEG.initialized) {
                Init(fromJPEG.data, fromJPEG.dataLength, 0, 0, 0,
                     fromJPEG.numberOfFrames, false, DITHER_NONE, IMAGE_TYPE_JPEG);
            } else {
                data = NULL;
            }
        }

        ImageContent(const ImageContent& content);

        ImageContent& operator=(const ImageContent& other);

        virtual ~ImageContent();

        uint8_t* GetData() const
        {
            return data;
        }

        uint32_t GetDataLength() const
        {
            return width * height * numberOfFrames * GetBytesPerPixel();
        }

        uint16_t GetNumberOfFrames() const
        {
            return numberOfFrames;
        }

        uint32_t GetWidth() const
        {
            return width;
        }

        uint32_t GetHeight() const
        {
            return height;
        }

        uint32_t GetColorFormat() const
        {
            return colorFormat;
        }

        uint32_t GetBytesPerPixel() const
        {
            return PixelFormatToBPP(colorFormat);
        }

        uint32_t GetNumberOfLines() const
        {
            return lines;
        }

        uint32_t GetPixelsPerLine() const
        {
            return pixelsPerLine;
        }

        bool HasAlphaChannel() const
        {
            return hasAlpha;
        }

        bool IsRotated() const
        {
            return rotated;
        }

        bool IsEmpty() const
        {
            return !data;
        }

        uintptr_t GetPLTE()
        {
            return PLTE;
        }

        // Returns number of colors.
        uint32_t GetPLTELength()
        {
            return PLTELength;
        }

        void Rotate90();

    private:
        uint8_t* data;
        uint16_t numberOfFrames;
        uint32_t width, height, lines, pixelsPerLine;
        ImageType imageType;
        uint32_t colorFormat;
        bool rotated, hasAlpha;
        uintptr_t PLTE;
        unsigned int PLTELength;

        void Init(uint8_t* data, uint32_t dataLength, uint32_t colorFormat, uint32_t width, uint32_t height,
                  uint16_t numberOfFrames, bool alpha, DitherMode dither, ImageType imageType);
        uint32_t XYToOffset(uint32_t x, uint32_t y, uint32_t byteCount, uint32_t bytesPerPixel, uint32_t wholeImageWidth);
    };

} /* namespace grvl */

#endif /* GRVL_IMAGECONTENT_H_ */
