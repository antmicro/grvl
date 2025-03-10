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

#include "Graph.h"

#include <iomanip>
#include <sstream>
#include <cassert>

#include "Manager.h"

namespace grvl {

    Graph* Graph::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* manager = &Manager::GetInstance();

        Graph* graph = new Graph(0, 0, 0, 0);
        graph->InitFromXML(xmlElement);

        static constexpr int32_t DEFAULT_PADDING{0};
        graph->SetHorizontalPadding(XMLSupport::GetAttributeOrDefault(xmlElement, "horizontalPadding", DEFAULT_PADDING));
        graph->SetVerticalPadding(XMLSupport::GetAttributeOrDefault(xmlElement, "verticalPadding", DEFAULT_PADDING));

        int32_t topPaddingValue{0};
        if (XMLSupport::TryGetIntAttribute(xmlElement, "topPadding", &topPaddingValue)) {
            graph->SetTopPadding(topPaddingValue);
        }

        int32_t bottomPaddingValue{0};
        if (XMLSupport::TryGetIntAttribute(xmlElement, "bottomPadding", &bottomPaddingValue)) {
            graph->SetBottomPadding(bottomPaddingValue);
        }

        static constexpr uint32_t DEFAULT_TEXT_VERTICAL_OFFSET{0};
        graph->SetTextVerticalOffset(XMLSupport::GetAttributeOrDefault(xmlElement, "textVerticalOffset", DEFAULT_TEXT_VERTICAL_OFFSET));

        static const char* defaultTextColor = "0xFFFFFFFF";
        graph->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", defaultTextColor));

        static const char* NoGradient = "0xFFFFFFFF";
        graph->SetStartingGradientColor(XMLSupport::ParseColor(xmlElement, "gradientStartColor", NoGradient));
        graph->SetEndingGradientColor(XMLSupport::ParseColor(xmlElement, "gradientEndColor", NoGradient));

        const auto* fontName = XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal");
        const auto* font = manager->GetFontPointer(fontName);
        if(!font) {
            grvl::Log("[WARNING] Setting invalid label font %s, the label isn't going to be drawn", fontName);
        }
        graph->SetTextFont(font);

