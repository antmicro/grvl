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

#include "ListItem.h"
#include "Manager.h"
#include "XMLSupport.h"

namespace grvl {

    ListItem::~ListItem()
    {
    }

    static constexpr auto listItemBeginOffset = 20;
    static constexpr auto listItemBeginOffsetSmall = 10;
    static constexpr auto listItemBeginOffsetBig = 40;
    static constexpr auto listItemWidthOffset = 30;

    void ListItem::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        if(!Visible) {
            return;
        }

        int32_t TextWidth;
        int32_t BeginX;
        int32_t BeginY;
        int32_t HeightClamped;
        bool CutTop = false;
        int32_t OffsetY = 0;

        if(ParentHeight < 0) {
            CutTop = true;
            OffsetY = -ParentHeight;
        }

        if(!CutTop) {
            HeightClamped = Clamp(Height, 0, ParentHeight);
        } else { // If negative height
            HeightClamped = Clamp(Height, 0, Height - OffsetY);
        }

        if(HeightClamped <= 0) {
            return;
        }

        BeginX = X;
        BeginY = Y;

        uint32_t BackColor = 0;
        uint32_t ForeColor = 0;
        uint32_t DescColor = 0;

        if(State == Off) {
            BackColor = BackgroundColor;
            ForeColor = ForegroundColor;
            DescColor = DescriptionColor;
        } else if(State == On) {
            BackColor = ActiveBackgroundColor;
            ForeColor = ActiveForegroundColor;
            DescColor = ActiveDescriptionColor;
        }

        // Fixme This might be moved to the Label or/and Button class.
        if(!roundingImage.IsEmpty()) {

            roundingImage.SetActiveFrame(State == Off ? 0 : 1);

            roundingImage.SetPosition(0, 0);
            roundingImage.Draw(painter, ParentX + X, ParentY + Y, Width, ParentHeight);

            roundingImage.SetPosition(Width - roundingImage.GetWidth(), 0);
            roundingImage.Draw(painter, ParentX + X, ParentY + Y, Width, ParentHeight);

            roundingImage.SetPosition(0, Height - roundingImage.GetHeight());
            roundingImage.Draw(painter, ParentX + X, ParentY + Y, Width, ParentHeight);

            roundingImage.SetPosition(Width - roundingImage.GetWidth(), Height - roundingImage.GetHeight());
            roundingImage.Draw(painter, ParentX + X, ParentY + Y, Width, ParentHeight);

            int topPartHeight = CutTop ? max(0, min(roundingImage.GetHeight() / 2 - OffsetY, roundingImage.GetHeight() / 2)) : min(HeightClamped, roundingImage.GetHeight() / 2);
            int bottomPartHeight = CutTop ? min(HeightClamped, roundingImage.GetHeight() / 2) : max(0, min((roundingImage.GetHeight() / 2) - Height + HeightClamped, roundingImage.GetHeight() / 2));

            if(topPartHeight > 0) {
                painter.FillRectangle(ParentX + X + roundingImage.GetWidth() / 2, OffsetY + ParentY + Y, Width - roundingImage.GetWidth(), topPartHeight, BackColor);
            }

            if(HeightClamped - bottomPartHeight - topPartHeight > 0) {
                painter.FillRectangle(ParentX + X, OffsetY + ParentY + Y + topPartHeight, Width, HeightClamped - topPartHeight - bottomPartHeight, BackColor);
            }

            if(bottomPartHeight > 0) {
                painter.FillRectangle(ParentX + X + roundingImage.GetWidth() / 2, OffsetY + ParentY + Y + HeightClamped - bottomPartHeight, Width - roundingImage.GetWidth(), bottomPartHeight, BackColor);
            }
        } else {
            painter.FillRectangle(ParentX + X, OffsetY + ParentY + Y, Width, HeightClamped, BackColor);
        }

        if(ButtonFont == NULL) {
            return; // font not available
        }

        TextWidth = ButtonFont->GetWidth(Text.c_str());
        if(Text.empty() && ButtonImage.IsEmpty()) {
            return;
        }

        if(!ButtonImage.IsEmpty()) {
            static constexpr auto listButtonPosX = 10;
            ButtonImage.SetPosition(listButtonPosX, Height / 2 - ButtonImage.GetHeight() / 2); // TODO: move to "add image"
            ButtonImage.Draw(painter, ParentX + X, ParentY + Y, Width, ParentHeight);
            BeginX += ButtonImage.GetWidth() + listItemBeginOffset;
        }

