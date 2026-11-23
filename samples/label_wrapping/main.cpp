// GRVL Label multiline / wrapping demonstration.
#include <cstdlib>
#include <memory>

#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/platform/LinuxGenericApp.h>

int main()
{
    constexpr int width = 800;
    constexpr int height = 950;

    grvl::Application* app = grvl::CreateGenericLinuxApp(width, height);
    grvl::Application::Init(app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::JSEngine::AddGlobalFunction("Exit", [](duk_context* ctx) -> duk_ret_t {
        std::exit(duk_to_int(ctx, 0));
        return grvl::JSEngine::NO_RETURN_VALUE; }, 1);

    auto ttf = std::make_shared<grvl::TrueTypeData>(SAMPLE_FONT_PATH);
    grvl::Manager& manager = grvl::Manager::GetInstance();
    manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont(ttf, 16));

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    while(app->ShouldRun()) {
        app->Render();
        app->Swap();
        app->Poll();
    }

    delete app;
    return 0;
}
