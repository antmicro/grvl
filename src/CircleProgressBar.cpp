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
 
#include "CircleProgressBar.h"
#include "grvl.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    CircleProgressBar::~CircleProgressBar()
    {
    }

    CircleProgressBar* CircleProgressBar::BuildFromXML(XMLElement* xmlElement)
    {
        int32_t startAngle;
        int32_t endAngle;
        int32_t radius;
        int32_t thickness;
        startAngle = XMLSupport::GetAttributeOrDefault(xmlElement, "startAngle", (uint32_t)0);
        static constexpr auto defaultEndAngle = 360;
        endAngle = XMLSupport::GetAttributeOrDefault(xmlElement, "endAngle", (uint32_t)defaultEndAngle);
        radius = XMLSupport::GetAttributeOrDefault(xmlElement, "radius", (uint32_t)0);
        thickness = XMLSupport::GetAttributeOrDefault(xmlElement, "thickness", (uint32_t)0);
        bool staticGradient = XMLSupport::GetAttributeOrDefault(xmlElement, "staticGradient", true);

        CircleProgressBar* result = new CircleProgressBar(0, 0, radius, thickness, startAngle, endAngle);

        result->InitFromXML(xmlElement);

        result->SetColors(
            XMLSupport::ParseColor(xmlElement, "startColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT),
            XMLSupport::ParseColor(xmlElement, "endColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));

        if(!staticGradient) {
            result->UseProportionalGradient();
        }

        return result;
    }

    void CircleProgressBar::Draw(Painter& painter, int32_t ParentX, int32_t ParentY)
    {
        if(!Visible) {
            return;
        }

        int middleAngle = (EndAngle - StartAngle);
        static constexpr auto progressFull = 100;
        middleAngle = StartAngle + (float)middleAngle * ((float)ProgressValue / progressFull);

        uint32_t endColor = 0;

        if(StaticGradient) {
            static constexpr auto alpha = 0xFF000000;
            static constexpr auto red = 0x00FF0000;
            static constexpr auto green = 0x0000FF00;
            static constexpr auto blue = 0x000000FF;
            // NOLINTBEGIN
            float alphaEnd = (float)(((int32_t)(EndColor & alpha) >> 24) - ((int32_t)(StartColor & alpha) >> 24))
                    * ((float)ProgressValue / progressFull)
                + ((int32_t)(StartColor & alpha) >> 24);
            float redEnd = (float)(((int32_t)(EndColor & red) >> 16) - ((int32_t)(StartColor & red) >> 16))
                    * ((float)ProgressValue / progressFull)
                + ((int32_t)(StartColor & red) >> 16);
            float greenEnd = (float)(((int32_t)(EndColor & green) >> 8) - ((int32_t)(StartColor & green) >> 8))
                    * ((float)ProgressValue / progressFull)
                + ((int32_t)(StartColor & green) >> 8);
            float blueEnd = (float)(((int32_t)(EndColor & blue) >> 0) - ((int32_t)(StartColor & blue) >> 0))
                    * ((float)ProgressValue / progressFull)
                + ((int32_t)(StartColor & blue) >> 0);

            endColor = (uint32_t)(alphaEnd) << 24 | (uint32_t)(redEnd) << 16 | (uint32_t)(greenEnd) << 8
                | (uint32_t)(blueEnd);
            // NOLINTEND
        } else {
            endColor = EndColor;
        }

        painter.FillArc(ParentX + X, ParentY + Y, StartAngle, middleAngle, Radius, Thickness, StartColor, endColor);
        if(BackgroundColor) { // Do not draw when background is transparent
            painter.FillArc(ParentX + X, ParentY + Y, middleAngle, EndAngle, Radius, Thickness, BackgroundColor, BackgroundColor);
        }

        DrawBorderIfNecessary(painter, ParentX + X, ParentY + Y, Width, Height);
    }

    void CircleProgressBar::SetStartAngle(float angle)
    {
        StartAngle = ConstrainAngle(angle);
    }

    void CircleProgressBar::SetEndAngle(float angle)
    {
        EndAngle = ConstrainAngle(angle);
    }

    float CircleProgressBar::GetStartAngle() const
    {
        return StartAngle;
    }

    float CircleProgressBar::GetEndAngle() const
    {
        return EndAngle;
    }

    void CircleProgressBar::SetRadius(int32_t radius)
    {
        Radius = radius;
    }

    void CircleProgressBar::SetThickness(int32_t thickness)
    {
        Thickness = thickness;
    }

    void CircleProgressBar::SetColors(uint32_t start, uint32_t end)
    {
        StartColor = start;
        EndColor = end;
    }

    void CircleProgressBar::SetColor(uint32_t color)
    {
        StartColor = color;
        EndColor = color;
    }

    int32_t CircleProgressBar::GetRadius() const
    {
        return Radius;
    }

    int32_t CircleProgressBar::GetThickness() const
    {
        return Thickness;
    }

    uint32_t CircleProgressBar::GetStartColor() const
    {
        return StartColor;
    }

    uint32_t CircleProgressBar::GetEndColor() const
    {
        return EndColor;
    }

    void CircleProgressBar::UseStaticGradient()
    {
        StaticGradient = true;
    }

    void CircleProgressBar::UseProportionalGradient()
    {
        StaticGradient = false;
    }

} /* namespace grvl */
