
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

    auto ttf = std::make_shared<grvl::TrueTypeData>(ROMFS_PATH "/fonts/Roboto.ttf.gz");

    // fonts
    manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont(ttf, 16));
    auto font = new grvl::TrueTypeFont(ttf, 60);
    

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);


    // Bounding box parameters
    constexpr int32_t startX = 200;
    constexpr int32_t startY = 150;
    constexpr int32_t width = 400;
    constexpr int32_t height = 300;

    const auto drawChar = [&](char c, int32_t x, int32_t y) {
        manager.painter.DrawAntialiasedChar(font, x - 15, y + 20, c, 0xFFAAAAAA);
        manager.painter.DrawAntialiasedCharInBound(font, x - 15, y + 20, startX, startY, width, height, c, 0xFFFF0000);
    };

    while (app->ShouldRun()) {
        app->Render();

        drawChar('A', startX - 50,         startY - 50);
        drawChar('B', startX,              startY - 50);
        drawChar('C', startX + width / 2,  startY - 50);
        drawChar('D', startX + width,      startY - 50);
        drawChar('E', startX + width + 50, startY - 50);

        drawChar('F', startX - 50,         startY);
        drawChar('G', startX,              startY);
        drawChar('H', startX + width / 2,  startY);
        drawChar('X', startX + width,      startY);
        drawChar('J', startX + width + 50, startY);

        drawChar('K', startX - 50,         startY + height / 2);
        drawChar('L', startX,              startY + height / 2);
        drawChar('M', startX + width / 2,  startY + height / 2);
        drawChar('N', startX + width,      startY + height / 2);
        drawChar('O', startX + width + 50, startY + height / 2);

        drawChar('P', startX - 50,         startY + height);
        drawChar('Q', startX,              startY + height);
        drawChar('R', startX + width / 2,  startY + height);
        drawChar('S', startX + width,      startY + height);
        drawChar('T', startX + width + 50, startY + height);

        drawChar('U', startX - 50,         startY + height + 50);
        drawChar('V', startX,              startY + height + 50);
        drawChar('W', startX + width / 2,  startY + height + 50);
        drawChar('X', startX + width,      startY + height + 50);
        drawChar('Y', startX + width + 50, startY + height + 50);

        manager.painter.DrawRectangle(startX, startY, width, height, 0xFF00FF00);

        app->Swap();
        app->Poll();
    }

    delete font;
    delete app;
}
