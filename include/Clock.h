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

#ifndef GRVL_CLOCK_H_
#define GRVL_CLOCK_H_

#include "stl.h"
#include <time.h>

#include "Font.h"
#include "Label.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Widget displaying current time.
    ///
    /// XML parameters:
    /// * id                    - widget identifier
    /// * x                     - widget position on x axis in pixels
    /// * y                     - widget position on y axis in pixels
    /// * width                 - widget width in pixels
    /// * height                - widget height in pixels
    /// * visible               - indicates if the widget is visible
    ///
    /// * foregroundColor       - text color (default: black)
    /// * backgroundColor       - background color (default: transparent)
    /// * font                  - text font (default: normal)
    /// * alignment             - horizontal text alignment (default: centered)
    ///
    /// XML events:
    /// * onClick               - event invoked when touch is released, but only when it has not left widget boundaries
    ///                           since pressing
    /// * onPress               - event invoked when touch is detected within widget boundaries
    /// * onRelease             - event invoked when touch is released within widget boundaries
    ///                           or when it leaves the boundaries
    ///
    class Clock : public Label {
    public:
        Clock()
            : Label()
            , isRunning(false)
            , lastCurrentTime(0)
        {
        }

        Clock(int32_t x, int32_t y, int32_t width, int32_t height)
            : Label(x, y, width, height)
            , isRunning(false)
            , lastCurrentTime(0)
        {
        }

        Clock(int32_t x, int32_t y, int32_t width, int32_t height, TextHorizontalAlignment alignment)
            : Label(x, y, width, height, "", alignment)
            , isRunning(false)
            , lastCurrentTime(0)
        {
        }

        Clock(const Clock& Obj)
            : Label(Obj)
            , isRunning(Obj.isRunning)
            , lastCurrentTime(Obj.lastCurrentTime)
        {
        }

        Clock& operator=(const Clock& Obj);

        virtual ~Clock();

        static Clock* BuildFromXML(XMLElement* xmlElement);

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);
        void SetVisibleSeconds(bool value) { visibleSeconds = value; }

        void Start();
        void Stop();
        bool IsRunning() const;

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;

        GENERATE_DUK_MEMBER_FUNCTION(Clock, Start)
        GENERATE_DUK_MEMBER_FUNCTION(Clock, Stop)
        GENERATE_DUK_BOOLEAN_GETTER(Clock, IsRunning, IsRunning)

    private:
        bool isRunning;
        bool visibleSeconds;
        time_t lastCurrentTime;
        static const int bufferSize = 10;
    };

} /* namespace grvl */

#endif /* GRVL_CLOCK_H_ */
