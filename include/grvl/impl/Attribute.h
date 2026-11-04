// Copyright 2026 Antmicro <antmicro.com>
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

#ifndef GRVL_ATTRIBUTE_H_
#define GRVL_ATTRIBUTE_H_

#include <algorithm>
#include <cctype>
#include <charconv>
#include <grvl/CallbackDefinition.h>
#include <grvl/JSEngine.h>
#include <grvl/ParsingUtils.h>
#include <grvl/XMLSupport.h>
#include <optional>
#include <string>
#include <tinyxml2.h>

static bool stringEquals(std::string_view a, std::string_view b)
{
    if(a.size() != b.size()) {
        return false;
    }

    return std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
        return std::tolower(static_cast<unsigned char>(ac)) == std::tolower(static_cast<unsigned char>(bc));
    });
}

template <typename R>
std::optional<R> stringParse(std::string_view str, int base)
{
    if(str.empty()) {
        return std::nullopt;
    }

    R result;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result, base);

    if(ec != std::errc {}) {
        return std::nullopt;
    }

    if(ptr != str.end()) {
        return std::nullopt;
    }

    return result;
}

template <typename R>
std::optional<R> stringParse(std::string_view str)
{
    // TODO replace with starts_with in C++20
    if(str.size() >= 2 && str[0] == '0' && str[1] == 'x') {
        str.remove_prefix(2);
        return stringParse<R>(str, 16);
    }

    return stringParse<R>(str, 10);
}

namespace grvl {

    template <typename T>
    class XmlAttribute {
    protected:
        tinyxml2::XMLElement* element;
        const char* attribute;
        T value;
        bool present;

    public:
        constexpr XmlAttribute(tinyxml2::XMLElement* element, const char* attribute)
            : element(element)
            , attribute(attribute)
            , value()
            , present(false)
        {
        }

        constexpr XmlAttribute(tinyxml2::XMLElement* element, const char* attribute, const T& value)
            : element(element)
            , attribute(attribute)
            , value(value)
            , present(true)
        {
        }

        /// Return the name of this XML attribute,
        /// the name can be empty if the attribute was not
        /// fetched from XML.
        std::string_view Name() const
        {
            return attribute;
        }

        /// Check if there is no value present for the attribute
        /// and the .Get() method can not be called. Use .Else*() chaining to supply alternative values,
        /// in place of manual IsEmpty() calls where possible.
        constexpr bool IsEmpty() const
        {
            return !present;
        }

        /// Check if there is value present for the attribute
        /// and the .Get() method can be called. Opposite of IsEmpty().
        /// Should be used to facilitate if-with-resources statements.
        constexpr operator bool() const
        {
            return !IsEmpty();
        }

        /// Use the given value if it is not present, after a call to this
        /// method the XmlAttribute object always has a valid value.
        /// Use this method before calling Get() to ensure the value is present.
        XmlAttribute<T> Else(T fallback) const
        {
            if(present) {
                return *this;
            }

            return XmlAttribute<T> { element, attribute, fallback };
        }

        /// Unwrap the underlying value of the attribute, this method MUST NOT
        /// be called if the attribute has no value, use Else() to supply a
        /// fallback value.
        T Get() const
        {
            if(IsEmpty()) {
                Log(ERROR, "Called XmlAttribute::Get() on an empty attribute %s!", attribute);
                return {};
            }

            return value;
        }

        /// Shortcut for the .Else(X).Get() construct,
        /// returns attribute value (if present), and the given fallback
        /// otherwise.
        T GetOr(T fallback) const
        {
            return Else(fallback).Get();
        }

        /// Convert value (if present) into a different value (of possibly different type) using
        /// the given mapping function, if the mapping function returns an empty optional
        /// the resulting XmlAttribute will hold no value.
        template <typename E>
        XmlAttribute<E> Map(std::optional<E> (*f)(const T& element)) const
        {
            if(present) {
                if(auto result = f(value)) {
                    return { element, attribute, result.value() };
                }
            }

            return { element, attribute };
        }
    };

