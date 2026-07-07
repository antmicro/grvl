// Copyright 2014-2025 Antmicro <antmicro.com>
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

#include <grvl/container/Division.h>
#include <grvl/Manager.h>
#include <grvl/XMLSupport.h>

namespace grvl {

    Component* Division::Clone() const
    {
        return new Division(*this);
    }

    Division* Division::BuildFromXML(XMLElement* xmlElement)
    {
        Division* parent = new Division(0, 0, 0, 0);
        Manager* man = &Manager::GetInstance();

        parent->InitFromXML(xmlElement);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }

        return parent;
    }

    void Division::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if (!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        painter.PushDrawingBoundsStackElement(RenderX, RenderY, RenderX + Width, RenderY + Height);

        DrawBackgroundItems(painter, RenderX, RenderY);
        DrawChildrenComponents(painter, RenderX, RenderY);

        painter.PopDrawingBoundsStackElement();
    }

    void Division::DrawBackgroundItems(Painter& painter, int32_t RenderX, int32_t RenderY)
    {
        FillBackground(painter, RenderX, RenderY);
        DrawBorderIfNecessary(painter, RenderX, RenderY, Width, Height);
        painter.AddBackgroundBlock(RenderY, Height, BackgroundColor);
    }

    void Division::FillBackground(Painter& painter, int32_t RenderX, int32_t RenderY)
    {
        if ((BackgroundColor & 0xFF000000) == 0) {
            return;
        }

        if (BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
            painter.FillRoundRectangle(RenderX, RenderY, Width, Height, BackgroundColor, BorderArcRadius);
        } else {
            painter.FillRectangle(RenderX, RenderY, Width, Height, BackgroundColor);
        }
    }

    void Division::DrawChildrenComponents(Painter& painter, int32_t RenderX, int32_t RenderY)
    {
        for (std::size_t i = 0; i < Elements.size(); ++i) {
            DrawChildComponent(Elements[i], painter, RenderX, RenderY);
        }
    }

    void Division::DrawChildComponent(Component* component, Painter& painter, int32_t RenderX, int32_t RenderY)
    {
        if (RenderY + component->GetY() + component->GetHeight() < RenderY) {
            return;
        }
        component->Draw(painter, RenderX, RenderY);
    }

} /* namespace grvl */
