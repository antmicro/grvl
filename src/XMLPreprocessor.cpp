// Copyright 2014-2026 Antmicro <antmicro.com>
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

#include "grvl/XMLPreprocessor.h"

#include "grvl/JSEngine.h"

#include <cstring>
#include <string>
#include <utility>
#include <vector>

using namespace tinyxml2;

namespace grvl {

    size_t XMLPreprocessor::FindExpressionEnd(const std::string& text, size_t bodyStart)
    {
        int depth = 1;

        for(size_t i = bodyStart; i < text.size(); ++i) {
            const char current = text[i];

            if(current == '\'' || current == '"') {
                for(++i; i < text.size() && text[i] != current; ++i) {
                    if(text[i] == '\\') {
                        ++i;
                    }
                }
            } else if(current == '/' && i + 1 < text.size() && text[i + 1] == '*') {
                const size_t end = text.find("*/", i + 2);
                if(end == std::string::npos) {
                    return std::string::npos;
                }
                i = end + 1;
            } else if(current == '/' && i + 1 < text.size() && text[i + 1] == '/') {
                const size_t end = text.find('\n', i + 2);
                if(end == std::string::npos) {
                    return std::string::npos;
                }
                i = end;
            } else if(current == '{') {
                ++depth;
            } else if(current == '}') {
                if(--depth == 0) {
                    return i;
                }
            }
        }

        return std::string::npos;
    }

    bool XMLPreprocessor::EscapeExpressions(const std::string& source, std::string& result, std::string& error)
    {
        static constexpr std::pair<const char*, const char*> passthrough[] = {
            { "<!--", "-->" },
            { "<![CDATA[", "]]>" },
            { "<?", "?>" },
        };

        result.clear();
        result.reserve(source.size() + source.size() / 8);

        size_t i = 0;
        while(i < source.size()) {
            bool skipped = false;

            for(const auto& region : passthrough) {
                const size_t openLength = std::strlen(region.first);
                if(source.compare(i, openLength, region.first) != 0) {
                    continue;
                }

                const size_t end = source.find(region.second, i + openLength);
                if(end == std::string::npos) {
                    error = std::string { "unterminated " } + region.first;
                    return false;
                }

                const size_t closeLength = std::strlen(region.second);
                result.append(source, i, end + closeLength - i);
                i = end + closeLength;
                skipped = true;
                break;
            }

            if(skipped) {
                continue;
            }

            if(source[i] == '$' && i + 1 < source.size() && source[i + 1] == '$') {
                result.append("$$");
                i += 2;
                continue;
            }

            if(source.compare(i, 2, EXPRESSION_START) == 0) {
                const size_t end = FindExpressionEnd(source, i + 2);
                if(end == std::string::npos) {
                    error = "unterminated ${ expression";
                    return false;
                }

                result.append(EXPRESSION_START);
                for(size_t k = i + 2; k < end; ++k) {
                    switch(source[k]) {
                        case '&':
                            result.append("&amp;");
                            break;
                        case '<':
                            result.append("&lt;");
                            break;
                        case '>':
                            result.append("&gt;");
                            break;
                        case '"':
                            result.append("&quot;");
                            break;
                        case '\'':
                            result.append("&apos;");
                            break;
                        default:
                            result.push_back(source[k]);
                    }
                }
                result.push_back('}');
                i = end + 1;
                continue;
            }

            result.push_back(source[i]);
            ++i;
        }

        return true;
    }

    bool XMLPreprocessor::ExpandString(const std::string& source, std::string& result, std::string& error)
    {
        result.clear();
        result.reserve(source.size());

        size_t i = 0;
        while(i < source.size()) {
            if(source[i] != '$' || i + 1 >= source.size()) {
                result.push_back(source[i++]);
                continue;
            }

            if(source[i + 1] == '$') {
                result.push_back('$');
                i += 2;
                continue;
            }

            if(source[i + 1] != '{') {
                result.push_back(source[i++]);
                continue;
            }

            const size_t end = FindExpressionEnd(source, i + 2);
            if(end == std::string::npos) {
                error = "unterminated ${ expression";
                return false;
            }

            const std::string expression = source.substr(i + 2, end - i - 2);

            std::string value;
            if(!JSEngine::EvaluateExpression(expression, value, error)) {
                error.append(" in ${").append(expression).append("}");
                return false;
            }

            result.append(value);
            i = end + 1;
        }

        return true;
    }

    bool XMLPreprocessor::CollectDefines(XMLElement* root, std::string& error)
    {
        std::vector<XMLElement*> blocks;

        for(XMLElement* block = root->FirstChildElement("defines"); block != nullptr;
            block = block->NextSiblingElement("defines")) {
            blocks.push_back(block);
        }

        for(XMLElement* block : blocks) {
            for(XMLElement* entry = block->FirstChildElement(); entry != nullptr;
                entry = entry->NextSiblingElement()) {

                if(std::strcmp(entry->Name(), "define") == 0) {
                    const char* name = entry->Attribute("name");
                    const char* value = entry->Attribute("value");
                    if(name == nullptr || value == nullptr) {
                        error = "<define> needs a name and a value attribute";
                        return false;
                    }

                    std::string expression;
                    if(!ExpandString(value, expression, error)) {
                        return false;
                    }

                    if(!JSEngine::DefineGlobalFromExpression(name, expression, error)) {
                        error.append(" in <define name=\"").append(name).append("\">");
                        return false;
                    }
                } else {
                    error = std::string { "unexpected <" } + entry->Name() + "> in <defines>";
                    return false;
                }
            }

            root->DeleteChild(block);
        }

        return true;
    }

    bool XMLPreprocessor::ExpandExpressions(XMLNode* node, std::string& error)
    {
        if(XMLElement* element = node->ToElement()) {
            // A script body is JavaScript already; it reads the globals direct.
            if(std::strcmp(element->Name(), "script") == 0) {
                return true;
            }

            std::vector<std::pair<std::string, std::string>> updates;

            for(const XMLAttribute* attribute = element->FirstAttribute(); attribute != nullptr;
                attribute = attribute->Next()) {
                std::string value;
                if(!ExpandString(attribute->Value(), value, error)) {
                    error.append(" in <").append(element->Name()).append(" ").append(attribute->Name()).append(">");
                    return false;
                }

                if(value != attribute->Value()) {
                    updates.emplace_back(attribute->Name(), value);
                }
            }

            for(const auto& update : updates) {
                element->SetAttribute(update.first.c_str(), update.second.c_str());
            }
        }

        if(XMLText* text = node->ToText()) {
            std::string value;
            if(!ExpandString(text->Value(), value, error)) {
                return false;
            }

            if(value != text->Value()) {
                text->SetValue(value.c_str());
            }
        }

        for(XMLNode* child = node->FirstChild(); child != nullptr; child = child->NextSibling()) {
            if(!ExpandExpressions(child, error)) {
                return false;
            }
        }

        return true;
    }

} /* namespace grvl */
