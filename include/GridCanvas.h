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

#ifndef GRVL_GRID_CANVAS_H_
#define GRVL_GRID_CANVAS_H_

#include "Canvas.h"

#include "XMLSupport.h"

namespace grvl {

    /// GUI widget for drawing regular grid as background.
    class GridCanvas : public Canvas {
    public:
        GridCanvas() = default;

        GridCanvas(int32_t x, int32_t y, int32_t width, int32_t height)
            : Canvas{x, y, width, height} {}

        static GridCanvas* BuildFromXML(XMLElement* xmlElement);

        void SetHorizontalGridElementWidth(int32_t width);
        void SetHorizontalGridElementHeight(int32_t height);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

    private:
        int32_t horizontalGridElementWidth{0};
        int32_t horizontalGridElementHeight{0};

        void DrawBackgroundItems(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight);
        void DrawForegroundItems(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight);
        void DrawHorizontalLines(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight);
        void DrawVerticalLines(Painter& painter, int32_t renderX, int32_t renderY, int32_t renderWidth, int32_t renderHeight);
    };

} /* namespace grvl */

#endif /* GRVL_CANVAS_H_ */
