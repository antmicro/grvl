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

#ifndef GRVL_XMLSUPPORT_H_
#define GRVL_XMLSUPPORT_H_

#include "Label.h"
#include "tinyxml2.h"
#include "CallbackDefinition.h"

using namespace tinyxml2;

namespace grvl {

    class XMLSupport {
    public:
        static uint32_t GetAttributeOrDefault(XMLElement* element, const char* attributeName, uint32_t defaultValue);
        static bool GetAttributeOrDefault(XMLElement* element, const char* attributeName, bool defaultValue);
        static const char* GetAttributeOrDefault(XMLElement* element, const char* attributeName, const char* defaultValue);
        static CallbackDefinition ParseCallback(const char* callbackDefinition);
        static enum Label::TextHorizontalAlignment ParseAlignmentOrDefault(XMLElement* element, const char* attributeName,
                                                                           enum Label::TextHorizontalAlignment defaultValue);
        static bool TryGetAttribute(XMLElement* element, const char* attributeName, const char** value);
        static bool TryGetIntAttribute(XMLElement* element, const char* attributeName, int32_t* value);
        static uint32_t GetPositionInCollection(const string& collectionString);
        static uint32_t GetCollectionSize(const string& collectionString);

        static uint32_t ParseColor(XMLElement* xmlElement, const char* attributeName, const char* defaultValue);
        static uint32_t ParseColor(XMLElement* xmlElement, const char* attributeName, uint32_t defaultValue);
    };

} /* namespace grvl */

#endif /* GRVL_XMLSUPPORT_H_ */
