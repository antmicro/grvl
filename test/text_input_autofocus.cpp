
#include <catch2/catch_test_macros.hpp>

#include <grvl/Font.h>
#include <grvl/Manager.h>
#include <grvl/component/TextInput.h>
#include <grvl/grvl.h>

#include <string>

using namespace grvl;

namespace {

    Manager& Build(const std::string& document, const char* screen = "home")
    {
        gui_callbacks_t callbacks {};
        grvl::grvl::Init(&callbacks);

        Manager::Initialize(120, 40, 4, false);
        Manager& manager = Manager::GetInstance();
        manager.AddFontToFontContainer("normal", new GrvlBakedFont(GRVL_TEST_FONT));

        REQUIRE(manager.BuildFromXMLString(document) != -1);
        manager.InitializationFinished();
        manager.SetActiveScreen(screen, 0);
        return manager;
    }

    std::string Screen(const char* id, const std::string& inputAttributes)
    {
        return "<customView id=\"" + std::string(id) + "\"><TextInput id=\"in_" + id
            + "\" x=\"0\" y=\"0\" width=\"120\" height=\"40\" font=\"normal\" " + inputAttributes + "/></customView>";
    }

} // namespace

TEST_CASE("TextInput autoFocus: nothing is focused by default", "[component][autofocus]")
{
    Manager& manager = Build("<doc><stylesheet></stylesheet>" + Screen("home", "") + "</doc>");
    REQUIRE(manager.GetActiveInput() == nullptr);
}

TEST_CASE("TextInput autoFocus: takes focus when the screen opens", "[component][autofocus]")
{
    Manager& manager = Build("<doc><stylesheet></stylesheet>" + Screen("home", R"(autoFocus="true")") + "</doc>");

    REQUIRE(manager.GetActiveInput() != nullptr);
    REQUIRE(manager.GetActiveInput() == manager.FindElementInTheActiveScreenById("in_home"));
}

TEST_CASE("TextInput autoFocus: typing reaches the focused input", "[component][autofocus]")
{
    Manager& manager = Build("<doc><stylesheet></stylesheet>" + Screen("home", R"(autoFocus="true")") + "</doc>");

    manager.ProcessTextInput("hi");

    auto* input = static_cast<TextInput*>(manager.FindElementInTheActiveScreenById("in_home"));
    REQUIRE(std::string(input->GetText()) == "hi");
}

TEST_CASE("TextInput autoFocus: a screen without the element keeps the focus", "[component][autofocus]")
{
    Manager& manager = Build("<doc><stylesheet></stylesheet>" + Screen("home", R"(autoFocus="true")")
                             + "<customView id=\"other\"></customView></doc>");

    TextInput* focused = manager.GetActiveInput();
    REQUIRE(focused != nullptr);

    manager.SetActiveScreen("other", 0);
    REQUIRE(manager.GetActiveInput() == focused);
}

TEST_CASE("TextInput autoFocus: a prefab instance takes focus, not the template",
          "[component][autofocus]")
{
    // The focus is resolved by id when the screen opens, so it lands on the
    // element inside the view and never on the prefab that was parsed earlier.
    Manager& manager = Build(
        "<doc><stylesheet></stylesheet>"
        "<prefab id=\"row\" width=\"120\" height=\"40\">"
        "<TextInput id=\"in_home\" x=\"0\" y=\"0\" width=\"120\" height=\"40\" font=\"normal\" autoFocus=\"true\"/>"
        "</prefab>"
        + Screen("home", R"(autoFocus="true")") + "</doc>");

    Component* inScreen = manager.FindElementInTheActiveScreenById("in_home");
    REQUIRE(inScreen != nullptr);
    REQUIRE(manager.GetActiveInput() == inScreen);
}

TEST_CASE("TextInput autoFocus: clone carries the flag", "[component][autofocus]")
{
    Manager& manager = Build("<doc><stylesheet></stylesheet>" + Screen("home", R"(autoFocus="true")") + "</doc>");

    auto* input = static_cast<TextInput*>(manager.FindElementInTheActiveScreenById("in_home"));
    auto* copy = static_cast<TextInput*>(input->Clone());

    REQUIRE(copy->IsAutoFocus());
    delete copy;
}