        if(!AdditionalImge.IsEmpty()) {
            static constexpr auto additionalImageOffset = 22;
            AdditionalImge.SetPosition(Width - AdditionalImge.GetWidth() - additionalImageOffset, Height / 2 - AdditionalImge.GetHeight() / 2);
            AdditionalImge.Draw(painter, ParentX + X, ParentY + Y, Width, ParentHeight);
        }

        if(Description.empty()) {
            if(Type == AlarmField) {
                BeginX = BeginX + listItemBeginOffset;
            } else {
                BeginX = (BeginX / 2) + (Width / 2) - (TextWidth / 2);
            }
            BeginY = BeginY + (Height / 2) - ButtonFont->GetHeight() / 2;
            painter.DisplayBoundedAntialiasedString(
                ButtonFont, ParentX + BeginX, ParentY + BeginY, ParentX + X, ParentY + Y, ParentWidth, ParentHeight,
                Text.c_str(), ForeColor);
        } else {
            int YOffset = (Type == DoubleImageField) ? ButtonFont->GetHeight() / 2 : 0;

            if(Type == AlarmField) {
                BeginX = BeginX + listItemBeginOffsetBig;
                BeginY = BeginY + (Height / 2) - ButtonFont->GetHeight() / 2;
                painter.DisplayBoundedAntialiasedString(
                    ButtonFont, ParentX + BeginX, ParentY + BeginY, ParentX + X, ParentY + Y, ParentWidth, ParentHeight,
                    Text.c_str(), ForeColor);
                painter.DisplayBoundedAntialiasedString(
                    DescriptionFont, ParentX + BeginX + ButtonFont->GetWidth(Text.c_str()) + listItemBeginOffset,
                    ParentY + BeginY + (ButtonFont->GetHeight() - DescriptionFont->GetHeight()), ParentX + X, ParentY + Y,
                    ParentWidth, ParentHeight, Description.c_str(), DescColor);
            } else if(ButtonFont && DescriptionFont){
                BeginX = BeginX + listItemBeginOffsetSmall;
                BeginY = BeginY + Height / 2 - (ButtonFont->GetHeight() + DescriptionFont->GetHeight() / 2) + YOffset;
                painter.DisplayBoundedAntialiasedString(
                    ButtonFont, ParentX + BeginX, ParentY + BeginY, ParentX + X, ParentY + Y, ParentWidth, ParentHeight,
                    Text.c_str(), ForeColor);
                painter.DisplayBoundedAntialiasedString(
                    DescriptionFont, ParentX + BeginX, ParentY + BeginY + ButtonFont->GetHeight(), ParentX + X, ParentY + Y,
                    ParentWidth, ParentHeight, Description.c_str(), DescColor);
            }
        }

