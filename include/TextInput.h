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
        class InputType {
            public: 
                enum Value {
                    INPUT_TYPE_TEXT = 0,
                    INPUT_TYPE_PASSWORD
                };
                
                InputType(Value v) : value(v) {}
                
                InputType(const char * v) {
                    if (std::strcmp(v, "text") == 0) {
                        value = INPUT_TYPE_TEXT; 
                        return;
                    }
                    if (std::strcmp(v, "password") == 0) {
                        value = INPUT_TYPE_PASSWORD;
                        return;
                    }
                    value = INPUT_TYPE_TEXT;
                }

                std::string ToString() const {
                    switch (value) {
                        case INPUT_TYPE_TEXT: return "text";
                        case INPUT_TYPE_PASSWORD: return "password";
                    }
                    return "text";
                }

                bool operator==(const InputType& other) const { return value == other.value; }
                bool operator==(Value v) const { return value == v; }
                InputType& operator=(const char* val) {
                    if (!val) {
                        return *this;
                    }
                    if (std::strcmp(val, "password") == 0) {
                        value = INPUT_TYPE_PASSWORD;
                    } else {
                        value = INPUT_TYPE_TEXT;
                    }
                    return *this;
                }

                InputType& operator=(Value v) {
                    value = v;
                    return *this;
                }

                operator Value() const { return value; }
            private:
                Value value;
         };

        TextInput()
            : Button{} {}

        TextInput(int32_t x, int32_t y, int32_t width, int32_t height)
            : Button{x, y, width, height} {}

        static TextInput* BuildFromXML(XMLElement* xmlElement);

        void SetOnTextInputEvent(Event event);
        void SetOnSubmitCallback(Event event);

        const char* GetBasicText() const;
        void SetBasicText(const char* basicText);

        void OnClick() override;

        void AddCharacter(char character);
        void Append(const char* text);
        void RemoveLastCharacter();
        void Clear();
        void Submit();
        void SetType(InputType type);
        const char* GetType() const;
        void SetType(const char* type);

        GENERATE_DUK_STRING_GETTER(TextInput, Type, GetType);
        GENERATE_DUK_STRING_SETTER(TextInput, Type, SetType);

        GENERATE_DUK_STRING_GETTER(TextInput, BasicText, GetBasicText);
        GENERATE_DUK_STRING_SETTER(TextInput, BasicText, SetBasicText);

    protected:
        Event onTextInput{};
        Event onSubmit{};
        std::string basicText{};
        std::string masked{};
        InputType type{InputType::INPUT_TYPE_TEXT};

        void DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight) override;

        const char* GetTextToDraw();
    };

} /* namespace grvl */

#endif /* GRVL_TEXT_INPUT_H_ */
