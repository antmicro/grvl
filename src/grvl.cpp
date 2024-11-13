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

#include "grvl.h"

#include "JSEngine.h"

namespace grvl {

    static gui_callbacks_t callbacks;

    void grvl::Init(gui_callbacks_t* n_callbacks)
    {
        if(n_callbacks) {
            memcpy(&callbacks, n_callbacks, sizeof(callbacks));
        } else {
            callbacks.free = free;
            callbacks.malloc = malloc;
            callbacks.dma_operation = NULL;
            callbacks.gui_printf = NULL;
            callbacks.mutex_create = NULL;
            callbacks.mutex_lock = NULL;
            callbacks.mutex_unlock = NULL;
            callbacks.mutex_destroy = NULL;
            callbacks.get_timestamp = NULL;

            callbacks.duk_alloc_func = NULL;
            callbacks.duk_realloc_func = NULL;
            callbacks.duk_free_func = NULL;
        }

        JSEngine::Initialize(n_callbacks);
    }

    void grvl::Destroy()
    {
        JSEngine::Destroy();
    }

    gui_callbacks_t* grvl::Callbacks()
    {
        return &callbacks;
    }

    void grvl::Log(const char* text, ...)
    {
        if(grvl::Callbacks()->gui_printf == 0) {
            return;
        }
        va_list argList;
        va_start(argList, text);
        grvl::Callbacks()->gui_printf(text, argList);
        va_end(argList);
    }

} /* namespace grvl */
