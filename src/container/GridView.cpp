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

#include <grvl/Key.h>
#include <grvl/Manager.h>
#include <grvl/container/GridView.h>

#include <cassert>

namespace grvl {

    Component* GridView::Clone() const
    {
        return new GridView(*this);
    }

    GridView* GridView::BuildFromXML(XmlElement xml)
    {
        GridView* gridView = new GridView();

        gridView->InitFromXML(xml);

        gridView->SetScrolling(xml.Attribute("scrollingEnabled").ElseStyle().Parse<bool>().Else(false).Get());
        gridView->SetOverscrollBarColor(xml.Attribute("overscrollColor").ElseStyle().ParseColor().Else(COLOR_ARGB8888_LIGHTGRAY).Get());
        gridView->SetScrollIndicatorColor(xml.Attribute("scrollIndicatorColor").ElseStyle().ParseColor().Else(COLOR_ARGB8888_TRANSPARENT).Get());

        int32_t elementWidth = xml.Attribute("elementWidth").ElseStyle().Parse<int32_t>().Else(0).Get();
        int32_t elementHeight = xml.Attribute("elementHeight").ElseStyle().Parse<int32_t>().Else(0).Get();
        int32_t horizontalOffset = xml.Attribute("horizontalOffset").ElseStyle().Parse<int32_t>().Else(0).Get();
        int32_t verticalOffset = xml.Attribute("verticalOffset").ElseStyle().Parse<int32_t>().Else(0).Get();
        gridView->SetGridParameters(elementWidth, elementHeight, horizontalOffset, verticalOffset);

        for(XmlElement child : xml) {
            if(Component* component = component_from_xml(child)) {
                gridView->AddElement(component);
            }
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
