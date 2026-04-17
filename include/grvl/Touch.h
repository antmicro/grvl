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

#ifndef GRVL_TOUCH_H_
#define GRVL_TOUCH_H_

#include <stdint.h>

namespace grvl {

    /// Represents event of touching screen.
    class Touch {
    public:
        enum TouchResponse {
            TouchNotApplicable = 0,
            TouchHandled,
            LongTouchHandled,
            TouchReleased
        };

        enum TouchState {
            Idle = 0,
            Pressed,
            Released,
            Moving
        };

        Touch() = default;

        Touch(TouchState initState, int32_t startX, int32_t startY)
            : state(initState)
            , startX(startX)
            , startY(startY)
        {
        }

        virtual ~Touch() = default;

        void SetStartPosition(int32_t x, int32_t y);
        void SetCurrentPosition(int32_t x, int32_t y);
        void SetState(Touch::TouchState state);

        int32_t GetDeltaX() const;
        int32_t GetDeltaY() const;
        int32_t GetCurrentX() const;
        int32_t GetCurrentY() const;
        int32_t GetStartX() const;
        int32_t GetStartY() const;
        TouchState GetState() const;

    private:
        TouchState state{TouchState::Idle};
        int32_t startX{0};
        int32_t startY{0};
        int32_t deltaX{0};
        int32_t deltaY{0};
    };

} /* namespace grvl */

#endif /* GRVL_TOUCH_H_ */
