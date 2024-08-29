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

#ifndef GRVL_PROGRESSBAR_H_
#define GRVL_PROGRESSBAR_H_

#include "Component.h"
#include "Painter.h"
#include "stl.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Rectangle progress bar widget.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * width                   - widget width in pixels
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * progressBarColor        - progress bar color (default: transparent)
    /// * backgroundColor         - background color (default: transparent)
    ///
    class ProgressBar : public Component {
    public:
        ProgressBar()
            : Component()
            , ProgressValue(0)
        {
        }

        ProgressBar(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
            , ProgressValue(0)
        {
        }
        virtual ~ProgressBar();

        void SetProgressValue(float value);
        int32_t GetProgressValue() const;

        static ProgressBar* BuildFromXML(XMLElement* xmlElement);

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);

    protected:
        float ProgressValue;
        virtual void InitFromXML(XMLElement* xmlElement);
    };

} /* namespace grvl */

#endif /* GRVL_PROGRESSBAR_H_ */
