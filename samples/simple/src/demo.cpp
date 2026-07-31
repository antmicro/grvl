
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

    manager.SetLoaderCallback([] (const std::string& name) {

        std::string resource = name;

        // let's load wifi later to check if that will work
        if (name == "wifi") {
            return;
        }

        // ... and first load 'left' in place of 'right', to check if replacing works
        if (name == "left") {
            resource = "right";
        }

        std::string path = ROMFS_PATH "/images/" + resource + ".png";
        grvl::Manager::GetInstance().AddImageContentToContainer(name, new grvl::ImageContent(path.c_str(), grvl::Format::ARGB8888));
    });

    // images
    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    grvl::JSEngine::MakeJavaScriptFunctionCall("InitializeCalendar");

    const uint64_t runtime = 10'000'000; // us
    const uint64_t start = ChronoGetTimestamp();
    int frames = 0;

    // draw one frame here to make sure nothing breaks when drawing without all images being loaded
    app->Render();

    manager.AddImageContentToContainer("wifi", new grvl::ImageContent(ROMFS_PATH "/images/wifi.png", grvl::Format::ARGB8888)); // load image after it is alredy in use
    manager.AddImageContentToContainer("left", new grvl::ImageContent(ROMFS_PATH "/images/left.png", grvl::Format::ARGB8888)); // replace existing image

    while (app->ShouldRun()) {
        frames ++;

        app->Render();

        // draw directly over the framebuffer
        grvl::Font* font = manager.GetFontFromContainer("mona16");
        manager.painter.DisplayAntialiasedString(font, 2, 16, "Hello Overlay!", 0xffff0000);

        grvl::JSEngine::MakeJavaScriptFunctionCall("UpdateCurrentTime");
        grvl::JSEngine::MakeJavaScriptFunctionCall("UpdatePositionOfCurrentTimeLine");

        if ((ChronoGetTimestamp() - start) > runtime) {
            printf("Goodbye!\n");
            break;
        }

        app->Swap();
        app->Poll();
    }

    const uint64_t time = ChronoGetTimestamp() - start;
    printf("Exited after %ldms, drawn %d frames, (avg. %d FPS)\n", time / 1000, frames, (int) (frames / (time / 1000000.0)));
    delete app;
}
