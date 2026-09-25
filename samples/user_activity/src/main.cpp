#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/component/Label.h>
#include <grvl/platform/LinuxGenericApp.h>

#define HEIGHT 600
#define WIDTH 800

std::atomic<bool> exit_requested { false };

uint64_t GetMonotonicTimestampMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

int main()
{
    grvl::PosixApp* app = grvl::CreateGenericLinuxApp(WIDTH, HEIGHT);
    grvl::Application::Init(app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::JSEngine::AddGlobalFunction("Exit", [] (duk_context*) -> duk_ret_t {
        exit_requested.store(true);
        return grvl::JSEngine::NO_RETURN_VALUE;
    }, 0);

    grvl::Manager& manager = grvl::Manager::GetInstance();
    auto ttf = std::make_shared<grvl::TrueTypeData>(SAMPLE_FONT_PATH);
    manager.SetFontCallback([ttf] (const std::string& name) {
        grvl::Manager::GetInstance().AddFontToFontContainer(name, new grvl::TrueTypeFont(ttf, 16));
    });

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    auto* screen = manager.GetScreen("home");
    if (!screen) {
        std::fprintf(stderr, "Failed to find sample home screen.\n");
        return 1;
    }

    auto* last_activity_label = dynamic_cast<grvl::Label*>(screen->GetElement("last_activity"));
    auto* idle_time_label = dynamic_cast<grvl::Label*>(screen->GetElement("idle_time"));
    if (!last_activity_label || !idle_time_label) {
        std::fprintf(stderr, "Failed to find sample status labels.\n");
        return 1;
    }

    uint64_t previous_activity = 0;
    auto last_ui_update = std::chrono::steady_clock::now();

    while (app->ShouldRun() && !exit_requested.load()) {
        app->Poll();

        const uint64_t activity = app->GetLastUserActivityTimestamp();
        const auto now = std::chrono::steady_clock::now();

        if (activity != previous_activity) {
            std::printf("User activity at monotonic timestamp %llu ms.\n",
                static_cast<unsigned long long>(activity));
            std::fflush(stdout);
            previous_activity = activity;
        }

        if (now - last_ui_update >= std::chrono::milliseconds(100)) {
            std::string activity_text;
            std::string idle_text;

            if (activity == 0) {
                activity_text = "Last activity: none";
                idle_text = "Idle: n/a";
            } else {
                const uint64_t now_ms = GetMonotonicTimestampMs();
                const uint64_t idle_ms = now_ms >= activity ? now_ms - activity : 0;

                activity_text = "Last activity: " + std::to_string(activity) + " ms";
                idle_text = "Idle: " + std::to_string(idle_ms) + " ms";
            }

            last_activity_label->SetText(activity_text.c_str());
            idle_time_label->SetText(idle_text.c_str());
            last_ui_update = now;
        }

        app->Render();
        app->Swap();
    }

    return 0;
}
