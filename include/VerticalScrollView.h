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
        {
            ClearWhileDrawMutex = grvl::Callbacks()->mutex_create();
            ClearWhileTouchMutex = grvl::Callbacks()->mutex_create();
        }

        VerticalScrollView(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractView(x, y, width, height)
        {
            ClearWhileDrawMutex = grvl::Callbacks()->mutex_create();
            ClearWhileTouchMutex = grvl::Callbacks()->mutex_create();
        }

        void AddElement(Component* component) override;
        void RemoveElement(const char* elementId) override;

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

        void Refresh();

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;
        static duk_ret_t JSRefreshWrapper(duk_context* ctx);

        GENERATE_DUK_INT_GETTER(VerticalScrollView, Scroll, GetScrollingValue)
        GENERATE_DUK_INT_SETTER(VerticalScrollView, Scroll, SetScrollingValue)

    protected:
        int32_t Scroll{0};
        int32_t ScrollMax{0};
        int32_t ScrollChange{0};
        int32_t prevDeltaY{0};
        int32_t prevDeltaX{0};
        int32_t itemsHeight{0};
        int8_t animation{0};
        float dSpeed{0};
        uint32_t ElementColor{COLOR_ARGB8888_BROWN};
        uint32_t SplitLineColor{COLOR_ARGB8888_TRANSPARENT};
        uint64_t scrollingTimestamp{0};
        bool scrollingEnabled{false};
        bool overscrollBarEnabled{false};
        bool scrollingByFinger{false};
        int32_t overscrollBarSize{50};
        int32_t currentOverscrollBarSize{0};
        uint32_t overscrollBarColor{COLOR_ARGB8888_LIGHTGRAY};
        uint8_t currentSample{0};
        int32_t speedSamples[3];
        uint64_t scrollIndicatorTimestamp{0};
        uint32_t scrollIndicatorColor{0};
        uint8_t scrollIndicatorOpacity{0};
        ImageContent* scrollIndicatorImage{nullptr};
        void* ClearWhileDrawMutex{nullptr};
        void* ClearWhileTouchMutex{nullptr};

        virtual void AdjustScrollViewHeight(Component* child);

        void InitFromXML(XMLElement* xmlElement);

        virtual Component* TryToGetElementFromChildContainer(Component* possible_container, const char* searched_component_id);
    };

} /* namespace grvl */

#endif /* GRVL_VERTICALSCROLLVIEW_H_ */
