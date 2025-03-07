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

#ifndef GRVL_SLIDER_H_
#define GRVL_SLIDER_H_

#include "Component.h"
#include "Image.h"
#include "Painter.h"
#include "XMLSupport.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Horizontal scroll bar widget.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible (default: true)
    ///
    /// * backgroundColor         - background color (default: transparent)
    /// * activeBackgroundColor   - background color when pressed (default: backgroundColor)
    /// * frameColor              - frame color (default: transparent)
    /// * selectedFrameColor      - frame color when pressed (default: frameColor)
    /// * barColor                - scroll bar background color (default: white)
    /// * activeBarColor          - scroll bar background color when pressed (default: barColor)
    /// * scrollColor             - scrolling element color (default: blue)
    /// * activeScrollColor       - scrolling element color when pressed (default: scrollColor)
    ///
    /// * minValue                - minimal value (the one on the left) (default: 0)
    /// * maxValue                - maximal value (the one on the right) (default: 100)
    ///
    /// * image                   - identifier of image content replacing standard scrolling element
    ///
    /// XML events:
    /// * onClick                 - event invoked when touch is released, but only when it has not left the boundaries of widget since pressing
    /// * onPress                 - event invoked when touch is detected in the boundaries of the widget
    /// * onRelease               - event invoked when touch is released
    /// * onValueChange           - event invoked when the scrolling element is moved to another value
    ///
    class Slider : public Component {
    public:
        Slider()
            : Component()
            , BarColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveBarColor(COLOR_ARGB8888_GRAY)
            , ScrollColor(COLOR_ARGB8888_LIGHTBLUE)
            , ActiveScrollColor(COLOR_ARGB8888_BLUE)
            , FrameColor(COLOR_ARGB8888_BLACK)
            , SelectedFrameColor(COLOR_ARGB8888_BLACK)
            , ScrollLength(0)
            , MinValue(0)
            , MaxValue(0)
            , Position(0.0)
            , Value(0)
            , ReportedValue(0)
            , ScrollImage()
            , onValueChange()
            , PreviousValueUpdateTimestamp(0)
            , KeepBoundaries(false)
        {
        }

        Slider(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
            , BarColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveBarColor(COLOR_ARGB8888_GRAY)
            , ScrollColor(COLOR_ARGB8888_LIGHTBLUE)
            , ActiveScrollColor(COLOR_ARGB8888_BLUE)
            , FrameColor(COLOR_ARGB8888_BLACK)
            , SelectedFrameColor(COLOR_ARGB8888_BLACK)
            , ScrollLength(0)
            , MinValue(0)
            , MaxValue(0)
            , Position(0.0)
            , Value(0)
            , ReportedValue(0)
            , ScrollImage()
            , onValueChange()
            , PreviousValueUpdateTimestamp(0)
            , KeepBoundaries(false)
        {
        }

        Slider(int32_t x, int32_t y, int32_t width, int32_t height, int32_t min, int32_t max)
            : Component(x, y, width, height)
            , BarColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveBarColor(COLOR_ARGB8888_GRAY)
            , ScrollColor(COLOR_ARGB8888_LIGHTBLUE)
            , ActiveScrollColor(COLOR_ARGB8888_BLUE)
            , FrameColor(COLOR_ARGB8888_BLACK)
            , SelectedFrameColor(COLOR_ARGB8888_BLACK)
            , ScrollLength(0)
            , MinValue(min)
            , MaxValue(max)
            , Position(0.0)
            , Value(0)
            , ReportedValue(0)
            , ScrollImage()
            , onValueChange()
            , PreviousValueUpdateTimestamp(0)
            , KeepBoundaries(false)
        {
        }

        Slider(const Slider& Obj)
            : Component(Obj)
            , BarColor(Obj.BarColor)
            , ActiveBarColor(COLOR_ARGB8888_GRAY)
            , ScrollColor(Obj.ScrollColor)
            , ActiveScrollColor(Obj.ActiveScrollColor)
            , FrameColor(Obj.FrameColor)
            , SelectedFrameColor(Obj.SelectedFrameColor)
            , ScrollLength(Obj.ScrollLength)
            , MinValue(Obj.MinValue)
            , MaxValue(Obj.MaxValue)
            , Position(Obj.Position)
            , Value(Obj.Value)
            , ReportedValue(Obj.ReportedValue)
            , ScrollImage(Obj.ScrollImage)
            , onValueChange(Obj.onValueChange)
            , PreviousValueUpdateTimestamp(Obj.PreviousValueUpdateTimestamp)
            , KeepBoundaries(Obj.KeepBoundaries)
        {
        }

        virtual ~Slider();

        Slider& operator=(const Slider& Obj);

        void SetScrollImage(const Image& image);
        Image* GetScrollImagePointer();

        void SetBarColor(uint32_t color);
        void SetActiveBarColor(uint32_t color);
        void SetScrollColor(uint32_t color);
        void SetActiveScrollColor(uint32_t color);
        void SetFrameColor(uint32_t color);
        void SetSelectedFrameColor(uint32_t color);
        void SetMinValue(float value);
        void SetMaxValue(float value);
        void SetValue(double value);
        void SetKeepBoundaries(bool value);
        void SetDiscrete(uint8_t value);
        void SetDivision(uint8_t value);
        void CalculateStep();
        void SetTextFont(Font const* font);
        void SetLimiters(float const array[], uint8_t size);
        static string GetValueOnButton(float val);

        uint32_t GetBarColor() const;
        uint32_t GetActiveBarColor() const;
        uint32_t GetScrollColor() const;
        uint32_t GetActiveScrollColor() const;
        uint32_t GetFrameColor() const;
        uint32_t GetSelectedFrameColor() const;
        int32_t GetMinValue() const;
        int32_t GetMaxValue() const;
        /// @return Scroll bar's current position.
        float GetValue() const;
        bool GetKeepBoundaries() const;

        virtual void OnPress();
        virtual void OnRelease();
        virtual void OnClick();

        void SetOnValueChangeEvent(const Event& event);

        virtual Touch::TouchResponse ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY);
        virtual bool IsTouchPointInObject(int32_t x, int32_t y, int32_t modificator);

        static Slider* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

    protected:
        uint32_t BarColor, ActiveBarColor, ScrollColor, ActiveScrollColor, FrameColor, SelectedFrameColor, ScrollLength;
        uint8_t discrete, division;
        uint32_t step;
        float MinValue, MaxValue;
        float Position, Value, ReportedValue;
        Image ScrollImage;
        Event onValueChange;
        uint64_t PreviousValueUpdateTimestamp;
        bool KeepBoundaries;

        Font const* SliderFont;

        float* limiters;
        uint8_t NumberOfLimiters;

        float ValueToPosition(float value) const;
        float PositionToValue(float position) const;
    };

} /* namespace grvl */

#endif /* GRVL_SLIDER_H_ */
