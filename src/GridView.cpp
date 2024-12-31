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

#include <cassert>

#include "Key.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    Component* GridView::Clone() const
    {
        return new GridView(*this);
    }

    GridView* GridView::BuildFromXML(XMLElement* xmlElement)
    {
        GridView* gridView = new GridView();

        gridView->InitFromXML(xmlElement);

        gridView->SetScrolling(XMLSupport::GetAttributeOrDefault(xmlElement, "scrollingEnabled", false));
        gridView->SetOverscrollBarColor(XMLSupport::ParseColor(xmlElement, "overscrollColor", (uint32_t)COLOR_ARGB8888_LIGHTGRAY));
        gridView->SetScrollIndicatorColor(XMLSupport::ParseColor(xmlElement, "scrollIndicatorColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));

        int32_t elementWidth = XMLSupport::GetAttributeOrDefault(xmlElement, "elementWidth", (uint32_t)0);
        int32_t elementHeight = XMLSupport::GetAttributeOrDefault(xmlElement, "elementHeight", (uint32_t)0);
        int32_t horizontalOffset = XMLSupport::GetAttributeOrDefault(xmlElement, "horizontalOffset", (uint32_t)0);
        int32_t verticalOffset = XMLSupport::GetAttributeOrDefault(xmlElement, "verticalOffset", (uint32_t)0);
        gridView->SetGridParameters(elementWidth, elementHeight, horizontalOffset, verticalOffset);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            Component* component = create_component(child->Name(), (void*)child);
            gridView->AddElement(component);
        }

        return gridView;
    }

    void GridView::SetGridParameters(uint32_t width, uint32_t height, uint32_t horizontalOffset, uint32_t verticalOffset)
    {
        ElementWidth = width;
        ElementHeight = height;
        HorizontalOffset = horizontalOffset;
        VerticalOffset = verticalOffset;
    }

    void GridView::AddElement(Component* component)
    {
        GridRow* gridRow = dynamic_cast<GridRow*>(component);
        assert(gridRow && "Child components of GridView must be/inherit GridRow class");

        unsigned int currentRowVerticalGap = Elements.empty() ? 0 : VerticalOffset;

        gridRow->SetPosition(0, itemsHeight + currentRowVerticalGap); // Position at the end of the list
        gridRow->SetSize(Width, ElementHeight);
        gridRow->SetElementWidth(ElementWidth);
        gridRow->SetHorizontalOffset(HorizontalOffset);
        gridRow->SetBackgroundColor(BackgroundColor);

        itemsHeight += ElementHeight + currentRowVerticalGap;
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }

        gridRow->SetAsSelection(IsSelection());
        Container::AddElement(component);
    }

    void GridView::SetBackgroundColor(uint32_t color)
    {
        BackgroundColor = color;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->SetBackgroundColor(color);
        }
    }
} /* namespace grvl */
