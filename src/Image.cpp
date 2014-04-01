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

#include "Image.h"

#include "Manager.h"
#include "Painter.h"
#include "XMLSupport.h"

namespace grvl {

    void Image::SetActiveFrame(uint32_t activeFrame)
    {
        if(Content && activeFrame < Content->GetNumberOfFrames()) {
            ActiveFrame = activeFrame;
        }
    }

    Image* Image::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        const char* type;
        int32_t x = 0;
        int32_t y = 0;
        Image* result = new Image();

        if(XMLSupport::TryGetAttribute(xmlElement, "contentId", &type)) {
            man->BindImageContentToImage(type, result);
        }

        XMLSupport::TryGetIntAttribute(xmlElement, "x", &x);
        XMLSupport::TryGetIntAttribute(xmlElement, "y", &y);

        result->SetPosition(x, y);
        result->SetVisible(XMLSupport::GetAttributeOrDefault(xmlElement, "visible", true));

        const char* id = xmlElement->Attribute("id");
        if(id != NULL) {
            result->SetID(id);
        }

        result->SetOnClickEvent(man->GetEventWithArguments(xmlElement->Attribute("onClick")));
        return result;
    }

    void Image::Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth, int32_t ParentHeight)
    {
        if(!Visible)
            return;

        if(!Content || Content->IsEmpty()) {
            if(!imageContentRequested) {
                painter.GetContentManager()->RequestBinding(this);
            }
            return;
        }
        int32_t TopOffset = 0;
        int32_t BottomOffset = 0;
        uint8_t PixelFormat = painter.GetActiveBufferPixelFormat();

        uintptr_t Address = (uintptr_t)Content->GetData();
        uint16_t Frames = Content->GetNumberOfFrames();
        uint32_t ColorFormat = Content->GetColorFormat();
        uint32_t Lines = Content->GetNumberOfLines();
        uint32_t PixelPerLine = Content->GetPixelsPerLine();

        if(ParentHeight == 0)
            return;

        if(ParentHeight < 0) { // Top offset
            TopOffset = -ParentHeight;
            TopOffset = Y - TopOffset;
            if(TopOffset > 0) {
                TopOffset = 0;
            } else {
                TopOffset = -TopOffset;
            }
        } else if(ParentHeight <= Y + Height) { // Bottom offset
            BottomOffset = Height - (ParentHeight - Y);
        }

        if(Height != 0 && Width != 0 && Address != 0 && (TopOffset + BottomOffset) < Height) {
            int32_t imgX = 0; // Left offset
            int32_t imgY = TopOffset;

            painter.DmaMoveImage(
                Address, painter.GetActiveBuffer(), imgX, imgY, ParentX + X, ParentY + Y + TopOffset, Width,
                Height - (BottomOffset + TopOffset), PixelPerLine, Lines, ActiveFrame, Frames, ColorFormat, PixelFormat,
                GetContentAlpha(), Content ? Content->GetPLTE() : 0);
        }
    }

    uint32_t Image::GetActiveFrame() const
    {
        return ActiveFrame;
    }

    bool Image::IsImageContentPending() const
    {
        return imageContentRequested;
    }

    bool Image::IsEmpty() const
    {
        return !bindingRegistered && (!Content || Content->IsEmpty());
    }

    void Image::DeleteAndReplaceImageContent(ImageContent* content)
    {
        if(content) {
            delete Content;
        }
        ReplaceImageContent(content);
    }

    void Image::DeleteAndCleanImageContent()
    {
        if(Content) {
            delete Content;
            CleanImageContent();
        }
    }

    void Image::CleanImageContent()
    {
        Content = NULL;
        Width = 0;
        Height = 0;
    }

    void Image::ReplaceImageContent(ImageContent* content)
    {
        Content = content;
        imageContentRequested = false;
        if(content != NULL) {
            Width = content->GetWidth();
            Height = content->GetHeight();
        } else {
            Width = 0;
            Height = 0;
        }
    }

} /* namespace grvl */
