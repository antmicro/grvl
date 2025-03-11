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
        if(this == &Obj) {
            return *this;
        }

        Container::operator=(Obj);

        if(Message != NULL) {
            delete Message;
        }
        Message = new Label(*Obj.Message);

        PopupTimestamp = Obj.PopupTimestamp;

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

    void Popup::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t renderX = ParentRenderX + X;
        int32_t renderY = ParentRenderY + Y;

        painter.PushDrawingBoundsStackElement(renderX, renderY, renderX + Width, renderY + Height);

        if(BackgroundImage) {
            BackgroundImage->Draw(painter, renderX, renderY);
        } else {
            painter.FillRectangle(renderX, renderY, Width, Height, BackgroundColor);
        }

        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->Draw(painter, renderX, renderY);
        }

        if(Message) {
            Message->Draw(painter, renderX, renderY);
        }

        painter.PopDrawingBoundsStackElement();
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
