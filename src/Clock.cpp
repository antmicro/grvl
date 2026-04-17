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

#include <grvl/Clock.h>
#include <grvl/Manager.h>
#include <grvl/XMLSupport.h>

namespace grvl {

    void Clock::Start()
    {
        isRunning = true;
    }

    void Clock::Stop()
    {
        isRunning = false;
    }

    bool Clock::IsRunning() const
    {
        return isRunning;
    }

    Clock* Clock::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();

        Clock* result = new Clock();
        result->InitFromXML(xmlElement);

        result->SetText("");

        result->SetTextColor(result->GetForegroundColor());
        result->SetTextFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal")));
        result->SetHorizontalAlignment(
            XMLSupport::ParseAlignmentOrDefault(xmlElement, "alignment", Label::Center));
        result->SetVisible(XMLSupport::GetAttributeOrDefault(xmlElement, "visible", true));

        const char* format = xmlElement->Attribute("format");
        if (format != NULL) {
            result->SetTimeFormat(format);
        }

        result->SetOnClickEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onClick"))));
        result->SetOnReleaseEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onRelease"))));
        result->SetOnPressEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onPress"))));

        result->Start();

        return result;
    }

    void Clock::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible || Width <= 0 || Height <= 0) {
            return;
        }

        if(isRunning) {
            time_t current_time;
            current_time = time(NULL);
            if(lastCurrentTime != current_time) {
                char buf[bufferSize];
                strftime(buf, bufferSize, format.c_str(), localtime(&current_time));
                SetText(buf);
                lastCurrentTime = current_time;
            }
        }
        Label::Draw(painter, ParentRenderX, ParentRenderY);
    }

    void Clock::SetTimeFormat(const char* fmt)
    {
        if (!fmt) {
            fmt = "%H:%M";
        }

        format = fmt;
    }

    void Clock::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Label::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("isRunning", Clock::JSGetIsRunningWrapper);
        jsObjectBuilder.AttachMemberFunction("Start", Clock::JSStartWrapper);
        jsObjectBuilder.AttachMemberFunction("Stop", Clock::JSStopWrapper);
    }

} /* namespace grvl */
