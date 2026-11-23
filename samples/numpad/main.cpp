#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/platform/LinuxGenericApp.h>

#include <memory>

bool exit_requested = false;

void SynchronizeNumLock(bool state)
{
    grvl::JSEngine::DefineGlobalNumber("numLockEnabled", state ? 1 : 0);
    grvl::JSEngine::MakeJavaScriptFunctionCall("UpdateNumLockLabels");
    grvl::Manager::GetInstance().ScheduleRedraw();
}

int main()
{
    grvl::PosixApp* app = grvl::CreateGenericLinuxApp(640, 900);
    if(!app) {
        return 1;
    }
    grvl::Application::Init(app);
    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::JSEngine::AddGlobalFunction("Exit", [](duk_context*) -> duk_ret_t {
        exit_requested = true;
        return grvl::JSEngine::NO_RETURN_VALUE; }, 0);

    auto& manager = grvl::Manager::GetInstance();
    auto data = std::make_shared<grvl::TrueTypeData>(DEMO_FONT_PATH);
    manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont(data, 23));
    manager.AddFontToFontContainer("small", new grvl::TrueTypeFont(data, 18));
    manager.AddFontToFontContainer("large", new grvl::TrueTypeFont(data, 31));

    if(manager.BuildFromXML(ROMFS_PATH "/gui.xml") != 0) {
        delete app;
        return 1;
    }
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    bool previous_numlock = app->IsNumLockEnabled();
    SynchronizeNumLock(previous_numlock);

    while(app->ShouldRun() && !exit_requested) {
        app->Render();
        app->Swap();
        app->Poll();

        const bool current_numlock = app->IsNumLockEnabled();
        if(previous_numlock != current_numlock) {
            previous_numlock = current_numlock;
            SynchronizeNumLock(current_numlock);
        }
    }
    delete app;
    return 0;
}
