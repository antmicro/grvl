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

#include "VerticalScrollView.h"
#include "Image.h"
#include "Manager.h"
//NOLINTBEGIN
#define min(x, y) (x < y ? x : y)
#define max(x, y) (x < y ? y : x)
//NOLINTEND
namespace grvl {

    void VerticalScrollView::AddElement(Component* item)
    {
        AdjustScrollViewHeight(item);
        item->SetParentID(GetID());
        Elements.push_back(item);
    }

    void VerticalScrollView::AdjustScrollViewHeight(Component* child)
    {
        itemsHeight += child->GetHeight();
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }
    }

    void VerticalScrollView::RemoveElement(const char* elementId)
    {
        Component* foundComponent{nullptr};
        for (int index = 0; index < Elements.size(); ++index) {
            if (strcmp(Elements[index]->GetID(), elementId) == 0) {
                foundComponent = Elements[index];
                Elements.erase(Elements.begin() + index);
                break;
            }
        }

        if (!foundComponent) {
            return;
        }

        itemsHeight -= foundComponent->GetHeight();
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        }

        delete foundComponent;
    }

    void VerticalScrollView::SetScrolling(bool enable)
    {
        scrollingEnabled = enable;
    }
    int32_t VerticalScrollView::GetScrollingValue() const
    {
        return Scroll;
    }

    void VerticalScrollView::SetScrollingValue(int32_t scrollVal)
    {
        int32_t indicatorSizeDiff = 0;
        int32_t previousOverscrollBarSize = currentOverscrollBarSize;
        if(overscrollBarEnabled && ScrollMax > 0) {
            if(Scroll == 0) {
                int tempScrollVal = scrollVal;
                if(tempScrollVal < 0)
                    tempScrollVal /= 2;
                currentOverscrollBarSize += tempScrollVal;
            } else if(Scroll == ScrollMax) {
                int tempScrollVal = scrollVal - ScrollMax;
                if(tempScrollVal > 0) {
                    tempScrollVal /= 2;
                }
                currentOverscrollBarSize += tempScrollVal;
            } else {
                currentOverscrollBarSize = 0;
            }

            if(currentOverscrollBarSize > 0) {
                currentOverscrollBarSize = Clamp(currentOverscrollBarSize, 0, overscrollBarSize);
            } else {
                currentOverscrollBarSize = Clamp(currentOverscrollBarSize, -overscrollBarSize, 0);
            }

            indicatorSizeDiff = currentOverscrollBarSize - previousOverscrollBarSize;
        }
        scrollVal = Clamp(scrollVal + currentOverscrollBarSize, 0, ScrollMax);
        ScrollChange += scrollVal - Scroll + indicatorSizeDiff;
        Scroll = scrollVal;
    }

    void VerticalScrollView::SetSize(int32_t width, int32_t height)
    {
        if(Width != width || Height != height) {
            Component::SetSize(width, height);
            for(uint32_t i = 0; i < Elements.size(); i++) {
                Elements[i]->SetSize(width, Elements[i]->GetHeight());
            }

            if(Height < itemsHeight) {
                ScrollMax = itemsHeight - Height;
            } else {
                ScrollMax = 0;
            }

            if(ScrollMax < Scroll)
                Scroll = ScrollMax;
        }
    }

    void VerticalScrollView::SetScrollIndicatorColor(uint32_t color)
    {
        scrollIndicatorColor = color;
    }

    Component* VerticalScrollView::GetElement(uint32_t index)
    {
        if(index < Elements.size()) {
            return Elements[index];
        }
        return 0;
    }

    Component* VerticalScrollView::GetElement(const char* id)
    {
        for(auto& CurrentElement : Elements) {
            if(Component* component = TryToGetElementFromChildContainer(CurrentElement, id)) {
                return component;
            }

            if(strcmp(CurrentElement->GetID(), id) == 0) {
                return CurrentElement;
            }
        }
        return nullptr;
    }

    Component* VerticalScrollView::TryToGetElementFromChildContainer(Component* possible_container, const char* searched_component_id)
    {
        if (Container* container = dynamic_cast<Container*>(possible_container)) {
            return container->GetElement(searched_component_id);
        }

        return nullptr;
    }

    Component* VerticalScrollView::GetLastElement()
    {
        if(!Elements.empty()) {
            return Elements.back();
        }
        return 0;
    }

    uint32_t VerticalScrollView::GetItemsCount()
    {
        return Elements.size();
    }

    void VerticalScrollView::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        painter.PushDrawingBoundsStackElement(ParentRenderX + X, ParentRenderY + Y, ParentRenderX + X + Width, ParentRenderY + Y + Height);

        static constexpr auto speedMultipler = 30;
        static constexpr auto speedScale = 1000;
        if(animation < 0) {
            dSpeed += speedMultipler * ((float)(grvl::Callbacks()->get_timestamp() - scrollingTimestamp) / speedScale);
            if(dSpeed > 0 || Scroll == 0) {
                dSpeed = 0;
                animation = 0;
            }
            scrollingTimestamp = grvl::Callbacks()->get_timestamp();
            SetScrollingValue(Scroll + dSpeed);
        } else if(animation > 0) {
            dSpeed -= speedMultipler * ((float)(grvl::Callbacks()->get_timestamp() - scrollingTimestamp) / speedScale);
            if(dSpeed < 0 || Scroll == ScrollMax) {
                dSpeed = 0;
                animation = 0;
            }
            scrollingTimestamp = grvl::Callbacks()->get_timestamp();
            SetScrollingValue(Scroll + dSpeed);
        }

        int tempScrollChange = ScrollChange; // Store current values
        ScrollChange = 0; // Start collecting new position changes
        int tempCurrentOverscrollSize = currentOverscrollBarSize;
        int tempScroll = Scroll + tempCurrentOverscrollSize;

        bool DrawHLine = true;

        const int ScrollChangeMultiplicator = 2; // predicting a required image content
        grvl::Callbacks()->mutex_lock(ClearWhileDrawMutex);
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(!Elements[i]->IsVisible()) {
                continue;
            }
            // Element to high - not visible
            if(Elements[i]->GetY() + Elements[i]->GetHeight() < tempScroll) {
                if(Elements[i]->GetY() + Elements[i]->GetHeight()
                   < tempScroll - abs(tempScrollChange) * ScrollChangeMultiplicator) {
                    continue;
                }

                // Request new images - scrolling up
                if(tempScrollChange < 0) {
                    Elements[i]->PrepareContent(painter.GetContentManager());
                }

                // Cancel request for passed images - scrolling down
                if(tempScrollChange > 0) {
                    Elements[i]->CancelPreparingContent(painter.GetContentManager());
                }
            }

            // Element to low - not visible
            else if(Elements[i]->GetY() > tempScroll + Height) {
                if(Elements[i]->GetY() > tempScroll + Height + abs(tempScrollChange) * ScrollChangeMultiplicator) {
                    continue; // No need to check more elements
                }

                // Request new images - scrolling down
                if(tempScrollChange > 0) {
                    Elements[i]->PrepareContent(painter.GetContentManager());
                }

                // Cancel request for passed images - scrolling up
                if(tempScrollChange < 0) {
                    Elements[i]->CancelPreparingContent(painter.GetContentManager());
                }

                continue;
            }

            // Visible elements
            else {
                int32_t ButtonHeight = 0;

                // Redraw new elements
                // top part of element invisible
                if(Elements[i]->GetY() < tempScroll) {
                    ButtonHeight = Elements[i]->GetY() - tempScroll;
                    DrawHLine = true;
                    int realButtonHeight = Elements[i]->GetHeight() + ButtonHeight;
                    painter.AddBackgroundBlock(
                        ParentRenderY + Y - tempScroll + Elements[i]->GetY() + Elements[i]->GetHeight() - realButtonHeight,
                        realButtonHeight, Elements[i]->GetCurrentBackgroundColor());
                }

                // bottom part of element invisible
                else if(Elements[i]->GetY() + Elements[i]->GetHeight() > tempScroll + Height) {
                    ButtonHeight = Elements[i]->GetHeight()
                        - (Elements[i]->GetY() + Elements[i]->GetHeight() - (tempScroll + Height));
                    DrawHLine = false;
                    painter.AddBackgroundBlock(
                        ParentRenderY + Y - tempScroll + Elements[i]->GetY(), ButtonHeight,
                        Elements[i]->GetCurrentBackgroundColor());
                }

                // center element - visible
                else {
                    ButtonHeight = Elements[i]->GetHeight();
                    DrawHLine = true;
                    painter.AddBackgroundBlock(
                        ParentRenderY + Y - tempScroll + Elements[i]->GetY(), Elements[i]->GetHeight(),
                        Elements[i]->GetCurrentBackgroundColor());
                }

                Elements[i]->Draw(painter, ParentRenderX + X, ParentRenderY + Y - tempScroll);

                if(i == Elements.size() - 1) {
                    DrawHLine = false;
                }

                if(DrawHLine && SplitLineColor > 0) {
                    painter.DrawHLine(
                        ParentRenderX + X, ParentRenderY + Y + Elements[i]->GetY() + Elements[i]->GetHeight() - tempScroll - 1,
                        Width, SplitLineColor);
                }
            }
        }
        grvl::Callbacks()->mutex_unlock(ClearWhileDrawMutex);
        if(overscrollBarEnabled && tempCurrentOverscrollSize > 0 && tempScroll - tempCurrentOverscrollSize == ScrollMax) {
            painter.FillRectangle(
                ParentRenderX + X, ParentRenderY + Y + Height - tempCurrentOverscrollSize, Width, tempCurrentOverscrollSize, overscrollBarColor);
            painter.AddBackgroundBlock(
                ParentRenderY + Y + Height - tempCurrentOverscrollSize, tempCurrentOverscrollSize, overscrollBarColor);
        } else if(overscrollBarEnabled && tempCurrentOverscrollSize < 0 && tempScroll - tempCurrentOverscrollSize == 0) {
            painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, -tempCurrentOverscrollSize, overscrollBarColor);
            painter.AddBackgroundBlock(ParentRenderY + Y, -tempCurrentOverscrollSize, overscrollBarColor);
        }

        // Scroll bar
        if(scrollingEnabled && ScrollMax > 0 && scrollIndicatorColor != 0 && scrollIndicatorImage) {
            uint64_t now = grvl::Callbacks()->get_timestamp();
            if((animation != 0 || touchActive) && (itemsHeight > Height)) {
                scrollIndicatorOpacity = (0xFF000000 & scrollIndicatorColor) >> 24; //NOLINT
                scrollIndicatorTimestamp = now;
            }

            if(scrollIndicatorOpacity > 0) {
                int scrollIndicatorY = (float)tempScroll / ((float)itemsHeight - (float)Height)
                    * (Height - scrollIndicatorImage->GetHeight());
                // bound value to be sure
                scrollIndicatorY = max(0, min(scrollIndicatorY, Height - (int32_t)scrollIndicatorImage->GetHeight()));
                // fade out
                uint32_t tempColor = (scrollIndicatorColor & 0x00FFFFFF) | (scrollIndicatorOpacity << 24); //NOLINT
                static constexpr auto scrollXOffset = 16;
                int scrollX = ParentRenderX + X + Width - scrollXOffset;
                int scrollY = ParentRenderY + Y + scrollIndicatorY;

                // Fill allocated space
                painter.FillMemory((uintptr_t)scrollIndicatorImage->GetData(),
                                   scrollIndicatorImage->GetWidth(),
                                   scrollIndicatorImage->GetHeight(),
                                   tempColor);

                // Check image rotation
                if(painter.IsRotated()) {
                    if(!scrollIndicatorImage->IsRotated()) {
                        scrollIndicatorImage->Rotate90();
                    }
                }

                // Transfer scroll indicator
                painter.DmaMoveImage((uintptr_t)scrollIndicatorImage->GetData(), painter.GetActiveBuffer(),
                                     0, 0, scrollX, scrollY, scrollIndicatorImage->GetWidth(), scrollIndicatorImage->GetHeight(),
                                     scrollIndicatorImage->GetPixelsPerLine(), scrollIndicatorImage->GetNumberOfLines(), 0, 1,
                                     scrollIndicatorImage->GetColorFormat(), painter.GetPixelFormat(), true);

                // Draw scroll indicator
                static constexpr auto scrollDelay = 800;

                if(now > scrollIndicatorTimestamp + scrollDelay) { // TODO: Introduce parameter
                    float tempValue = speedMultipler * ((float)(now - scrollIndicatorTimestamp) / speedScale);
                    if((int)scrollIndicatorOpacity - tempValue > 0) { // Overflow check
                        scrollIndicatorOpacity -= tempValue;
                    } else {
                        scrollIndicatorOpacity = 0;
                    }
                }
            }
        }

        // Clear empty space over a list.
        if(ScrollMax == 0 && itemsHeight < Height) {
            painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y + itemsHeight, Width, Height - itemsHeight, BackgroundColor);
            painter.AddBackgroundBlock(ParentRenderY + Y + itemsHeight, Height - itemsHeight, BackgroundColor);
        }

        painter.PopDrawingBoundsStackElement();
    }

    void VerticalScrollView::PrepareToOpen()
    {
        Scroll = 0;
    }

    void VerticalScrollView::PrepareToClose()
    {
        scrollIndicatorOpacity = 0;
    }

    int32_t VerticalScrollView::GetScrollMaxValue() const
    {
        return ScrollMax;
    }

    void VerticalScrollView::SetSplitLineColor(uint32_t color)
    {
        SplitLineColor = color;
    }

    Touch::TouchResponse VerticalScrollView::ProcessTouch(const Touch& tp, int32_t ParentRenderX, int32_t ParentRenderY,
                                                          int32_t modificator)
    {
        if(tp.GetState() == Touch::Pressed) {
            if(!animation) { // still scrolling
                touchActive = false;
                childDropped = false;
                prevDeltaY = 0;
                prevDeltaX = 0;
                animation = 0;
                dSpeed = 0;
                ClearSelection();
                currentOverscrollBarSize = 0;

                bool ownCheck = IsTouchPointInObject(tp.GetCurrentX() - ParentRenderX, tp.GetCurrentY() - ParentRenderY);

                if(ownCheck) { // Container selected
                    touchActive = true;
                    grvl::Callbacks()->mutex_lock(ClearWhileTouchMutex);
                    for(uint32_t i = 0; i < Elements.size(); i++) {
                        Touch::TouchResponse touch = Elements[i]->ProcessTouch(tp, ParentRenderX + X, ParentRenderY + Y - Scroll);
                        if(Touch::TouchHandled == touch || Touch::LongTouchHandled == touch) {
                            SetCurrentlySelectedComponent(Elements[i]);
                            grvl::Callbacks()->mutex_unlock(ClearWhileTouchMutex);
                            return Touch::TouchHandled; // Child took control
                        }
                    }
                    grvl::Callbacks()->mutex_unlock(ClearWhileTouchMutex);
                    childDropped = true;
                    return Touch::TouchHandled;
                }
                return Touch::TouchNotApplicable;
            }
            animation = 0;
            touchActive = true; // Continue scrolling
            prevDeltaY = tp.GetDeltaY();
        }

        if(!touchActive) {
            return Touch::TouchNotApplicable;
        }

        if(lastActiveChild && !childDropped) { // push data to the child

            Touch::TouchResponse res = lastActiveChild->ProcessTouch(tp, ParentRenderX + X, ParentRenderY + Y - Scroll);

            if(res == Touch::TouchReleased || res == Touch::TouchNotApplicable) {
                childDropped = true;

                // Skip already scrolled distance - avoid glitchy adjustment
                prevDeltaY = tp.GetDeltaY();
            }
        }

        if(childDropped) {
            int deltaY = prevDeltaY - tp.GetDeltaY();
            prevDeltaY = tp.GetDeltaY();
            int deltaX = tp.GetDeltaX() - prevDeltaX;
            prevDeltaX = tp.GetDeltaX();

            // Lock list while scrolling by finger
            static constexpr auto scrollingByFingerInertiaScale = 10;
            if(abs(tp.GetDeltaY()) >= Height / scrollingByFingerInertiaScale && !scrollingByFinger) {
                scrollingByFinger = true;
            }

            // Slide Event
            if(abs(deltaY) < Height / scrollingByFingerInertiaScale && abs(deltaX) > Width / scrollingByFingerInertiaScale && !scrollingByFinger) {
                if(deltaX > 0) {
                    Manager::GetInstance().GetEventsQueueInstance().push(&onSlideToLeft);
                } else if(deltaX < 0) {
                    Manager::GetInstance().GetEventsQueueInstance().push(&onSlideToRight);
                }

                touchActive = false;
                prevDeltaY = 0;
                prevDeltaX = 0;
                currentOverscrollBarSize = 0;
                scrollIndicatorOpacity = 0;
                scrollIndicatorTimestamp = 0;
                return Touch::TouchReleased;
            }

            int avg = 0;
            // NOLINTBEGIN
            if(scrollingEnabled) {
                if(deltaY != 0) {
                    // calculate scrolling speed by sampling 3 Touch::Moving Events
                    const int SAMPLES = 3;
                    speedSamples[++currentSample % SAMPLES] = deltaY;
                    int validSamples = 0;
                    for(int i = 0; i < SAMPLES; i++) {
                        if(speedSamples[i] != 0) {
                            avg += speedSamples[i];
                            ++validSamples;
                        }
                    }
                    if(validSamples < 3 && abs(avg) > 2) {
                        // undersampling, assume high acceleration due to a very short touch
                        avg = 45 * (avg < 0 ? -1 : 1);
                    } else {
                        avg /= validSamples;
                        // acceleration factor (experimental)
                        avg *= 1.2f;
                        if(abs(avg) < 25) {
                            // increase slow speeds, to force a user feedback
                            avg *= 2.f;
                        }
                        // set speed directly
                        dSpeed = avg;
                    }
                }
                if(tp.GetState() == Touch::Moving) {
                    // hold and drag scrolling
                    //            redrawList = true;
                    SetScrollingValue(Scroll + deltaY);
                }
            }

            if(tp.GetState() == Touch::Released) {
                if(!tp.GetDeltaY() && !avg) {
                    // stop the list, if there is no movement in the last few samples
                    dSpeed = 0;
                    animation = 0;
                } else {
                    if(dSpeed > 0.1) {
                        animation = 1;
                    } else if(dSpeed < -0.1) {
                        animation = -1;
                    } else {
                        animation = 0;
                    }
                }

                scrollingTimestamp = grvl::Callbacks()->get_timestamp();
                currentOverscrollBarSize = 0;
                touchActive = false;
                scrollingByFinger = false;
                prevDeltaY = 0;
                prevDeltaX = 0;
            }
        }

        // NOLINTEND

        return Touch::TouchHandled;
    }

    void VerticalScrollView::CheckPlacement()
    {
        if(!grvl::Callbacks()->gui_printf) {
            return;
        }
        if(header) {
            header->CheckPlacement();
        }
    }

    void VerticalScrollView::EnableScrolling()
    {
        scrollingEnabled = true;
    }

    void VerticalScrollView::DisableScrolling()
    {
        scrollingEnabled = false;
    }

    void VerticalScrollView::EnableOverscrollBar()
    {
        overscrollBarEnabled = true;
    }

    void VerticalScrollView::DisableOverscrollBar()
    {
        overscrollBarEnabled = false;
    }

    void VerticalScrollView::SetOverscrollBar(bool enable)
    {
        overscrollBarEnabled = enable;
    }

    void VerticalScrollView::SetOverscrollBarColor(uint32_t color)
    {
        overscrollBarColor = color;
    }

    void VerticalScrollView::SetOverscrollBarSize(int32_t size)
    {
        if(size >= 0) {
            overscrollBarSize = size;
        }
    }

    void VerticalScrollView::InitFromXML(XMLElement* xmlElement)
    {
        this->SetOverscrollBar(XMLSupport::GetAttributeOrDefault(xmlElement, "overscrollEnabled", true));
        static constexpr auto defaultOverscrollBarSize = 50;
        this->SetOverscrollBarSize(XMLSupport::GetAttributeOrDefault(xmlElement, "overscrollHeight", (uint32_t)defaultOverscrollBarSize));
        this->SetVisible(XMLSupport::GetAttributeOrDefault(xmlElement, "visible", true));

        AbstractView::InitFromXML(xmlElement);
    }

    void VerticalScrollView::Refresh()
    {
        itemsHeight = 0;
        for (auto& child : Elements) {
            if (child->IsVisible()) {
                AdjustScrollViewHeight(child);
            }
        }
        ScrollMax = max(itemsHeight - Height, 0);
    }

    void VerticalScrollView::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        AbstractView::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AttachMemberFunction("Refresh", VerticalScrollView::JSRefreshWrapper, 0);
        jsObjectBuilder.AddProperty("scroll", VerticalScrollView::JSGetScrollWrapper, VerticalScrollView::JSSetScrollWrapper);
    }

    duk_ret_t VerticalScrollView::JSRefreshWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        VerticalScrollView* verticalScrollView = static_cast<VerticalScrollView*>(duk_to_pointer(ctx, -1));
        if (!verticalScrollView) {
            return 0;
        }

        verticalScrollView->Refresh();

        return 0;
    }

} /* namespace grvl */
