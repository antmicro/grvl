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

#include <grvl/JSEngine.h>
#include <grvl/grvl.h>

#include <grvl/Blitter.h>

namespace grvl {

    static gui_callbacks_t callbacks;

    void grvl::Init(gui_callbacks_t* n_callbacks)
    {
        if(n_callbacks) {
            if (n_callbacks->dma_fill) {
                Log("[WARN] Deprecated callback .dma_fill used, move to .fill!");
                n_callbacks->fill = UseOldDmaFill;
            }

            if (n_callbacks->dma_operation) {
                Log("[WARN] Deprecated callback .dma_operation used, move to .blit!");
                n_callbacks->blit = UseOldDmaBlit;
            }

             if (n_callbacks->dma_operation_clt) {
                Log("[WARN] Deprecated callback .dma_operation_clt used, move to .blit_clt!");
                n_callbacks->blit_clt = UseOldDmaBlitClt;
            }

            if (n_callbacks->fill == nullptr) n_callbacks->fill = FallbackFill;
            if (n_callbacks->blit == nullptr) n_callbacks->blit = FallbackBlit;
            if (n_callbacks->blit_clt == nullptr) n_callbacks->blit_clt = FallbackBlitClt;

            memcpy(&callbacks, n_callbacks, sizeof(callbacks));
        } else {
            callbacks.dma_operation = nullptr;
            callbacks.gui_printf = nullptr;
            callbacks.get_timestamp = nullptr;

            callbacks.fill = FallbackFill;
            callbacks.blit = FallbackBlit;
            callbacks.blit_clt = FallbackBlitClt;

            callbacks.duk_alloc_func = nullptr;
            callbacks.duk_realloc_func = nullptr;
            callbacks.duk_free_func = nullptr;
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
