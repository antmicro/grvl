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

#include "GridView.h"
#include "Key.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    GridView::~GridView()
    {
        Elements_vec::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }
    }

    GridView* GridView::BuildFromXML(XMLElement* xmlElement)
    {
        GridView* parent = new GridView();

        parent->InitFromXML(xmlElement);

        parent->SetScrolling(XMLSupport::GetAttributeOrDefault(xmlElement, "scrollingEnabled", false));
        parent->SetOverscrollBarColor(
            XMLSupport::ParseColor(xmlElement, "overscrollColor", (uint32_t)COLOR_ARGB8888_LIGHTGRAY));
        parent->SetScrollIndicatorColor(
            XMLSupport::ParseColor(
                xmlElement, "scrollIndicatorColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));

        int32_t elementWidth;
        int32_t elementHeight;
        int32_t verticalOffset;
        elementWidth = XMLSupport::GetAttributeOrDefault(xmlElement, "elementWidth", (uint32_t)0);
        elementHeight = XMLSupport::GetAttributeOrDefault(xmlElement, "elementHeight", (uint32_t)0);
        verticalOffset = XMLSupport::GetAttributeOrDefault(xmlElement, "verticalOffset", (uint32_t)0);
        parent->SetGridParameters(elementWidth, elementHeight, verticalOffset);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            // TODO: we should verify correct class
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }

        return parent;
    }

    void GridView::SetGridParameters(uint32_t width, uint32_t height, uint32_t verticalOffset)
    {
        ElementWidth = width;
        ElementHeight = height;
        VerticalOffset = verticalOffset;
    }

    void GridView::AddElement(Component* item)
    {
        ((GridRow*)item)->SetPosition(0, itemsHeight + VerticalOffset); // Position at the end of the list
        ((GridRow*)item)->SetSize(Width, ElementHeight);
        ((GridRow*)item)->SetElementWidth(ElementWidth);
        ((GridRow*)item)->SetBackgroundColor(BackgroundColor);

        itemsHeight += ElementHeight + VerticalOffset;
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }

        Elements.push_back((GridRow*)item);
    }

    void GridView::SetBackgroundColor(uint32_t color)
    {
        BackgroundColor = color;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->SetBackgroundColor(color);
        }
    }
} /* namespace grvl */
