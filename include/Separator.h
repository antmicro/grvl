// Copyright 2014-2025 Antmicro <antmicro.com>
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

#ifndef GRVL_SEPARATOR_H_
#define GRVL_SEPARATOR_H_

#include <cstdint>
#include <unordered_map>
#include <string>

#include "Component.h"
#include "XMLSupport.h"

namespace grvl {

    class Separator : public Component {
    public:
        Separator()
            : Component{} {}

        Separator(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component{x, y, width, height} {}

        Separator(const Separator& Other)
            : Component{Other} {}

        static Separator* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& Painter, int32_t ParentX, int32_t ParentY) override;
    };

} /* namespace grvl */

#endif /* GRVL_SEPARATOR_H_ */
