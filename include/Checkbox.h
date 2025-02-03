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

#ifndef GRVL_CHECKBOX_H_
#define GRVL_CHECKBOX_H_

#include "SwitchButton.h"
#include "Event.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"
#include "JSObjectBuilder.h"

using namespace tinyxml2;

namespace grvl {

    class Checkbox : public SwitchButton {
    public:
        Checkbox()
            : SwitchButton{} {}

        Checkbox(int32_t x, int32_t y, int32_t width, int32_t height)
            : SwitchButton{x, y, width, height} {}

        Component* Clone() const override;

        static Checkbox* BuildFromXML(XMLElement* xmlElement);

    protected:
        virtual void DrawActiveState(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight);
        virtual void DrawInactiveState(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight);
    };

} /* namespace grvl */

#endif /* GRVL_CHECKBOX_H_ */
