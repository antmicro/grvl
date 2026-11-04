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

#ifndef GRVL_ELEMENT_H_
#define GRVL_ELEMENT_H_

#include <grvl/impl/Attribute.h>
#include <string>
#include <string_view>
#include <tinyxml2.h>

namespace grvl {

    class XmlElementIterator;
    class XmlElementSiblingSet;

    class XmlElement {
    private:
        tinyxml2::XMLElement* self;

    public:
        constexpr XmlElement(tinyxml2::XMLElement* element)
            : self(element)
        {
        }

        /// Get the next element in the same parent, or nullopt if this was the last element.
        /// Optionally a tag name can be given to restrict the search to elements of a specific name.
        std::optional<XmlElement> NextSibling(const char* name = nullptr) const
        {
            tinyxml2::XMLElement* element = name
                ? self->NextSiblingElement(name)
                : self->NextSiblingElement();

            if(element == nullptr) {
                return std::nullopt;
            }

            return XmlElement { element };
        }

        std::optional<XmlElement> FirstChild(const char* name) const
        {
            tinyxml2::XMLElement* element = self->FirstChildElement(name);

            if(element == nullptr) {
                return std::nullopt;
            }

            return XmlElement { element };
        }

        XmlStringAttribute<const char*> Attribute(const char* name)
        {
            const char* raw = self->Attribute(name);

            if(raw == nullptr) {
                return XmlStringAttribute<const char*> { self, name };
            }

            return XmlStringAttribute<const char*> { self, name, raw };
        }

        template <typename... Args>
        XmlStringAttribute<const char*> AnyAttribute(Args... args)
        {
            const char* names[sizeof...(args)] = { args... };
            XmlStringAttribute<const char*> prev { self, "" };

            for(const char* name : names) {
                auto attribute = Attribute(name);

                if(attribute) {
                    return attribute;
                }

                prev = attribute;
            }

            // return last empty attribute,
            // or the unnamed one if there were no names given
            return prev;
        }

        /// Get the name of this XML element (its tag), returns a valid (non-null) c-string
        /// Try using TagName() when possible
        const char* Name() const
        {
            return self->Name();
        }

        /// Get the name of this XML element (its tag),
        /// this always returns a non-empty string_view
        bool Is(std::string_view tag) const
        {
            return tag == self->Name();
        }

        /// Get a iterator to the first child of this element
        /// returns a valid element iterator
        XmlElementIterator begin() const;

        /// Get a iterator to the one-past-last child of this element
        /// returns an empty element iterator
        constexpr XmlElementIterator end() const;

        /// Create a iterable object that when iterated over
        /// iterates over children with a matching tag name
        XmlElementSiblingSet ForEach(const char* name) const;

        /// Get a pointer to the underlying API pointer
        /// avoid directly using this method
        [[deprecated]]
        constexpr tinyxml2::XMLElement* RawNode() const
        {
            return self;
        }

        std::string_view GetRawText() const
        {
            const char* raw = self->GetText();

            if(raw == nullptr) {
                return "";
            }

            return raw;
        }

        constexpr bool operator!=(const XmlElement& other) const
        {
            return other.self != this->self;
        }

        constexpr bool operator==(const XmlElement& other) const
        {
            return other.self == this->self;
        }
    };

    class XmlElementIterator {
    private:
        tinyxml2::XMLElement* element = nullptr;
        const char* name = nullptr;

    public:
        constexpr XmlElementIterator() = default;

        constexpr XmlElementIterator(tinyxml2::XMLElement* element, const char* name)
            : element(element)
            , name(name)
        {
        }

        void operator++()
        {
            element = element ? element->NextSiblingElement(name) : nullptr;
        }

        constexpr XmlElement operator*() const
        {
            return { element }; // calling on empty (end) iterator is invalid
        }

        constexpr bool operator!=(const XmlElementIterator& other) const
        {
            return other.element != this->element;
        }

        constexpr bool operator==(const XmlElementIterator& other) const
        {
            return other.element == this->element;
        }
    };

    class XmlElementSiblingSet {
    private:
        tinyxml2::XMLElement* element;
        const char* name;

    public:
        XmlElementSiblingSet(tinyxml2::XMLElement* element, const char* name)
            : element(element)
            , name(name)
        {
        }

        XmlElementIterator begin() const
        {
            tinyxml2::XMLElement* child = name
                ? element->FirstChildElement(name)
                : element->FirstChildElement();

            return { child, name };
        }

        constexpr XmlElementIterator end() const
        {
            return {};
        }
    };

    inline XmlElementIterator XmlElement::begin() const
    {
        return { self->FirstChildElement(), nullptr };
    }

    inline constexpr XmlElementIterator XmlElement::end() const
    {
        return {};
    }

    inline XmlElementSiblingSet XmlElement::ForEach(const char* name) const
    {
        return { self, name };
    }

}

#endif
