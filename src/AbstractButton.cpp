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

#include "AbstractButton.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    AbstractButton::~AbstractButton()
    {
    }

    void AbstractButton::SetText(const char* text)
    {
        Text = string(text);
    }

    void AbstractButton::SetImage(const Image& image)
    {

        if(image.IsEmpty()) {
            return;
        }
        ButtonImage = image;
    }

    Font const* AbstractButton::GetButtonFont()
    {
        return ButtonFont;
    }

    void AbstractButton::SetTextFont(Font const* font)
    {
        ButtonFont = font;
    }

    void AbstractButton::ClearButtonFont()
    {
        ButtonFont = 0;
    }

    void AbstractButton::OnPress()
    {
        TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
        Component::OnPress();
    }

    void AbstractButton::OnRelease()
    {
        TouchActivatedTimestamp = 0;
        longTouchActive = false;
        Component::OnRelease();
    }

    void AbstractButton::OnClick()
    {
        Component::OnClick();
    }

    AbstractButton& AbstractButton::operator=(const AbstractButton& Obj)
    {
        if(this != &Obj) {
            Component::operator=(Obj);
            Text = Obj.Text;
            ButtonImage = Obj.ButtonImage;
            ButtonFont = Obj.ButtonFont;
            TouchActivatedTimestamp = 0;
            longTouchActive = Obj.longTouchActive;
        }
        return *this;
    }

    void AbstractButton::SetOnLongPressEvent(const Event& event)
    {
        onLongPress = event;
        onLongPress.SetSenderPointer(this);
    }

    void AbstractButton::SetOnLongPressRepeatEvent(const Event& event)
    {
        onLongPressRepeat = event;
        onLongPressRepeat.SetSenderPointer(this);
    }

    const char* AbstractButton::GetText()
    {
        return Text.c_str();
    }

    Image* AbstractButton::GetImagePointer()
    {
        return &ButtonImage;
    }

    Touch::TouchResponse AbstractButton::ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY)
    {
        Touch::TouchResponse res;
        res = Component::ProcessMove(StartX, StartY, DeltaX, DeltaY); // Generic part

        if(res != Touch::TouchHandled) {
            return res;
        }

        static constexpr auto longpressThreshold = 1000;
        if(!longTouchActive && TouchActivatedTimestamp < (grvl::Callbacks()->get_timestamp() - longpressThreshold)) { // Long press
            if(onLongPress.IsSet()) {
                longTouchActive = true;
                TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
                Manager::GetInstance().GetEventsQueueInstance().push(&onLongPress);
            } else {
                return Touch::TouchReleased;
            }
        }

        static constexpr auto longpressRepeatOffset = 500;
        if(longTouchActive && TouchActivatedTimestamp < (grvl::Callbacks()->get_timestamp() - longpressRepeatOffset)) { // Long press repeat
            TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
            Manager::GetInstance().GetEventsQueueInstance().push(&onLongPressRepeat);
        }

        if(longTouchActive) {
            return Touch::LongTouchHandled;
        }
        return Touch::TouchHandled;
    }

    void AbstractButton::ClearTouch()
    {
        Component::ClearTouch();
        longTouchActive = false;
    }

    void AbstractButton::InitFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        Component::InitFromXML(xmlElement);

        this->SetText(XMLSupport::GetAttributeOrDefault(xmlElement, "text", ""));
        this->SetTextFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal")));
    }

    void AbstractButton::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("text", AbstractButton::JSGetTextWrapper, AbstractButton::JSSetTextWrapper);
    }

} /* namespace grvl */
