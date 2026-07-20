
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

#include <grvl/Manager.h>
#include <grvl/JSEngine.h>
#include <grvl/platform/LinuxGenericApp.h>

#define WIDTH 800
#define HEIGHT 600

grvl::ImageContent* ic = nullptr;
grvl::ImageContent* dummy = nullptr;

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

    ic = new grvl::ImageContent(ROMFS_PATH "/images/spinner.gif");
    uint32_t* dummy_data  = static_cast<uint32_t*>(malloc(64*64*4));
    for (uint8_t y = 0; y < 64; ++y) {
        for (uint8_t x = 0; x < 64; ++x) {
            uint32_t value = 0xFFFF0000 | (y << 2) << 8 | (x << 2);
            dummy_data[64 * y + x] = value;
        }
    }
    dummy = new grvl::ImageContent(reinterpret_cast<uint8_t*>(dummy_data), 64, 64, 1);

    grvl::JSEngine::AddGlobalFunction("Rotate90", [] (duk_context* ctx) -> duk_ret_t {
        ic->Rotate90();
        dummy->Rotate90();
        return grvl::JSEngine::NO_RETURN_VALUE;
    }, 0);

    auto ttf = std::make_shared<grvl::TrueTypeData>(ROMFS_PATH "/fonts/Roboto.ttf.gz");
    manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont(ttf, 16));

    manager.AddImageContentToContainer("img", ic);
    manager.AddImageContentToContainer("dummy", dummy);

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    while (app->ShouldRun()) {
        app->Render();
        app->Swap();
        app->Poll();
    }

    delete app;
}
