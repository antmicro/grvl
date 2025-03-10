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

#ifndef GRVL_ABSTRACTBUTTON_H_
#define GRVL_ABSTRACTBUTTON_H_

#include "Component.h"
#include "Font.h"
#include "grvl.h"
#include "Image.h"
#include "Painter.h"
#include "stl.h"

namespace grvl {

    /// Represents base class for all buttons.
    class AbstractButton : public Component {
    public:
        AbstractButton() = default;

        AbstractButton(int32_t x, int32_t y, int32_t width, int32_t height)
            : Component(x, y, width, height)
        {
        }

        AbstractButton(const AbstractButton& Obj)
            : Component(Obj)
            , Text(Obj.Text)
            , ButtonImage(Obj.ButtonImage)
            , ButtonFont(Obj.ButtonFont)
        {
        }

        AbstractButton& operator=(const AbstractButton& Obj);

        void SetText(const char* text);
        void SetImage(const Image& image);
        void SetTextFont(Font const* font);

        void ClearButtonFont();

        const char* GetText();
        Font const* GetButtonFont();
        Image* GetImagePointer();

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder) override;

        GENERATE_DUK_STRING_GETTER(AbstractButton, Text, GetText)
        GENERATE_DUK_STRING_SETTER(AbstractButton, Text, SetText)

    protected:
        std::string Text{};
        Image ButtonImage{};
        Font const* ButtonFont{nullptr};

        virtual void InitFromXML(XMLElement* xmlElement);
    };

} /* namespace grvl */

#endif /* GRVL_ABSTRACTBUTTON_H_ */
