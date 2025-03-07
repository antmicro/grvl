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

#include "Clock.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    Clock::~Clock()
    {
    }

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

        int32_t x;
        int32_t y;
        int32_t height;
        int32_t width;
        if(!XMLSupport::TryGetIntAttribute(xmlElement, "x", &x)
           || !XMLSupport::TryGetIntAttribute(xmlElement, "y", &y)
           || !XMLSupport::TryGetIntAttribute(xmlElement, "width", &width)
           || !XMLSupport::TryGetIntAttribute(xmlElement, "height", &height)) {
            return NULL;
        }

        Clock* result = new Clock(x, y, width, height);
        const char* id = xmlElement->Attribute("id");
        if(id != NULL) {
            result->SetID(id);
        }
        result->SetText("");
        static constexpr uint32_t defaultFg = 0xFFFFFFFF; /*white*/
        result->SetForegroundColor(XMLSupport::ParseColor(xmlElement, "foregroundColor", defaultFg));
        result->SetBackgroundColor(XMLSupport::GetAttributeOrDefault(xmlElement, "backgroundColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));

        result->SetTextFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal")));
        result->SetHorizontalAlignment(
            XMLSupport::ParseAlignmentOrDefault(xmlElement, "alignment", Label::Center));
        result->SetVisible(XMLSupport::GetAttributeOrDefault(xmlElement, "visible", true));
        result->SetVisibleSeconds(XMLSupport::GetAttributeOrDefault(xmlElement, "seconds", false));
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
                strftime(buf, bufferSize, visibleSeconds ? "%H:%M:%S" : "%H:%M", localtime(&current_time));
                SetText(buf);
                lastCurrentTime = current_time;
            }
        }
        Label::Draw(painter, ParentRenderX, ParentRenderY);
    }

    Clock& Clock::operator=(const Clock& Obj)
    {
        if(this != &Obj) {
            Label::operator=(Obj);
            isRunning = Obj.isRunning;
            lastCurrentTime = Obj.lastCurrentTime;
        }
        return *this;
    }
    
    void Clock::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Label::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("isRunning", Clock::JSGetIsRunningWrapper);
        jsObjectBuilder.AttachMemberFunction("Start", Clock::JSStartWrapper);
        jsObjectBuilder.AttachMemberFunction("Stop", Clock::JSStopWrapper);
    }

} /* namespace grvl */
