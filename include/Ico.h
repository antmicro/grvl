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

#ifndef GRVL_ICO_H_
#define GRVL_ICO_H_

#include "Component.h"
#include "Font.h"
#include "Painter.h"

namespace grvl {

    /// Represents single-character icon.
    class Ico : public Component {
    protected:
        int32_t IcoChar;
        Font const* IcoFont;

    public:
        Ico()
            : Component()
            , IcoChar(-1)
            , IcoFont(NULL)
        {
        }
        Ico(uint32_t width, uint32_t height, int32_t charCode, Font* IcoFont)
            : Component(0, 0, width, height)
            , IcoChar(charCode)
            , IcoFont(IcoFont)
        {
        }
        Ico(int32_t charCode, Font* IcoFont);

        virtual ~Ico();

        int32_t GetIcoChar() const;
        void SetIcoChar(int32_t icoChar);
        Font const* GetIcoFont() const;
        void SetIcoFont(Font const* icoFont);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

    private:
        void AdjustSize();
    };

} /* namespace grvl */

#endif /* GRVL_ICO_H_ */
