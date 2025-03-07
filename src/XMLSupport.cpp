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

//NOLINTBEGIN(readability-magic-numbers)
#include "XMLSupport.h"
#include "Manager.h"
#include "ParsingUtils.h"

namespace grvl {

    bool TryGetAttributeFromCollection(const map<string, char*>& collection, const char* key, const char* attributeName, const char** value)
    {
        map<string, char*>::const_iterator search;

        if(!key) {
            return 0;
        }
        string collectionKey(key);
        collectionKey.append("+");
        collectionKey.append(attributeName);
        search = collection.find(collectionKey);
        if(search != collection.end()) {
            *value = search->second;
            return true;
        }
        return false;
    }

    const char* GetAttributeFromStylesheet(XMLElement* element, const char* attributeName)
    {
        const char* value;

        if(TryGetAttributeFromCollection(Manager::GetInstance().idAttributes, element->Attribute("id"), attributeName, &value) || TryGetAttributeFromCollection(Manager::GetInstance().classAttributes, element->Attribute("class"), attributeName, &value) || TryGetAttributeFromCollection(Manager::GetInstance().widgetAttributes, element->Value(), attributeName, &value)) {
            return value;
        }
        return NULL;
    }

    uint32_t XMLSupport::GetAttributeOrDefault(XMLElement* element, const char* attributeName, uint32_t defaultValue)
    {
        const char* stylesheetValue;

        if(element->QueryUnsignedAttribute(attributeName, (unsigned int*)&defaultValue) != XML_SUCCESS) {
            if((stylesheetValue = GetAttributeFromStylesheet(element, attributeName))) {
                XMLUtil::ToUnsigned(stylesheetValue, &defaultValue);
            }
        }

        return defaultValue;
    }

    int32_t XMLSupport::GetAttributeOrDefault(XMLElement* element, const char* attributeName, int32_t defaultValue)
    {
        const char* stylesheetValue;

        if(element->QueryUnsignedAttribute(attributeName, (unsigned int*)&defaultValue) != XML_SUCCESS) {
            if((stylesheetValue = GetAttributeFromStylesheet(element, attributeName))) {
                XMLUtil::ToInt(stylesheetValue, &defaultValue);
            }
        }

        return defaultValue;
    }

    bool XMLSupport::GetAttributeOrDefault(XMLElement* element, const char* attributeName, bool defaultValue)
    {
        const char* stylesheetValue;

        if(element->QueryBoolAttribute(attributeName, &defaultValue) != XML_SUCCESS) {
            if((stylesheetValue = GetAttributeFromStylesheet(element, attributeName))) {
                XMLUtil::ToBool(stylesheetValue, &defaultValue);
            }
        }

        return defaultValue;
    }

    const char* XMLSupport::GetAttributeOrDefault(XMLElement* element, const char* attributeName,
                                                  const char* defaultValue)
    {
        const char* stylesheetValue;

        const char* val = element->Attribute(attributeName);
        if(val == NULL) {
            if((stylesheetValue = GetAttributeFromStylesheet(element, attributeName))) {
                return stylesheetValue;
            }
            return defaultValue;
        }

        return val;
    }

    CallbackDefinition XMLSupport::ParseCallback(const char* callbackDefinition)
    {
        if (!callbackDefinition || strlen(callbackDefinition) == 0) {
            return CallbackDefinition{};
        }

        CallbackDefinition result_callback_definition{};
        result_callback_definition.functionName = ParsingUtils::GetFunctionName(callbackDefinition);
        result_callback_definition.args = ParsingUtils::GetArguments(callbackDefinition);

        return result_callback_definition;
    }

    bool XMLSupport::TryGetIntAttribute(XMLElement* element, const char* attributeName, int32_t* value)
    {
        const char* stylesheetValue;

        if(element->QueryIntAttribute(attributeName, (int*)value) != XML_SUCCESS) {
            return (stylesheetValue = GetAttributeFromStylesheet(element, attributeName)) && XMLUtil::ToInt(stylesheetValue, value);
        }

        return true;
    }

    bool XMLSupport::TryGetAttribute(XMLElement* element, const char* attributeName, const char** value)
    {
        *value = element->Attribute(attributeName);
        if(*value == NULL) {
            return (*value = GetAttributeFromStylesheet(element, attributeName));
        }

        return true;
    }

    enum Label::TextHorizontalAlignment XMLSupport::ParseAlignmentOrDefault(XMLElement* element, const char* attributeName,
                                                                            enum Label::TextHorizontalAlignment defaultValue)
    {
        const char* stylesheetValue;

        const char* value = element->Attribute(attributeName);
        if(value == NULL) {
            if((stylesheetValue = GetAttributeFromStylesheet(element, attributeName))) {
                value = stylesheetValue;
            }
        }

        if(value != NULL && *value != 0) {
            if(strcmp("Center", value) == 0) {
                return Label::Center;
            }
            if(strcmp("Left", value) == 0) {
                return Label::Left;
            }
            if(strcmp("Right", value) == 0) {
                return Label::Right;
            }
        }

        return defaultValue;
    }

    static int atoi(const string& str)
    {
        int32_t n = 0;
        for(uint32_t i = 0; i < str.size(); i += 1) {
            char digit = str.at(i);
            if(digit < '0' || digit > '9') {
                return n;
            }
            n *= 10;
            n += digit - '0';
        }
        return n;
    }

    uint32_t XMLSupport::GetPositionInCollection(const string& collectionString)
    {
        uint32_t result = 0;
        string subString = collectionString.substr(0, collectionString.find_first_of("/", 0)); //NOLINT
        result = atoi(subString);
        return result;
    }

    uint32_t XMLSupport::GetCollectionSize(const string& collectionString)
    {
        uint32_t result = 0;
        string subString = collectionString.substr(collectionString.find_first_of("/", 0) + 1, collectionString.length()); //NOLINT
        result = atoi(subString);
        return result;
    }

    uint32_t XMLSupport::ParseColor(XMLElement* xmlElement, const char* attributeName, const char* defaultValue = "0")
    {
        const char* colorStr = XMLSupport::GetAttributeOrDefault(xmlElement, attributeName, defaultValue);

        uint32_t parsedColor = 0;
        bool parsingSuccess = 0;
        switch(colorStr[0]) {
            case '#': {
                parsingSuccess = sscanf(colorStr + 1, "%x", &parsedColor);
                break;
            }
            default: {
                parsingSuccess = sscanf(colorStr, "%u", &parsedColor);
            }
        }

        if(parsingSuccess)
            return parsedColor;

        return XMLSupport::ParseColor(xmlElement, attributeName, defaultValue);
    }

    uint32_t XMLSupport::ParseColor(XMLElement* xmlElement, const char* attributeName, uint32_t defaultValue = 0)
    {
        const char* colorStr = XMLSupport::GetAttributeOrDefault(xmlElement, attributeName, "ERR");

        if(strcmp(colorStr, "ERR") == 0)
            return defaultValue;

        uint32_t parsedColor = 0;
        bool parsingSuccess = 0;
        switch(colorStr[0]) {
            case '#': {
                parsingSuccess = sscanf(colorStr + 1, "%x", &parsedColor);
                break;
            }
            default: {
                parsingSuccess = sscanf(colorStr, "%u", &parsedColor);
            }
        }

        if(parsingSuccess)
            return parsedColor;

        return defaultValue;
    }

} /* namespace grvl */
//NOLINTEND(readability-magic-numbers)
