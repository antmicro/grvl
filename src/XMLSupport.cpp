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

// NOLINTBEGIN(readability-magic-numbers)

#include <grvl/Manager.h>
#include <grvl/ParsingUtils.h>
#include <grvl/XMLSupport.h>
#include <grvl/impl/Attribute.h>

#include <string>

namespace grvl {

    static bool TryGetAttributeFromCollection(const std::unordered_map<std::string, char*>& collection, const char* key, const char* attributeName, const char** value)
    {
        if(!key) {
            return false;
        }

        std::string collectionKey(key);
        collectionKey.append("+");
        collectionKey.append(attributeName);
        auto it = collection.find(collectionKey);

        if(it != collection.end()) {
            *value = it->second;
            return true;
        }

        return false;
    }

    const char* XMLSupport::GetAttributeFromStylesheet(XmlElement element, const char* attributeName)
    {
        const char* value;
        Manager& manager = Manager::GetInstance();

        if(TryGetAttributeFromCollection(manager.idAttributes, element.Attribute("id").GetOr(nullptr), attributeName, &value)) {
            return value;
        }

        if(TryGetAttributeFromCollection(manager.classAttributes, element.Attribute("class").GetOr(nullptr), attributeName, &value)) {
            return value;
        }

        if(TryGetAttributeFromCollection(manager.widgetAttributes, element.Name(), attributeName, &value)) {
            return value;
        }

        return nullptr;
    }

} /* namespace grvl */
// NOLINTEND(readability-magic-numbers)
