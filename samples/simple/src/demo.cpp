
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

#include <grvl/Manager.h>
#include <grvl/JSEngine.h>
#include <grvl/platform/LinuxGenericApp.h>

#define HEIGHT 600
#define WIDTH 800

static uint64_t ChronoGetTimestamp()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

int main()
{
    grvl::Application* app = grvl::CreateGenericLinuxApp(WIDTH, HEIGHT);
    grvl::Application::Init(app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::Manager& manager = grvl::Manager::GetInstance();

    auto ttf = std::make_shared<grvl::TrueTypeData>(ROMFS_PATH "/fonts/Roboto.ttf.gz");

    // fonts
    manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont(ttf, 10));
    manager.AddFontToFontContainer("mona10", new grvl::TrueTypeFont(ttf, 12));
    manager.AddFontToFontContainer("mona12", new grvl::TrueTypeFont(ttf, 14));
    manager.AddFontToFontContainer("mona14", new grvl::TrueTypeFont(ttf, 16));
    manager.AddFontToFontContainer("mona16", new grvl::TrueTypeFont(ttf, 18));

    grvl::Format fmt = grvl::Format::ARGB8888;

    // images
    manager.AddImageContentToContainer("dots", new grvl::ImageContent(ROMFS_PATH "/images/dots.png", fmt));
    manager.AddImageContentToContainer("light_gray_left_vector", new grvl::ImageContent(ROMFS_PATH "/images/left.png", fmt));
    manager.AddImageContentToContainer("light_gray_right_vector", new grvl::ImageContent(ROMFS_PATH "/images/right.png", fmt));
    manager.AddImageContentToContainer("signal", new grvl::ImageContent(ROMFS_PATH "/images/signal.png", fmt));
    manager.AddImageContentToContainer("wifi", new grvl::ImageContent(ROMFS_PATH "/images/wifi.png", fmt));
    manager.AddImageContentToContainer("battery", new grvl::ImageContent(ROMFS_PATH "/images/battery.png", fmt));
    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    grvl::JSEngine::MakeJavaScriptFunctionCall("InitializeCalendar");

    const uint64_t start = ChronoGetTimestamp();
    int frames = 0;

    while (app->ShouldRun()) {
        frames ++;

        app->Render();

        // draw directly over the framebuffer
        grvl::Font* font = manager.GetFontFromContainer("mona16");
        manager.painter.DisplayAntialiasedString(font, 2, 16, "Hello Overlay!", 0xffff0000);

        grvl::JSEngine::MakeJavaScriptFunctionCall("UpdateCurrentTime");
        grvl::JSEngine::MakeJavaScriptFunctionCall("UpdatePositionOfCurrentTimeLine");

        app->Swap();
        app->Poll();
    }

    const uint64_t time = ChronoGetTimestamp() - start;
    printf("Exited after %ldms, drawn %d frames, (avg. %d FPS)\n", time / 1000, frames, (int) (frames / (time / 1000000.0)));
    delete app;
}
