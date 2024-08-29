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

#include "Key.h"
#include "Manager.h"

namespace grvl {

    Key::~Key()
    {
    }

    void Key::SetID(const char* id)
    {
        ID = string(id);
    }

    const char* Key::GetID()
    {
        return ID.c_str();
    }

    void Key::SetOnPressEvent(const Event& event)
    {
        onPress = event;
    }

    void Key::SetOnReleaseEvent(const Event& event)
    {
        onRelease = event;
    }

    void Key::SetOnLongPressEvent(const Event& event)
    {
        onLongPress = event;
    }

    void Key::SetOnLongPressRepeatEvent(const Event& event)
    {
        onLongPressRepeat = event;
    }

    Key::KeyState Key::TriggerOnPressEvent()
    {
        if(onPress.IsSet()) {
            Manager::GetInstance().GetEventsQueueInstance().push(&onPress);
            return Key::KeyReleased;
        }
        return Key::NA;
    }

    Key::KeyState Key::TriggerOnReleaseEvent()
    {
        if(onRelease.IsSet()) {
            Manager::GetInstance().GetEventsQueueInstance().push(&onRelease);
            return Key::KeyReleased;
        }
        return Key::NA;
    }

    Key::KeyState Key::TriggerOnLongPressEvent()
    {
        if(onLongPress.IsSet()) {
            Manager::GetInstance().GetEventsQueueInstance().push(&onLongPress);
            return Key::KeyReleased;
        }
        return Key::NA;
    }

    Key::KeyState Key::TriggerOnLongPressRepeatEvent()
    {
        if(onLongPressRepeat.IsSet()) {
            Manager::GetInstance().GetEventsQueueInstance().push(&onLongPressRepeat);
            return Key::KeyReleased;
        }
        return Key::NA;
    }

    Key Key::BuildFromXML(XMLElement* xmlElement)
    {
        Key result = Key();
        Manager* man = &Manager::GetInstance();
        const char* tempChar = xmlElement->Attribute("id");
        if(tempChar) {
            result.SetID(tempChar);
        }

        result.SetOnReleaseEvent(man->GetEventWithArguments(xmlElement->Attribute("onRelease")));
        result.SetOnPressEvent(man->GetEventWithArguments(xmlElement->Attribute("onPress")));
        result.SetOnLongPressEvent(man->GetEventWithArguments(xmlElement->Attribute("onLongPress")));
        result.SetOnLongPressRepeatEvent(man->GetEventWithArguments(xmlElement->Attribute("onLongPressRepeat")));

        return result;
    }

    void Key::SetEventSenderPointer(void* sender)
    {
        onPress.SetSenderPointer(sender);
        onRelease.SetSenderPointer(sender);
    }

} /* namespace grvl */
