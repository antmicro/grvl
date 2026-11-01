
#include <catch2/catch_test_macros.hpp>

#include <grvl/File.h>
#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/container/Division.h>
#include <grvl/XMLPreprocessor.h>
#include <grvl/grvl.h>

#include <cstring>
#include <string>
#include <tinyxml2.h>
#include <unordered_map>
#include <utility>

using namespace grvl;
using namespace tinyxml2;

namespace {

    struct JSFixture {
        JSFixture()
        {
            gui_callbacks_t callbacks { };
            grvl::grvl::Init(&callbacks);
        }
    };

    // Returns the "v" attribute of the first child of the root.
    std::string Evaluate(const std::string& document)
    {
        std::string source;
        std::string error;
        REQUIRE(XMLPreprocessor::EscapeExpressions(document, source, error));

        XMLDocument doc;
        REQUIRE(doc.Parse(source.c_str(), source.length()) == XML_SUCCESS);
        REQUIRE(doc.RootElement() != nullptr);

        // Mirrors Manager: the scripts run before the defines and the spans.
        for(XMLElement* script = doc.RootElement()->FirstChildElement("script"); script != nullptr;
            script = script->NextSiblingElement("script")) {
            if(const char* code = script->GetText()) {
                REQUIRE(JSEngine::LoadJavaScriptCode(code, std::strlen(code)));
            }
        }

        REQUIRE(XMLPreprocessor::CollectDefines(doc.RootElement(), error));
        REQUIRE(XMLPreprocessor::ExpandExpressions(&doc, error));

        XMLElement* target = doc.RootElement()->FirstChildElement("a");
        REQUIRE(target != nullptr);
        REQUIRE(target->Attribute("v") != nullptr);
        return target->Attribute("v");
    }

    std::string EvaluateExpr(const std::string& expression)
    {
        return Evaluate("<doc><a v=\"${" + expression + "}\"/></doc>");
    }

    bool Fails(const std::string& document)
    {
        std::string source;
        std::string error;
        if(!XMLPreprocessor::EscapeExpressions(document, source, error)) {
            return true;
        }

        XMLDocument doc;
        if(doc.Parse(source.c_str(), source.length()) != XML_SUCCESS || doc.RootElement() == nullptr) {
            return true;
        }

        return !XMLPreprocessor::CollectDefines(doc.RootElement(), error)
            || !XMLPreprocessor::ExpandExpressions(&doc, error);
    }

} // namespace

TEST_CASE_METHOD(JSFixture, "Expressions: arithmetic and built-ins", "[xml][expressions]")
{
    REQUIRE(EvaluateExpr("2 + 4") == "6");
    REQUIRE(EvaluateExpr("Math.round(1920 / 7)") == "274");
    REQUIRE(EvaluateExpr("Math.max(3, 9)") == "9");
    REQUIRE(EvaluateExpr("(1920 / 1080).toFixed(3)") == "1.778");
}

TEST_CASE_METHOD(JSFixture, "Expressions: XML special characters stay unescaped", "[xml][expressions]")
{
    REQUIRE(EvaluateExpr("5 > 3 && 2 < 4") == "true");
    REQUIRE(Evaluate(R"(<doc><a v="${100 > 5 ? "big" : "small"}"/></doc>)") == "big");
    REQUIRE(Evaluate(R"(<doc><a v="${'x' + "y"}"/></doc>)") == "xy");
}

TEST_CASE_METHOD(JSFixture, "Expressions: braces inside a span", "[xml][expressions]")
{
    REQUIRE(EvaluateExpr(" {a:1,b:{c:7}}.b.c ") == "7");
    REQUIRE(EvaluateExpr(" '}' ") == "}");
    REQUIRE(EvaluateExpr(" (function(){ return 3; })() ") == "3");
    REQUIRE(EvaluateExpr(" [1,2,3].map(function(n){return n*2;}).join('-') ") == "2-4-6");
    REQUIRE(EvaluateExpr(" 1 /* } */ + 2 ") == "3");
}

TEST_CASE_METHOD(JSFixture, "Expressions: literal dollar sign", "[xml][expressions]")
{
    REQUIRE(Evaluate(R"(<doc><a v="$${literal}"/></doc>)") == "${literal}");
    REQUIRE(Evaluate(R"(<doc><a v="costs 5$$"/></doc>)") == "costs 5$");
}

TEST_CASE_METHOD(JSFixture, "Expressions: defines block", "[xml][expressions]")
{
    const std::string document = R"XML(
        <doc>
          <defines>
            <define name="baseWidth" value="1920"/>
            <define name="halfWidth" value="baseWidth / 2"/>
          </defines>
          <script>function px(v) { return Math.round(v * 2); }</script>
          <a v="${halfWidth} ${px(5)}"/>
        </doc>
    )XML";

    REQUIRE(Evaluate(document) == "960 10");
}

