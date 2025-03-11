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

#include "AbstractView.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

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

    void AbstractView::InitFromXML(XMLElement* xmlElement)
    {
        Container::InitFromXML(xmlElement);
        Manager* man = &Manager::GetInstance();
        this->SetOnSlideToLeftEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSlideToLeft"))));
        this->SetOnSlideToRightEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSlideToRight"))));

        this->SetOnLongPressEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPress"))));
        this->SetOnLongPressRepeatEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPressRepeat"))));

        string collectionString = XMLSupport::GetAttributeOrDefault(xmlElement, "collection", "0/0");
        this->SetCollectionSize(XMLSupport::GetCollectionSize(collectionString));
        this->SetPositionInCollection(XMLSupport::GetPositionInCollection(collectionString));

        this->SetGlobalPanelVisibility(XMLSupport::GetAttributeOrDefault(xmlElement, "globalPanelVisible", true));

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            if(strcmp("key", child->Name()) == 0) {
                Key key(Key::BuildFromXML(child));
                key.SetEventSenderPointer(this);
                this->AddKeyToCollection(key);
                continue;
            }
        }
    }

} /* namespace grvl */
