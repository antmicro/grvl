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

// Hashing functions used in unordered_map implementations were created by
// Austin Appleby (https://sites.google.com/site/murmurhash/) under MIT license.

#pragma once

namespace ustl {

    inline uint64_t Murmur264(const uint8_t* key, uint64_t len, uint64_t seed)
    {
        const uint64_t m = 0xc6a4a7935bd1e995;
        const uint8_t r = 47;
        uint64_t h = len + seed;

        for(; len >= 8; len -= 8, key += 8) {
            uint64_t k = *(uint64_t*)key * m;
            k ^= k >> r;
            k *= m;
            h = (h * m) ^ k;
        }

        switch(len & 7) {
            case 7:
                h ^= uint64_t(key[6]) << 48;
            case 6:
                h ^= uint64_t(key[5]) << 40;
            case 5:
                h ^= uint64_t(key[4]) << 32;
            case 4:
                h ^= uint64_t(key[3]) << 24;
            case 3:
                h ^= uint64_t(key[2]) << 16;
            case 2:
                h ^= uint64_t(key[1]) << 8;
            case 1:
                h ^= uint64_t(key[0]);
                h *= m;
        }

        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        return h;
    }

}
