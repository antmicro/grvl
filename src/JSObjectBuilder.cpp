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

#include "JSObjectBuilder.h"

#include "JSEngine.h"
#include "JSObject.h"

namespace grvl {

    JSObjectBuilder::JSObjectBuilder(duk_context* ctx, Component* sourceObject)
        : ctx{ctx}, sourceObject{sourceObject}, objIdx{CreateObject()}
    {
        AttachAllocatedObjectPointer();
    }

    duk_idx_t JSObjectBuilder::CreateObject()
    {
        return duk_push_object(ctx);
    }

    void JSObjectBuilder::AttachAllocatedObjectPointer()
    {
        duk_push_string(ctx, JSObject::C_OBJECT_POINTER_KEY);
        duk_push_pointer(ctx, sourceObject);
        duk_put_prop(ctx, objIdx);
    }

    JSObjectBuilder& JSObjectBuilder::AddProperty(const char* propertyName, duk_c_function getter, duk_c_function setter)
    {
        duk_push_string(ctx, propertyName);
        duk_push_c_function(ctx, getter, 0);
        duk_push_c_function(ctx, setter, 1);
        duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

        return *this;
    }

    JSObjectBuilder& JSObjectBuilder::AddProperty(const char* propertyName, duk_c_function getter)
    {
        duk_push_string(ctx, propertyName);
        duk_push_c_function(ctx, getter, 0);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

        return *this;
    }

    JSObjectBuilder& JSObjectBuilder::AttachMemberFunction(const char* functionName, duk_c_function func, duk_idx_t nargs)
    {
        duk_push_string(ctx, functionName);
        duk_push_c_function(ctx, func, nargs);
        duk_put_prop(ctx, objIdx);

        return *this;
    }

} /* namespace grvl */
