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

#ifndef GRVL_BORDER_H_
#define GRVL_BORDER_H_

#include <cstdint>
#include <unordered_map>
#include <string>

namespace grvl {

    enum class BorderTypeBits : uint32_t {
        NONE = 0,
        BOX = 1,
        TOP = 2,
        RIGHT = 4,
        BOTTOM = 8,
        LEFT = 16
    };

    class Border {
    public:
        static BorderTypeBits ParseBorderTypeFromString(const std::string& BorderType);

    private:
        inline static std::unordered_map<std::string, BorderTypeBits> BORDER_TYPE_BITS_MAPPINGS
        {
            {"none", BorderTypeBits::NONE},
            {"box", BorderTypeBits::BOX},
            {"top", BorderTypeBits::TOP},
            {"right", BorderTypeBits::RIGHT},
            {"bottom", BorderTypeBits::BOTTOM},
            {"left", BorderTypeBits::LEFT}
        };
    };

} /* namespace grvl */

#endif /* GRVL_BORDER_H_ */