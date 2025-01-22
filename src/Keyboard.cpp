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

#include "Keyboard.h"

#include <cassert>

#include "KeyboardKey.h"

namespace grvl {

    Keyboard* Keyboard::BuildFromXML(XMLElement* xmlElement)
    {
        Keyboard* keyboard = new Keyboard();

        keyboard->InitFromXML(xmlElement);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            KeyboardKey* keyboardKey = dynamic_cast<KeyboardKey*>(create_component(child->Name(), (void*)child));
            assert(keyboardKey && "Child component must be valid keyboard key");
            keyboardKey->SetParentKeyboard(keyboard);
            keyboard->AddElement(keyboardKey);
        }
        return keyboard;
    }

    void Keyboard::SetCurrentInputDestination(TextInput* destinationInput)
    {
        currentDestinationInput = destinationInput;
    }

    void Keyboard::SwitchKeys()
    {
        for (auto& possibleKey : Elements) {
            if (auto key = dynamic_cast<KeyboardKey*>(possibleKey)) {
                key->SwitchKeyValue();
            }
        }
    }

} /* namespace grvl */
