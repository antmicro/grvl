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
#include <grvl/container/Panel.h>

namespace grvl {

    Component* Panel::Clone() const
    {
        return new Panel(*this);
    }

    Panel* Panel::BuildFromXML(XmlElement xml)
    {
        Panel* parent = new Panel(0, 0, 0, 0);
        Manager* man = &Manager::GetInstance();

        parent->InitFromXML(xml);

        parent->SetSize(
            (int32_t)man->GetWidth(),
            xml.Attribute("height").ElseStyle().Parse<int32_t>().GetOr(30));

        for(XmlElement child : xml) {
            if(Component* element = component_from_xml(child)) {
                parent->AddElement(element);
            }
        }

        return parent;
    }

    void Panel::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        painter.PushDrawingBoundsStackElement(ParentRenderX, ParentRenderY, ParentRenderX + Width, ParentRenderY + Height);

        if(BackgroundImage) {
            BackgroundImage->Draw(painter, X + ParentRenderX, Y + ParentRenderY);
        } else {
            if(BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
                painter.FillRoundRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor, BorderArcRadius);
            } else {
                painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor);
            }
            DrawBorderIfNecessary(painter, X + ParentRenderX, Y + ParentRenderY, Width, Height);
            painter.AddBackgroundBlock(Y + ParentRenderY, Height, BackgroundColor);
        }

        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->Draw(painter, ParentRenderX + X, ParentRenderY + Y);
        }

        painter.PopDrawingBoundsStackElement();
    }

} /* namespace grvl */
