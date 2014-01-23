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

#include "Misc.h"
#include <math.h>

namespace grvl {
    static constexpr auto fullAngle = 360;
    static constexpr auto halfAngle = 180;
    int32_t Clamp(int32_t val, int32_t left, int32_t right)
    {
        if(val < left) {
            return left;
        }
        if(val > right) {
            return right;
        }
        return val;
    }

    inline static float FloatMod(float a, float b)
    {
        return (a - b * floor(a / b));
    }

    float ConstrainAngle(float angle)
    {
        angle = FloatMod(angle, fullAngle);
        if(angle <= 0) {
            angle += fullAngle;
        }
        return angle;
    }

    float AngleDiff(float angle1, float angle2)
    {
        double dif = FloatMod(angle2 - angle1 + halfAngle, fullAngle);
        if(dif < 0) {
            dif += fullAngle;
        }
        return dif - halfAngle;
    }

} /* namespace grvl */
