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

#ifndef GRVL_JSEOBJECTBUILDER_H_
#define GRVL_JSEOBJECTBUILDER_H_

#include <duktape.h>

#include "JSObject.h"

namespace grvl {

    class Component;

    class JSObjectBuilder {
    public:
        JSObjectBuilder(duk_context* ctx, Component* sourceObject);

        JSObjectBuilder& AddProperty(const char* propertyName, duk_c_function getter, duk_c_function setter);
        JSObjectBuilder& AddProperty(const char* propertyName, duk_c_function getter);
        JSObjectBuilder& AttachMemberFunction(const char* functionName, duk_c_function func, duk_idx_t nargs = 0);

    private:
        duk_context* ctx;
        Component* sourceObject;

        duk_idx_t CreateObject();

        duk_idx_t objIdx;

        void AttachAllocatedObjectPointer();
    };

} /* namespace grvl */

#endif /* GRVL_JSEOBJECTBUILDER_H_ */
