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

#ifndef GRVL_XMLPREPROCESSOR_H_
#define GRVL_XMLPREPROCESSOR_H_

#include <string>
#include <tinyxml2.h>

namespace grvl {

    /// Expands ${...} JavaScript expressions in GUI XML documents.
    /// See the Expressions section of the XML reference.
    class XMLPreprocessor {
    public:
        /// Escapes the XML specials inside each ${...} span of a raw document,
        /// so the expression can be written in plain JavaScript. Comments,
        /// CDATA sections and processing instructions are left alone. Must run
        /// before XMLDocument::Parse().
        static bool EscapeExpressions(const std::string& source, std::string& result, std::string& error);

        /// Evaluates and removes every <defines> block below the root. Must run
        /// after the includes and before ExpandExpressions().
        static bool CollectDefines(tinyxml2::XMLElement* root, std::string& error);

        /// Replaces each ${...} span in the attribute values and the text nodes
        /// with its evaluation result.
        static bool ExpandExpressions(tinyxml2::XMLNode* node, std::string& error);

    private:
        /// Finds the "}" matching the "${" whose body starts at bodyStart, or
        /// npos. Knows JavaScript string literals and comments.
        static size_t FindExpressionEnd(const std::string& text, size_t bodyStart);

        static bool ExpandString(const std::string& source, std::string& result, std::string& error);

        static constexpr const char* EXPRESSION_START { "${" };
    };

} /* namespace grvl */

#endif /* GRVL_XMLPREPROCESSOR_H_ */
