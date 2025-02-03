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

#include "Checkbox.h"

#include "Manager.h"

namespace grvl {

    Component* Checkbox::Clone() const
    {
        return new Checkbox(*this);
    }

    Checkbox* Checkbox::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        Checkbox* result = new Checkbox();

        result->InitFromXML(xmlElement);
        static constexpr auto defaultTextColor = 0xFFFFFFFF;

        result->SetOnSwitchONEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSwitchON"))));
        result->SetOnSwitchOFFEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSwitchOFF"))));
        result->SetOnLongPressEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPress"))));
        result->SetOnLongPressRepeatEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPressRepeat"))));

        return result;
    }

    void Checkbox::DrawActiveState(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        painter.FillRoundRectangle(RenderX, RenderY, RenderWidth, RenderHeight, ActiveBackgroundColor, BorderArcRadius);
        DrawBorderIfNecessary(painter, RenderX, RenderY, RenderWidth, RenderHeight);
    }

    void Checkbox::DrawInactiveState(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        painter.FillRoundRectangle(RenderX, RenderY, RenderWidth, RenderHeight, BackgroundColor, BorderArcRadius);
        DrawBorderIfNecessary(painter, RenderX, RenderY, RenderWidth, RenderHeight);
    }

} /* namespace grvl */
