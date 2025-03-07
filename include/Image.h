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

#ifndef GRVL_IMAGE_H_
#define GRVL_IMAGE_H_

#include "Component.h"
#include "ContentManager.h"
#include "ImageContent.h"
#include "Painter.h"
#include "stl.h"

using namespace tinyxml2;

namespace grvl {
    class Manager;

    /// Widget displaying an image.
    ///
    /// XML parameters:
    /// * id                      - widget identifier
    /// * x                       - widget position on x axis in pixels
    /// * y                       - widget position on y axis in pixels
    /// * visible                 - indicates if the widget is visible
    ///
    /// * contentId               - identifier of image content to display (default: none)
    ///
    /// @remark
    /// Width and height of a widget are deduced from image content.
    class Image : public Component {
    public:
        Image()
            : Component()
            , ActiveFrame(0)
            , Content(NULL)
            , bindingRegistered(false)
            , imageContentRequested(false)
        {
        }

        Image(ImageContent* content, uint32_t x = 0, uint32_t y = 0, uint32_t activeFrame = 0)
            : Component(x, y, 0, 0)
            , ActiveFrame(activeFrame)
            , Content(content)
            , bindingRegistered(false)
            , imageContentRequested(false)
        {
            if(content != NULL) {
                Width = content->GetWidth();
                Height = content->GetHeight();
            }
        }

        virtual ~Image()
        {
        }

        void SetActiveFrame(uint32_t activeFrame);

        uint32_t GetActiveFrame() const;

        ImageContent* GetContent()
        {
            return Content;
        }

        // Deletes the old imageContent and sets the new one
        void DeleteAndReplaceImageContent(ImageContent* content);
        // Changes the pointer only
        void ReplaceImageContent(ImageContent* content);
        void DeleteAndCleanImageContent();
        void CleanImageContent();

        uint8_t* GetContentData() const
        {
            if (Content) return Content->GetData();
            return 0;
        }

        uint32_t GetContentBytesPerPixel() const
        {
            if(Content) return Content->GetBytesPerPixel();
            return 0;
        }

        uint32_t GetContentColorFormat() const
        {
            if(Content) return Content->GetColorFormat();
            return 0;
        }

        bool GetContentAlpha() const
        {
            if(!Content) return false;
            return Content->HasAlphaChannel();
        }

        bool IsEmpty() const;
        bool IsImageContentPending() const;

        static Image* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

    private:
        uint32_t ActiveFrame;
        ImageContent* Content;

        bool bindingRegistered; // Binding request has already been sent
        bool imageContentRequested; // Waiting for content to be filled by ContentManager

        friend void ContentManager::BindImageContentToImage(const string& contentName, Image* image);
        friend void ContentManager::CancelRequest(Image* image);
        friend void ContentManager::RequestBinding(Image* image);
    };

} /* namespace grvl */

#endif /* GRVL_IMAGE_H_ */
