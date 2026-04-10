// Copyright 2026 Antmicro <antmicro.com>
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

#ifndef GRVL_ENDIAN_H_
#define GRVL_ENDIAN_H_

#if (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || defined(__BIG_ENDIAN__)
#    define GRVL_BIG_ENDIAN true
#else
#    define GRVL_BIG_ENDIAN false
#endif

namespace grvl {

    inline uint16_t ByteSwap16(uint16_t x)
    {
        return (((uint16_t) (x & 0xFF) << 8) | (x >> 8));
    }

    inline uint32_t ByteSwap32(uint32_t x)
    {
        return (((uint32_t) ByteSwap16(x & 0xFFFF) << 16) | ByteSwap16(x >> 16));
    }

    inline uint64_t ByteSwap64(uint64_t x)
    {
        return (((uint64_t) ByteSwap32(x & 0xFFFFFFFF) << 32) | ByteSwap32(x >> 32));
    }

    /// Native to Big Endian or Big Endian to Native, 16 bit
    inline uint32_t BigEndian16(uint16_t x)
    {
#if GRVL_BIG_ENDIAN
        return x;
#else
        return ByteSwap16(x);
#endif
    }

    /// Native to Big Endian or Big Endian to Native, 32 bit
    inline uint32_t BigEndian32(uint32_t x)
    {
#if GRVL_BIG_ENDIAN
        return x;
#else
        return ByteSwap32(x);
#endif
    }

    /// Native to Big Endian or Big Endian to Native, 64 bit
    inline uint64_t BigEndian64(uint64_t x)
    {
#if GRVL_BIG_ENDIAN
        return x;
#else
        return ByteSwap64(x);
#endif
    }

};

#endif /* GRVL_ENDIAN_H_ */
