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

#ifndef GRVL_ACTION_MUTEX_H_
#define GRVL_ACTION_MUTEX_H_

#include <pthread.h>

namespace grvl {

    class Mutex {
    public:

        Mutex();
        ~Mutex();
        Mutex(const Mutex& mutex);
        Mutex(Mutex&& mutex);

        void lock();
        void unlock();

    private:

        pthread_mutex_t m;

    };

    class Guard {
    public:

        Guard(Mutex& m);
        ~Guard();

    private:

        Mutex& m;

    };

}

#endif /* GRVL_ACTION_MUTEX_H_ */
