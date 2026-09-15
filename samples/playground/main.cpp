
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

#include <grvl/Manager.h>
#include <grvl/JSEngine.h>
#include <grvl/platform/LinuxGenericApp.h>

#define WIDTH 800
#define HEIGHT 600

void animate_progress_bar(grvl::ProgressBar* progressBar) 
{
    static auto start = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
    int progress = (int) (elapsed_seconds.count() * 20) % 100;	
    progressBar->SetProgressValue(progress);
}

int main()
{
    grvl::Application* app = grvl::CreateGenericLinuxApp(WIDTH, HEIGHT);
    grvl::Application::Init(app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::Manager& manager = grvl::Manager::GetInstance();

    auto ttf = std::make_shared<grvl::TrueTypeData>(ROMFS_PATH "/Roboto.ttf.gz");
    auto font = new grvl::TrueTypeFont(ttf, 16);

    manager.AddFontToFontContainer("normal", font);
    manager.AddFontToFontContainer("roboto-medium", font);

    manager.BuildFromXML(ROMFS_PATH "/example.xml");
    manager.AddImageContentToContainer("button_image", new grvl::ImageContent(ROMFS_PATH "/images/pngtest.png"));
    manager.InitializationFinished();
    manager.SetActiveScreen("start", 0);
    
    grvl::ProgressBar* progressBar = dynamic_cast<grvl::ProgressBar*>(manager.GetActiveScreen()->GetElement("progress_bar"));
    grvl::ProgressBar* circleProgressBar = dynamic_cast<grvl::CircleProgressBar*>(manager.GetActiveScreen()->GetElement("circle_progress_bar"));


    while (app->ShouldRun()) {
        app->Render();
        app->Swap();
        app->Poll();
        
        animate_progress_bar(progressBar);
        animate_progress_bar(circleProgressBar);

    }

    delete app;
}
