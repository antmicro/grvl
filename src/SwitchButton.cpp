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

#include "SwitchButton.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    void SwitchButton::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        // Late image assignment
        if(Height + Width <= 0 && !ButtonImage.IsEmpty()) {
            Height = ButtonImage.GetHeight();
            Width = ButtonImage.GetWidth();
        }

        if(switchState) {
            DrawActiveState(painter, RenderX, RenderY, Width, Height);
        } else {
            DrawInactiveState(painter, RenderX, RenderY, Width, Height);
        }

        DrawBorderIfNecessary(painter, RenderX, RenderY, Width, Height);
    }

    void SwitchButton::DrawActiveState(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        if(!ButtonImage.IsEmpty()) {
            ButtonImage.SetActiveFrame(1);
            ButtonImage.Draw(painter, RenderX, RenderY);
        }

        uint32_t padding = (RenderHeight - stateIndicatorHeight) / 2;
        painter.FillRoundRectangle(RenderX, RenderY, RenderWidth, RenderHeight, ActiveBackgroundColor, BorderArcRadius);
        painter.FillRoundRectangle(
            RenderX + RenderWidth - stateIndicatorWidth - padding,
            RenderY + padding,
            stateIndicatorWidth,
            stateIndicatorHeight,
            ActiveForegroundColor, stateIndicatorArcRadius);
    }

    void SwitchButton::DrawInactiveState(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        if(!ButtonImage.IsEmpty()) {
            ButtonImage.SetActiveFrame(0);
            ButtonImage.Draw(painter, RenderX, RenderY);
            return;
        }

        uint32_t padding = (RenderHeight - stateIndicatorHeight) / 2;
        painter.FillRoundRectangle(RenderX, RenderY, RenderWidth, RenderHeight, BackgroundColor, BorderArcRadius);
        painter.FillRoundRectangle(
            RenderX + padding,
            RenderY + padding,
            stateIndicatorWidth,
            stateIndicatorHeight,
            ForegroundColor, stateIndicatorArcRadius);
    }

    void SwitchButton::OnPress()
    {
        previousSwitchState = switchState;
        Component::OnPress();
    }

    void SwitchButton::OnRelease()
    {
        if(previousSwitchState != switchState) {
            if(switchState) {
                Manager::GetInstance().GetEventsQueueInstance().push(&onSwitchON);
            } else {
                Manager::GetInstance().GetEventsQueueInstance().push(&onSwitchOFF);
            }
        }
        Component::OnRelease();
    }

    void SwitchButton::OnClick()
    {

        if(previousSwitchState == switchState) {
            if(!switchState) {
                switchState = true;
                Manager::GetInstance().GetEventsQueueInstance().push(&onSwitchON);
            } else if(switchState) {
                switchState = false;
                Manager::GetInstance().GetEventsQueueInstance().push(&onSwitchOFF);
            }
        }
        Component::OnClick();
    }

    void SwitchButton::SetSwitchState(bool state)
    {
        switchState = state;
    }

    bool SwitchButton::GetSwitchState() const
    {
        return switchState;
    }

    void SwitchButton::SetStateIndicatorWidth(uint32_t value)
    {
        stateIndicatorWidth = value;
    }

    void SwitchButton::SetStateIndicatorHeight(uint32_t value)
    {
        stateIndicatorHeight = value;
    }

    void SwitchButton::SetStateIndicatorArcRadius(uint32_t value)
    {
        stateIndicatorArcRadius = value;
    }

    SwitchButton* SwitchButton::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        SwitchButton* result = new SwitchButton();

        result->InitFromXML(xmlElement);
        static constexpr auto defaultTextColor = 0xFFFFFFFF;

        const char* tempChar = xmlElement->Attribute("image");
        if(tempChar) {
            result->SetImage(Image());
            man->BindImageContentToImage(tempChar, result->GetImagePointer());
        }

        if((result->GetWidth() + result->GetHeight()) <= 0 && !result->GetImagePointer()->IsEmpty()) {
            result->SetSize(result->GetImagePointer()->GetWidth(), result->GetImagePointer()->GetHeight());
        }

        result->SetOnSwitchONEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSwitchON"))));
        result->SetOnSwitchOFFEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSwitchOFF"))));
        result->SetOnLongPressEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPress"))));
        result->SetOnLongPressRepeatEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPressRepeat"))));
        result->SetStateIndicatorWidth(XMLSupport::GetAttributeOrDefault(xmlElement, "stateIndicatorWidth", result->GetHeight()));
        result->SetStateIndicatorHeight(XMLSupport::GetAttributeOrDefault(xmlElement, "stateIndicatorHeight", result->GetHeight()));
        result->SetStateIndicatorArcRadius(XMLSupport::GetAttributeOrDefault(xmlElement, "stateIndicatorArcRadius", result->GetBorderArcRadius()));

        return result;
    }

    Touch::TouchResponse SwitchButton::ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY)
    {
        Touch::TouchResponse res = Touch::TouchHandled;

        if(!previousSwitchState) {
            if(StartX < Width / 2 && DeltaX > Width / 2) {
                switchState = true; //NOLINT
            } // Slide detected
            else {
                switchState = false;
            }

            if(StartX > Width / 2 && DeltaX < -Width / 3) {
                res = Touch::TouchReleased;
            } // Ignore slide
        } else if(previousSwitchState) {
            if(StartX > Width / 2 && DeltaX < -Width / 2) {
                switchState = false; //NOLINT
            } // Slide detected
            else {
                switchState = true;
            }

            if(StartX < Width / 2 && DeltaX > Width / 3) {
                res = Touch::TouchReleased;
            } // Ignore slide
        }

        return res;
    }

    void SwitchButton::SetOnSwitchONEvent(const Event& event)
    {
        onSwitchON = event;
        onSwitchON.SetSenderPointer(this);
    }

    void SwitchButton::SetOnSwitchOFFEvent(const Event& event)
    {
        onSwitchOFF = event;
        onSwitchOFF.SetSenderPointer(this);
    }

    void SwitchButton::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        AbstractButton::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("switchState", SwitchButton::JSGetSwitchStateWrapper);
    }

} /* namespace grvl */
