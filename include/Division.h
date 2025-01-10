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

#ifndef GRVL_DIVISION_H_
#define GRVL_DIVISION_H_

#include "Container.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    class Division : public Container {

    public:
        Division() = default;

        Division(int32_t x, int32_t y, int32_t width, int32_t height)
            : Container{x, y, width, height} {}

        Component* Clone() const override;

        static Division* BuildFromXML(XMLElement* xmlElement);

        virtual void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY);

    private:
        void DrawBackgroundItems(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight);
        void FillBackground(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight);
        void DrawChildrenComponents(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight, int32_t verticalOffset);
        void DrawChildComponent(Component* component, Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight, int32_t verticalOffset);
    };

} /* namespace grvl */

#endif /* GRVL_DIVISION_H_ */
