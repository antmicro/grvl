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

#include "Slider.h"
#include "grvl.h"
#include "Manager.h"

//NOLINTBEGIN(readability-magic-numbers, bugprone-branch-clone, bugprone-integer-division)

namespace grvl {

    Slider::~Slider()
    {
        delete[] limiters;
    }

    void Slider::SetScrollImage(const Image& image)
    {
        ScrollImage = image;
    }

    Image* Slider::GetScrollImagePointer()
    {
        return &ScrollImage;
    }

    void Slider::SetBarColor(uint32_t color)
    {
        BarColor = color;
    }

    void Slider::SetScrollColor(uint32_t color)
    {
        ScrollColor = color;
    }

    void Slider::SetActiveScrollColor(uint32_t color)
    {
        ActiveScrollColor = color;
    }

    void Slider::SetFrameColor(uint32_t color)
    {
        FrameColor = color;
    }

    void Slider::SetMinValue(float value)
    {
        Value = value;
        MinValue = value;
    }

    void Slider::SetMaxValue(float value)
    {
        MaxValue = value;
    }

    void Slider::SetValue(double value)
    {
        if(value >= MinValue && value <= MaxValue) {
            Value = value;
            Position = ValueToPosition(Value);
        }
    }

    void Slider::SetDiscrete(uint8_t value)
    {
        discrete = value;
    }

    void Slider::SetDivision(uint8_t value)
    {
        division = value;
        CalculateStep();
    }

    void Slider::CalculateStep()
    {
        step = (MaxValue - MinValue) / division;
    }

    void Slider::SetTextFont(Font const* font)
    {
        SliderFont = font;
    }

    void Slider::SetLimiters(float const array[], uint8_t size)
    {
        NumberOfLimiters = size;
        limiters = new float[NumberOfLimiters];
        for(uint8_t i = 0; i < NumberOfLimiters; i++)
            limiters[i] = array[i];
        MinValue = 0;
        MaxValue = size - 1.0;
        discrete = 2;
    }

    uint32_t Slider::GetBarColor() const
    {
        return BarColor;
    }

    uint32_t Slider::GetScrollColor() const
    {
        return ScrollColor;
    }

    uint32_t Slider::GetActiveScrollColor() const
    {
        return ActiveScrollColor;
    }

    uint32_t Slider::GetFrameColor() const
    {
        return FrameColor;
    }

    int32_t Slider::GetMinValue() const
    {
        return MinValue;
    }

    int32_t Slider::GetMaxValue() const
    {
        return MaxValue;
    }

    float Slider::GetValue() const
    {
        return Value;
    }

#define EPSILON 2.2204460492503131e-16

    bool AreSame(float a, float b)
    {
        return fabs(a - b) < EPSILON;
    }

    string GetFriction(float val)
    {
        if(AreSame(4.0, val))
            return "4";
        if(AreSame(3.0, val))
            return "3";
        if(AreSame(2.0, val))
            return "2";
        if(AreSame(1.3, val))
            return "4/3";
        if(AreSame(1.0, val))
            return "1";
        if(AreSame(0.66, val))
            return "2/3";
        if(AreSame(0.33, val))
            return "1/3";
        if(AreSame(16.0, val))
            return "16";
        if(AreSame(14.0, val))
            return "14";
        if(AreSame(12.0, val))
            return "12";
        if(AreSame(10.0, val))
            return "10";
        if(AreSame(8.0, val))
            return "8";
        if(AreSame(6.0, val))
            return "6";
        if(AreSame(4.0, val))
            return "4";
        if(AreSame(3.0, val))
            return "3";
        if(AreSame(2.0, val))
            return "2";
        if(AreSame(0.001, val))
            return "1/1000";
        if(AreSame(0.002, val))
            return "1/500";
        if(AreSame(0.004, val))
            return "1/250";
        if(AreSame(0.008, val))
            return "1/125";
        if(AreSame(0.016, val))
            return "1/60";
        if(AreSame(0.033, val))
            return "1/30";
        if(AreSame(0.066, val))
            return "1/15";
        if(AreSame(0.125, val))
            return "1/8";
        if(AreSame(0.25, val))
            return "1/4";
        if(AreSame(0.5, val))
            return "1/2";
        return "0";
    }

    string Slider::GetValueOnButton(float val)
    {
        return GetFriction(val);
    }

