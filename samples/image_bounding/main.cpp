#include <algorithm>
#include <chrono>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

#include <grvl/Manager.h>
#include <grvl/JSEngine.h>
#include <grvl/platform/LinuxGenericApp.h>

#define WIDTH 800
#define HEIGHT 600

int main(int argc, char** argv)
{
    const bool rotated = std::any_of(
        argv + 1,
        argv + argc,
        [](const char* argument) {
            return std::string_view(argument) == "--rotated";
        }
    );

    grvl::Application* app = grvl::CreateGenericLinuxApp(WIDTH, HEIGHT, rotated);
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
    
    grvl::ImageContent red_square(ROMFS_PATH "/images/red_square.png");
    grvl::ImageContent gray_square(ROMFS_PATH "/images/gray_square.png");
    grvl::ImageContent spinner(ROMFS_PATH "/images/spinner.gif");

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);


    // Bounding box parameters
    constexpr int32_t startX = 200;
    constexpr int32_t startY = 150;
    constexpr int32_t width = 400;
    constexpr int32_t height = 300;

    const auto drawSquare = [&](int32_t x, int32_t y, uint32_t frame) {
        manager.painter.DrawImage(x - 20, y - 20, &gray_square);
        manager.painter.PushDrawingBoundsStackElement(startX, startY, startX + width, startY + height);
        manager.painter.DrawImage(x - 20, y - 20, &red_square);
        manager.painter.DrawImage(x - 50, y - 50, &spinner, frame);
        manager.painter.PopDrawingBoundsStackElement();
    };

    auto start = std::chrono::steady_clock::now();
    while (app->ShouldRun()) {
        app->Render();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        auto frame = (ms / 100) % spinner.GetNumberOfFrames();
        manager.painter.DrawRectangle(startX, startY, width, height, 0xFF00FF00);

        drawSquare(startX - 80,         startY - 80, frame);
        drawSquare(startX,              startY - 80, frame);
        drawSquare(startX + width / 2,  startY - 80, frame);
        drawSquare(startX + width,      startY - 80, frame);
        drawSquare(startX + width + 80, startY - 80, frame);

        drawSquare(startX - 80,         startY, frame);
        drawSquare(startX,              startY, frame);
        drawSquare(startX + width / 2,  startY, frame);
        drawSquare(startX + width,      startY, frame);
        drawSquare(startX + width + 80, startY, frame);

        drawSquare(startX - 80,         startY + height / 2, frame);
        drawSquare(startX,              startY + height / 2, frame);
        drawSquare(startX + width / 2,  startY + height / 2, frame);
        drawSquare(startX + width,      startY + height / 2, frame);
        drawSquare(startX + width + 80, startY + height / 2, frame);

        drawSquare(startX - 80,         startY + height, frame);
        drawSquare(startX,              startY + height, frame);
        drawSquare(startX + width / 2,  startY + height, frame);
        drawSquare(startX + width,      startY + height, frame);
        drawSquare(startX + width + 80, startY + height, frame);

        drawSquare(startX - 80,         startY + height + 80, frame);
        drawSquare(startX,              startY + height + 80, frame);
        drawSquare(startX + width / 2,  startY + height + 80, frame);
        drawSquare(startX + width,      startY + height + 80, frame);
        drawSquare(startX + width + 80, startY + height + 80, frame);

        app->Swap();
        app->Poll();
    }

    delete font;
    delete app;
}
