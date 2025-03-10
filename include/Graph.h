// Copyright 2014-2025 Antmicro <antmicro.com>
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

#ifndef GRVL_GRAPH_H_
#define GRVL_GRAPH_H_

#include <limits>

#include "Component.h"
#include "Painter.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {

    class Graph : public Component {
    public:
        Graph() = default;

        Graph(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component{x, y, width, height}
        {
        }

        static Graph* BuildFromXML(XMLElement* xmlElement);

        void SetHorizontalPadding(uint32_t horizontalPadding);
        void SetVerticalPadding(uint32_t verticalPadding);
        void SetTopPadding(uint32_t topPadding);
        void SetBottomPadding(uint32_t bottomPadding);

        void SetTextVerticalOffset(uint32_t textVerticalOffset);
        void SetTextColor(uint32_t textColor);
        void SetTextFont(const Font* font);

        void SetStartingGradientColor(uint32_t startingGradientColor) { GradientStartColor = startingGradientColor; }
        void SetEndingGradientColor(uint32_t endingGradientColor) { GradientEndColor = endingGradientColor; }

        void AddData(float value);
        void ClearData();

        float GetGraphMinValue() const { return graphMinValue; }
        float GetGraphMaxValue() const { return graphMaxValue; }

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;
        static duk_ret_t JSAddDataWrapper(duk_context* ctx);
        static duk_ret_t JSClearDataWrapper(duk_context* ctx);

    private:
        uint32_t HorizontalPadding{0};
        uint32_t TopPadding{0};
        uint32_t BottomPadding{0};

        uint32_t TextVerticalOffset{0};
        uint32_t TextColor{0};
        const Font* TextFont{nullptr};

        bool HasGradientBackground() const { return GradientStartColor != 0 && GradientEndColor != 0; }

        uint32_t GradientStartColor{0};
        uint32_t GradientEndColor{0};

        std::vector<float> graphData{};

        struct CubicSpline {
            std::vector<float> a{};
            std::vector<float> b{};
            std::vector<float> c{};
            std::vector<float> d{};

            bool valid() const { return !a.empty() && !b.empty() && !c.empty() && !d.empty(); }
        };

        CubicSpline CubicSplineInterpolation(const std::vector<float>& dataValues);
        float EvaluateSplineCurve(float x);

        CubicSpline cubicSpline{};

        float graphMinValue{std::numeric_limits<float>::max()};
        float graphMaxValue{std::numeric_limits<float>::lowest()};

        struct Point {
            int32_t x{0};
            int32_t y{0};
        };

        void DrawBackgroundItems(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight);
        void DrawDataLabel(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight, Point dataPoint, float dataValue);
        void DrawLinesConnectingColumnPoints(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight);
        Point CalculateGraphPointFromData(int graphDataValueIndex, int32_t RenderWidth, int32_t RenderHeight);
        void DrawCubicSpline(Painter& painter, int32_t StartX, int32_t StartY, int32_t EndX, int32_t GraphHeight, float a, float b, float c, float d);
        void DrawGradientUnderTheCurveIfNeeded(Painter& painter, int32_t PixelX, int32_t PixelY, int32_t EndY, float normalizedCurveYPosition);
    };

} /* namespace grvl */

#endif /* GRVL_GRAPH_H_ */
