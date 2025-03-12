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

#include "JSEngine.h"

#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>

#include "Manager.h"
#include "JSObjectBuilder.h"

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

    std::vector<char> JSEngine::LoadCodeFromFile(const char* filePath)
    {
        File javaScriptCodeFile{filePath};
        std::vector<char> source(javaScriptCodeFile.GetSize());
        
        if (File::noFS) {
            uint8_t* buffer;
            javaScriptCodeFile.ReadToBuffer(buffer);

            memcpy(source.data(), buffer, source.size());
        }
        else {
            uint8_t* buffer = (uint8_t*)source.data();
            javaScriptCodeFile.ReadToBuffer(buffer);
        }

        return source;
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
            PushComponentAsJSObjectOntoStack(callerAsComponent);
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
            PushComponentAsJSObjectOntoStack(foundComponent);
            return RETURN_VALUE_PRESENT;
        }

        return NO_RETURN_VALUE;
    }

    void JSEngine::PushComponentAsJSObjectOntoStack(Component* component)
    {
        JSObjectBuilder jsObjectBuilder{ctx, component};
        component->PopulateJavaScriptObject(jsObjectBuilder);
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

    duk_ret_t JSEngine::GetTopPanel(duk_context* ctx)
    {
        Manager& managerInstance = Manager::GetInstance();
        Panel* topPanel = managerInstance.GetTopPanel();
        PushComponentAsJSObjectOntoStack(topPanel);
    
        return RETURN_VALUE_PRESENT;
    }

    duk_ret_t JSEngine::GetBottomPanel(duk_context* ctx)
    {
        Manager& managerInstance = Manager::GetInstance();
        Panel* bottomPanel = managerInstance.GetBottomPanel();
        PushComponentAsJSObjectOntoStack(bottomPanel);
    
        return RETURN_VALUE_PRESENT;
    }

    duk_ret_t JSEngine::GetPrefabById(duk_context* ctx)
    {
        const char* prefabId = duk_to_string(ctx, 0);

        Manager& managerInstance = Manager::GetInstance();
        Component* prefab = managerInstance.GetPrefabByID(prefabId);
        PushComponentAsJSObjectOntoStack(prefab);

        return RETURN_VALUE_PRESENT;
    }

} /* namespace grvl */
