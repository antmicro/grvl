#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <sstream>

#include <grvl/Manager.h>
#include <grvl/JSEngine.h>
#include <grvl/platform/LinuxGenericApp.h>
#include <grvl/platform/LinuxNativeApp.h>

#define WIDTH 1920
#define HEIGHT 1080

std::string CreateDisplaysJavaScript(const std::vector<grvl::NativeDisplay>& displays)
{
    std::string source = "var connectedDisplays = [";

    for (size_t display_index = 0; display_index < displays.size(); ++display_index) {
        const auto& display = displays[display_index];
        if (display_index != 0) {
            source += ',';
        }

        source += "{drmPath:\"" + display.drm_path + "\","
                  "connectorId:" + std::to_string(display.connector_id) + ","
                  "connectorType:" + std::to_string(display.connector_type) + ","
                  "connectorTypeId:" + std::to_string(display.connector_type_id) + ","
                  "width:" + std::to_string(display.width) + ","
                  "height:" + std::to_string(display.height) + ","
                  "refresh:" + std::to_string(display.refresh) + ","
                  "builtIn:" + std::string(display.built_in ? "true" : "false") + ","
                  "modes:[";

        for (size_t mode_index = 0; mode_index < display.modes.size(); ++mode_index) {
            const auto& mode = display.modes[mode_index];
            if (mode_index != 0) {
                source += ',';
            }

            source += "{name:\"" + mode.name + "\","
                      "width:" + std::to_string(mode.width) + ","
                      "height:" + std::to_string(mode.height) + ","
                      "refresh:" + std::to_string(mode.refresh) + ","
                      "preferred:" + std::string(mode.preferred ? "true" : "false") + "}";
        }

        source += "]}";
    }

    source += "];";

    return source;
}

bool FindPreferredDisplay(grvl::NativeDisplay& display)
{
    const auto displays = grvl::LinuxNativeApp::EnumerateConnectedDisplays();
    if (displays.empty()) {
        return false;
    }

    const auto external = std::find_if(
        displays.begin(), displays.end(),
        [] (const grvl::NativeDisplay& candidate) {
            return !candidate.built_in;
        });

    display = external != displays.end() ? *external : displays.front();
    return true;
}

int main()
{
    const auto displays = grvl::LinuxNativeApp::EnumerateConnectedDisplays();
    if (displays.empty()) {
        std::fprintf(stderr, "No connected DRM displays found.\n");
        return 1;
    }

    grvl::NativeDisplay display;
    if (!FindPreferredDisplay(display)) {
        std::fprintf(stderr, "Unable to select a DRM display.\n");
        return 1;
    }

    std::printf("Using connector %u on %s (%ux%u@%uHz, %s).\n", display.connector_id,
                display.drm_path.c_str(), display.width, display.height, display.refresh,
                display.built_in ? "built-in fallback" : "external display");
    
    display.width = WIDTH;
    display.height = HEIGHT;
    display.refresh = -1;
    grvl::LinuxNativeApp app(display);
    grvl::Application::Init(&app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::Manager& manager = grvl::Manager::GetInstance();

    grvl::JSEngine::AddGlobalFunction("Exit", [] (duk_context* ctx) -> duk_ret_t {
        int status = duk_to_int(ctx, 0);
        exit(status);
        return grvl::JSEngine::NO_RETURN_VALUE;
    }, 1);
    
    const std::string displays_source = CreateDisplaysJavaScript(displays);
    if (!grvl::JSEngine::LoadJavaScriptCode(displays_source.c_str(), displays_source.size())) {
        std::fprintf(stderr, "Unable to pass connected display data to JavaScript.\n");
        return 1;
    }

    auto ttf = std::make_shared<grvl::TrueTypeData>(ROMFS_PATH "/fonts/Roboto.ttf.gz");
    manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont(ttf, 16));
    manager.AddFontToFontContainer("small", new grvl::TrueTypeFont(ttf, 14));

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");

    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);
    grvl::JSEngine::MakeJavaScriptFunctionCall("InitializeDisplayLists");

    while (app.ShouldRun()) {
        app.Render();
        app.Swap();
        app.Poll();
    }

    return 0;
}