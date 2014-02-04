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

#ifndef GRVL_ABSTRACTBUTTON_H_
#define GRVL_ABSTRACTBUTTON_H_

#include "Component.h"
#include "Font.h"
#include "grvl.h"
#include "Image.h"
#include "Painter.h"
#include "stl.h"

namespace grvl {

    /// Represents base class for all buttons.
    class AbstractButton : public Component {
    public:
        AbstractButton()
            : Component()
            , Text("")
            , ButtonImage()
            , ButtonFont(NULL)
            , TouchActivatedTimestamp(0)
            , longTouchActive(false)
            , onLongPress()
            , onLongPressRepeat()
        {
        }

        AbstractButton(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
            , Text("")
            , ButtonImage()
            , ButtonFont(NULL)
            , TouchActivatedTimestamp(0)
            , longTouchActive(false)
            , onLongPress()
            , onLongPressRepeat()
        {
        }

        AbstractButton(const AbstractButton& Obj)
            : Component(Obj)
            , Text(Obj.Text)
            , ButtonImage(Obj.ButtonImage)
            , ButtonFont(Obj.ButtonFont)
            , TouchActivatedTimestamp(0)
            , longTouchActive(false)
            , onLongPress()
            , onLongPressRepeat()
        {
        }

        virtual ~AbstractButton();

        AbstractButton& operator=(const AbstractButton& Obj);

        void SetText(const char* text);
        void SetImage(const Image& image);
        void SetTextFont(Font const* font);
        void SetOnLongPressEvent(const Event& event);
        void SetOnLongPressRepeatEvent(const Event& event);

        void ClearButtonFont();

        virtual void OnPress();
        virtual void OnRelease();
        virtual void OnClick();

        virtual void PrepareContent(ContentManager* contentManager)
        {
        }

        virtual void CancelPreparingContent(ContentManager* contentManager)
        {
        }

        Touch::TouchResponse ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY);

        string& GetText();
        Font const* GetButtonFont();
        Image* GetImagePointer();

        virtual void ClearTouch();

    protected:
        string Text;
        Image ButtonImage;
        Font const* ButtonFont;
        uint64_t TouchActivatedTimestamp;

        // Long touch support
        bool longTouchActive;
        Event onLongPress, onLongPressRepeat;

        virtual void InitFromXML(XMLElement* xmlElement);
    };

} /* namespace grvl */

#endif /* GRVL_ABSTRACTBUTTON_H_ */
