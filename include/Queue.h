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

#include <stdint.h>

#include "grvl.h"
#include "stl.h"

namespace grvl {

    template <class T>
    class Queue {

    public:
        Queue();
        ~Queue();
        void push(T* element);
        T* pop();

    private:
        void* mutex;
        queue<T*> elementsQueue;
    };

    template <class T>
    Queue<T>::Queue()
    {
        mutex = grvl::Callbacks()->mutex_create();
    }

    template <class T>
    Queue<T>::~Queue()
    {
        grvl::Callbacks()->mutex_destroy(mutex);
    }

    template <class T>
    void Queue<T>::push(T* element)
    {
        if(grvl::Callbacks()->mutex_lock(mutex) != 0) {
            return;
        }

        elementsQueue.push(element);

        grvl::Callbacks()->mutex_unlock(mutex);
    }

    template <class T>
    T* Queue<T>::pop()
    {
        if(grvl::Callbacks()->mutex_lock(mutex) != 0) {
            return NULL;
        }

        T* result;
        if(elementsQueue.empty()) {
            result = NULL;
        } else {
            result = elementsQueue.front();
            elementsQueue.pop();
        }

        grvl::Callbacks()->mutex_unlock(mutex);
        return result;
    }

} /* namespace grvl */

#endif /* GRVL_Queue_H_ */
