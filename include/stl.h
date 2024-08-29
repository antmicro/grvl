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

#include <stdio.h>
#ifdef CYGONCE_LIBC_STDIO_STDIO_H
#include "UstlUnorderedMap.h"
#include <ustl.h>
using namespace ustl;
#else
#include <map>
#include <queue>
#include <string>
#include <tr1/unordered_map>
#include <vector>
using namespace std;
#endif
