// Copyright 2026 Antmicro <antmicro.com>
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

#include <grvl/Mutex.h>

namespace grvl {

    Mutex::Mutex() {
        pthread_mutex_init(&m, nullptr);
    }

    Mutex::~Mutex() {
        pthread_mutex_destroy(&m);
    }

    Mutex::Mutex(const Mutex& other)
        : Mutex() {
    }

    Mutex::Mutex(Mutex&& other)
        : Mutex() {
    }

    void Mutex::lock() {
        pthread_mutex_lock(&m);
    }

    void Mutex::unlock() {
        pthread_mutex_unlock(&m);
    }

    Guard::Guard(Mutex& m) : m(m) {
        m.lock();
    }

    Guard::~Guard() {
        m.unlock();
    }

}
