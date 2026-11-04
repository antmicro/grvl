
#include <catch2/catch_test_macros.hpp>

#include <grvl/container/AbstractView.h>
#include <grvl/impl/Attribute.h>
#include <grvl/impl/Element.h>

using namespace grvl;
using namespace tinyxml2;

TEST_CASE("XML Iteration", "[xml]")
{

    std::string document = R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc>
            <test a="1"/>
            <test a="22" b="2"/>
            <foo a="11" c="3"/>
            <bar/>
        </doc>
    )XML";

    XMLDocument doc;
    REQUIRE(doc.Parse(document.c_str(), document.length()) == XML_SUCCESS);
    XMLElement* raw = doc.RootElement();

    XmlElement root { raw };

    int a = 0;

    for(XmlElement child : root) {
        a += child.Attribute("a").Parse<int>().Else(100).Get();
    }

    REQUIRE(a == (100 + 11 + 22 + 1));

    a = 0;

    for(XmlElement child : root.ForEach("test")) {
        a += child.Attribute("a").Parse<int>().Else(100).Get();
    }

    REQUIRE(a == (22 + 1));
}

TEST_CASE("XML Alternate", "[xml]")
{

    std::string document = R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc bar="11" baz="12"/>
    )XML";

    XMLDocument doc;
    REQUIRE(doc.Parse(document.c_str(), document.length()) == XML_SUCCESS);
    XMLElement* raw = doc.RootElement();

    XmlElement root { raw };

    REQUIRE(root.Attribute("foo").IsEmpty());
    REQUIRE(!root.Attribute("bar").IsEmpty());
    REQUIRE(!root.Attribute("baz").IsEmpty());

    REQUIRE(!root.AnyAttribute("foo", "bar").IsEmpty());
    REQUIRE(!root.AnyAttribute("foo", "foo", "bar", "foo").IsEmpty());
    REQUIRE(!root.AnyAttribute("foo", "foo", "baz", "foo").IsEmpty());
    REQUIRE(root.AnyAttribute("foo", "foo", "foo").IsEmpty());

    REQUIRE(root.AnyAttribute().Name() == "");
    REQUIRE(root.AnyAttribute("foo").Name() == "foo");
    REQUIRE(root.AnyAttribute("foo", "bar").Name() == "bar");
    REQUIRE(root.AnyAttribute("foo", "bar", "foo").Name() == "bar");

    REQUIRE(root.AnyAttribute("foo", "bar", "baz").Parse<int>().Get() == 11);
    REQUIRE(root.AnyAttribute("foo", "baz", "bar").Parse<int>().Get() == 12);
}

TEST_CASE("XML Color", "[xml]")
{

    std::string document = R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc c="#55667711" d="123" e="" f="0xff"/>
    )XML";

    XMLDocument doc;
    REQUIRE(doc.Parse(document.c_str(), document.length()) == XML_SUCCESS);
    XMLElement* raw = doc.RootElement();

    XmlElement root { raw };

    uint32_t c = root.Attribute("c").ParseColor().Else(0).Get();
    REQUIRE(c == 0x55667711);

    uint32_t d = root.Attribute("d").ParseColor().Else(0).Get();
    REQUIRE(d == 123);

    REQUIRE(root.Attribute("e").ParseColor().IsEmpty() == true);
    REQUIRE(root.Attribute("f").ParseColor().IsEmpty() == true);
}

TEST_CASE("XML Bools", "[xml]")
{

    std::string document = R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc a="1" b="true" c="True" d="FALSE"/>
    )XML";

    XMLDocument doc;
    REQUIRE(doc.Parse(document.c_str(), document.length()) == XML_SUCCESS);
    XMLElement* raw = doc.RootElement();

    XmlElement root { raw };

    bool a = root.Attribute("a").Parse<bool>().Else(false).Get();
    REQUIRE(a == true);

    bool b = root.Attribute("b").Parse<bool>().Else(false).Get();
    REQUIRE(b == true);

    bool c = root.Attribute("c").Parse<bool>().Else(false).Get();
    REQUIRE(c == true);

    bool d = root.Attribute("d").Parse<bool>().Else(true).Get();
    REQUIRE(d == false);
}

TEST_CASE("XML Ints", "[xml]")
{

    std::string document = R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc a="123" b="0x3f" c="0xfood" d="123abc" e=""/>
    )XML";

    XMLDocument doc;
    REQUIRE(doc.Parse(document.c_str(), document.length()) == XML_SUCCESS);
    XMLElement* raw = doc.RootElement();

    XmlElement root { raw };

    int a = root.Attribute("a").Parse<int>().Else(0).Get();
    REQUIRE(a == 123);

    int b = root.Attribute("b").Parse<int>().Else(0).Get();
    REQUIRE(b == 0x3f);

    REQUIRE(root.Attribute("c").Parse<int>().IsEmpty() == true);
    REQUIRE(root.Attribute("d").Parse<int>().IsEmpty() == true);
    REQUIRE(root.Attribute("e").Parse<int>().IsEmpty() == true);
}

TEST_CASE("XML CollectionDefinition Parser", "[xml]")
{

    gui_callbacks_t callbacks {};
    grvl::grvl::Init(&callbacks);

    // valid definitions

    {
        CollectionDefinition test = CollectionDefinition::Parse("0/0");
        REQUIRE(test.position == 0);
        REQUIRE(test.size == 0);
    }

    {
        CollectionDefinition test = CollectionDefinition::Parse("11/42");
        REQUIRE(test.position == 11);
        REQUIRE(test.size == 42);
    }

    // invalid definitions

    {
        CollectionDefinition test = CollectionDefinition::Parse("abc/42");
        REQUIRE(test.position == 0);
        REQUIRE(test.size == 42);
    }

    {
        CollectionDefinition test = CollectionDefinition::Parse("11/abc");
        REQUIRE(test.position == 11);
        REQUIRE(test.size == 0);
    }

    {
        CollectionDefinition test = CollectionDefinition::Parse("11");
        REQUIRE(test.position == 0);
        REQUIRE(test.size == 0);
    }

    grvl::grvl::Destroy();
}
