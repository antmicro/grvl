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

#ifndef GRVL_EVENT_H_
#define GRVL_EVENT_H_

#include "stl.h"

#include <functional>
#include <stdint.h>

namespace grvl {

    /// Represents event.
    class Event {
    public:
        typedef vector<string> ArgVector;
        typedef void (*CallbackPointer)(void* senderPtr, const Event::ArgVector&);
        using CallbackFunction = std::function<void(void* senderPtr, const Event::ArgVector&)>;

        Event()
            : senderPtr(NULL)
            , eventCallback(NULL)
        {
        }
        Event(CallbackPointer callback, const ArgVector& argVector)
            : argVec(argVector)
            , senderPtr(NULL)
            , eventCallback(callback)
        {
        }
        Event(void* sender, CallbackPointer callback, const ArgVector& argVector)
            : argVec(argVector)
            , senderPtr(sender)
            , eventCallback(callback)
        {
        }
        Event(const Event& Obj)
            : argVec(Obj.argVec)
            , senderPtr(NULL)
            , eventCallback(Obj.eventCallback)
        {
        }

        Event(CallbackFunction eventCallback)
            : eventCallback{std::move(eventCallback)} {}

        virtual ~Event();

        Event& operator=(const Event& Obj);

        void Trigger();
        void SetSenderPointer(void* sender);
        void SetCallback(CallbackPointer Callback, const ArgVector& argVector);

        bool IsSet();

    private:
        ArgVector argVec;
        void* senderPtr;
        CallbackFunction eventCallback;
    };

} /* namespace grvl */

#endif /* GRVL_EVENT_H_ */
