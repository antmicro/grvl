
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

#include <grvl/Manager.h>
#include <grvl/JSEngine.h>
#include <grvl/platform/LinuxGenericApp.h>

#define WIDTH 800
#define HEIGHT 600

int main()
{
    grvl::Application* app = grvl::CreateGenericLinuxApp(WIDTH, HEIGHT);
    grvl::Application::Init(app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::Manager& manager = grvl::Manager::GetInstance();

    grvl::JSEngine::AddGlobalFunction("Exit", [] (duk_context* ctx) -> duk_ret_t {
        int status = duk_to_int(ctx, 0);
        exit(status);
        return grvl::JSEngine::NO_RETURN_VALUE;
    }, 1);

    grvl::JSEngine::AddGlobalFunction("SetDebugOverlay", [] (duk_context* ctx) -> duk_ret_t {
        auto type = duk_to_int(ctx, 0) ? grvl::Performance::FULL : grvl::Performance::NONE;
        grvl::Manager::GetInstance().perf.overlay = type;
        return grvl::JSEngine::NO_RETURN_VALUE;
    }, 1);

    auto ttf = std::make_shared<grvl::TrueTypeData>(ROMFS_PATH "/Roboto.ttf.gz");
    auto font = new grvl::TrueTypeFont(ttf, 16);

    manager.AddFontToFontContainer("normal", font);
    manager.AddFontToFontContainer("roboto-medium", font);

    manager.BuildFromXML(ROMFS_PATH "/example.xml");
    manager.InitializationFinished();

    while (app->ShouldRun()) {
        app->Render();
        app->Swap();
        app->Poll();
    }

    delete app;
}
