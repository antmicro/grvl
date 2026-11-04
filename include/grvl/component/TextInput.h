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

#include <grvl/Event.h>
#include <grvl/component/Button.h>

#include <array>
#include <stdint.h>

namespace grvl {

    class TextInput : public Button {
    public:
        static constexpr const char* name = "TextInput";
        class InputType {
        public:
            enum InputTypeValue {
                TEXT = 0,
                PASSWORD,
                _COUNT
            };

            InputType(InputTypeValue v)
                : value(v)
            {
            }
            InputType(const char* v)
                : value(fromString(v))
            {
            }

            bool operator==(const InputType& other) const { return value == other.value; }
            bool operator==(InputTypeValue v) const { return value == v; }
            InputType& operator=(const char* val)
            {
                value = fromString(val);
                return *this;
            }

            InputType& operator=(InputTypeValue v)
            {
                value = v;
                return *this;
            }

            operator InputTypeValue() const { return value; }
            operator std::string() const { return toString(); }
            operator const char*() const { return toString(); }

        private:
            InputTypeValue value;

            static InputTypeValue fromString(const char* s)
            {
                if(!s)
                    return TEXT;

                static const std::unordered_map<std::string, InputTypeValue> lookup = {
                    { "text", TEXT },
                    { "password", PASSWORD }
                };
                auto it = lookup.find(s);
                return (it != lookup.end()) ? it->second : TEXT;
            }

            const char* toString() const
            {
                static const char* reverse_lookup[] = {
                    "text",
                    "password"
                };

                if(value >= 0 && value < _COUNT) {
                    return reverse_lookup[value];
                }
                return "text";
            }
        };

        TextInput()
            : Button {}
        {
        }

        TextInput(int32_t x, int32_t y, int32_t width, int32_t height)
            : Button { x, y, width, height }
        {
        }

        TextInput(const TextInput& other);
        TextInput& operator=(const TextInput& other);
        Component* Clone() const override;

        static TextInput* BuildFromXML(XmlElement xml);

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

        static constexpr int32_t CURSOR_WIDTH { 1 };
        /// The gap and the overhang are this fraction of the font height.
        static constexpr int32_t CURSOR_METRIC_DIVIDER { 4 };
        /// Milliseconds the caret spends visible, and then hidden.
        static constexpr uint64_t CURSOR_BLINK_PERIOD_MS { 500 };

        /// Blank columns kept between the text and the caret.
        static int32_t CursorGap(int32_t fontHeight);
        /// Pixels the caret adds above and below the font height.
        static int32_t CursorOverhang(int32_t fontHeight);

        /// Restarts the blink cycle, so the caret is visible straight away.
        void ResetCursorBlink();

        void SetAutoFocus(bool enabled);
        bool IsAutoFocus() const;

        void SetCursorEnabled(bool enabled);
        bool IsCursorEnabled() const;
        void SetCursorColor(uint32_t color);
        uint32_t GetCursorColor() const;
        void SetCursorBlink(bool enabled);
        bool IsCursorBlinking() const;

        size_t GetUTF8CharacterCount() const;

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;

        GENERATE_DUK_STRING_GETTER(TextInput, Type, GetType);
        GENERATE_DUK_STRING_SETTER(TextInput, Type, SetType);

        GENERATE_DUK_STRING_GETTER(TextInput, BasicText, GetBasicText);
        GENERATE_DUK_STRING_SETTER(TextInput, BasicText, SetBasicText);

    protected:
        Event onTextInput {};
        Event onSubmit {};
        std::string basicText {};
        std::string masked {};
        InputType type { InputType::TEXT };
        bool autoFocus { false };
        bool cursorEnabled { false };
        uint32_t cursorColor { 0 };
        bool cursorBlink { true };
        uint64_t cursorBlinkStart { 0 };

        /// Draws the caret after the typed text, clamped to the field.
        void DrawCursor(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight,
                        int32_t BaselineY, const char* textToDraw);

        void DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight) override;

        const char* GetTextToDraw();
    };

} /* namespace grvl */

#endif /* GRVL_TEXT_INPUT_H_ */
