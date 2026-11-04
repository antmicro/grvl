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

#include <grvl/Manager.h>
#include <grvl/container/AbstractView.h>

#include <charconv>

namespace grvl {

    CollectionDefinition CollectionDefinition::Parse(const std::string& definition)
    {
        const std::string_view view { definition };
        const int index = view.find_first_of("/", 0);

        if(index == -1) {
            Log(ERROR, "Missing '/' in collection definition '%s'", definition.c_str());
            return {};
        }

        const std::string_view first = view.substr(0, index);
        const std::string_view second = view.substr(index + 1);

        CollectionDefinition result {};

        if(std::from_chars(first.begin(), first.end(), result.position).ec != std::errc {}) {
            std::string copy { first };
            Log(WARN, "Invalid position '%s' in collection definition '%s'", copy.c_str(), definition.c_str());
        }

        if(std::from_chars(second.begin(), second.end(), result.size).ec != std::errc {}) {
            std::string copy { second };
            Log(WARN, "Invalid size '%s' in collection definition '%s'", copy.c_str(), definition.c_str());
        }

        return result;
    }

    AbstractView::~AbstractView()
    {
        delete header;
    }

    AbstractView& AbstractView::operator=(
        const AbstractView& Obj)
    {
        if(this != &Obj) { // TODO: Make it AbstractView-specific. Now it uses operator from Component.
            Component::operator=(Obj);
        }

        return *this;
    }

    void AbstractView::SetHeader(Panel* headerPtr)
    {
        header = headerPtr;
    }

    void AbstractView::SetGlobalPanelVisibility(bool visible)
    {
        globalPanelVisible = visible;
    }

    bool AbstractView::GetGlobalPanelVisibility() const
    {
        return globalPanelVisible;
    }

    Panel* AbstractView::GetHeader()
    {
        return header;
    }

    void AbstractView::SetOnSlideToLeftEvent(const Event& event)
    {
        onSlideToLeft = event;
        onSlideToLeft.SetSenderPointer(this);
    }

    void AbstractView::SetOnSlideToRightEvent(const Event& event)
    {
        onSlideToRight = event;
        onSlideToRight.SetSenderPointer(this);
    }

    uint32_t AbstractView::GetCollectionSize() const
    {
        return collectionSize;
    }

    uint32_t AbstractView::GetPositionInCollection() const
    {
        return positionInCollection;
    }

    void AbstractView::SetCollectionSize(uint32_t size)
    {
        collectionSize = size;
    }

    void AbstractView::SetPositionInCollection(uint32_t position)
    {
        positionInCollection = position;
    }

    void AbstractView::AddKeyToCollection(const Key& key)
    {
        KeyCollection.push_back(key);
    }

    Key::KeyState AbstractView::PressKey(const char* id)
    {
        for(uint32_t i = 0; i < KeyCollection.size(); i++) {
            if(strcmp(KeyCollection[i].GetID(), id) == 0) {
                KeyCollection[i].TriggerOnPressEvent();
                return Key::KeyPressed;
            }
        }
        return Key::NA;
    }

    Key::KeyState AbstractView::ReleaseKey(const char* id)
    {
        for(uint32_t i = 0; i < KeyCollection.size(); i++) {
            if(strcmp(KeyCollection[i].GetID(), id) == 0) {
                KeyCollection[i].TriggerOnReleaseEvent();
                return Key::KeyReleased;
            }
        }
        return Key::NA;
    }

    Key::KeyState AbstractView::LongPressKey(const char* id)
    {
        for(uint32_t i = 0; i < KeyCollection.size(); i++) {
            if(strcmp(KeyCollection[i].GetID(), id) == 0) {
                KeyCollection[i].TriggerOnLongPressEvent();
                return Key::KeyReleased;
            }
        }
        return Key::NA;
    }

    Key::KeyState AbstractView::LongPressRepeatKey(const char* id)
    {
        for(uint32_t i = 0; i < KeyCollection.size(); i++) {
            if(strcmp(KeyCollection[i].GetID(), id) == 0) {
                KeyCollection[i].TriggerOnLongPressRepeatEvent();
                return Key::KeyReleased;
            }
        }
        return Key::NA;
    }

    void AbstractView::ClearTouch()
    {
        if(touchActive && lastActiveChild) {
            lastActiveChild->ClearTouch();
            childDropped = false;
            lastActiveChild = NULL;
        }
    }

    void AbstractView::OnPress()
    {
        TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
        Component::OnPress();
    }

    void AbstractView::OnRelease()
    {
        TouchActivatedTimestamp = 0;
        longTouchActive = false;
        Component::OnRelease();
    }

    void AbstractView::SetOnLongPressEvent(const Event& event)
    {
        onLongPress = event;
        onLongPress.SetSenderPointer(this);
    }

    void AbstractView::SetOnLongPressRepeatEvent(const Event& event)
    {
        onLongPressRepeat = event;
        onLongPressRepeat.SetSenderPointer(this);
    }

    void AbstractView::InitFromXML(XmlElement xml)
    {
        Container::InitFromXML(xml);
        Manager* man = &Manager::GetInstance();

        this->SetOnSlideToLeftEvent(man->GetOrCreateCallback(xml.Attribute("onSlideToLeft").ParseCallback().GetOr({})));
        this->SetOnSlideToRightEvent(man->GetOrCreateCallback(xml.Attribute("onSlideToRight").ParseCallback().GetOr({})));

        this->SetOnLongPressEvent(man->GetOrCreateCallback(xml.Attribute("onLongPress").ParseCallback().GetOr({})));
        this->SetOnLongPressRepeatEvent(man->GetOrCreateCallback(xml.Attribute("onLongPressRepeat").ParseCallback().GetOr({})));

        std::string definition = xml.Attribute("collection").ElseStyle().GetOr("0/0");
        CollectionDefinition collection = CollectionDefinition::Parse(definition);
        this->SetCollectionSize(collection.size);
        this->SetPositionInCollection(collection.position);

        this->SetGlobalPanelVisibility(xml.Attribute("globalPanelVisible").ElseStyle().Parse<bool>().GetOr(true));

        for(XmlElement child : xml.ForEach("key")) {
            Key key(Key::BuildFromXML(child));
            key.SetEventSenderPointer(this);
            this->AddKeyToCollection(key);
        }
    }

} /* namespace grvl */
