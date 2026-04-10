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

#include "JSEngine.h"

#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>

#include "Manager.h"
#include "JSObjectBuilder.h"
#include "duktape.h"

namespace grvl {

    void JSEngine::Initialize(gui_callbacks_t* n_callbacks)
    {
        InitializeDukContext(n_callbacks);
        RegisterBasicAPIFunctions();
    }

    void JSEngine::InitializeDukContext(gui_callbacks_t* n_callbacks)
    {
        ctx = duk_create_heap(n_callbacks->duk_alloc_func, n_callbacks->duk_realloc_func, n_callbacks->duk_free_func, NULL, NULL);
        if (!ctx) {
            printf("Failed to create a Duktape heap.\n");
            exit(1);
        }
        duk_push_heap_stash(ctx);
        duk_push_array(ctx);
        duk_put_prop_literal(ctx, -2, STASH_REMEBERED_OBJECTS_KEY);
    }

    void JSEngine::RegisterBasicAPIFunctions()
    {
        AddGlobalFunction("GetElementById", JSEngine::GetElementById, 1);
        AddGlobalFunction("Print", JSEngine::Print, 1);
        AddGlobalFunction("ShowPopup", JSEngine::ShowPopup, 1);
        AddGlobalFunction("ClosePopup", JSEngine::ClosePopup, 0);
        AddGlobalFunction("SetActiveScreen", JSEngine::SetActiveScreen, 2);
        AddGlobalFunction("GetTopPanel", JSEngine::GetTopPanel, 0);
        AddGlobalFunction("GetBottomPanel", JSEngine::GetBottomPanel, 0);
        AddGlobalFunction("GetPrefabById", JSEngine::GetPrefabById, 1);
    }

    void JSEngine::AddGlobalFunction(const char* functionName, duk_c_function func, duk_idx_t nargs)
    {
        duk_push_c_function(ctx, func, nargs);
        duk_put_global_string(ctx, functionName);
    }

    void JSEngine::AddGlobalObject(const char* name, void* ptr, const std::map<const char*, duk_c_function>& methods)
    {
        duk_push_object(ctx);
    
        duk_push_pointer(ctx, ptr);
        duk_put_prop_string(ctx, -2, JSObject::C_OBJECT_POINTER_KEY);

        for (auto const& [methodName, func] : methods) {
            duk_push_c_function(ctx, func, DUK_VARARGS);
            duk_put_prop_string(ctx, -2, methodName);
        }

        duk_put_global_string(ctx, name);

    }

    void JSEngine::AddGlobalEnum(const std::string& enumName, const std::map<std::string, int>& values)
    {
        duk_push_object(ctx);

        for (auto const& [key, value] : values) {
            duk_push_int(ctx, value);
            duk_put_prop_string(ctx, -2, key.c_str());
        
            // TODO: the line below crashes for some reason, would be good to make that right
            //duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE); 
        }

        duk_put_global_string(ctx, enumName.c_str());
    }

    void JSEngine::Destroy()
    {
        duk_destroy_heap(ctx);
    }

    void JSEngine::SetSourceCodeWorkingDirectory(std::string path)
    {
        sourceCodeWorkingDirectory = std::move(path);
    }

    bool JSEngine::LoadJavaScriptCode(const std::string& filePath)
    {
        if (filePath.empty()) {
            return true;
        }

        std::string finalPath = sourceCodeWorkingDirectory.append("/").append(filePath);
        std::vector<char> source = LoadCodeFromFile(finalPath.c_str());

        return LoadJavaScriptCode(source.data(), source.size());
    }

    std::vector<char> JSEngine::LoadCodeFromFile(const char* path)
    {
        File file {path};
        return file.Read();
    }

    bool JSEngine::LoadJavaScriptCode(const char* code, std::size_t codeSize)
    {
        duk_push_lstring(ctx, code, static_cast<duk_size_t>(codeSize));

        if (duk_peval(ctx) != 0)
        {
            grvl::Log("Error occurred while parsing JavaScript code: %s", duk_safe_to_string(ctx, -1));
            duk_pop(ctx);
            return false;
        }

        return true;
    }

    void JSEngine::MakeJavaScriptFunctionCall(const char* functionName)
    {
        PrepareJavaScriptFunctionCall(functionName);

        constexpr int NO_ARGS{0};
        ExecutePreparedJavaScriptFunctionCall(functionName, NO_ARGS);
    }

    void JSEngine::PrepareJavaScriptFunctionCall(const char* functionName)
    {
        duk_get_global_string(ctx, functionName);
    }

    void JSEngine::ExecutePreparedJavaScriptFunctionCall(const char* functionName, int numOfArgs)
    {
        if (duk_pcall(ctx, numOfArgs) != 0) {
            grvl::Log("Error while trying to execute JavaScript function call %s: %s", functionName, duk_safe_to_string(ctx, -1));
        }
        duk_pop(ctx);
    }

    void JSEngine::ExecuteJavaScriptCallback(const char* functionName, void* caller, const Event::ArgVector& args)
    {
        PrepareJavaScriptFunctionCall(functionName);

        int numOfArgs = args.size();

        if (auto callerAsComponent = static_cast<Component*>(caller)) {
            numOfArgs += 1;
            callerAsComponent->PushJSObjectOnStack(ctx);
        }

        for (const auto& arg : args) {
            duk_push_string(ctx, arg.c_str());
        }

        ExecutePreparedJavaScriptFunctionCall(functionName, numOfArgs);
    }

