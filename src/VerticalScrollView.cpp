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

    VerticalScrollView::~VerticalScrollView()
    {
        Elements_vec::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }
        grvl::Callbacks()->mutex_destroy(ClearWhileDrawMutex);
        grvl::Callbacks()->mutex_destroy(ClearWhileTouchMutex);
    }

    void VerticalScrollView::AddElement(AbstractButton* item)
    {
        // TODO: This requires some improvement on height calculation
        static constexpr auto heightIncrement = 80;
        itemsHeight += heightIncrement;
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }

        Elements.push_back(item);
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
        Component* Element = NULL;
        for(uint32_t i = 0; i < Elements.size(); i++) {
            if(strcmp(Elements[i]->GetID(), id) == 0) {
                Element = Elements[i];
                break;
            }
        }
        return Element;
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

    void VerticalScrollView::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth,
                                  int32_t ParentHeight)
    {
        if(!Visible) {
            return;
        }
        static constexpr auto speedMultipler = 30;
        static constexpr auto speedScale= 1000;
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
                    break; // No need to check more elements
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
                        ParentY + Y - tempScroll + Elements[i]->GetY() + Elements[i]->GetHeight() - realButtonHeight,
                        realButtonHeight, Elements[i]->GetCurrentBackgroundColor());
                }

                // bottom part of element invisible
                else if(Elements[i]->GetY() + Elements[i]->GetHeight() > tempScroll + Height) {
                    ButtonHeight = Elements[i]->GetHeight()
                        - (Elements[i]->GetY() + Elements[i]->GetHeight() - (tempScroll + Height));
                    DrawHLine = false;
                    painter.AddBackgroundBlock(
                        ParentY + Y - tempScroll + Elements[i]->GetY(), ButtonHeight,
                        Elements[i]->GetCurrentBackgroundColor());
                }

                // center element - visible
                else {
                    ButtonHeight = Elements[i]->GetHeight();
                    DrawHLine = true;
                    painter.AddBackgroundBlock(
                        ParentY + Y - tempScroll + Elements[i]->GetY(), Elements[i]->GetHeight(),
                        Elements[i]->GetCurrentBackgroundColor());
                }

                Elements[i]->Draw(painter, ParentX + X, ParentY + Y - tempScroll, Width, ButtonHeight);

                if(i == Elements.size() - 1) {
                    DrawHLine = false;
                }

                if(DrawHLine && SplitLineColor > 0) {
                    painter.DrawHLine(
                        ParentX + X, ParentY + Y + Elements[i]->GetY() + Elements[i]->GetHeight() - tempScroll - 1,
                        Width, SplitLineColor);
                }
            }
        }
        grvl::Callbacks()->mutex_unlock(ClearWhileDrawMutex);
        if(overscrollBarEnabled && tempCurrentOverscrollSize > 0 && tempScroll - tempCurrentOverscrollSize == ScrollMax) {
            painter.FillRectangle(
                ParentX + X, ParentY + Y + Height - tempCurrentOverscrollSize, Width, tempCurrentOverscrollSize, overscrollBarColor);
            painter.AddBackgroundBlock(
                ParentY + Y + Height - tempCurrentOverscrollSize, tempCurrentOverscrollSize, overscrollBarColor);
        } else if(overscrollBarEnabled && tempCurrentOverscrollSize < 0 && tempScroll - tempCurrentOverscrollSize == 0) {
            painter.FillRectangle(ParentX + X, ParentY + Y, Width, -tempCurrentOverscrollSize, overscrollBarColor);
            painter.AddBackgroundBlock(ParentY + Y, -tempCurrentOverscrollSize, overscrollBarColor);
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
                int scrollX = ParentX + X + Width - scrollXOffset;
                int scrollY = ParentY + Y + scrollIndicatorY;

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
            painter.FillRectangle(ParentX + X, ParentY + Y + itemsHeight, Width, Height - itemsHeight, BackgroundColor);
            painter.AddBackgroundBlock(ParentY + Y + itemsHeight, Height - itemsHeight, BackgroundColor);
        }
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

    Touch::TouchResponse VerticalScrollView::ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY,
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
                activeChild = NULL;
                currentOverscrollBarSize = 0;

                bool ownCheck = IsTouchPointInObject(tp.GetCurrentX() - ParentX, tp.GetCurrentY() - ParentY);

                if(ownCheck) { // Container selected
                    touchActive = true;
                    grvl::Callbacks()->mutex_lock(ClearWhileTouchMutex);
                    for(uint32_t i = 0; i < Elements.size(); i++) {
                        Touch::TouchResponse touch = Elements[i]->ProcessTouch(tp, ParentX + X, ParentY + Y - Scroll);
                        if(Touch::TouchHandled == touch || Touch::LongTouchHandled == touch) {
                            activeChild = Elements[i];
                            grvl::Callbacks()->mutex_unlock(ClearWhileTouchMutex);
                            return Touch::TouchHandled; // Child took control
                        }
                    }
                    grvl::Callbacks()->mutex_unlock(ClearWhileTouchMutex);
                    childDropped = true;
                    return Touch::TouchHandled;
                }
                return Touch::TouchNA;
            }
            animation = 0;
            touchActive = true; // Continue scrolling
            prevDeltaY = tp.GetDeltaY();
        }

        if(!touchActive) {
            return Touch::TouchNA;
        }

        if(activeChild && !childDropped) { // push data to the child

            Touch::TouchResponse res = activeChild->ProcessTouch(tp, ParentX + X, ParentY + Y - Scroll);

            if(res == Touch::TouchReleased || res == Touch::TouchNA) {
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

} /* namespace grvl */
