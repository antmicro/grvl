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

#include "GridCanvas.h"

#include "Manager.h"

namespace grvl {

    GridCanvas* GridCanvas::BuildFromXML(XMLElement* xmlElement)
    {
        Manager& manager = Manager::GetInstance();
        GridCanvas* gridCanvas = new GridCanvas();

        gridCanvas->InitFromXML(xmlElement);

        gridCanvas->SetHorizontalGridElementWidth(XMLSupport::GetAttributeOrDefault(xmlElement, "horizontalGridElementWidth", 0u));
        gridCanvas->SetHorizontalGridElementHeight(XMLSupport::GetAttributeOrDefault(xmlElement, "horizontalGridElementHeight", 0u));

        return gridCanvas;
    }

    void GridCanvas::SetHorizontalGridElementWidth(int32_t width)
    {
        horizontalGridElementWidth = width;
    }

    void GridCanvas::SetHorizontalGridElementHeight(int32_t height)
    {
        horizontalGridElementHeight = height;
    }

    void GridCanvas::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if (!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        DrawBackgroundItems(painter, RenderX, RenderY, Width, Height);
        DrawForegroundItems(painter, RenderX, RenderY, Width, Height);
    }

    void GridCanvas::DrawBackgroundItems(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight)
    {
        if (BackgroundColor & 0xFF000000 == 0) {
            return;
        }

        painter.FillRectangle(renderX, renderY, renderWidth, renderHeight, BackgroundColor);
        DrawBorderIfNecessary(painter, renderX, renderY, renderWidth, renderHeight);
    }

    void GridCanvas::DrawForegroundItems(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight)
    {
        if (ForegroundColor & 0xFF000000 == 0) {
            return;
        }

        DrawHorizontalLines(painter, renderX, renderY, renderWidth, renderHeight);
        DrawVerticalLines(painter, renderX, renderY, renderWidth, renderHeight);
    }

    void GridCanvas::DrawHorizontalLines(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight)
    {
        if (horizontalGridElementHeight <= 0) {
            return;
        }

        for (int32_t currentY = renderY; currentY <= renderY + renderHeight; currentY += horizontalGridElementHeight) {
            painter.DrawHLine(renderX, currentY, renderWidth, ForegroundColor);
        }
    }

    void GridCanvas::DrawVerticalLines(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight)
    {
        if (horizontalGridElementWidth <= 0) {
            return;
        }

        for (int32_t currentX = renderX; currentX <= renderX + renderWidth; currentX += horizontalGridElementWidth) {
            painter.DrawVLine(currentX, renderY, renderHeight, ForegroundColor);
        }
    }

} /* namespace grvl */
