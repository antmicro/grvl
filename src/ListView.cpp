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

#include "ListView.h"
#include "Image.h"
#include "Manager.h"

namespace grvl {

    ListView::~ListView()
    {
        Elements_vec::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }
    }

    ListView* ListView::BuildFromXML(XMLElement* xmlElement)
    {
        ListView* parent = new ListView();

        parent->EnableScrolling();

        parent->SetSplitLineColor(XMLSupport::ParseColor(xmlElement, "splitLineColor", (uint32_t)COLOR_ARGB8888_TRANSPARENT));
        parent->SetOverscrollBarColor(XMLSupport::ParseColor(xmlElement, "overscrollColor", (uint32_t)COLOR_ARGB8888_LIGHTGRAY));
        parent->SetScrollIndicatorColor(XMLSupport::ParseColor(xmlElement, "scrollIndicatorColor", (uint32_t)COLOR_ARGB8888_DARKGRAY));

        parent->InitFromXML(xmlElement);

        XMLElement* child = xmlElement->FirstChildElement();
        for(; child != NULL; child = child->NextSiblingElement()) {
            // TODO: we should verify the correct class
            Component* element = create_component(child->Name(), (void*)child);
            if(element)
                parent->AddElement(element);
        }

        return parent;
    }

    void ListView::AddElement(Component* citem)
    {
        ListItem* item = (ListItem*)citem;
        item->SetPosition(0, itemsHeight); // Position at the end of the list
        item->SetSize(Width, item->GetHeight());
        item->SetParent(this);

        itemsHeight += item->GetHeight();
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

            int sHeight = max((int)((float)Height / (float)itemsHeight * Height), minSHeight);

            if(scrollIndicatorImage != NULL) {

                // Remove the old scroll indicator image
                grvl::Callbacks()->free(scrollIndicatorImage->GetData());
                delete scrollIndicatorImage;
            }

            // allocate memory for a new image content
            uint8_t* imgContent = (uint8_t*)grvl::Callbacks()->malloc(4 * static_cast<size_t>(sWidth * sHeight));
            scrollIndicatorImage = new ImageContent(ImageContent::FromRAW(imgContent, sWidth, sHeight, 1, COLOR_FORMAT_ARGB8888));
        }

        Elements.push_back(item);
    }

    void ListView::ClearList()
    {
        grvl::Callbacks()->mutex_lock(ClearWhileTouchMutex);
        grvl::Callbacks()->mutex_lock(ClearWhileDrawMutex);
        Elements_vec::iterator it;
        for(it = Elements.begin(); it != Elements.end();) {
            delete *it;
            it = Elements.erase(it);
        }

        Scroll = ScrollMax = ScrollChange = itemsHeight = animation = 0; //NOLINT
        grvl::Callbacks()->mutex_unlock(ClearWhileDrawMutex);
        grvl::Callbacks()->mutex_unlock(ClearWhileTouchMutex);
    }

    void ListView::Refresh()
    {
        Elements_vec::iterator it;
        ScrollMax = 0;
        itemsHeight = 0;

        grvl::Callbacks()->mutex_lock(ClearWhileTouchMutex);
        grvl::Callbacks()->mutex_lock(ClearWhileDrawMutex);
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
        grvl::Callbacks()->mutex_unlock(ClearWhileDrawMutex);
        grvl::Callbacks()->mutex_unlock(ClearWhileTouchMutex);
    }

    bool ListView::AddToList(Manager* man, string& listContent)
    {

        XMLDocument xmlContent;

        if(listContent.length() == 0) {
            return false;
        }

        XMLError Error = xmlContent.Parse(listContent.c_str(), listContent.length());

        if(Error != XML_SUCCESS) {
            return false;
        }
        XMLNode* Root = xmlContent.LastChild();

        if(Root == 0) {
            return false;
        }

        XMLElement* childElement = Root->FirstChildElement("listItem");
        while(childElement != 0) {
            ListItem* item = ListItem::BuildFromXML(childElement);

            if(item) {
                this->AddElement(item);
            }
            childElement = (XMLElement*)childElement->NextSiblingElement("listItem");
        }

        xmlContent.Clear();

        return true;
    }

} /* namespace grvl */
