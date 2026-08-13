#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/platform/LinuxNativeApp.h>

volatile std::sig_atomic_t stop_requested = 0;
std::atomic<bool> ui_exit_requested{false};
std::atomic<int> ui_exit_status{0};

void HandleSignal(int)
{
    stop_requested = 1;
}

void PrintUsage(const char* program)
{
    std::printf(
        "Usage:\n"
        "  %s -l\n"
        "  %s -s <screen-index> [-t <seconds>]\n"
        "\n"
        "Options:\n"
        "  -l                  List connected DRM screens and exit.\n"
        "  -s <screen-index>   Run the GRVL DRM lease test on the screen shown by -l.\n"
        "  -t <seconds>        Exit after N seconds.\n"
        "                      Without -t, use the Exit button, Ctrl+C, or SIGTERM.\n"
        "  -h                  Show this help and exit.\n",
        program, program
    );
}

bool ParseUnsigned(const char* text, unsigned long& value)
{
    if (!text || text[0] == '\0' || text[0] == '-') {
        return false;
    }

    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0') {
        return false;
    }

    value = parsed;
    return true;
}

void PrintDisplays(const std::vector<grvl::NativeDisplay>& displays)
{
    if (displays.empty()) {
        std::printf("No connected DRM screens found.\n");
        return;
    }

    std::printf("Connected DRM screens:\n");
    for (size_t i = 0; i < displays.size(); ++i) {
        const auto& display = displays[i];
        std::printf(
            "  [%zu] %s connector=%u type=%u.%u %ux%u@%uHz %s\n",
            i,
            display.drm_path.c_str(),
            display.connector_id,
            display.connector_type,
            display.connector_type_id,
            display.width,
            display.height,
            display.refresh,
            display.built_in ? "built-in" : "external"
        );
    }
}

bool InstallSignalHandlers()
{
    struct sigaction action {};
    action.sa_handler = HandleSignal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, nullptr) != 0) {
        std::perror("sigaction(SIGINT)");
        return false;
    }

    if (sigaction(SIGTERM, &action, nullptr) != 0) {
        std::perror("sigaction(SIGTERM)");
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    bool list_only = false;
    bool screen_selected = false;
    bool timeout_set = false;
    size_t screen_index = 0;
    unsigned long timeout_seconds = 0;

    int option = 0;
    while ((option = getopt(argc, argv, "hls:t:")) != -1) {
        switch (option) {
        case 'h':
            PrintUsage(argv[0]);
            return 0;
        case 'l':
            list_only = true;
            break;
        case 's': {
            unsigned long parsed = 0;
            if (!ParseUnsigned(optarg, parsed) || parsed > std::numeric_limits<size_t>::max()) {
                std::fprintf(stderr, "Invalid screen index: '%s'.\n", optarg);
                return 2;
            }
            screen_index = static_cast<size_t>(parsed);
            screen_selected = true;
            break;
        }
        case 't':
            if (!ParseUnsigned(optarg, timeout_seconds)) {
                std::fprintf(stderr, "Invalid timeout: '%s'.\n", optarg);
                return 2;
            }
            timeout_set = true;
            break;
        default:
            PrintUsage(argv[0]);
            return 2;
        }
    }

    if (optind != argc) {
        std::fprintf(stderr, "Unexpected argument: '%s'.\n", argv[optind]);
        PrintUsage(argv[0]);
        return 2;
    }

    if (list_only && (screen_selected || timeout_set)) {
        std::fprintf(stderr, "-l cannot be combined with -s or -t.\n");
        return 2;
    }

    if (!list_only && !screen_selected) {
        PrintUsage(argv[0]);
        return 0;
    }

    if (timeout_set && !screen_selected) {
        std::fprintf(stderr, "-t requires -s.\n");
        return 2;
    }

    const auto displays = grvl::LinuxNativeApp::EnumerateConnectedDisplays();

    if (list_only) {
        PrintDisplays(displays);
        return displays.empty() ? 1 : 0;
    }

    if (screen_index >= displays.size()) {
        std::fprintf(stderr, "Screen index %zu is out of range.\n", screen_index);
        PrintDisplays(displays);
        return 2;
    }

    if (!InstallSignalHandlers()) {
        return 1;
    }

    grvl::NativeDisplay display = displays[screen_index];
    std::printf(
        "Starting GRVL DRM lease test on screen [%zu]: %s connector=%u (%ux%u@%uHz).\n",
        screen_index,
        display.drm_path.c_str(),
        display.connector_id,
        display.width,
        display.height,
        display.refresh
    );

    const char* x_display = std::getenv("DISPLAY");
    if (x_display) {
        std::printf("X11 DISPLAY=%s. If X owns DRM master, GRVL will request an XRandR lease.\n", x_display);
    } else {
        std::printf("DISPLAY is not set. The backend may use DRM master directly instead of the lease path.\n");
    }

    grvl::LinuxNativeApp app(display);
    grvl::Application::Init(&app);

    grvl::JSEngine::SetSourceCodeWorkingDirectory(ROMFS_PATH);
    grvl::Manager& manager = grvl::Manager::GetInstance();

    grvl::JSEngine::AddGlobalFunction("Exit", [] (duk_context* ctx) -> duk_ret_t {
        ui_exit_status.store(duk_to_int(ctx, 0));
        ui_exit_requested.store(true);
        return grvl::JSEngine::NO_RETURN_VALUE;
    }, 1);

    auto ttf = std::make_shared<grvl::TrueTypeData>(SAMPLE_FONT_PATH);
    manager.SetFontCallback([ttf] (const std::string& name) {
        grvl::Manager::GetInstance().AddFontToFontContainer(name, new grvl::TrueTypeFont(ttf, 16));
    });

    manager.BuildFromXML(ROMFS_PATH "/gui.xml");
    manager.InitializationFinished();
    manager.SetActiveScreen("home", 0);

    const auto started = std::chrono::steady_clock::now();

    while (app.ShouldRun() && !stop_requested && !ui_exit_requested.load()) {
        if (timeout_set && std::chrono::steady_clock::now() - started >= std::chrono::seconds(timeout_seconds)) {
            std::printf("Timeout expired after %lu seconds.\n", timeout_seconds);
            break;
        }

        app.Render();
        app.Swap();
        app.Poll();
    }

    if (stop_requested) {
        std::printf("Termination signal received.\n");
    } else if (ui_exit_requested.load()) {
        std::printf("Exit requested from the GRVL UI.\n");
    }

    std::printf("Shutting down GRVL and releasing the DRM device/lease.\n");
    return ui_exit_status.load();
}
