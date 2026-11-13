#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <memory>
#include <string>

#include <grvl/platform/LinuxGenericApp.h>

#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/Misc.h>
#include <grvl/grvl.h>

#include <grvl/component/Label.h>
#include <grvl/container/Division.h>
#include <grvl/container/ListView.h>
#include <grvl/container/ScrollPanel.h>

namespace fs = std::filesystem;

static constexpr uint32_t TARGET_FRAMERATE { 60 };
static constexpr uint32_t WIDTH { 640 };
static constexpr uint32_t HEIGHT { 480 };

static void load_fonts(fs::path& rpath, grvl::Manager& manager)
{
    Log(grvl::TRACE, "Loading fonts");

    auto inter_regular_data = std::make_shared<grvl::TrueTypeData>(
        (rpath / "fonts/inter-regular.ttf").string().c_str());
    auto inter_medium_data = std::make_shared<grvl::TrueTypeData>(
        (rpath / "fonts/inter-medium.ttf").string().c_str());
    auto geistmono_regular_data = std::make_shared<grvl::TrueTypeData>(
        (rpath / "fonts/geistmono-regular.ttf").string().c_str());
    auto inter_regular_12 = new grvl::TrueTypeFont(inter_regular_data, 12);
    auto inter_regular_14 = new grvl::TrueTypeFont(inter_regular_data, 14);
    auto inter_regular_16 = new grvl::TrueTypeFont(inter_regular_data, 16);
    auto inter_regular_20 = new grvl::TrueTypeFont(inter_regular_data, 20);
    auto inter_regular_24 = new grvl::TrueTypeFont(inter_regular_data, 24);
    auto inter_medium_20 = new grvl::TrueTypeFont(inter_medium_data, 20);
    auto geistmono_regular_12 = new grvl::TrueTypeFont(geistmono_regular_data, 12);

    manager.AddFontToFontContainer("normal", inter_regular_16);
    manager.AddFontToFontContainer("inter-regular-12", inter_regular_12);
    manager.AddFontToFontContainer("inter-regular-14", inter_regular_14);
    manager.AddFontToFontContainer("inter-regular-16", inter_regular_16);
    manager.AddFontToFontContainer("inter-regular-20", inter_regular_20);
    manager.AddFontToFontContainer("inter-regular-24", inter_regular_24);
    manager.AddFontToFontContainer("inter-medium-20", inter_medium_20);
    manager.AddFontToFontContainer("geistmono-regular-12", geistmono_regular_12);
}

static void load_images(fs::path& rpath, grvl::Manager& manager)
{
    auto load_image = [&](const char* name, const char* path, grvl::Format format) {
        manager.AddImageContentToContainer(name,
                                           new grvl::ImageContent((rpath / path).c_str(), format));
    };

    Log(grvl::INFO, "Loading images");

    load_image("cursor", "images/cursor.png", grvl::Format::AL88);
    load_image("next", "images/next.png", grvl::Format::AL88);
    load_image("avatar", "images/avatar.png", grvl::Format::AL88);
    load_image("shutdown", "images/shutdown.png", grvl::Format::AL88);
    load_image("reboot", "images/reboot.png", grvl::Format::AL88);
    load_image("background", "images/background.png", grvl::Format::L8);
    load_image("grvl", "images/grvl.png", grvl::Format::AL88);
    load_image("success", "images/success.png", grvl::Format::ARGB8888);
}

static void ShutdownClicked(void* sender, const grvl::Event::ArgVector& Args)
{
    exit(0);
}

static void RebootClicked(void* sender, const grvl::Event::ArgVector& Args)
{
    exit(0);
}

static void EnterClicked(void* sender, const grvl::Event::ArgVector& Args)
{
    auto& mgr { grvl::Manager::GetInstance() };
    auto& text_input = dynamic_cast<grvl::TextInput&>(
        *mgr.GetActiveScreen()->GetElement("answer"));

    text_input.Submit();
}

static void ToggleDebugOverlay(void* sender, const grvl::Event::ArgVector& Args)
{
    auto& mgr { grvl::Manager::GetInstance() };

    mgr.perf.overlay = mgr.perf.overlay == grvl::Performance::NONE
        ? grvl::Performance::FULL
        : grvl::Performance::NONE;
}

int main()
{
    std::unique_ptr<grvl::Application> app { grvl::CreateGenericLinuxApp(WIDTH, HEIGHT) };

    grvl::Application::Init(app.get());
    auto& mgr = grvl::Manager::GetInstance();

    fs::path romfs_path { ROMFS_PATH };

    grvl::JSEngine::SetSourceCodeWorkingDirectory(romfs_path.string());

    load_fonts(romfs_path, mgr);
    load_images(romfs_path, mgr);

    mgr.AddCallbackToContainer("ShutdownClicked", ShutdownClicked);
    mgr.AddCallbackToContainer("RebootClicked", RebootClicked);
    mgr.AddCallbackToContainer("EnterClicked", EnterClicked);
    mgr.AddCallbackToContainer("ToggleDebugOverlay", ToggleDebugOverlay);

    mgr.BuildFromXML((romfs_path / "gui.xml").string().c_str());
    mgr.InitializationFinished();
    mgr.SetActiveScreen("start", 0);
    mgr.SetIdleThrottle(false);

#ifdef GIT_HASH
    grvl::Label& version_label = dynamic_cast<grvl::Label&>(
        *mgr.GetActiveScreen()->GetElement("version"));
    version_label.SetText("GRVL " GIT_HASH);
#endif // GIT_HASH

    grvl::Component* cursor = mgr.GetActiveScreen()->GetElement("cursor");

    grvl::Log(grvl::INFO, "Entering the main loop.");

    while(app->ShouldRun()) {
        app->Poll();
        app->Render();
        app->Swap();
    }
}
