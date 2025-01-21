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

    SwitchButton::~SwitchButton()
    {
    }

    void SwitchButton::Draw(Painter& painter, int32_t ParentX, int32_t ParentY)
    {
        if(!Visible) {
            return;
        }

        uint32_t TempFrameColor = 0;
        static constexpr auto widthMultiplier = 0.4;

        // Late image assignment
        if(Height + Width <= 0 && !ButtonImage.IsEmpty()) {
            Height = ButtonImage.GetHeight();
            Width = ButtonImage.GetWidth();
        }

        if(switchState) {
            // If button pressed
            if(!ButtonImage.IsEmpty()) {
                ButtonImage.SetActiveFrame(1);
                ButtonImage.Draw(painter, ParentX + X, ParentY + Y);
            } else {
                if(Text != "ON") {
                    Text = "ON";
                    TextWidth = ButtonFont->GetWidth(Text.c_str());
                }

                if(Width > 0 && Height > 0) {
                    if (BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
                        painter.FillRoundRectangle(ParentX + X, ParentY + Y, Width, Height, BackgroundColor, BorderArcRadius);
                    } else {
                        painter.FillRectangle(ParentX + X, ParentY + Y, Width, Height, BackgroundColor);
                    }
                    DrawBorderIfNecessary(painter, X + ParentX, Y + ParentY, Width, Height);

                    painter.FillRectangle(
                        ParentX + X + ((uint32_t)Width * widthMultiplier), ParentY + Y + 1, ((uint32_t)Width * (1.0 - widthMultiplier)), Height - 1,
                        ActiveSwitchColor);
                }

                if(!Text.empty()) {
                    uint16_t BeginX = X + Width - (TextWidth) - (Height / 2);
                    uint16_t BeginY = Y + (Height / 2) - (ButtonFont->GetHeight() / 2);
                    painter.DisplayAntialiasedString(
                        ButtonFont, ParentX + BeginX, ParentY + BeginY, Text.c_str(), ActiveTextColor);
                }
            }
        } else {
            // If button released
            if(!ButtonImage.IsEmpty()) {
                ButtonImage.SetActiveFrame(0);
                ButtonImage.Draw(painter, ParentX + X, ParentY + Y);
            } else {
                if(Text != "OFF") {
                    Text = "OFF";
                    TextWidth = ButtonFont->GetWidth(Text.c_str());
                }

                if(Height > 0 && Width > 0) {
                    if (BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
                        painter.FillRoundRectangle(ParentX + X, ParentY + Y, Width, Height, BackgroundColor, BorderArcRadius);
                    } else {
                        painter.FillRectangle(ParentX + X, ParentY + Y, Width, Height, BackgroundColor);
                    }
                    DrawBorderIfNecessary(painter, X + ParentX, Y + ParentY, Width, Height);
                    painter.FillRectangle(ParentX + X, ParentY + Y, ((uint32_t)Width * (1.0 - widthMultiplier)), Height, SwitchColor);
                }

                if(!Text.empty()) {
                    uint16_t BeginX = X + (Height / 2);
                    uint16_t BeginY = Y + (Height / 2) - (ButtonFont->GetHeight() / 2);
                    painter.DisplayAntialiasedString(
                        ButtonFont, ParentX + BeginX, ParentY + BeginY, Text.c_str(), TextColor);
                }
            }
        }
    }

    void SwitchButton::SetSwitchColor(uint32_t color)
    {
        SwitchColor = color;
    }

    void SwitchButton::SetActiveSwitchColor(uint32_t color)
    {
        ActiveSwitchColor = color;
    }

    void SwitchButton::SetTextColor(uint32_t color)
    {
        TextColor = color;
    }

    void SwitchButton::SetActiveTextColor(uint32_t color)
    {
        ActiveTextColor = color;
    }

    uint32_t SwitchButton::GetSwitchColor() const
    {
        return SwitchColor;
    }

    uint32_t SwitchButton::GetActiveSwitchColor() const
    {
        return ActiveSwitchColor;
    }

    uint32_t SwitchButton::GetTextColor() const
    {
        return TextColor;
    }

    uint32_t SwitchButton::GetActiveTextColor() const
    {
        return ActiveTextColor;
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

    SwitchButton* SwitchButton::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        SwitchButton* result = new SwitchButton();

        result->InitFromXML(xmlElement);
        static constexpr auto defaultTextColor = 0xFFFFFFFF;
        result->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", defaultTextColor));
        result->SetActiveTextColor(XMLSupport::ParseColor(xmlElement, "activeTextColor", result->GetTextColor()));
        result->SetSwitchColor(XMLSupport::ParseColor(xmlElement, "switchColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));
        result->SetActiveSwitchColor(XMLSupport::ParseColor(xmlElement, "activeSwitchColor", (uint32_t)result->GetSwitchColor()));

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