        return graph;
    }

    void Graph::SetHorizontalPadding(uint32_t horizontalPadding)
    {
        HorizontalPadding = horizontalPadding;
    }

    void Graph::SetVerticalPadding(uint32_t verticalPadding)
    {
        SetTopPadding(verticalPadding);
        SetBottomPadding(verticalPadding);
    }

    void Graph::SetTopPadding(uint32_t topPadding)
    {
        TopPadding = topPadding;
    }

    void Graph::SetBottomPadding(uint32_t bottomPadding)
    {
        BottomPadding = bottomPadding;
    }

    void Graph::SetTextVerticalOffset(uint32_t textVerticalOffset)
    {
        TextVerticalOffset = textVerticalOffset;
    }

    void Graph::SetTextColor(uint32_t textColor)
    {
        TextColor = textColor;
    }

    void Graph::SetTextFont(Font const* font)
    {
        TextFont = font;
    }

    void Graph::AddData(float value)
    {
        graphMinValue = std::min(graphMinValue, value);
        graphMaxValue = std::max(graphMaxValue, value);
        graphData.emplace_back(value);

        cubicSpline = CubicSplineInterpolation(graphData);
    }

    void Graph::ClearData()
    {
        graphMinValue = std::numeric_limits<float>::max();
        graphMaxValue = std::numeric_limits<float>::lowest();

        graphData.clear();
        cubicSpline = CubicSpline{};
    }

    // From https://en.wikipedia.org/wiki/Spline_(mathematics)#Algorithm_for_computing_natural_cubic_splines
    Graph::CubicSpline Graph::CubicSplineInterpolation(const std::vector<float>& dataValues)
    {
        int n = dataValues.size() - 1;

        if (n < 2) {
            return CubicSpline{};
        }

        CubicSpline cubicSpline{};

        cubicSpline.a.insert(cubicSpline.a.begin(), dataValues.begin(), dataValues.end());
        cubicSpline.b = std::vector<float>(n);
        cubicSpline.d = std::vector<float>(n);
        std::vector<float> h;
        for (int i = 0; i < n; ++i) {
            h.emplace_back(1);
        }

        std::vector<float> alpha;
        alpha.emplace_back(0);
        for (int i = 1; i < n; ++i) {
            alpha.emplace_back(3 * (cubicSpline.a[i + 1] - cubicSpline.a[i]) / h[i] - 3 * (cubicSpline.a[i] - cubicSpline.a[i - 1]) / h[i - 1]);
        }

        cubicSpline.c = std::vector<float>(n + 1);
        auto l = std::vector<float>(n + 1);
        auto mu = std::vector<float>(n + 1);
        auto z = std::vector<float>(n + 1);

        l[0] = 1;
        mu[0] = 0;
        z[0] = 0;

        for (int i = 1; i < n; ++i) {
            l[i] = 2 * (i + 1 - (i - 1)) - h[i - 1] * mu[i - 1];
            mu[i] = h[i] / l[i];
            z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
        }

        l[n] = 1;
        z[n] = 0;
        cubicSpline.c[n] = 0;

        for (int j = n - 1; j >= 0; --j) {
            cubicSpline.c[j] = z[j] - mu[j] * cubicSpline.c[j + 1];
            cubicSpline.b[j] = (cubicSpline.a[j + 1] - cubicSpline.a[j]) / h[j] - h[j] * (cubicSpline.c[j + 1] + 2 * cubicSpline.c[j]) / 3;
            cubicSpline.d[j] = (cubicSpline.c[j + 1] - cubicSpline.c[j]) / 3 / h[j];
        }

        return cubicSpline;
    }

    void Graph::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if (!Visible) {
            return;
        }

        if(Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        DrawBackgroundItems(painter, RenderX, RenderY, Width, Height);

        int32_t PaddedRenderX = RenderX + HorizontalPadding;
        int32_t PaddedRenderY = RenderY + TopPadding;
        int32_t PaddedRenderWidth = Width - HorizontalPadding * 2;
        int32_t PaddedRenderHeight = Height - TopPadding - BottomPadding;
        DrawLinesConnectingColumnPoints(painter, PaddedRenderX, PaddedRenderY, PaddedRenderWidth, PaddedRenderHeight);
    }

    void Graph::DrawBackgroundItems(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        painter.FillRectangle(RenderX, RenderY, RenderWidth, RenderHeight, BackgroundColor);
        painter.AddBackgroundBlock(RenderY, RenderHeight, BackgroundColor);
        DrawBorderIfNecessary(painter, RenderX, RenderY, RenderWidth, RenderHeight);
    }

    void Graph::DrawLinesConnectingColumnPoints(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        if (graphData.size() <= 2) {
            return;
        }

        int interval = RenderWidth / (graphData.size() - 1);

        for (int index = 0; index < graphData.size() - 1; ++index) {
            DrawCubicSpline(painter, RenderX + interval * index, RenderY, RenderX + interval * (index + 1), RenderHeight, cubicSpline.a[index], cubicSpline.b[index], cubicSpline.c[index], cubicSpline.d[index]);

            Point currentPoint = CalculateGraphPointFromData(index, RenderWidth, RenderHeight);
            DrawDataLabel(painter, RenderX, RenderY, RenderWidth, RenderHeight, currentPoint, graphData[index]);
        }

        Point currentPoint = CalculateGraphPointFromData(graphData.size() - 1, RenderWidth, RenderHeight);
        DrawDataLabel(painter, RenderX, RenderY, RenderWidth, RenderHeight, currentPoint, graphData[graphData.size() - 1]);
    }

    Graph::Point Graph::CalculateGraphPointFromData(int graphDataValueIndex, int32_t RenderWidth, int32_t RenderHeight)
    {
        assert(graphDataValueIndex >= 0 && graphDataValueIndex < graphData.size());

        float data = graphData[graphDataValueIndex];

        int32_t XPosition = RenderWidth * graphDataValueIndex / (graphData.size() - 1);

        float normalizedYCoordinate = 1.0f - (data - graphMinValue) / (graphMaxValue - graphMinValue);
        int32_t YPosition = RenderHeight * normalizedYCoordinate;

        return {XPosition, YPosition};
    }

    void Graph::DrawCubicSpline(Painter& painter, int32_t StartX, int32_t StartY, int32_t EndX, int32_t GraphHeight, float a, float b, float c, float d)
    {
        float dx = 1.0f / static_cast<float>(EndX - StartX);

        for (int i = 0; i < EndX - StartX; ++i) {
            float x = dx * i;
            float curveValue = a + b * x + c * x * x + d * x * x * x;
            float normalizedCurveValue = 1.0f - (curveValue - graphMinValue) / (graphMaxValue - graphMinValue);
            float PixelX = StartX + i;
            float PixelY = static_cast<float>(StartY) + static_cast<float>(GraphHeight) * normalizedCurveValue;
            DrawGradientUnderTheCurveIfNeeded(painter, PixelX, PixelY, StartY + GraphHeight, normalizedCurveValue);
            painter.DrawAntialiasedPixel(PixelX, PixelY, ForegroundColor);
        }
    }

    float Graph::EvaluateSplineCurve(float x)
    {
        int n = graphData.size();

        int i = static_cast<int>(std::floor(x));

        double dx = x - i;
        return cubicSpline.a[i] + cubicSpline.b[i] * dx + cubicSpline.c[i] * dx * dx + cubicSpline.d[i] * dx * dx * dx;
    }

    void Graph::DrawGradientUnderTheCurveIfNeeded(Painter& painter, int32_t PixelX, int32_t PixelY, int32_t EndY, float normalizedCurveYPosition)
    {
        if (HasGradientBackground()) {
            painter.DrawGradientVLine(PixelX, PixelY, EndY, GradientStartColor, GradientEndColor, normalizedCurveYPosition);
        }
    }

    void Graph::DrawDataLabel(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight, Point dataPoint, float dataValue)
    {
        std::string dataLabel = std::to_string(static_cast<int>(dataValue));
        uint16_t TextSize = TextFont->GetWidth(dataLabel.c_str());
        uint16_t BeginX = dataPoint.x - (TextSize / 2);
        uint16_t BeginY = dataPoint.y - TextFont->GetHeight() - TextVerticalOffset;
        painter.DisplayAntialiasedString(
            TextFont,
            RenderX + BeginX,
            RenderY + BeginY,
            dataLabel.c_str(),
            TextColor);
    }

    void Graph::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AttachMemberFunction("AddData", Graph::JSAddDataWrapper, 1);
        jsObjectBuilder.AttachMemberFunction("ClearData", Graph::JSClearDataWrapper, 0);
    }

    duk_ret_t Graph::JSAddDataWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        auto graph = static_cast<Graph*>(duk_to_pointer(ctx, -1));
        if (!graph) {
            return 0;
        }

        float data = duk_to_number(ctx, 0);
        graph->AddData(data);

        return 0;
    }

    duk_ret_t Graph::JSClearDataWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);
        auto graph = static_cast<Graph*>(duk_to_pointer(ctx, -1));
        if (!graph) {
            return 0;
        }

        graph->ClearData();

        return 0;
    }

} /* namespace grvl */
