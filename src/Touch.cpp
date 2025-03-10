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

#include "Touch.h"

namespace grvl {

    int32_t Touch::GetDeltaX() const
    {
        return deltaX;
    }

    int32_t Touch::GetDeltaY() const
    {
        return deltaY;
    }

    void Touch::SetStartPosition(int32_t x, int32_t y)
    {
        startX = x;
        startY = y;
    }

    void Touch::SetState(Touch::TouchState state)
    {
        this->state = state;
    }

    Touch::TouchState Touch::GetState() const
    {
        return state;
    }

    void Touch::SetCurrentPosition(int32_t x, int32_t y)
    {
        deltaX = x - startX;
        deltaY = y - startY;
    }

    int32_t Touch::GetStartX() const
    {
        return startX;
    }

    int32_t Touch::GetStartY() const
    {
        return startY;
    }

    int32_t Touch::GetCurrentX() const
    {
        return deltaX + startX;
    }

    int32_t Touch::GetCurrentY() const
    {
        return deltaY + startY;
    }

} /* namespace grvl */
