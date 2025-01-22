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

#ifndef GRVL_KEYBOARD_KEY_H_
#define GRVL_KEYBOARD_KEY_H_

#include "Button.h"
#include "Event.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"
#include "JSObjectBuilder.h"

using namespace tinyxml2;

namespace grvl {

    class Keyboard;

    class KeyboardKey : public Button {
    public:
        KeyboardKey()
            : Button{} {}

        KeyboardKey(int32_t x, int32_t y, int32_t width, int32_t height)
            : Button{x, y, width, height} {}

        void SetParentKeyboard(Keyboard* keyboard);
        Keyboard* GetParentKeyboard() const { return parentKeyboard; }

        void SetSecondaryText(const char* text);
        void SetSecondaryTextFont(Font const* font);
        void SetSecondaryTextColor(uint32_t color);
        void SetActiveSecondaryTextColor(uint32_t color);
        const char* GetSecondaryText();
        const char* GetCurrentValue() const { return currentTextValueIndex == 0 ? Text.c_str() : secondaryText.c_str(); }

        void SwitchKeyValue();

        Component* Clone() const override;

        static KeyboardKey* BuildFromXML(XMLElement* xmlElement);

    protected:
        Keyboard* parentKeyboard{nullptr};
        std::string secondaryText{};
        Font const* secondaryTextFont{nullptr};
        uint32_t secondaryTextColor{0};
        uint32_t activeSecondaryTextColor{0};

        unsigned int currentTextValueIndex{0};

        void DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight) override;
    };

} /* namespace grvl */

#endif /* GRVL_SWITCHBUTTON_H_ */
