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

    GridRow::~GridRow()
    {
        vector<Component*>::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }
    }

    Component* GridRow::GetElement(const char* id)
    {
        Component* Element = NULL;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(strcmp(Elements[i]->GetID(), id) == 0) {
                Element = Elements[i];
            }
        }
        return Element;
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
        int32_t distance = Width - (Elements.size() * ElementWidth);

        if(distance >= 0) {
            ignoreTouchModificator = false; // Distance between buttons allow to use touch area modificator
            distance /= Elements.size() + 1;

            int32_t offset = 0;
            for(uint32_t i = 0; i < Elements.size(); i++) {
                offset += distance + ElementWidth * (i > 0);
                Elements[i]->SetPosition(offset, 0);
            }
        } else {
            // The second one
            ignoreTouchModificator = true; // Distance between buttons too small
            int32_t distance = Width / (Elements.size() + 1);

            for(uint32_t i = 0; i < Elements.size(); i++) {
                int32_t offset = distance * (i + 1);
                Elements[i]->SetPosition(offset - (Elements[i]->GetWidth() / 2), 0);
            }
        }
    }

    void GridRow::AddElement(Component* gitem)
    {
        AbstractButton* item = (AbstractButton*)gitem;
        if(item) {
            Elements.push_back(item);
        }
        ReorderElements();
    }

    Touch::TouchResponse GridRow::ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator)
    {
        if(tp.GetState() == Touch::Pressed) { // init state, find child
            touchActive = false;
            childDropped = false;

            bool ownCheck = IsTouchPointInObject(tp.GetCurrentX() - ParentX, tp.GetCurrentY() - ParentY);

            if(ownCheck) { // Container selected
                touchActive = true;
                for(int32_t i = Elements.size() - 1; i >= 0; i--) {
                    if(Touch::TouchHandled == Elements[i]->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator)) { // Trigger onPress
                        activeChild = Elements[i];
                        break;
                    }
                }
            } else {
                return Touch::TouchNA;
            }
        }

        if(touchActive) {
            if(childDropped || activeChild == NULL) { // Process data by screen - sliders.
                Touch::TouchResponse touch = Component::ProcessMove(tp.GetStartX() - ParentX - X, tp.GetStartY() - ParentY - Y, tp.GetDeltaX(), tp.GetDeltaY());
                if(touch == Touch::TouchReleased) {
                    touchActive = false;
                }
                return touch;
            } // Push data
            if(tp.GetState() != Touch::Pressed) { // Not to trigger onPress twice.
                Touch::TouchResponse childResponse = activeChild->ProcessTouch(
                    tp, ParentX + X, ParentY + Y, ignoreTouchModificator ? 0 : modificator);
                if(childResponse == Touch::TouchReleased || childResponse == Touch::TouchNA) { // Drop
                    childDropped = true;
                    activeChild = NULL;
                }
            }
            return Touch::TouchHandled;
        }
        return Touch::TouchNA;
    }

    void GridRow::CheckPlacement()
    {
    }

    void GridRow::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        if(!Visible) {
            return;
        }

        int32_t HeightClamped;
        bool CutTop = false;
        int32_t OffsetY = 0;

        if(ParentHeight < 0) {
            CutTop = true;
            OffsetY = -ParentHeight;
        }

        if(!CutTop) {
            HeightClamped = Clamp(Height, 0, ParentHeight);
        } else { // If negative height
            HeightClamped = Clamp(Height, 0, Height - OffsetY);
        }

        if(HeightClamped == 0) {
            return;
        }

        painter.FillRectangle(ParentX + X, OffsetY + ParentY + Y, ParentWidth, HeightClamped, BackgroundColor);

        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->Draw(painter, ParentX + X, ParentY + Y, ParentWidth, ParentHeight);
        }
    }

    void GridRow::SetElementWidth(int32_t elementWidth)
    {
        ElementWidth = elementWidth;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            Elements[i]->SetSize(ElementWidth, Height);
        }
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
        parent->SetBackgroundColor(
            XMLSupport::ParseColor(
                xmlElement, "backgroundColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));

        return parent;
    }

} /* namespace grvl */
