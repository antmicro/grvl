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

#ifndef GRVL_Queue_H_
#define GRVL_Queue_H_

#include <grvl/grvl.h>
#include <grvl/stl.h>
#include <grvl/Mutex.h>

#include <stdint.h>

namespace grvl {

    template <class T>
    class Queue {

    public:
        void push(T* element);
        T* pop();

    private:
        Mutex m;
        queue<T*> elementsQueue;
    };

    template <class T>
    void Queue<T>::push(T* element)
    {
        Guard lock {m};
        elementsQueue.push(element);
    }

    template <class T>
    T* Queue<T>::pop()
    {
        Guard lock {m};

        if(elementsQueue.empty()) {
            return nullptr;
        }

        T* result = elementsQueue.front();
        elementsQueue.pop();
        return result;
    }

} /* namespace grvl */

#endif /* GRVL_Queue_H_ */
