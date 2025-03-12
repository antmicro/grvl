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

#include "ScrollPanel.h"

namespace grvl {

    ScrollPanel* ScrollPanel::BuildFromXML(XMLElement* xmlElement)
    {
        ScrollPanel* parent = new ScrollPanel();

        parent->EnableScrolling();

        parent->SetSplitLineColor(XMLSupport::ParseColor(xmlElement, "splitLineColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));
        parent->SetOverscrollBarColor(XMLSupport::ParseColor(xmlElement, "overscrollColor", (uint32_t)COLOR_ARGB8888_LIGHTGRAY));
        parent->SetScrollIndicatorColor(XMLSupport::ParseColor(xmlElement, "scrollIndicatorColor", (uint32_t)COLOR_ARGB8888_DARKGRAY));

        parent->InitFromXML(xmlElement);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }

        return parent;
    }

    void ScrollPanel::AdjustScrollViewHeight(Component* child)
    {
        itemsHeight = std::max(itemsHeight, child->GetY() + child->GetHeight());
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }
    }

} /* namespace grvl */
