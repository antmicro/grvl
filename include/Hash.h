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

#pragma once

#include "HashFunctions.h"

#include <unistd.h>

namespace ustl {
    template <typename T>
    struct hash {
        typedef T argument_type;
        typedef size_t result_type;
        typedef argument_type* hash_type;

        result_type operator()(argument_type value) const
        {
            return Murmur264((uint8_t*)&value, sizeof(hash_type), 9901);
        }
    };

    template <>
    struct hash<uint16_t> {
        typedef uint16_t argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type value) const
        {
            const void* value_address = &value;
            const uint8_t* data = static_cast<const uint8_t*>(value_address);
            uint64_t hash = Murmur264(data, sizeof(argument_type), 9901);
            return hash;
        }
    };
}