    void Slider::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        if(discrete == 0) {
            if (BorderArcRadius > 0 && BorderType == BorderTypeBits::BOX) {
                painter.FillRoundRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor, BorderArcRadius);
            } else {
                painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, BackgroundColor);
            }
            DrawBorderIfNecessary(painter, X + ParentRenderX, Y + ParentRenderY, Width, Height);

            if(ScrollImage.IsEmpty()) {

                if(Width > Height) {
                    painter.FillRectangle(
                        ParentRenderX + X + (Width - ScrollLength) * Position, ParentRenderY + Y, Width * (1.0 - Position), Height,
                        BarColor);
                    painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width * Position, Height, ActiveBarColor);

                    ScrollLength = (uint32_t)(Width / 10);
                    painter.FillRectangle(
                        ParentRenderX + X + ((Width - ScrollLength) * Position), ParentRenderY + Y, ScrollLength, Height, ScrollColor);
                } else {
                    painter.FillRectangle(
                        ParentRenderX + X, ParentRenderY + Y + Height * (1.0 - Position), Width, ceil(Height * Position),
                        ActiveBarColor);
                    painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height * (1.0 - Position), BarColor);

                    ScrollLength = (uint32_t)(Height / 10);
                    painter.FillRectangle(
                        ParentRenderX + X, ParentRenderY + Y + ((Height - ScrollLength) * (1.0 - Position)), Width, ScrollLength,
                        ScrollColor);
                }
                // Frame
                painter.DrawRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height, FrameColor);
            } else { // With scroll ico
                if(Width > Height) {
                    painter.FillRectangle(ParentRenderX + X + (Width * Position), ParentRenderY + Y, Width * (1.0 - Position), Height,
                                          BarColor);
                    painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width * Position, Height, ActiveBarColor);

                    if(KeepBoundaries) { // image limited to scroll size
                        ScrollImage.SetPosition(
                            (Width - ScrollImage.GetWidth()) * Position, (Height / 2) - (ScrollImage.GetWidth() / 2));
                    } else {
                        ScrollImage.SetPosition(
                            Width * Position - ScrollImage.GetWidth() / 2, (Height / 2) - (ScrollImage.GetWidth() / 2));
                    }
                } else {
                    painter.FillRectangle(
                        ParentRenderX + X, ParentRenderY + Y + Height * (1.0 - Position), Width, ceil(Height * Position),
                        ActiveBarColor);
                    painter.FillRectangle(ParentRenderX + X, ParentRenderY + Y, Width, Height * (1.0 - Position), BarColor);

                    if(KeepBoundaries) { // image limited to scroll size
                        ScrollImage.SetPosition(
                            (-ScrollImage.GetHeight() / 2) + (Width / 2),
                            ((Height - ScrollImage.GetHeight())) * (1.0 - Position));
                    } else {
                        ScrollImage.SetPosition(
                            (-ScrollImage.GetHeight() / 2) + (Width / 2),
                            Height * (1.0 - Position) - ScrollImage.GetHeight() / 2);
                    }
                }

                ScrollImage.Draw(painter, ParentRenderX + X, ParentRenderY + Y);
            }
        } else if(discrete == 1) {
            uint8_t j = 0;
            for(uint32_t i = MinValue; i <= MaxValue; i = i + step) {
                uint16_t x = ParentRenderX + X + j * (Width / division);
                uint16_t y = ParentRenderY + Y;
                painter.DrawLine(x, ParentRenderY + Y, x, y + Height, 0xFFFFFFFF);
                char buffer[4];
                snprintf(buffer, sizeof(buffer), "%d", j * step);
                painter.DisplayAntialiasedString(SliderFont, x - 3, y - 35, buffer, 0xFFFFFFFF);
                j++;
            }

            if(KeepBoundaries) { // image limited to scroll size
                ScrollImage.SetPosition(
                    (Width - ScrollImage.GetWidth()) * Position, (Height / 2) - (ScrollImage.GetWidth() / 2));
            } else {
                ScrollImage.SetPosition(
                    Width * Position - ScrollImage.GetWidth() / 2, (Height / 2) - (ScrollImage.GetWidth() / 2));
            }

            ScrollImage.Draw(painter, ParentRenderX + X, ParentRenderY + Y);
        } else if(discrete == 2) {
            for(uint16_t i = MinValue; i <= MaxValue; i++) {
                float x = ParentRenderX + X + (float)i * (float)Width / (float)MaxValue;
                painter.DrawLine(x, Y, x, Y + Height, 0xFFFFFFFF);
                // char buffer[10];
                string buff;
                if(MaxValue == 10) {
                    if(i % 2 == 0) {
                        buff = GetFriction(limiters[i]);
                        // snprintf(buffer, sizeof(buffer), "%.3f", limiters[i]);
                        painter.DisplayAntialiasedString(SliderFont, x - buff.size() * 2, Y - 35, buff.c_str(), 0xFFFFFFFF);
                    }

                } else {
                    buff = GetFriction(limiters[i]);

                    // snprintf(buffer, sizeof(buffer), "%c", buff);
                    painter.DisplayAntialiasedString(SliderFont, x - buff.size() * 2, Y - 35, buff.c_str(), 0xFFFFFFFF);
                }
            }

            if(KeepBoundaries) { // image limited to scroll size
                ScrollImage.SetPosition(
                    (Width - ScrollImage.GetWidth()) * Position, (Height / 2) - (ScrollImage.GetWidth() / 2));
            } else {
                ScrollImage.SetPosition(
                    Width * Position - ScrollImage.GetWidth() / 2, (Height / 2) - (ScrollImage.GetWidth() / 2));
            }

            ScrollImage.Draw(painter, ParentRenderX + X, ParentRenderY + Y);
        }
    }

    void Slider::SetSelectedFrameColor(uint32_t color)
    {
        SelectedFrameColor = color;
    }

    uint32_t Slider::GetSelectedFrameColor() const
    {
        return SelectedFrameColor;
    }

    Slider& Slider::operator=(const Slider& Obj)
    {
        if(this != &Obj) {
            Component::operator=(Obj);
            ScrollImage = Obj.ScrollImage;
            BarColor = Obj.BarColor;
            ScrollColor = Obj.ScrollColor;
            ActiveScrollColor = Obj.ActiveScrollColor;
            FrameColor = Obj.FrameColor;
            SelectedFrameColor = Obj.SelectedFrameColor;
            ScrollLength = Obj.ScrollLength;
            MinValue = Obj.MinValue;
            MaxValue = Obj.MaxValue;
            Position = Obj.Position;
            Value = Obj.Value;
            onValueChange = Obj.onValueChange;
            KeepBoundaries = Obj.KeepBoundaries;
        }

        return *this;
    }

    bool Slider::IsTouchPointInObject(int32_t x, int32_t y, int32_t modificator)
    {

        if(!Visible) {
            return false;
        }

        if(ScrollImage.IsEmpty()) {
            return Component::IsTouchPointInObject(x, y, modificator);
        }
        if(Width > Height) { // Horizontal
            int imgModificator = KeepBoundaries ? 0 : (ScrollImage.GetWidth() / 2);
            return (bool)(x > X - modificator - imgModificator
                            && x < (X + Width + modificator + imgModificator)
                            && y > (Y + (Height / 2) - (ScrollImage.GetHeight() / 2) - modificator)
                            && y < (Y + (Height / 2) + (ScrollImage.GetHeight() / 2) + modificator));
        }
        int imgModificator = KeepBoundaries ? 0 : (ScrollImage.GetHeight() / 2);
        return (bool)(x > (X + (Width / 2) - (ScrollImage.GetWidth() / 2) - modificator)
                        && x < (X + (Width / 2) + (ScrollImage.GetWidth() / 2) + modificator)
                        && y > (Y - modificator - imgModificator)
                        && y < (Y + Height + modificator + imgModificator));
    }

    void Slider::OnPress()
    {
        PreviousValueUpdateTimestamp = grvl::Callbacks()->get_timestamp();
        Component::OnPress();
    }

    void Slider::OnRelease()
    {
        PreviousValueUpdateTimestamp = 0;
        if(onValueChange.IsSet() && ReportedValue != Value) {
            ReportedValue = Value;
            Manager::GetInstance().GetEventsQueueInstance().push(&onValueChange);
        }
        Component::OnRelease();
    }

    void Slider::OnClick()
    {
        Component::OnClick();
    }

    Slider* Slider::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();

        Slider* result = new Slider();

        result->InitFromXML(xmlElement);

        result->SetFrameColor(XMLSupport::ParseColor(xmlElement, "frameColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));
        result->SetSelectedFrameColor(XMLSupport::ParseColor(xmlElement, "selectedFrameColor", (uint32_t)result->GetFrameColor()));
        result->SetBarColor(XMLSupport::ParseColor(xmlElement, "barColor", (uint32_t)COLOR_ARGB8888_WHITE));
        result->SetActiveBarColor(XMLSupport::ParseColor(xmlElement, "activeBarColor", (uint32_t)result->GetBarColor()));
        result->SetScrollColor(XMLSupport::ParseColor(xmlElement, "scrollColor", (uint32_t)COLOR_ARGB8888_BLUE));
        result->SetActiveScrollColor(XMLSupport::ParseColor(xmlElement, "activeScrollColor", (uint32_t)result->GetScrollColor()));

        result->SetMaxValue(XMLSupport::GetAttributeOrDefault(xmlElement, "maxValue", (uint32_t)100));
        result->SetMinValue(XMLSupport::GetAttributeOrDefault(xmlElement, "minValue", (uint32_t)0));

        result->SetKeepBoundaries(XMLSupport::GetAttributeOrDefault(xmlElement, "keepBoundaries", false));

        result->SetDiscrete(XMLSupport::GetAttributeOrDefault(xmlElement, "isDiscrete", (uint32_t)0));

        result->SetDivision(XMLSupport::GetAttributeOrDefault(xmlElement, "division", (uint32_t)0));

        result->SetTextFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal")));

        const char* tempChar = xmlElement->Attribute("image");
        if(tempChar) {
            result->SetScrollImage(Image());
            man->BindImageContentToImage(tempChar, result->GetScrollImagePointer());
        }

        result->SetOnValueChangeEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onValueChange"))));

        return result;
    }

    Touch::TouchResponse Slider::ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY)
    {
        float Offset = 0;
        if(!ScrollImage.IsEmpty()) {
            if(Width > Height) {
                Offset = KeepBoundaries ? (ScrollImage.GetWidth() / 2) : 0;
            } else {
                Offset = KeepBoundaries ? (ScrollImage.GetHeight() / 2) : 0;
            }
        } else
            Offset = ScrollLength;

        float tempVal = 0;

        if(Width > Height) {
            tempVal = PositionToValue(((float)(StartX + DeltaX - Offset)) / ((float)Width - 2 * Offset));
        } else {
            tempVal = PositionToValue((float)((Height - Offset) - (StartY + DeltaY)) / ((float)Height - 2 * Offset));
        }

        if(tempVal > MaxValue) {
            tempVal = MaxValue;
        } else if(tempVal < MinValue) {
            tempVal = MinValue;
        }

        Position = ValueToPosition(tempVal);
        Value = tempVal;

        if(onValueChange.IsSet() && Value != ReportedValue && PreviousValueUpdateTimestamp < (grvl::Callbacks()->get_timestamp() - 400)) { // Report time
            PreviousValueUpdateTimestamp = grvl::Callbacks()->get_timestamp();
            ReportedValue = Value;
            Manager::GetInstance().GetEventsQueueInstance().push(&onValueChange);
        }

        return Touch::TouchHandled;
    }

    float Slider::ValueToPosition(float value) const
    {
        return (value - MinValue) / (MaxValue - MinValue);
    }

    float Slider::PositionToValue(float position) const
    {
        if(discrete == 1) {
            float range = MaxValue - MinValue;
            float newValue = range * position;
            int val = newValue / step;
            newValue = val * step;
            return newValue + MinValue;
        }
        if(discrete == 0) {
            float range = MaxValue - MinValue;
            float newValue = range * position;
            return newValue + MinValue;
        }
        if(discrete == 2) {
            float range = MaxValue - MinValue;
            float newValue = (int)roundf(range * position);
            return newValue + MinValue;
        }
    }

    void Slider::SetOnValueChangeEvent(const Event& event)
    {
        onValueChange = event;
        onValueChange.SetSenderPointer(this);
    }

    void Slider::SetActiveBarColor(uint32_t color)
    {
        ActiveBarColor = color;
    }

    uint32_t Slider::GetActiveBarColor() const
    {
        return ActiveBarColor;
    }

    void Slider::SetKeepBoundaries(bool value)
    {
        KeepBoundaries = value;
    }

    bool Slider::GetKeepBoundaries() const
    {
        return KeepBoundaries;
    }

} /* namespace grvl */

//NOLINTEND(readability-magic-numbers, bugprone-branch-clone, bugprone-integer-division)
