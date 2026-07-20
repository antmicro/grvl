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

#include <grvl/component/Image.h>
#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/Painter.h>
#include <grvl/XMLSupport.h>

namespace grvl {
    Component* Image::Clone() const
    {
        return new Image(*this);
    }

    void Image::SetActiveFrame(uint32_t activeFrame)
    {
        ImageContent* content = GetContent();

        if(content && activeFrame < content->GetNumberOfFrames()) {
            ActiveFrame = activeFrame;
            LastFrameChange = std::chrono::steady_clock::now();
        }
    }

    uint32_t Image::GetActiveFrame() const
    {
        return ActiveFrame;
    }

    void Image::SetAnimationEnabled(bool enabled) 
    {
        AnimationEnabled = enabled;
        LastFrameChange = std::chrono::steady_clock::now();
    }

    bool Image::IsAnimationEnabled() const
    {
        return AnimationEnabled && HasContent() && Delegate->Get()->IsAnimated();
    }

    void Image::SetAnimationLoop(bool loop)
    {
        AnimationLoop = loop;
    }

    bool Image::IsAnimationLoopEnabled() const
    {
        return AnimationLoop;
    }

    void Image::RestartAnimation()
    {
        ActiveFrame = 0;
        AnimationEnabled = true;
        LastFrameChange = std::chrono::steady_clock::now();
    }

    void Image::updateAnimation()
    {
        if (!IsAnimationEnabled()) {
            return;
        }

        auto content = Delegate->Get();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - LastFrameChange).count();

        while (AnimationEnabled) {
            const uint32_t duration = content->GetFrameDuration(ActiveFrame);
            if (duration == 0 || elapsed < duration) {
                break;
            }

            elapsed -= duration;
            LastFrameChange += std::chrono::milliseconds(duration);

            if (ActiveFrame + 1 < content->GetNumberOfFrames()) {
                ++ActiveFrame;
                continue;
            }

            if (AnimationLoop) {
                ActiveFrame = 0;
            } else {
                AnimationEnabled = false;
            }
        }
    }

    Image* Image::BuildFromXML(XMLElement* xmlElement)
    {
        Manager* man = &Manager::GetInstance();
        const char* name;
        Image* result = new Image();
        result->InitFromXML(xmlElement);

        if(XMLSupport::TryGetAttribute(xmlElement, "contentId", &name)) {
            man->BindImageContentToImage(name, result);
        }
        return result;
    }

    void Image::Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY)
    {
        if(!Visible) return;

        ImageContent* content = GetContent();
        if (content == nullptr) return;

        int w = content->GetWidth();
        int h = content->GetHeight();
        SetSize(w, h);

        if (w <= 0 || h <= 0) {
            return;
        }

        int32_t RenderX = ParentRenderX + X;
        int32_t RenderY = ParentRenderY + Y;

        updateAnimation();
        painter.DrawImage(RenderX, RenderY, content, ActiveFrame);
    }

    bool Image::IsEmpty() const
    {
        const ImageContent* content = GetContent();
        return !content || content->IsEmpty();
    }

    void Image::RemoveDelegate()
    {
        Width = 0;
        Height = 0;
        Delegate = nullptr;
    }

    void Image::ReplaceDelegate(const std::shared_ptr<ImageDelegate>& delegate)
    {
        if (Delegate == delegate) {
            return;
        }

        RemoveDelegate();

        ActiveFrame = 0;
        LastFrameChange = std::chrono::steady_clock::now();
        Delegate = delegate;
    }

    void Image::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        Component::PopulateJavaScriptObject(jsObjectBuilder);

        jsObjectBuilder
            .AddProperty("activeFrame", Image::JSGetActiveFrameWrapper, Image::JSSetActiveFrameWrapper)
            .AddProperty("animationEnabled", Image::JSGetAnimationEnabledWrapper, Image::JSSetAnimationEnabledWrapper)
            .AddProperty("animationLoop", Image::JSGetAnimationLoopWrapper, Image::JSSetAnimationLoopWrapper)
            .AttachMemberFunction("RestartAnimation", Image::JSRestartAnimationWrapper);
    }

    duk_ret_t Image::JSRestartAnimationWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);

        auto* image = static_cast<Image*>(duk_to_pointer(ctx, -1));

        if(image) {
            image->RestartAnimation();
        }

        return JSEngine::NO_RETURN_VALUE;
    }

} /* namespace grvl */
