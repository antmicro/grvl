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

#ifndef GRVL_MISC_H_
#define GRVL_MISC_H_
#include "Definitions.h"
#include <ctype.h>
#include <stdint.h>

namespace grvl {

    static inline void string_to_lower(char* s)
    {
        int i = 0;
        while(s[i] != 0) {
            s[i] = tolower(s[i]);
            i++;
        }
    }

    inline uint32_t PixelFormatToBPP(uint32_t pf)
    {
        switch(pf) {
            case COLOR_FORMAT_RGB888:
                return 3;
            case COLOR_FORMAT_ARGB8888:
            case COLOR_FORMAT_AXXX8888:
                return 4;
            case COLOR_FORMAT_A8:
            case COLOR_FORMAT_L8:
                return 1;
            case COLOR_FORMAT_AL88:
            case COLOR_FORMAT_RGB565:
            case COLOR_FORMAT_ARGB4444:
            default:
                return 2;
        }
    }

    int32_t Clamp(int32_t val, int32_t left, int32_t right);

    float ConstrainAngle(float angle);

    float AngleDiff(float angle1, float angle2);

} /* namespace grvl */

#endif /* GRVL_MISC_H_ */
