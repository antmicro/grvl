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

#include <grvl/component/KeyboardKey.h>
#include <grvl/container/Keyboard.h>

#include <cassert>

namespace grvl {

    Keyboard* Keyboard::BuildFromXML(XmlElement xml)
    {
        Keyboard* keyboard = new Keyboard();

        keyboard->InitFromXML(xml);
        keyboard->SetAutoPopup(xml.Attribute("autoPopup").Parse<bool>().GetOr(true));

        for(XmlElement child : xml) {
            KeyboardKey* keyboardKey = dynamic_cast<KeyboardKey*>(component_from_xml(child));
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
        for(auto& possibleKey : Elements) {
            if(auto key = dynamic_cast<KeyboardKey*>(possibleKey)) {
                key->SwitchKeyValue();
            }
        }
    }

} /* namespace grvl */