TEST_CASE_METHOD(JSFixture, "Expressions: a span calls a function from <script>", "[xml][expressions]")
{
    const std::string document = R"XML(
        <doc>
          <script>
            function double(v) { return v * 2; }
            var unit = 7;
          </script>
          <a v="${double(unit)}"/>
        </doc>
    )XML";

    REQUIRE(Evaluate(document) == "14");
}

TEST_CASE_METHOD(JSFixture, "Expressions: a define calls a function from <script>", "[xml][expressions]")
{
    const std::string document = R"XML(
        <doc>
          <script>function half(v) { return v / 2; }</script>
          <defines>
            <define name="col" value="half(640)"/>
          </defines>
          <a v="${col}"/>
        </doc>
    )XML";

    REQUIRE(Evaluate(document) == "320");
}

TEST_CASE_METHOD(JSFixture, "Expressions: a script body is left alone", "[xml][expressions]")
{
    // ${...} inside a script is JavaScript, not a span to expand.
    std::string source;
    std::string error;
    const std::string document = R"(<doc><script>var s = "${nope}";</script><a v="1"/></doc>)";

    REQUIRE(XMLPreprocessor::EscapeExpressions(document, source, error));

    XMLDocument doc;
    REQUIRE(doc.Parse(source.c_str(), source.length()) == XML_SUCCESS);
    REQUIRE(XMLPreprocessor::ExpandExpressions(&doc, error));

    XMLElement* script = doc.RootElement()->FirstChildElement("script");
    REQUIRE(script != nullptr);
    REQUIRE(std::string(script->GetText()).find("${nope}") != std::string::npos);
}

TEST_CASE_METHOD(JSFixture, "Expressions: <defines> rejects a nested script", "[xml][expressions]")
{
    REQUIRE(Fails(R"(<doc><defines><script>var a = 1;</script></defines><a v="1"/></doc>)"));
}

TEST_CASE_METHOD(JSFixture, "Expressions: defines block is removed from the tree", "[xml][expressions]")
{
    std::string source;
    std::string error;
    REQUIRE(XMLPreprocessor::EscapeExpressions(
        R"(<doc><defines><define name="q" value="1"/></defines><a v="${q}"/></doc>)", source, error));

    XMLDocument doc;
    REQUIRE(doc.Parse(source.c_str(), source.length()) == XML_SUCCESS);
    REQUIRE(XMLPreprocessor::CollectDefines(doc.RootElement(), error));

    REQUIRE(doc.RootElement()->FirstChildElement("defines") == nullptr);
}

TEST_CASE_METHOD(JSFixture, "Expressions: results are escaped on output", "[xml][expressions]")
{
    REQUIRE(EvaluateExpr(" 'a < b & c' ") == "a < b & c");
}

