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

#include <grvl/ContentManager.h>
#include <grvl/ImageContent.h>
#include <grvl/JSObjectBuilder.h>
#include <grvl/Painter.h>
#include <grvl/component/Component.h>

#include <chrono>

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
            , Delegate(nullptr)
            , AnimationEnabled(true)
            , AnimationLoop(true)
            , LastFrameChange(std::chrono::steady_clock::now())
        {
        }

        Image(const Image& other) = default;
        Image& operator=(const Image& other) = default;
        Component* Clone() const override;

        void SetActiveFrame(uint32_t activeFrame);

        uint32_t GetActiveFrame() const;

        void SetAnimationEnabled(bool enabled);

        bool IsAnimationEnabled() const;

        void SetAnimationLoop(bool loop);

        bool IsAnimationLoopEnabled() const;

        void RestartAnimation();

        ImageContent* GetContent()
        {
            return HasContent() ? Delegate->Get() : nullptr;
        }

        const ImageContent* GetContent() const
        {
            return HasContent() ? Delegate->Get() : nullptr;
        }

        bool HasContent() const
        {
            return Delegate && Delegate->HasContent();
        }

        void ReplaceDelegate(const std::shared_ptr<ImageDelegate>& delegate);
        void RemoveDelegate();

        uint8_t* GetContentData() const
        {
            return HasContent() ? Delegate->Get()->GetData() : nullptr;
        }

        uint32_t GetContentBytesPerPixel() const
        {
            return HasContent() ? Delegate->Get()->GetBytesPerPixel() : 0;
        }

        Format GetContentColorFormat() const
        {
            return HasContent() ? Delegate->Get()->GetColorFormat() : Format::ARGB8888;
        }

        bool GetContentAlpha() const
        {
            return HasContent() ? Delegate->Get()->HasAlphaChannel() : false;
        }

        bool IsEmpty() const;

        void PopulateJavaScriptObject(JSObjectBuilder& jsObjectjsObjectBuilder) override;
        GENERATE_DUK_UNSIGNED_INT_GETTER(Image, ActiveFrame, GetActiveFrame)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Image, ActiveFrame, SetActiveFrame)

        GENERATE_DUK_BOOLEAN_GETTER(Image, AnimationEnabled, IsAnimationEnabled)
        GENERATE_DUK_BOOLEAN_SETTER(Image, AnimationEnabled, SetAnimationEnabled)

        GENERATE_DUK_BOOLEAN_GETTER(Image, AnimationLoop, IsAnimationLoopEnabled)
        GENERATE_DUK_BOOLEAN_SETTER(Image, AnimationLoop, SetAnimationLoop)

        static duk_ret_t JSRestartAnimationWrapper(duk_context* ctx);

        static Image* BuildFromXML(XMLElement* xmlElement);

        void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) override;

    private:
        uint32_t ActiveFrame;
        std::shared_ptr<ImageDelegate> Delegate;

        bool AnimationEnabled;
        bool AnimationLoop;
        std::chrono::steady_clock::time_point LastFrameChange;

        void updateAnimation();
    };

} /* namespace grvl */

#endif /* GRVL_IMAGE_H_ */
