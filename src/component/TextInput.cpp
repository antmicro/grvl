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

#include <grvl/Manager.h>

#include <algorithm>
#include <grvl/component/TextInput.h>

namespace grvl {
    TextInput::TextInput(const TextInput& other)
        : Button(other)
        , basicText(other.basicText)
        , type(other.type)
        , autoFocus(other.autoFocus)
        , cursorEnabled(other.cursorEnabled)
        , cursorColor(other.cursorColor)
        , cursorBlink(other.cursorBlink)
    {
        onTextInput.SetSenderPointer(this);
        onSubmit.SetSenderPointer(this);
    }

    TextInput& TextInput::operator=(const TextInput& other)
    {
        if(this == &other) {
            return *this;
        }

        Button::operator=(other);

        basicText = other.basicText;
        type = other.type;
        autoFocus = other.autoFocus;
        cursorEnabled = other.cursorEnabled;
        cursorColor = other.cursorColor;
        cursorBlink = other.cursorBlink;
        onTextInput.SetSenderPointer(this);
        onSubmit.SetSenderPointer(this);

        return *this;
    }

    Component* TextInput::Clone() const
    {
        return new TextInput(*this);
    }

    TextInput* TextInput::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        TextInput* result = new TextInput();

        result->InitFromXML(xmlElement);

        result->SetOnTextInputEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onTextInput"))));
        result->SetOnSubmitCallback(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onSubmit"))));

        result->SetBasicText(XMLSupport::GetAttributeOrDefault(xmlElement, "basicText", ""));

        result->SetTextColor(XMLSupport::ParseColor(xmlElement, "textColor", "#ffffffff"));
        result->SetActiveTextColor(XMLSupport::ParseColor(xmlElement, "activeTextColor", result->GetTextColor()));

        result->SetType(XMLSupport::GetAttributeOrDefault(xmlElement, "type", "text"));

        result->SetAutoFocus(XMLSupport::GetAttributeOrDefault(xmlElement, "autoFocus", false));
        if(result->IsAutoFocus()) {
            man->RequestAutoFocus(result->GetID());
        }

        result->SetCursorEnabled(XMLSupport::GetAttributeOrDefault(xmlElement, "cursor", false));
        result->SetCursorColor(XMLSupport::ParseColor(xmlElement, "cursorColor", result->GetTextColor()));
        result->SetCursorBlink(XMLSupport::GetAttributeOrDefault(xmlElement, "cursorBlink", true));

        return result;
    }

    void TextInput::SetOnTextInputEvent(Event event)
    {
        onTextInput = std::move(event);
        onTextInput.SetSenderPointer(this);
    }

    void TextInput::SetOnSubmitCallback(Event event)
    {
        onSubmit = std::move(event);
        onSubmit.SetSenderPointer(this);
    }

    const char* TextInput::GetBasicText() const
    {
        return basicText.c_str();
    }

    void TextInput::SetBasicText(const char* text)
    {
        basicText = text;
    }

    void TextInput::OnClick()
    {
        Button::OnClick();

        Manager& manager = Manager::GetInstance();
        manager.SetActiveInput(this);
    }

