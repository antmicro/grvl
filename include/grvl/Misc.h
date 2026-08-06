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

#include <grvl/Format.h>
#include <grvl/grvl.h>

#include <ctype.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <array>
#include <chrono>

namespace grvl {

    using PerfClock = std::chrono::steady_clock;

    // free() functor for c_unique_ptr
    struct c_deleter {
        template <typename T>
        void operator()(T* ptr) const { free(ptr); }
    };

    // a std::unique_ptr that calls free() not delete
    template <typename T>
    using c_unique_ptr = std::unique_ptr<T, c_deleter>;

    template <typename T>
    static inline void string_to_lower(T& s)
    {
        int i = 0;
        while(s[i] != 0) {
            s[i] = tolower(s[i]);
            i++;
        }
    }

    struct Stopwatch {

        using Point = std::chrono::time_point<PerfClock>;

        Point begin;
        Point end;

        Stopwatch() {
            end = begin = PerfClock::now();
        }

        size_t stop() {
            end = PerfClock::now();
            return get();
        }

        template <typename Unit = std::chrono::nanoseconds>
        size_t get() const {
            return std::chrono::duration_cast<Unit>(end - begin).count();
        }

    };

    template <typename T, size_t S>
    class SimpleRing
    {
        std::array<T, S> buffer;
        size_t index = 0;

    public:

        SimpleRing() {
            for (T& v : buffer) {
                v = 0;
            }
        }

        void put(const T& value) {
            buffer[index] = value;

            index++;
            if (index >= S) {
                index = 0;
            }
        }

        uint64_t sum() const {
            uint64_t total = 0;
            for (auto i : buffer) total += i;
            return total;
        }

        uint64_t avg() const {
            return static_cast<uint64_t>(sum() / static_cast<float>(S));
        }

        const std::array<T, S>& view() const {
            return buffer;
        }
    };

    int32_t Clamp(int32_t val, int32_t left, int32_t right);

    float ConstrainAngle(float angle);

    float AngleDiff(float angle1, float angle2);

    extern const uint8_t greyscaleCltPalette[3 * 0x100];
} /* namespace grvl */

#endif /* GRVL_MISC_H_ */
