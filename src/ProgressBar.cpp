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

#include "ProgressBar.h"
#include "grvl.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    ProgressBar::~ProgressBar()
    {
    }
    static constexpr auto progressMax = 100;
    void ProgressBar::SetProgressValue(float value)
    {
        if(value < 0) {
            value = 0;
        } else if(value > progressMax) {
            value = progressMax;
        }
        ProgressValue = value;
    }

    int32_t ProgressBar::GetProgressValue() const
    {
        return ProgressValue;
    }

    ProgressBar* ProgressBar::BuildFromXML(XMLElement* xmlElement)
    {
        ProgressBar* result = new ProgressBar();
        result->InitFromXML(xmlElement);

        result->SetForegroundColor(XMLSupport::ParseColor(xmlElement, "progressBarColor", "#ffffffff"));

        return result;
    }

    void ProgressBar::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        if(!Visible) {
            return;
        }

        painter.FillRectangle(ParentX + X, ParentY + Y, Width, Height, BackgroundColor);
        int barWidth = (Width) * ((ProgressValue) / 100.0);
        if(barWidth > 0) {
            painter.FillRectangle(ParentX + X, ParentY + Y, barWidth, Height, ForegroundColor);
        }
    }

    void ProgressBar::InitFromXML(XMLElement* xmlElement)
    {
        Component::InitFromXML(xmlElement);
    }

} /* namespace grvl */