    template <typename T>
    class XmlStringAttribute : public XmlAttribute<T> {
    protected:
        inline static std::optional<uint32_t> ColorParser(const T& value)
        {
            std::string_view str { value };

            if(str.empty()) {
                return std::nullopt;
            }

            int base = 10;

            if(str[0] == '#') {
                str.remove_prefix(1);
                base = 16;
            }

            return stringParse<uint32_t>(str, base);
        }

        template <typename R>
        inline static std::optional<R> PrimitiveParser(const T& value)
        {
            std::string_view str { value };

            if constexpr(std::is_same_v<R, bool>) {
                if(str == "1" || stringEquals(str, "true")) {
                    return { true };
                }

                if(str == "0" || stringEquals(str, "false")) {
                    return { false };
                }
            } else {
                return stringParse<R>(str);
            }

            return std::nullopt;
        }

    public:
        constexpr XmlStringAttribute(tinyxml2::XMLElement* element, const char* attribute)
            : XmlAttribute<T>(element, attribute)
        {
        }

        constexpr XmlStringAttribute(tinyxml2::XMLElement* element, const char* attribute, const T& value)
            : XmlAttribute<T>(element, attribute, value)
        {
        }

        XmlStringAttribute<T> Else(T fallback) const
        {
            if(this->present) {
                return *this;
            }

            return XmlStringAttribute<T> { this->element, this->attribute, fallback };
        }

        /// Try fetching the value from the stylesheet if it is not
        /// present, stylesheet may not contain a value for the requested attribute,
        /// so this method mey still return an empty XmlAttribute.
        XmlStringAttribute<T> ElseStyle() const
        {
            if(this->present) {
                return *this;
            }

            if(const char* stylesheet = XMLSupport::GetAttributeFromStylesheet(this->element, this->attribute)) {
                return XmlStringAttribute<T> { this->element, this->attribute, T { stylesheet } };
            }

            return *this;
        }

        /// Converts a string_view attribute to an arbitrary enum using the given
        /// unordered_map as a lookup table. If the attribute value is not present in the
        /// map an empty attribute is returned.
        template <typename E>
        XmlAttribute<E> ParseEnum(const std::unordered_map<std::string_view, E>& mapping)
        {
            if(!this->present) {
                return XmlAttribute<E> { this->element, this->attribute };
            }

            std::string_view key { this->value };
            auto it = mapping.find(key);

            if(it != mapping.end()) {
                return XmlAttribute<E> { this->element, this->attribute, it->second };
            }

            return XmlAttribute<E> { this->element, this->attribute };
        }

        /// Convert to an attribute of a different type. The 'To<>()' method
        /// supports convering from string to simple types, and between simple
        /// types, e.g. from double to int, and std::string_view to bool.
        template <typename E>
        XmlAttribute<E> Parse() const
        {
            return this->Map(PrimitiveParser<E>);
        }

        /// Converts XmlStringAttribute to a ARGB888 color code attribute,
        /// or an empty attribute if the string can not be parsed as a color.
        /// The expected format is "#<hexadecimal>" or "<decimal>".
        XmlAttribute<uint32_t> ParseColor()
        {
            return this->Map(ColorParser);
        }

        /// Converts XmlStringAttribute to a JavaScript CallbackDefinition attribute
        /// or an empty attribute if the string can not be parsed as a CallbackDefinition.
        /// The expected format is "foo(1, 2, 3)" or "bar".
        XmlAttribute<CallbackDefinition> ParseCallback()
        {
            if(!this->present) {
                return XmlAttribute<CallbackDefinition> { this->element, this->attribute };
            }

            CallbackDefinition result {};
            result.functionName = ParsingUtils::GetFunctionName(this->value);
            result.args = ParsingUtils::GetArguments(this->value);

            return XmlAttribute<CallbackDefinition> { this->element, this->attribute, result };
        }
    };

}

#endif
