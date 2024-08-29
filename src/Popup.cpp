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

#include "Popup.h"

#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    Popup::~Popup()
    {
        delete Message;
    }

    Popup& Popup::operator=(const Popup& Obj)
    {
        if(this != &Obj) {
            Container::operator=(Obj);
            if(Message != NULL) {
                delete Message;
            }
            Message = new Label(*Obj.Message);
            PopupTimestamp = Obj.PopupTimestamp;
        }
        return *this;
    }

    void Popup::SetMessage(const char* message)
    {
        if(Message) {
            Message->SetText(message);
        }
    }

    Label* Popup::GetMessagePointer()
    {
        return Message;
    }

    // TODO: shouldn't popup be just a screen? i see duplicate code here
    Popup* Popup::BuildFromXML(XMLElement* xmlElement)
    {
        Popup* parent = new Popup();

        parent->InitFromXML(xmlElement);

        if(parent->GetMessagePointer()) {
            parent->GetMessagePointer()->SetBackgroundColor(parent->GetBackgroundColor());
        }

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }
        return parent;
    }

    void Popup::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        if(!Visible || Height <= 0 || Width <= 0)
            return;

        if(BackgroundImage) {
            BackgroundImage->Draw(painter, ParentX + X, ParentY + Y, Width, Height); // TODO: Needs some verification
        } else {
            painter.FillRectangle(X + ParentX, Y + ParentY, Width, Height, BackgroundColor);
        }

        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->Draw(painter, ParentX + X, ParentY + Y, Width, Height);
        }

        if(Message) {
            Message->Draw(painter, ParentX + X, ParentY + Y, Width, Height);
        }
    }

    void Popup::SetMessagePointer(Label* message)
    {
        Message = message;
    }

    void Popup::SetTimestamp(uint64_t currentTimestamp)
    {
        PopupTimestamp = currentTimestamp;
    }

    uint64_t Popup::GetTimetamp() const
    {
        return PopupTimestamp;
    }

} /* namespace grvl */
