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

#include "Panel.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    Panel::~Panel()
    {
    }

    Panel* Panel::BuildFromXML(XMLElement* xmlElement)
    {
        Panel* parent = new Panel(0, 0, 0, 0);
        Manager* man = &Manager::GetInstance();

        parent->InitFromXML(xmlElement);

        int32_t hValue;
        if(xmlElement->QueryIntAttribute("height", (int*)&hValue) != XML_SUCCESS) {
            static constexpr auto defaultHeight = 30;
            hValue = defaultHeight;
        }
        parent->SetSize((int32_t)man->GetWidth(), hValue);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }


        return parent;
    }

    void Panel::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        if(!Visible || Height <= 0 || Width <= 0)
            return;

        if(BackgroundImage) {
            BackgroundImage->Draw(painter, ParentX, ParentY, ParentWidth, ParentHeight); // TODO: Needs verification
        } else {
            painter.FillRectangle(X + ParentX, Y + ParentY, ParentWidth, ParentHeight, BackgroundColor);
            painter.AddBackgroundBlock(Y + ParentY, ParentHeight, BackgroundColor);
        }

        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->Draw(painter, ParentX + X, ParentY + Y, Width, Height);
        }
    }

} /* namespace grvl */
