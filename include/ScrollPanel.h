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

#ifndef GRVL_SCROLLPANEL_H_
#define GRVL_SCROLLPANEL_H_

#include "VerticalScrollView.h"

namespace grvl {

    class ScrollPanel : public VerticalScrollView {
    public:
        ScrollPanel() = default;

        ScrollPanel(int32_t x, int32_t y, int32_t width, int32_t height)
            : VerticalScrollView{x, y, width, height}
        {
        }

        static ScrollPanel* BuildFromXML(XMLElement* xmlElement);

    protected:
        void AdjustScrollViewHeight(Component* child) override;
    };

} /* namespace grvl */

#endif /* GRVL_SCROLLPANEL_H_ */