        if(Type == LeftArrowField) {
            painter.DisplayBoundedAntialiasedString(
                ButtonFont, ParentX + listItemBeginOffset, ParentY + BeginY, ParentX + X, ParentY + Y, ParentWidth, ParentHeight, "<",
                ForeColor);
        } else if(Type == RightArrowField) {
            painter.DisplayBoundedAntialiasedString(
                ButtonFont, ParentX + Width - listItemWidthOffset, ParentY + BeginY, ParentX + X, ParentY + Y, ParentWidth, ParentHeight,
                ">", ForeColor);
        } else if(Type == Dots) {
            painter.DisplayBoundedAntialiasedString(
                ButtonFont, ParentX + Width - ButtonFont->GetHeight(), ParentY + BeginY, ParentX + X, ParentY + Y,
                ParentWidth, ParentHeight, "...", ForeColor);
        }
    }

    void ListItem::PrepareContent(ContentManager* contentManager)
    {
        if(!ButtonImage.IsEmpty() && ButtonImage.GetContent() == NULL && !ButtonImage.IsImageContentPending()) {
            contentManager->RequestBinding(&ButtonImage);
        }
    }

    void ListItem::CancelPreparingContent(ContentManager* contentManager)
    {
        if(!ButtonImage.IsEmpty() && ButtonImage.IsImageContentPending()) {
            contentManager->CancelRequest(&ButtonImage);
        }
    }

    void ListItem::SetDescription(const char* desc)
    {
        if(!desc) {
            return;
        }
        Description = string(desc);
    }

    void ListItem::SetDescriptionFont(Font const* font)
    {
        if(!font) {
            return;
        }
        DescriptionFont = font;
    }

    void ListItem::SetType(ItemType type)
    {
        Type = type;
    }

    void ListItem::SetDescriptionColor(uint32_t color)
    {
        DescriptionColor = color;
    }

    void ListItem::SetActiveDescriptionColor(uint32_t color)
    {
        ActiveDescriptionColor = color;
    }

    uint32_t ListItem::GetDescriptionColor() const
    {
        return DescriptionColor;
    }

    uint32_t ListItem::GetActiveDescriptionColor() const
    {
        return ActiveDescriptionColor;
    }

    enum ListItem::ItemType ListItem::ParseListItemTypeOrDefault(const char* value, enum ListItem::ItemType defaultValue)
    {
        if(value != NULL) {
            if(strcmp("StdListField", value) == 0) {
                return ListItem::StdListField;
            }
            if(strcmp("LeftArrowField", value) == 0) {
                return ListItem::LeftArrowField;
            }
            if(strcmp("RightArrowField", value) == 0) {
                return ListItem::RightArrowField;
            }
            if(strcmp("EmptyField", value) == 0) {
                return ListItem::EmptyField;
            }
            if(strcmp("Dots", value) == 0) {
                return ListItem::Dots;
            }
            if(strcmp("DoubleImageField", value) == 0) {
                return ListItem::DoubleImageField;
            }
            if(strcmp("AlarmField", value) == 0) {
                return ListItem::AlarmField;
            }
        }

        return defaultValue;
    }

    ListItem* ListItem::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();

        const char* tempChar = xmlElement->Attribute("type");
        ListItem::ItemType type = (ParseListItemTypeOrDefault(tempChar, ListItem::StdListField));

        ListItem* result = new ListItem(0, 0, 0, 0, type);

        result->InitFromXML(xmlElement);

        result->SetDescription(XMLSupport::GetAttributeOrDefault(xmlElement, "description", ""));

        const char* descriptionFontName = XMLSupport::GetAttributeOrDefault(xmlElement, "descriptionFont", "");
        if(descriptionFontName[0] != 0) { // Not an empty string
            result->SetDescriptionFont(man->GetFontPointer(descriptionFontName));
        }
        static constexpr uint32_t defaultFg = 0xFFFFFFFF;
        result->SetForegroundColor(XMLSupport::ParseColor(xmlElement, "textColor", defaultFg));
        result->SetActiveForegroundColor(XMLSupport::ParseColor(xmlElement, "activeTextColor", result->GetForegroundColor()));
        result->SetDescriptionColor(XMLSupport::ParseColor(xmlElement, "descriptionColor", defaultFg));
        result->SetActiveDescriptionColor(XMLSupport::ParseColor(xmlElement, "activeDescriptionColor", result->GetDescriptionColor()));

        tempChar = xmlElement->Attribute("image");
        if(tempChar) {
            man->BindImageContentToImage(tempChar, result->GetImagePointer());
        }

        tempChar = xmlElement->Attribute("additionalImage");
        if(tempChar) {
            man->BindImageContentToImage(tempChar, result->GetAdditionalImagePointer());
        }

        tempChar = xmlElement->Attribute("roundingImage");
        if(tempChar) {
            man->BindImageContentToImage(tempChar, result->GetRoundingImagePointer());
        }

        result->SetOnLongPressEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPress"))));
        result->SetOnLongPressRepeatEvent(man->GetOrCreateCallback(XMLSupport::ParseCallback(xmlElement->Attribute("onLongPressRepeat"))));

        return result;
    }

    void ListItem::SetAdditionalImage(const Image& image)
    {
        AdditionalImge = image;
    }

    Image* ListItem::GetAdditionalImagePointer()
    {
        return &AdditionalImge;
    }

    void ListItem::SetParent(Component* NewParent)
    {
        if(NewParent) {
            Parent = NewParent;
        }
    }

    Component* ListItem::GetParent()
    {
        return Parent;
    }

    void ListItem::SetVisible(bool state)
    {
        ListView* parent;

        Visible = state;
        parent = (ListView*)this->GetParent();
        if(parent) {
            parent->Refresh();
        }
    }

    void ListItem::Hide()
    {
        this->SetVisible(false);
    }

    void ListItem::Show()
    {
        this->SetVisible(true);
    }

    void ListItem::SetRoundingImage(const Image& image)
    {
        roundingImage = image;
    }

    Image* ListItem::GetRoundingImagePointer()
    {
        return &roundingImage;
    }

} /* namespace grvl */
