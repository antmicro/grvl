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

#include <grvl/ContentManager.h>
#include <grvl/component/Image.h>

namespace grvl {

    void ContentManager::RegisterContent(const std::string& name, ImageContent* ic)
    {
        GetByName(name)->Set(ic);
    }

    std::shared_ptr<ImageDelegate> ContentManager::RequestImage(const std::string& name)
    {
        auto delegate = GetByName(name);

        return delegate;
    }

    std::shared_ptr<ImageDelegate> ContentManager::GetByName(const std::string& name)
    {
        auto it = content_registry.find(name);

        if (it == content_registry.end()) {
            auto [iterator, _] = content_registry.emplace(name, std::make_shared<ImageDelegate>());
            it = iterator;
        }

        return it->second;
    }

} /* namespace grvl */
