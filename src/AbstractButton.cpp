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

#include "AbstractButton.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    AbstractButton::AbstractButton(const AbstractButton& other)
        : Component{other}, Text{other.Text}, ButtonImage{other.ButtonImage}, ButtonFont{other.ButtonFont}
    {
    }

    AbstractButton& AbstractButton::operator=(const AbstractButton& other)
    {
        if (this == &other) {
            return *this;
        }

        Component::operator=(other);

        Text = other.Text;
        ButtonImage = other.ButtonImage;
        ButtonFont = other.ButtonFont;

        return *this;
    }

    void AbstractButton::SetText(const char* text)
    {
        Text = string(text);
    }

    void AbstractButton::SetImage(const Image& image)
    {
        if(image.IsEmpty()) {
            return;
        }
        ButtonImage = image;
    }

    Font const* AbstractButton::GetButtonFont()
    {
        return ButtonFont;
    }

    void AbstractButton::SetTextFont(Font const* font)
    {
        ButtonFont = font;
    }

    void AbstractButton::ClearButtonFont()
    {
        ButtonFont = 0;
    }

    bool AbstractButton::IsEmpty() const
    {
        return Text.empty() && ButtonImage.IsEmpty();
    }

    const char* AbstractButton::GetText()
    {
        return Text.c_str();
    }

    Image* AbstractButton::GetImagePointer()
    {
        return &ButtonImage;
    }

    void AbstractButton::InitFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        Component::InitFromXML(xmlElement);

        this->SetText(XMLSupport::GetAttributeOrDefault(xmlElement, "text", ""));
        this->SetTextFont(man->GetFontPointer(XMLSupport::GetAttributeOrDefault(xmlElement, "font", "normal")));
    }

    void AbstractButton::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("text", AbstractButton::JSGetTextWrapper, AbstractButton::JSSetTextWrapper);
    }

} /* namespace grvl */
