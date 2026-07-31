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

#ifndef GRVL_CONTENTMANAGER_H_
#define GRVL_CONTENTMANAGER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

#include <grvl/ImageContent.h>

namespace grvl {

    class Image;
    class ImageContent;

    /// Represents a managed image resource, the ContentManager can freely swap the underlying ImageContent pointer
    /// thus this object should be queried for it each time it is used.
    class ImageDelegate {
    public:

        constexpr void Set(ImageContent* ptr)
        {
            if (ptr == content) return;
            if (content) delete content;
            this->content = ptr;
        }

        constexpr ImageContent* Get() const
        {
            return content;
        }

        constexpr bool HasContent() const
        {
            return content != nullptr;
        }

        ~ImageDelegate()
        {
            Set(nullptr);
        }

    private:
        ImageContent* content = nullptr;
    };

    /// Represents manager for shared resources, e.g., image contents.
    class ContentManager {
    public:

        virtual ~ContentManager() = default;

        /// Updates all users of the image with the given name
        void RegisterContent(const std::string& name, ImageContent* ic);

        /// Used by Image component to get a ImageContent delegate
        std::shared_ptr<ImageDelegate> RequestImage(const std::string& name);

        /// Get an image delegate without loading if it is missing
        std::shared_ptr<ImageDelegate> GetByName(const std::string& name);

    private:

        // mapping of resource handles to resource delegates
        std::unordered_map<std::string, std::shared_ptr<ImageDelegate>> content_registry;
    };

} /* namespace grvl */

#endif /* GRVL_CONTENTMANAGER_H_ */
