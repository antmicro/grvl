
#include <catch2/catch_test_macros.hpp>

#include <grvl/grvl.h>
#include <grvl/Manager.h>
#include <grvl/JSEngine.h>

using namespace grvl;

static thread_local int last_callback = 0;

duk_ret_t SimpleJavascriptCallback(duk_context* ctx)
{
    last_callback = duk_to_int(ctx, 1);
    return 0;
}

static void PrintfNewline(const char* text, va_list argList)
{
    vprintf(text, argList);
    printf("\n");
}

TEST_CASE("Button Component", "[component]")
{

    gui_callbacks_t callbacks {};
    callbacks.gui_printf = PrintfNewline;
    grvl::grvl::Init(&callbacks);

    Manager::Initialize(50, 50, 4, false);
    Manager& manager = grvl::Manager::GetInstance();

    int parsed = manager.BuildFromXMLString(R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc>
            <stylesheet></stylesheet>
            <customView id="home" backgroundColor="#FFFF0000">
                <button id="0" text="" x="0" y="0" width="30" height="30" backgroundColor="#FF00FF00" activeBackgroundColor="#FF0000FF" borderType="box" textColor="#FFFFFFFF" onPress="ButtonCallback('123')" />
            </customView>
        </doc>
    )XML");

    REQUIRE(parsed != -1);

    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    JSEngine::AddGlobalFunction("ButtonCallback", SimpleJavascriptCallback, 2);

    // inside buton bounds
    manager.ProcessTouchPoint(true, 10, 10);
    manager.MainLoopIteration();
    REQUIRE(last_callback == 123);

    last_callback = 0;

    // stop pressing
    manager.ProcessTouchPoint(false, 10, 10);
    manager.MainLoopIteration();
    REQUIRE(last_callback == 0);

    // outside button bounds
    manager.ProcessTouchPoint(true, 35, 10);
    manager.MainLoopIteration();
    REQUIRE(last_callback == 0);

    grvl::grvl::Destroy();

}

TEST_CASE("Checkbox Component", "[component]")
{

    gui_callbacks_t callbacks {};
    callbacks.gui_printf = PrintfNewline;
    grvl::grvl::Init(&callbacks);

    Manager::Initialize(50, 50, 4, false);
    Manager& manager = grvl::Manager::GetInstance();

    int parsed = manager.BuildFromXMLString(R"XML(
        <?xml version="1.0" encoding="UTF-8"?>
        <doc>
            <stylesheet></stylesheet>
            <customView id="home" backgroundColor="#FFFF0000">
                <checkbox id="0" text="" x="0" y="0" width="30" height="30" />
            </customView>
        </doc>
    )XML");

    REQUIRE(parsed != -1);

    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    Checkbox* box = dynamic_cast<Checkbox*>(manager.FindElementInTheActiveScreenById("0"));
    REQUIRE(box != nullptr);

    REQUIRE(box->GetSwitchState() == false);

    // checkbox should be selected on release
    manager.ProcessTouchPoint(true, 15, 15);
    manager.MainLoopIteration();
    REQUIRE(box->GetSwitchState() == false);

    manager.ProcessTouchPoint(false, 15, 15);
    manager.MainLoopIteration();
    REQUIRE(box->GetSwitchState() == true);

    grvl::grvl::Destroy();

}
