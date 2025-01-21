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

#ifndef GRVL_ABSTRACTVIEW_H_
#define GRVL_ABSTRACTVIEW_H_

#include <math.h>

#include "Component.h"
#include "Image.h"
#include "Key.h"
#include "Painter.h"
#include "Panel.h"
#include "stl.h"

namespace grvl {

    /// Represents base class for all screens.
    class AbstractView : public Container {
    public:
        AbstractView()
            : Container()
            , globalPanelVisible(false)
            , header(NULL)
            , onSlideToLeft()
            , onSlideToRight()
            , collectionSize(0)
            , positionInCollection(0)
            , onLongPress()
            , onLongPressRepeat()
            , TouchActivatedTimestamp(0)
            , longTouchActive(false)
        {
            BackgroundColor = COLOR_ARGB8888_WHITE;
        }

        AbstractView(const AbstractView& Obj)
            : Container(Obj)
            , globalPanelVisible(Obj.lastActiveChild)
            , header(NULL)
            , onSlideToLeft()
            , onLongPress()
            , onLongPressRepeat()
            , onSlideToRight()
            , collectionSize(0)
            , positionInCollection(0)
            , TouchActivatedTimestamp(0)
            , longTouchActive(false)
        {
        }

        AbstractView(int32_t x, int32_t y, int32_t width, int32_t height)
            : Container(x, y, width, height)
            , globalPanelVisible(false)
            , header(NULL)
            , onSlideToLeft()
            , onSlideToRight()
            , collectionSize(0)
            , onLongPress()
            , onLongPressRepeat()
            , positionInCollection(0)
            , TouchActivatedTimestamp(0)
            , longTouchActive(false)
        {
            BackgroundColor = COLOR_ARGB8888_WHITE;
        }

        virtual ~AbstractView();

        AbstractView& operator=(const AbstractView& Obj);

        void SetGlobalPanelVisibility(bool visible);
        bool GetGlobalPanelVisibility() const;
        virtual Component* GetLastElement() = 0;

        uint32_t GetCollectionSize() const;
        uint32_t GetPositionInCollection() const;

        void SetCollectionSize(uint32_t size);
        void SetPositionInCollection(uint32_t position);

        void SetOnSlideToLeftEvent(const Event& event);
        void SetOnSlideToRightEvent(const Event& event);

        void SetHeader(Panel* headerPtr);
        Panel* GetHeader();

        virtual void OnPress();
        virtual void OnRelease();

        virtual void PrepareToOpen() = 0;
        virtual void PrepareToClose() = 0;
        virtual void CheckPlacement() = 0;

        Key::KeyState PressKey(const char* id);
        Key::KeyState ReleaseKey(const char* id);
        Key::KeyState LongPressKey(const char* id);
        Key::KeyState LongPressRepeatKey(const char* id);

        void SetOnLongPressEvent(const Event& event);
        void SetOnLongPressRepeatEvent(const Event& event);

        void AddKeyToCollection(const Key& key);

        virtual void ClearTouch();

    protected:
        bool globalPanelVisible;
        Panel* header;
        Event onSlideToLeft, onSlideToRight;
        Event onLongPress, onLongPressRepeat;
        uint32_t collectionSize, positionInCollection;
        vector<Key> KeyCollection;

        virtual void InitFromXML(XMLElement* xmlElement);

        uint64_t TouchActivatedTimestamp;

        // Long touch support
        bool longTouchActive;
    };

} /* namespace grvl */

#endif /* GRVL_ABSTRACTVIEW_H_ */
