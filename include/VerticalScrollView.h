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

#ifndef GRVL_VERTICALSCROLLVIEW_H_
#define GRVL_VERTICALSCROLLVIEW_H_

#include "AbstractView.h"
#include "ListItem.h"
#include "Painter.h"
#include "stl.h"

namespace grvl {

    /// Represents widget allowing to scroll its content vertically.
    ///
    class VerticalScrollView : public AbstractView {
    public:
        VerticalScrollView()
            : AbstractView()
            , Scroll(0)
            , ScrollMax(0)
            , ScrollChange(0)
            , prevDeltaY(0)
            , prevDeltaX(0)
            , itemsHeight(0)
            , animation(false)
            , dSpeed(0)
            , ElementColor(COLOR_ARGB8888_BROWN)
            , SplitLineColor(COLOR_ARGB8888_TRANSPARENT)
            , scrollingTimestamp(0)
            , scrollingEnabled(true)
            , overscrollBarEnabled(false)
            , scrollingByFinger(false)
            , overscrollBarSize(50)
            , currentOverscrollBarSize(0)
            , overscrollBarColor(COLOR_ARGB8888_LIGHTGRAY)
            , currentSample(0)
            , scrollIndicatorTimestamp(0)
            , scrollIndicatorColor(0)
            , scrollIndicatorOpacity(0)
            , scrollIndicatorImage(NULL)
        {
            ClearWhileDrawMutex = grvl::Callbacks()->mutex_create();
            ClearWhileTouchMutex = grvl::Callbacks()->mutex_create();
        }

        VerticalScrollView(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractView(x, y, width, height)
            , Scroll(0)
            , ScrollMax(0)
            , ScrollChange(0)
            , prevDeltaY(0)
            , prevDeltaX(0)
            , itemsHeight(0)
            , animation(false)
            , dSpeed(0)
            , ElementColor(COLOR_ARGB8888_BROWN)
            , SplitLineColor(COLOR_ARGB8888_TRANSPARENT)
            , scrollingTimestamp(0)
            , scrollingEnabled(true)
            , overscrollBarEnabled(false)
            , scrollingByFinger(false)
            , overscrollBarSize(50)
            , currentOverscrollBarSize(0)
            , overscrollBarColor(COLOR_ARGB8888_LIGHTGRAY)
            , currentSample(0)
            , scrollIndicatorTimestamp(0)
            , scrollIndicatorColor(0)
            , scrollIndicatorOpacity(0)
            , scrollIndicatorImage(NULL)
        {
            ClearWhileDrawMutex = grvl::Callbacks()->mutex_create();
            ClearWhileTouchMutex = grvl::Callbacks()->mutex_create();
        }

        virtual ~VerticalScrollView();

        virtual void AddElement(AbstractButton* item);

        void SetScrollingValue(int32_t scrollVal);
        void SetSplitLineColor(uint32_t color);

        void SetOverscrollBarColor(uint32_t color);

        int32_t GetScrollingValue() const;
        int32_t GetScrollMaxValue() const;
        virtual Component* GetElement(uint32_t index);
        virtual Component* GetElement(const char* id);
        virtual Component* GetLastElement();
        uint32_t GetItemsCount();

        virtual Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY,
                                                  int32_t modificator = 0);
        void EnableScrolling();
        void DisableScrolling();
        void SetScrolling(bool enable);

        void EnableOverscrollBar();
        void DisableOverscrollBar();
        void SetOverscrollBar(bool enable);
        void SetOverscrollBarSize(int32_t size);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

        virtual void SetSize(int32_t width, int32_t height);
        virtual void PrepareToOpen();
        virtual void PrepareToClose();

        virtual void CheckPlacement();

        void SetScrollIndicatorColor(uint32_t color);

    protected:
        int32_t Scroll, ScrollMax, ScrollChange, prevDeltaY, prevDeltaX, itemsHeight;
        int8_t animation;
        float dSpeed;
        uint32_t ElementColor, SplitLineColor;
        typedef vector<AbstractButton*> Elements_vec;
        Elements_vec Elements;
        uint64_t scrollingTimestamp;
        bool scrollingEnabled, overscrollBarEnabled, scrollingByFinger;
        int32_t overscrollBarSize, currentOverscrollBarSize;
        uint32_t overscrollBarColor;
        uint8_t currentSample;
        int32_t speedSamples[3];
        uint64_t scrollIndicatorTimestamp;
        uint32_t scrollIndicatorColor;
        uint8_t scrollIndicatorOpacity;
        ImageContent* scrollIndicatorImage;
        void* ClearWhileDrawMutex;
        void* ClearWhileTouchMutex;

        void InitFromXML(XMLElement* xmlElement);
    };

} /* namespace grvl */

#endif /* GRVL_VERTICALSCROLLVIEW_H_ */
