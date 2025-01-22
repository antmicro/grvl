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

#include "TextInput.h"

#include "Manager.h"

namespace grvl {

    TextInput* TextInput::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        TextInput* result = new TextInput();

        result->InitFromXML(xmlElement);

        result->SetOnTextInputEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onTextInput"))));

        result->SetBasicText(XMLSupport::GetAttributeOrDefault(xmlElement, "basicText", ""));

        result->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", "#ffffffff"));
        result->SetActiveTextColor(XMLSupport::ParseColor(xmlElement, "activeTextColor", result->GetTextColor()));

        return result;
    }

    void TextInput::SetOnTextInputEvent(Event event)
    {
        onTextInput = std::move(event);
        onTextInput.SetSenderPointer(this);
    }

    void TextInput::SetBasicText(const char* text)
    {
        basicText = text;
    }

    void TextInput::OnClick()
    {
        Button::OnClick();

        Manager& manager = Manager::GetInstance();
        manager.ShowKeyboard(this);
    }

    void TextInput::AddCharacter(char character)
    {
        Text += character;
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::Append(const char* text)
    {
        Text += text;
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::RemoveLastCharacter()
    {
        if (Text.empty()) {
            return;
        }

        Text.erase(Text.length() - 1);
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::Clear()
    {
        Text.clear();
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        const char* textToDraw = Text.empty() ? basicText.c_str() : Text.c_str();

        uint32_t CurrentTextColor = GetTextColor();
        uint16_t TextSize = ButtonFont->GetWidth(textToDraw);
        uint16_t BeginX = 10;
        uint16_t BeginY = RenderHeight / 2 - (ButtonFont->GetHeight() / 2) + TextTopOffset;

        painter.DisplayBoundedAntialiasedString(
            ButtonFont,
            RenderX + BeginX,
            RenderY + BeginY,
            RenderX,
            RenderY,
            RenderWidth,
            RenderHeight,
            textToDraw,
            CurrentTextColor);
    }

} /* namespace grvl */
