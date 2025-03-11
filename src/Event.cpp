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

#include "Event.h"

namespace grvl {

    void Event::Trigger()
    {
        if(eventCallback == NULL) {
            return;
        }
        eventCallback(senderPtr, argVec);
    }

    void Event::SetCallback(CallbackPointer Callback, const ArgVector& argVector)
    {
        if(Callback) {
            eventCallback = Callback;
            argVec = argVector;
        }
    }

    void Event::SetSenderPointer(void* sender)
    {
        if(sender) {
            senderPtr = sender;
        }
    }

    bool Event::IsSet()
    {
        return (bool)eventCallback;
    }

    Event& Event::operator=(const Event& Obj)
    {
        if(this != &Obj) {
            argVec = Obj.argVec;
            senderPtr = NULL; // todo it must be initialized after construction. Not sure if we have such mechanism though.
            eventCallback = Obj.eventCallback;
        }
        return *this;
    }

} /* namespace grvl */
