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

#ifndef GRVL_TEXT_INPUT_H_
#define GRVL_TEXT_INPUT_H_

#include "Button.h"

#include "Event.h"
#include "XMLSupport.h"
#include "stl.h"

#include <array>
#include <stdint.h>

namespace grvl {

    class TextInput : public Button {
    public:
        TextInput()
            : Button{} {}

        TextInput(int32_t x, int32_t y, int32_t width, int32_t height)
            : Button{x, y, width, height} {}

        static TextInput* BuildFromXML(XMLElement* xmlElement);

        void SetOnTextInputEvent(Event event);

        void SetBasicText(const char* basicText);

        void OnClick() override;

        void AddCharacter(char character);
        void Append(const char* text);
        void RemoveLastCharacter();
        void Clear();

    protected:
        Event onTextInput{};
        std::string basicText{};

        void DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight) override;
    };

} /* namespace grvl */

#endif /* GRVL_TEXT_INPUT_H_ */