    void TextInput::AddCharacter(char character)
    {
        Text += character;
        ResetCursorBlink();
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::Append(const char* text)
    {
        Text += text;
        ResetCursorBlink();
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::RemoveLastCharacter()
    {
        if(Text.empty()) {
            return;
        }

        size_t pos = Text.size() - 1;
        while(pos > 0 && (static_cast<unsigned char>(Text[pos]) & 0xC0) == 0x80) {
            --pos;
        }
        Text.erase(pos);
        ResetCursorBlink();

        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::Clear()
    {
        Text.clear();
        ResetCursorBlink();
        Manager::GetInstance().GetEventsQueueInstance().push(&onTextInput);
    }

    void TextInput::Submit()
    {
        Manager::GetInstance().GetEventsQueueInstance().push(&onSubmit);
    }

    void TextInput::SetType(InputType type)
    {
        this->type = type;
    }

    const char* TextInput::GetType() const
    {
        return type;
    }

    void TextInput::SetType(const char* type)
    {
        this->type = type;
    }

    void TextInput::SetAutoFocus(bool enabled)
    {
        autoFocus = enabled;
    }

    bool TextInput::IsAutoFocus() const
    {
        return autoFocus;
    }

    int32_t TextInput::CursorGap(int32_t fontHeight)
    {
        return std::max(1, fontHeight / CURSOR_METRIC_DIVIDER);
    }

    int32_t TextInput::CursorOverhang(int32_t fontHeight)
    {
        return std::max(1, fontHeight / CURSOR_METRIC_DIVIDER);
    }

    void TextInput::ResetCursorBlink()
    {
        cursorBlinkStart = grvl::Callbacks()->get_timestamp();
    }

    void TextInput::SetCursorBlink(bool enabled)
    {
        cursorBlink = enabled;
    }

    bool TextInput::IsCursorBlinking() const
    {
        return cursorBlink;
    }

    void TextInput::SetCursorEnabled(bool enabled)
    {
        cursorEnabled = enabled;
    }

    bool TextInput::IsCursorEnabled() const
    {
        return cursorEnabled;
    }

    void TextInput::SetCursorColor(uint32_t color)
    {
        cursorColor = color;
    }

    uint32_t TextInput::GetCursorColor() const
    {
        return cursorColor;
    }

    size_t TextInput::GetUTF8CharacterCount() const
    {
        size_t count = 0;

        for(unsigned char c : Text) {
            if((c & 0xC0) != 0x80) {
                ++count;
            }
        }

        return count;
    }

    void TextInput::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Button::PopulateJavaScriptObject(jsObjectBuilder);
        jsObjectBuilder.AddProperty("basicText", TextInput::JSGetBasicTextWrapper, TextInput::JSSetBasicTextWrapper);
        jsObjectBuilder.AddProperty("type", TextInput::JSGetTypeWrapper, TextInput::JSSetTypeWrapper);
    }

    void TextInput::DrawText(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth, int32_t RenderHeight)
    {
        if(!ButtonFont) {
            return;
        }

        const char* textToDraw = GetTextToDraw();

        uint32_t CurrentTextColor = GetTextColor();
        uint16_t BeginX = HorizontalPadding;
        uint16_t BeginY = RenderHeight / 2 + (ButtonFont->GetFontHeight() / 2) + TextTopOffset;

        painter.DisplayBoundedAntialiasedString(
            ButtonFont,
            RenderX + BeginX,
            RenderY + BeginY,
            RenderX,
            RenderY,
            RenderWidth - HorizontalPadding,
            RenderHeight,
            textToDraw,
            CurrentTextColor);

        DrawCursor(painter, RenderX, RenderY, RenderWidth, RenderHeight, RenderY + BeginY, textToDraw);
    }

    void TextInput::DrawCursor(Painter& painter, int32_t RenderX, int32_t RenderY, int32_t RenderWidth,
                               int32_t RenderHeight, int32_t BaselineY, const char* textToDraw)
    {
        if(!cursorEnabled || Manager::GetInstance().GetActiveInput() != this) {
            return;
        }

        // GetTextToDraw() returns the placeholder when nothing is typed, so the
        // caret must not sit behind it.
        const int32_t fontHeight = ButtonFont->GetFontHeight();
        const int32_t overhang = CursorOverhang(fontHeight);

        if(cursorBlink) {
            // Frames must keep coming, or the caret freezes mid-cycle.
            painter.RequestNextFrame();

            const uint64_t elapsed = grvl::Callbacks()->get_timestamp() - cursorBlinkStart;
            if((elapsed / CURSOR_BLINK_PERIOD_MS) % 2 != 0) {
                return;
            }
        }

        const int32_t typedWidth = Text.empty() ? 0 : ButtonFont->GetWidth(textToDraw) + CursorGap(fontHeight);
        const int32_t height = fontHeight + 2 * overhang;
        const int32_t lastColumn = RenderX + RenderWidth - HorizontalPadding - CURSOR_WIDTH;

        int32_t x = RenderX + HorizontalPadding + typedWidth;
        if(x > lastColumn) {
            x = lastColumn;
        }

        painter.FillRectangle(x, BaselineY - height + overhang, CURSOR_WIDTH, height, cursorColor);
    }

    const char* TextInput::GetTextToDraw()
    {
        if(Text.empty()) {
            return basicText.c_str();
        }

        switch(type) {
            case InputType::TEXT:
                return Text.c_str();

            case InputType::PASSWORD: {
                const size_t count = GetUTF8CharacterCount();

                constexpr char mask_char[] = "\xE2\x97\x8F"; // Unicode character 'Black Circle'
                constexpr size_t mask_char_len = sizeof(mask_char) - 1;

                masked.clear();
                masked.reserve(count * mask_char_len);

                for(size_t i = 0; i < count; ++i) {
                    masked += mask_char;
                }

                return masked.c_str();
            }
        }

        return "";
    }

} /* namespace grvl */
