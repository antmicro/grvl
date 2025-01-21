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

#include "CustomView.h"
#include "Manager.h"
#include "XMLSupport.h"
namespace grvl {

    CustomView* CustomView::BuildFromXML(XMLElement* xmlElement)
    {
        CustomView* parent = new CustomView();

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            if(strcmp(child->Name(), "header") == 0) {
                Panel* hdr = (Panel*)create_component("panel", (void*)child);
                parent->SetHeader(hdr);
            } else {
                Component* element = create_component(child->Name(), (void*)child);
                if(element)
                    parent->AddElement(element);
            }
        }

        parent->InitFromXML(xmlElement);

        return parent;
    }

    void CustomView::AddElement(Component* element)
    {
        Elements.push_back(element);
    }

    Component* CustomView::GetElement(uint32_t index)
    {
        if(index < Elements.size()) {
            return Elements[index];
        }
        return NULL;
    }

    Component* CustomView::GetLastElement()
    {
        if(!Elements.empty()) {
            return Elements.back();
        }
        return NULL;
    }

    Component* CustomView::GetElement(const char* id)
    {
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(strcmp(Elements[i]->GetID(), id) == 0) {
                return Elements[i];
            }

            if(CustomView* screen = dynamic_cast<CustomView*>(Elements[i])) {
                Component* cmp = screen->GetElement(id);
                if(cmp != NULL) {
                    return cmp;
                }
            }

            if(VerticalScrollView* screen = dynamic_cast<VerticalScrollView*>(Elements[i])) {
                Component* cmp = screen->GetElement(id);
                if(cmp != NULL) {
                    return cmp;
                }
            }
        }
        return NULL;
    }

    void CustomView::PrepareToOpen()
    {
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(AbstractView* screen = dynamic_cast<AbstractView*>(Elements[i])) {
                screen->PrepareToOpen();
            }
        }
    }
    void CustomView::PrepareToClose()
    {
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(AbstractView* screen = dynamic_cast<AbstractView*>(Elements[i])) {
                screen->PrepareToClose();
            }
        }
    }

    void CustomView::OnPress()
    {
        AbstractView::OnPress();
    }

    void CustomView::OnRelease()
    {
        AbstractView::OnRelease();
    }

    void CustomView::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        if(BackgroundImage && !BackgroundImage->IsEmpty()) {
            BackgroundImage->Draw(painter, X + ParentRenderX, Y + ParentRenderY);
        } else {
            painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor);
            painter.AddBackgroundBlock(ParentRenderY + Y, Height, BackgroundColor);
        }

        uint32_t i;
        uint32_t size;

        size = Elements.size();
        for(i = 0; i < size; i++) {
            Elements[i]->Draw(painter, ParentRenderX + X, ParentRenderY + Y);
        }

        painter.PopDrawingBoundsStackElement();
    }

    Touch::TouchResponse CustomView::ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator)
    {
        if(tp.GetState() == Touch::Pressed) { // init state, find child
            touchActive = false;
            childDropped = false;

            bool ownCheck = IsTouchPointInObject(tp.GetCurrentX() - ParentX, tp.GetCurrentY() - ParentY);

            if(ownCheck) { // Container selected
                touchActive = true;
                for(int32_t i = Elements.size() - 1; i >= 0; i--) {
                    Touch::TouchResponse touch = Elements[i]->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator); // Trigger onPress
                    if(Touch::TouchHandled == touch || Touch::LongTouchHandled == touch) {
                        lastActiveChild = Elements[i];
                        break;
                    }
                }
            } else {
                return Touch::TouchNotApplicable;
            }
        }
        static constexpr auto longPressDelay = 1000;
        if(TouchActivatedTimestamp != 0 && !longTouchActive && TouchActivatedTimestamp < (grvl::Callbacks()->get_timestamp() - longPressDelay)) { // Long press
            if(onLongPress.IsSet()) {
                longTouchActive = true;
                TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
                Manager::GetInstance().GetEventsQueueInstance().push(&onLongPress);
            } else {
                return Touch::TouchReleased;
            }
        }
        static constexpr auto longPressRepeatDelay = 500;
        if(longTouchActive && TouchActivatedTimestamp < (grvl::Callbacks()->get_timestamp() - longPressRepeatDelay)) { // Long press repeat
            TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
            Manager::GetInstance().GetEventsQueueInstance().push(&onLongPressRepeat);
        }

        if(longTouchActive) {
            return Touch::LongTouchHandled;
        }

        if(touchActive) {
            if(childDropped || lastActiveChild == NULL) { // Process data by screen - sliders.
                static constexpr auto deltaOffsetScale = 10;
                if(tp.GetState() != Touch::Released && abs(tp.GetDeltaY()) < Height / deltaOffsetScale
                   && abs(tp.GetDeltaX()) > Width / deltaOffsetScale) {
                    if(tp.GetDeltaX() > 0 && abs(tp.GetDeltaX()) > abs(tp.GetDeltaY())) {
                        Manager::GetInstance().GetEventsQueueInstance().push(&onSlideToLeft);
                        touchActive = false;
                        return Touch::TouchReleased;
                    }
                    if(tp.GetDeltaX() < 0 && abs(tp.GetDeltaX()) > abs(tp.GetDeltaY())) {
                        Manager::GetInstance().GetEventsQueueInstance().push(&onSlideToRight);
                        touchActive = false;
                        return Touch::TouchReleased;
                    }
                }
            } else { // Push data
                if(tp.GetState() != Touch::Pressed) { // Not to trigger onPress twice.
                    Touch::TouchResponse childResponse = lastActiveChild->ProcessTouch(tp, ParentX + X, ParentY + Y, modificator);
                    if(childResponse != Touch::TouchHandled && childResponse != Touch::LongTouchHandled) { // Drop
                        childDropped = true;
                        lastActiveChild = NULL;
                    }
                }
                return Touch::TouchHandled;
            }
        }
        return Touch::TouchNotApplicable;
    }

    void CustomView::CheckPlacement()
    {
        if(!grvl::Callbacks()->gui_printf) {
            return;
        }

        if(header) {
            header->CheckPlacement();
        }
        static constexpr auto bufferSize = 100;
        char elid[bufferSize];
        char pid[bufferSize];

        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(!Component::CheckSizeValidity(Elements[i])) {
                snprintf(elid, bufferSize, "%s", Elements[i]->GetID());
                snprintf(pid, bufferSize, "%s", this->GetID());
                if(strcmp(elid, "") == 0) {
                    sprintf(elid, "(unnamed)");
                }
                if(strcmp(pid, "") == 0) {
                    sprintf(pid, "(unnamed)");
                }
                grvl::Log("Element \"%s\" inside \"%s\" misplaced", elid, pid);
            }
            Elements[i]->CheckPlacement();
        }
    }

} /* namespace grvl */
