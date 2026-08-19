// Copyright 2014-2026 Antmicro <antmicro.com>
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

#ifndef GRVL_GRVL_H_
#define GRVL_GRVL_H_

#include <grvl/Blitter.h>
#include <grvl/Format.h>
#include <grvl/Log.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <duktape.h>

namespace grvl {

    typedef struct {
        DmaFillFunction fill;
        DmaBlitFunction blit;
        DmaBlitCltFunction blit_clt;
        Logger logger;

        void (*set_layer_pointer)(uintptr_t addr);
        void (*wait_for_vsync)();
        void (*flipping_completed)();

        [[deprecated("Use .logger")]]
        void (*gui_printf)(const char* text, va_list argList);
        uint64_t (*get_timestamp)(void);
    } gui_callbacks_t;

    /// Class used to initialize the library.
    class grvl {

    public:
        /// Method accepting callbacks to platform-dependent functions
        static void Init(gui_callbacks_t* n_callbacks);
        static void Destroy();
        static gui_callbacks_t* Callbacks();
    };

    template <typename... Args>
    inline void Log(LogLevel level, const char* text, Args... args)
    {
        static auto CallUserPrint = [](LogLevel level, const char* format, ...) {
            va_list list;
            int bytes = 0;

            // get required buffer size
            va_start(list, format);
            bytes = vsnprintf(NULL, 0, format, list) + 1;
            va_end(list);

            char buffer[bytes];

            // format message
            va_start(list, format);
            vsnprintf(buffer, bytes, format, list);
            va_end(list);

            // call the user callback
            grvl::Callbacks()->logger(level, buffer);
        };

        CallUserPrint(level, text, args...);
    }

} /* namespace grvl */

#endif /* GRVL_GRVL_H */
