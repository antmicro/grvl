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

#include <grvl/Manager.h>
#include <grvl/component/Image.h>
#include <grvl/container/ListView.h>

#include <tinyxml2.h>

namespace grvl {

    ListView* ListView::BuildFromXML(XmlElement xml)
    {
        ListView* parent = new ListView();

        parent->EnableScrolling();

        parent->SetSplitLineColor(xml.Attribute("splitLineColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_TRANSPARENT));
        parent->SetOverscrollBarColor(xml.Attribute("overscrollColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_LIGHTGRAY));
        parent->SetScrollIndicatorColor(xml.Attribute("scrollIndicatorColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_DARKGRAY));

        parent->SetVerticalGap(xml.Attribute("verticalGap").ElseStyle().Parse<int>().GetOr(0));

        parent->InitFromXML(xml);

        for(XmlElement child : xml) {
            if(Component* component = component_from_xml(child)) {
                parent->AddElement(component);
            }
        }

        return parent;
    }

    void ListView::SetVerticalGap(float value)
    {
        verticalGap = value;
        Refresh();
    }

    void ListView::AddElement(Component* component)
    {
        component->SetPosition(0, itemsHeight); // Position at the end of the list
        component->SetSize(Width, component->GetHeight());

        itemsHeight += component->GetHeight() + verticalGap;
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }

        // Prepare scroll image
        if(scrollingEnabled && ScrollMax > 0) {
            static constexpr auto minSWitdth = 10;
            static constexpr auto minSHeight = 50;
            int sWidth = minSWitdth;

            int sHeight = std::max((int)((float)Height / (float)itemsHeight * Height), minSHeight);

            if(scrollIndicatorImage != NULL) {
                delete scrollIndicatorImage;
            }

            scrollIndicatorImage = new ImageContent(sWidth, sHeight);
        }

        Elements.push_back(component);
    }

    void ListView::RemoveElement(const char* elementId)
    {
        int elementsSize = Elements.size();
        Component* foundComponent { nullptr };
        int index = 0;
        for(; index < Elements.size(); ++index) {
            if(strcmp(Elements[index]->GetID(), elementId) == 0) {
                foundComponent = Elements[index];
                Elements.erase(Elements.begin() + index);
                break;
            }
        }

        if(!foundComponent) {
            return;
        }

        if(index < elementsSize - 1) {

            int y = 0;
            if(index > 0) {
                y = Elements[index - 1]->GetY() + Elements[index - 1]->GetHeight();
            }

            for(; index < Elements.size(); ++index) {
                Elements[index]->SetY(y);
                y += Elements[index]->GetHeight();
            }
        }

        itemsHeight -= foundComponent->GetHeight();
        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        }

        delete foundComponent;
    }

    void ListView::ClearList()
    {
        Guard touch_lock { ClearWhileTouchMutex };
        Guard draw_lock { ClearWhileDrawMutex };

        std::vector<Component*>::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }

        Scroll = ScrollMax = ScrollChange = itemsHeight = animation = 0; // NOLINT
    }

    void ListView::Refresh()
    {
        std::vector<Component*>::iterator it;
        ScrollMax = 0;
        itemsHeight = 0;

        Guard touch_lock { ClearWhileTouchMutex };
        Guard draw_lock { ClearWhileDrawMutex };

        for(it = Elements.begin(); it != Elements.end(); it++) {
            if((*it)->IsVisible()) {
                itemsHeight += (*it)->GetHeight();
            }
        }

        if(Height < itemsHeight) {
            ScrollMax = itemsHeight - Height;
        } else {
            ScrollMax = 0;
        }

        if(Scroll > ScrollMax) {
            Scroll = ScrollMax;
        }
    }

    bool ListView::AddToList(Manager* man, std::string& listContent)
    {
        using namespace tinyxml2;
        XMLDocument xmlContent;

        if(listContent.length() == 0) {
            return false;
        }

        XMLError Error = xmlContent.Parse(listContent.c_str(), listContent.length());

        if(Error != XML_SUCCESS) {
            return false;
        }
        XMLElement* raw = xmlContent.LastChild()->ToElement();

        if(raw == nullptr) {
            return false;
        }

        XmlElement root { raw };

        for(auto child : root.ForEach("listItem")) {
            ListItem* item = ListItem::BuildFromXML(child);
            this->AddElement(item);
        }

        xmlContent.Clear();

        return true;
    }

} /* namespace grvl */
