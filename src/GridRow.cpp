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

#include "GridRow.h"
#include "Button.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    GridRow& GridRow::operator=(const GridRow& other)
    {
        if (this == &other) {
            return *this;
        }

        Container::operator=(other);
        ElementWidth = other.ElementWidth;
        HorizontalOffset = other.HorizontalOffset;

        childDropped = false;
        ignoreTouchModificator = false;
        lastActiveChild = nullptr;

        return *this;
    }

    Component* GridRow::Clone() const
    {
        return new GridRow(*this);
    }

    void GridRow::SetSize(int32_t width, int32_t height)
    {
        Width = width;
        Height = height;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->SetSize(Elements[i]->GetWidth(), height);
        }
        ReorderElements();
    }

    void GridRow::ReorderElements()
    {
        if(Elements.empty()) {
            return;
        }

        // The first algorithm
        int32_t distance = Width - Elements.size() * ElementWidth - (Elements.size() - 1) * HorizontalOffset;

        ignoreTouchModificator = distance < 0; // Distance between buttons allow to use touch area modificator

        for(uint32_t i = 0; i < Elements.size(); ++i) {
            int32_t XOffset = (ElementWidth + HorizontalOffset) * i;
            Elements[i]->SetPosition(XOffset, 0);
        }
    }

    void GridRow::AddElement(Component* item)
    {
        Container::AddElement(item);
        ReorderElements();
    }

    void GridRow::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        painter.PushDrawingBoundsStackElement(RenderX, RenderY, RenderX + Width, RenderY + Height);

        painter.FillRectangle(RenderX, RenderY, Width, Height, BackgroundColor);

        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->Draw(painter, RenderX, RenderY);
        }

        painter.PopDrawingBoundsStackElement();
    }

    void GridRow::SetElementWidth(int32_t elementWidth)
    {
        ElementWidth = elementWidth;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->SetSize(ElementWidth, Height);
        }
        ReorderElements();
    }

    void GridRow::SetHorizontalOffset(int32_t horizontalOffset)
    {
        HorizontalOffset = horizontalOffset;
        ReorderElements();
    }

    GridRow* GridRow::BuildFromXML(XMLElement* xmlElement)
    {
        GridRow* parent = new GridRow();

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            // TODO: we should verify right class
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }

        return parent;
    }

} /* namespace grvl */
