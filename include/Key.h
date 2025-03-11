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

#ifndef GRVL_KEY_H_
#define GRVL_KEY_H_

#include "Component.h"
#include "Event.h"
#include "XMLSupport.h"
#include "stl.h"
#include <stdint.h>

namespace grvl {

    /// Represents physical key.
    ///
    /// @remark
    /// This class can be used to register callbacks for pressing physical keys
    /// in XML document.
    ///
    /// XML events:
    /// * onPress                 - event invoked when the physical button is pressed
    /// * onRelease               - event invoked when the physical button is released
    /// * onLongPress             - event invoked when the physical button is pressed for longer than a second
    /// * onLongPressRepeat       - event invoked periodically (every half a second) while the physical button is being pressed
    ///
    class Key {
    public:
        enum KeyState {
            NA,
            KeyReleased,
            KeyPressed
        };

        virtual ~Key() = default;

        Key()
            : ID("")
            , onPress()
        {
        }

        static Key BuildFromXML(XMLElement* xmlElement);

        void SetID(const char* id);
        void SetOnPressEvent(const Event& event);
        void SetOnReleaseEvent(const Event& event);
        void SetOnLongPressEvent(const Event& event);
        void SetOnLongPressRepeatEvent(const Event& event);

        void SetEventSenderPointer(void* sender);

        KeyState TriggerOnPressEvent();
        KeyState TriggerOnReleaseEvent();
        KeyState TriggerOnLongPressEvent();
        KeyState TriggerOnLongPressRepeatEvent();

        const char* GetID();

    private:
        string ID;
        Event onPress, onRelease, onLongPress, onLongPressRepeat;
    };

} /* namespace grvl */

#endif /* GRVL_KEY_H_ */
