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

#ifndef GRVL_JSENGINE_H_
#define GRVL_JSENGINE_H_

#include <string>
#include <unordered_map>

#include <duktape.h>

#include "Event.h"
#include "Component.h"
#include "CallbackDefinition.h"
#include "JSObject.h"

#include "grvl.h"

namespace grvl {

    class JSEngine {
    public:
        static void Initialize(gui_callbacks_t* n_callbacks);
        static void Destroy();

        static void SetSourceCodeWorkingDirectory(std::string path);
        static bool LoadJavaScriptCode(const std::string& filePath);
        static bool LoadJavaScriptCode(const char* code, std::size_t size);
        static void AddGlobalFunction(const char* functionName, duk_c_function func, duk_idx_t nargs);

        static void MakeJavaScriptFunctionCall(const char* functionName);
        static void ExecuteJavaScriptCallback(const char* functionName, void* caller, const Event::ArgVector& args);
        static Event CreateJavaScriptFunctionCallEvent(const std::string& functionName, const Event::ArgVector& args);
        static void PushComponentAsJSObjectOntoStack(Component* component);

        static duk_ret_t Print(duk_context* ctx);
        static duk_ret_t GetElementById(duk_context* ctx);
        static duk_ret_t ShowPopup(duk_context* ctx);
        static duk_ret_t ClosePopup(duk_context* ctx);
        static duk_ret_t SetActiveScreen(duk_context* ctx);
        static duk_ret_t GetTopPanel(duk_context* ctx);
        static duk_ret_t GetBottomPanel(duk_context* ctx);

    private:
        static constexpr duk_ret_t NO_RETURN_VALUE{0};
        static constexpr duk_ret_t RETURN_VALUE_PRESENT{1};

        static void InitializeDukContext(gui_callbacks_t* n_callbacks);
        static void RegisterBasicAPIFunctions();

        inline static duk_context* ctx;

        static std::vector<char> LoadCodeFromFile(const char* filePath);

        inline static std::string sourceCodeWorkingDirectory;

        static void PrepareJavaScriptFunctionCall(const char* functionName);
        static void ExecutePreparedJavaScriptFunctionCall(const char* functionName, int numOfArgs);
    };

} /* namespace grvl */

#endif /* GRVL_JSENGINE_H_ */
