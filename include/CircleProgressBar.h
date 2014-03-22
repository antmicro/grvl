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

#ifndef GRVL_CIRCLEPROGRESSBAR_H_
#define GRVL_CIRCLEPROGRESSBAR_H_

#include "Painter.h"
#include "ProgressBar.h"
#include "stl.h"
#include "tinyxml2.h"

namespace grvl {

    /// Represents circular progress bar widget.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * startAngle              - angle at which the ring starts in degrees (default: 0)
    /// * endAngle                - angle at which the ring ends in degrees (default: 360)
    /// * radius                  - outer rim of the ring radius in pixels (default: 0)
    /// * thickness               - ring thickness in pixels (default: 0)
    /// * staticGradient          - indicates if static gradient method should be used instead of proportional one
    ///                             (default: true)
    /// * startColor              - gradient color at the beginning of the ring in ARGB8888 format
    ///                             (default: transparent)
    /// * endColor                - gradient color at the end of the ring in ARGB8888 format (default: transparent)
    /// * backgroundColor         - ring background color
    ///
    class CircleProgressBar : public ProgressBar {
    public:
        CircleProgressBar()
            : ProgressBar()
            , Radius(0)
            , Thickness(0)
            , StartAngle(0)
            , EndAngle(0)
            , StartColor(ForegroundColor)
            , EndColor(0xFFFFFFFF)
            , StaticGradient(true)
        {
        }

        CircleProgressBar(int32_t x, int32_t y, int32_t radius, int32_t thickness, int32_t startAngle, int32_t endAngle)
            : ProgressBar(x, y, 0, 0)
            , Radius(radius)
            , Thickness(thickness)
            , StartAngle(startAngle)
            , EndAngle(endAngle)
            , StartColor(ForegroundColor)
            , EndColor(0xFFFFFFFF)
            , StaticGradient(true)
        {
        }

        virtual ~CircleProgressBar();

        void SetStartAngle(float angle);
        void SetEndAngle(float angle);
        void SetRadius(int32_t radius);
        void SetThickness(int32_t thickness);
        void SetColors(uint32_t start, uint32_t end);
        void SetColor(uint32_t color);

        float GetStartAngle() const;
        float GetEndAngle() const;
        int32_t GetRadius() const;
        int32_t GetThickness() const;
        uint32_t GetStartColor() const;
        uint32_t GetEndColor() const;

        void UseStaticGradient();
        void UseProportionalGradient();

        static CircleProgressBar* BuildFromXML(XMLElement* xmlElement);
        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);

    private:
        int32_t Radius;
        int32_t Thickness;
        float StartAngle, EndAngle;
        uint32_t& StartColor;
        uint32_t EndColor;
        bool StaticGradient;
    };

} /* namespace grvl */

#endif /* GRVL_CIRCLEPROGRESSBAR_H_ */
