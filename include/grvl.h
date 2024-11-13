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

#ifndef GRVL_GRVL_H_
#define GRVL_GRVL_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <duktape.h>

namespace grvl {

    typedef struct {
        void (*dma_operation)(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t NumberOfLine,
                              uint32_t PixelPerLine, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                              uint32_t inPixelFormat, uint32_t backgroundPixelFormat, uint32_t outPixelFormat, uint32_t frontColor);
        void (*dma_operation_clt)(uintptr_t inputMem, uintptr_t backgroundMem, uintptr_t outputMem, uint32_t NumberOfLine,
                                  uint32_t PixelPerLine, uint32_t inOffset, uint32_t backgroundOffset, uint32_t outOffset,
                                  uint32_t inPixelFormat, uint32_t backgroundPixelFormat, uint32_t outPixelFormat, uint32_t frontColor, uintptr_t backCLT, uintptr_t frontCLT);
        void (*dma_fill)(uintptr_t dst, uint32_t xs, uint32_t ys, uint32_t offset, uint32_t color_index,
                         uint32_t pixel_format);
        void (*set_layer_pointer)(uintptr_t addr);
        void* (*malloc)(size_t);
        void (*free)(void*);
        void (*wait_for_vsync)();
        void (*flipping_completed)();
        void (*gui_printf)(const char* text, va_list argList);
        void* (*mutex_create)(void);
        int (*mutex_lock)(void* mutex);
        void (*mutex_unlock)(void* mutex);
        void (*mutex_destroy)(void* mutex);
        uint64_t (*get_timestamp)(void);

        duk_alloc_function duk_alloc_func;
        duk_realloc_function duk_realloc_func;
        duk_free_function duk_free_func;
    } gui_callbacks_t;

    /// Class used to initialize the library.
    class grvl {

    public:
        /// Method accepting callbacks to platform-dependent functions, e.g. for memory management.
        static void Init(gui_callbacks_t* n_callbacks);
        static void Destroy();
        static gui_callbacks_t* Callbacks();
        static void Log(const char* text, ...);
    };

} /* namespace grvl */

#endif /* GRVL_GRVL_H */