TEST_CASE_METHOD(JSFixture, "Expressions: errors are reported", "[xml][expressions]")
{
    REQUIRE(Fails(R"(<doc><a v="${undefinedName}"/></doc>)"));
    REQUIRE(Fails(R"(<doc><a v="${ 1 + }"/></doc>)"));
    REQUIRE(Fails(R"(<doc><a v="${ {1 }"/></doc>)"));
    REQUIRE(Fails(R"(<doc><defines><define name="q"/></defines><a v="1"/></doc>)"));
    REQUIRE(Fails(R"(<doc><defines><bogus/></defines><a v="1"/></doc>)"));
}

TEST_CASE_METHOD(JSFixture, "Expressions: comments and CDATA pass through", "[xml][expressions]")
{
    std::string source;
    std::string error;
    REQUIRE(XMLPreprocessor::EscapeExpressions("<doc><!-- ${a > b} --><a v=\"1\"/></doc>", source, error));

    REQUIRE(source.find("${a > b}") != std::string::npos);
}

TEST_CASE_METHOD(JSFixture, "Expressions: grvl injects the screen globals", "[xml][expressions]")
{
    Manager::Initialize(1920, 1080, 4, false);
    Manager& manager = Manager::GetInstance();

    REQUIRE(manager.BuildFromXMLString(R"XML(
        <doc>
          <stylesheet></stylesheet>
          <customView id="home">
            <button id="ok" text="" x="${screenWidth}" y="${screenHeight}"
                    width="${Math.round(screenRatio * 100)}" height="10"/>
          </customView>
        </doc>
    )XML") != -1);

    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    Component* button = manager.FindElementInTheActiveScreenById("ok");
    REQUIRE(button != nullptr);
    REQUIRE(button->GetX() == 1920);
    REQUIRE(button->GetY() == 1080);
    REQUIRE(button->GetWidth() == 178); // 16/9 * 100
}

TEST_CASE_METHOD(JSFixture, "Expressions: one template, two resolutions", "[xml][expressions]")
{
    static constexpr const char* TEMPLATE = R"XML(
        <doc>
          <defines>
            <define name="scale" value="screenWidth / 100"/>
          </defines>
          <script>function px(v) { return Math.round(v * scale); }</script>
          <stylesheet></stylesheet>
          <customView id="home">
            <button id="ok" text="" x="${px(10)}" y="0" width="${px(30)}" height="10"/>
          </customView>
        </doc>
    )XML";

    const auto widthAt = [](uint32_t screenWidth) {
        Manager::Initialize(screenWidth, 100, 4, false);
        Manager& manager = Manager::GetInstance();

        REQUIRE(manager.BuildFromXMLString(TEMPLATE) != -1);
        manager.InitializationFinished();
        manager.SetActiveScreen("home", 0);

        Component* button = manager.FindElementInTheActiveScreenById("ok");
        REQUIRE(button != nullptr);
        return std::make_pair(button->GetX(), button->GetWidth());
    };

    const auto small = widthAt(100);
    REQUIRE(small.first == 10);
    REQUIRE(small.second == 30);

    const auto large = widthAt(400);
    REQUIRE(large.first == 40);
    REQUIRE(large.second == 120);
}

TEST_CASE_METHOD(JSFixture, "Expressions: includes work with no filesystem", "[xml][expressions]")
{
    static char metrics[] = R"XML(<defines>
        <define name="unit" value="screenWidth / 10"/>
    </defines>)XML";

    std::unordered_map<std::string, File::MapEntry> files {
        { "metrics.xml", { reinterpret_cast<unsigned char*>(metrics), sizeof(metrics) - 1 } },
    };

    const bool hadFilesystem = !File::noFS;
    File::NoFilesystem(&files);

    Manager::Initialize(200, 100, 4, false);
    Manager& manager = Manager::GetInstance();
    manager.SetWorkingDirectory("romfs");

    const int32_t parsed = manager.BuildFromXMLString(R"XML(
        <doc xmlns:xi="http://www.w3.org/2001/XInclude">
          <xi:include href="metrics.xml"/>
          <stylesheet></stylesheet>
          <customView id="home">
            <button id="ok" text="" x="0" y="0" width="${unit * 3}" height="10"/>
          </customView>
        </doc>
    )XML");

    REQUIRE(parsed != -1);
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    Component* button = manager.FindElementInTheActiveScreenById("ok");
    REQUIRE(button != nullptr);
    REQUIRE(button->GetWidth() == 60); // 200 / 10 * 3

    if(hadFilesystem) {
        File::noFS = false;
    }
}

TEST_CASE_METHOD(JSFixture, "Expressions: documents with no span are unchanged", "[xml][expressions]")
{
    const std::string document = R"(<doc><a v="plain" w="a &gt; b"/></doc>)";

    std::string source;
    std::string error;
    REQUIRE(XMLPreprocessor::EscapeExpressions(document, source, error));
    REQUIRE(source == document);
}

TEST_CASE_METHOD(JSFixture, "Expressions: an included <doc> root is a wrapper", "[xml][expressions]")
{
    static char views[] = R"XML(<doc>
        <prefab id="item" width="${screenWidth / 4}" height="10"/>
        <customView id="list">
          <button id="ok" text="" x="0" y="0" width="${screenWidth / 2}" height="10"/>
        </customView>
    </doc>)XML";

    std::unordered_map<std::string, File::MapEntry> files {
        { "views.xml", { reinterpret_cast<unsigned char*>(views), sizeof(views) - 1 } },
    };

    const bool hadFilesystem = !File::noFS;
    File::NoFilesystem(&files);

    Manager::Initialize(200, 100, 4, false);
    Manager& manager = Manager::GetInstance();
    manager.SetWorkingDirectory("romfs");

    const int32_t parsed = manager.BuildFromXMLString(R"XML(
        <doc xmlns:xi="http://www.w3.org/2001/XInclude">
          <stylesheet></stylesheet>
          <xi:include href="views.xml"/>
        </doc>
    )XML");

    REQUIRE(parsed != -1);
    manager.InitializationFinished();

    Division* item = manager.GetPrefabByID("item");
    REQUIRE(item != nullptr);
    REQUIRE(item->GetWidth() == 50);

    manager.SetActiveScreen("list", 0);
    Component* button = manager.FindElementInTheActiveScreenById("ok");
    REQUIRE(button != nullptr);
    REQUIRE(button->GetWidth() == 100);

    if(hadFilesystem) {
        File::noFS = false;
    }
}