    Event JSEngine::CreateJavaScriptFunctionCallEvent(const std::string& functionName, const Event::ArgVector& args)
    {
        if (functionName.empty()) {
            return Event{};
        }

        auto callback = [=] (void* caller, const Event::ArgVector&) {
            ExecuteJavaScriptCallback(functionName.c_str(), caller, args);
        };

        return Event{std::move(callback)};
    }

    void JSEngine::PushRemeberedObjectsArray(duk_context* ctx)
    {
        duk_push_heap_stash(ctx);
        duk_get_prop_literal(ctx, -1, STASH_REMEBERED_OBJECTS_KEY);
        duk_remove(ctx, -2);
    }

    void JSEngine::RememberObject(void *objectPtr)
    {
        PushRemeberedObjectsArray(ctx);
        duk_uarridx_t len = duk_get_length(ctx, -1);
        duk_uarridx_t idx = lowestMaybeEmptyRememberedObjectIdx;
        while (idx < len) {
            if (!duk_has_prop_index(ctx, -1, idx)) break;
            idx += 1;
        }
        lowestMaybeEmptyRememberedObjectIdx = idx + 1;
        duk_push_heapptr(ctx, objectPtr);
        duk_put_prop_index(ctx, -2, idx);
        duk_pop(ctx);
    }

    void JSEngine::ForgetObject(void *objectPtr)
    {
        PushRemeberedObjectsArray(ctx);
        duk_uarridx_t len = duk_get_length(ctx, -1);
        duk_uarridx_t idx = 0;
        for (; idx < len; duk_pop(ctx), idx += 1) {
            if (!duk_get_prop_index(ctx, -1, idx)) {
                continue;
            }
            if (duk_get_heapptr(ctx, -1) != objectPtr) {
                continue;
            }
            duk_pop(ctx);
            break;
        }
        if (idx == len) {
            // Object was already forgotten?
            return;
        }
        duk_del_prop_index(ctx, -1, idx);
        if (idx < lowestMaybeEmptyRememberedObjectIdx) {
            lowestMaybeEmptyRememberedObjectIdx = idx;
        }
        duk_pop(ctx);
    }

    duk_ret_t JSEngine::Print(duk_context* ctx)
    {
        const char* log = duk_to_string(ctx, 0);
        grvl::Log("%s", log);

        return NO_RETURN_VALUE;
    }

    duk_ret_t JSEngine::GetElementById(duk_context* ctx)
    {
        const char* componentName = duk_to_string(ctx, 0);
        Manager& managerInstance = Manager::GetInstance();
        if (Component* foundComponent = managerInstance.FindElementInTheActiveScreenById(componentName)) {
            foundComponent->PushJSObjectOnStack(ctx);
            return RETURN_VALUE_PRESENT;
        }

        return NO_RETURN_VALUE;
    }

    duk_ret_t JSEngine::ShowPopup(duk_context* ctx)
    {
        const char* popupText = duk_to_string(ctx, 0);
        Manager& managerInstance = Manager::GetInstance();
        managerInstance.ShowPopup(popupText);

        return NO_RETURN_VALUE;
    }

    duk_ret_t JSEngine::ClosePopup(duk_context* ctx)
    {
        Manager& managerInstance = Manager::GetInstance();
        managerInstance.ClosePopup();

        return NO_RETURN_VALUE;
    }

    duk_ret_t JSEngine::SetActiveScreen(duk_context* ctx)
    {
        const char* screenName = duk_to_string(ctx, 0);
        int animationDirection = duk_to_int(ctx, 1);

        Manager& managerInstance = Manager::GetInstance();
        managerInstance.SetActiveScreen(screenName, animationDirection);

        return NO_RETURN_VALUE;
    }

    duk_ret_t JSEngine::SetActiveInput(duk_context* ctx)
    {
        Manager& managerInstance = Manager::GetInstance();
        if (duk_is_null(ctx,0)) {
            managerInstance.SetActiveInput(nullptr);
            return NO_RETURN_VALUE;
        }

        const char* inputName = duk_to_string(ctx, 0);
        auto inputField = managerInstance.FindElementInTheActiveScreenById(inputName);
        managerInstance.SetActiveInput(dynamic_cast<TextInput*>(inputField));

        return NO_RETURN_VALUE;
    }

    duk_ret_t JSEngine::GetTopPanel(duk_context* ctx)
    {
        Manager& managerInstance = Manager::GetInstance();
        Panel* topPanel = managerInstance.GetTopPanel();
        topPanel->PushJSObjectOnStack(ctx);

        return RETURN_VALUE_PRESENT;
    }

    duk_ret_t JSEngine::GetBottomPanel(duk_context* ctx)
    {
        Manager& managerInstance = Manager::GetInstance();
        Panel* bottomPanel = managerInstance.GetBottomPanel();
        bottomPanel->PushJSObjectOnStack(ctx);

        return RETURN_VALUE_PRESENT;
    }

    duk_ret_t JSEngine::GetPrefabById(duk_context* ctx)
    {
        const char* prefabId = duk_to_string(ctx, 0);

        Manager& managerInstance = Manager::GetInstance();
        Component* prefab = managerInstance.GetPrefabByID(prefabId);
        prefab->PushJSObjectOnStack(ctx);

        return RETURN_VALUE_PRESENT;
    }

} /* namespace grvl */
