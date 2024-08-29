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

#ifndef GRVL_LISTITEM_H_
#define GRVL_LISTITEM_H_

#include "AbstractButton.h"
#include "Font.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Widget representing an element of a list displayed by ListView.
    ///
    /// XML events:
    /// * id                      - widget identifier
    /// * height                  - widget height in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * type                    - list item type; available values: StdListField, LeftArrowListField,
    ///                             RightArrowListField, EmptyField, Dots, DoubleImageField
    /// * text                    - caption text (default: none)
    /// * description             - description text (default: none)
    /// * font                    - caption text font (default: "normal")
    /// * descriptionFont         - description text font (default: "small")
    /// * textColor               - caption text color (default: black)
    /// * activeTextColor         - caption text color when pressed (default: textColor)
    /// * descriptionColor        - description text color (default: black)
    /// * activeDescriptionColor  - description text color when pressed (default: descriptionColor)
    /// * backgroundColor         - background color (default: transparent)
    /// * activeBackgroundColor   - background color when pressed (default: backgroundColor)
    /// * image                   - image content identifier
    /// * additionalImage         - second image content identifier
    ///
    class ListItem : public AbstractButton {
    public:
        enum ItemType {
            StdListField,
            LeftArrowField,
            RightArrowField,
            Dots,
            DoubleImageField,
            AlarmField,
            EmptyField
        } Type;

    public:
        ListItem()
            : AbstractButton()
            , Type(StdListField)
            , Description("")
            , DescriptionFont(NULL)
            , DescriptionColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveDescriptionColor(COLOR_ARGB8888_BLACK)
            , AdditionalImge()
            , Parent(NULL)
            , roundingImage()
        {
        }

        ListItem(int32_t x, int32_t y, int32_t width, int32_t height)
            : AbstractButton(x, y, width, height)
            , Type(StdListField)
            , Description("")
            , DescriptionFont(NULL)
            , DescriptionColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveDescriptionColor(COLOR_ARGB8888_BLACK)
            , AdditionalImge()
            , Parent(NULL)
            , roundingImage()
        {
        }

        ListItem(int32_t x, int32_t y, int32_t width, int32_t height, ItemType type)
            : AbstractButton(x, y, width, height)
            , Type(type)
            , Description("")
            , DescriptionFont(NULL)
            , DescriptionColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveDescriptionColor(COLOR_ARGB8888_BLACK)
            , AdditionalImge()
            , Parent(NULL)
            , roundingImage()
        {
        }

        ListItem(const ListItem& Obj)
            : AbstractButton(Obj)
            , Type(Obj.Type)
            , Description("")
            , DescriptionFont(NULL)
            , DescriptionColor(COLOR_ARGB8888_LIGHTGRAY)
            , ActiveDescriptionColor(COLOR_ARGB8888_BLACK)
            , AdditionalImge()
            , Parent(NULL)
            , roundingImage()
        {
        }

        virtual ~ListItem();

        void SetDescription(const char* desc);
        void SetDescriptionFont(Font const* font);

        /// Sets type of the list item.
        ///
        /// @param type New list item's type.
        void SetType(ItemType type);
        void SetParent(Component* NewParent);
        virtual void SetVisible(bool state);

        void SetDescriptionColor(uint32_t color);
        void SetActiveDescriptionColor(uint32_t color);

        void SetRoundingImage(const Image& image);
        Image* GetRoundingImagePointer();

        void SetAdditionalImage(const Image& image);
        /// @return Pointer to the second image set for this item or NULL.
        Image* GetAdditionalImagePointer();
        Component* GetParent();

        virtual void Hide();
        virtual void Show();

        uint32_t GetDescriptionColor() const;
        uint32_t GetActiveDescriptionColor() const;

        static ListItem* BuildFromXML(XMLElement* xmlElement);

        virtual void PrepareContent(ContentManager* contentManager);
        virtual void CancelPreparingContent(ContentManager* contentManager);

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight);

    protected:
        string Description;
        Font const* DescriptionFont;
        uint32_t DescriptionColor, ActiveDescriptionColor;
        Image AdditionalImge;
        Component* Parent;

        // This image is used to make rounding corners. It should be an antialiased circle.
        Image roundingImage;
        static ItemType ParseListItemTypeOrDefault(const char* value, ItemType defaultValue);
    };

} /* namespace grvl */

#endif /* GRVL_LISTITEM_H_ */
