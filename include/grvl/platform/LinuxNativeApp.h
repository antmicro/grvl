#ifndef LINUXNATIVEAPP_H_
#define LINUXNATIVEAPP_H_

#if defined(GRVL_LINUX_NATIVE_SUPPORT)

#include <unistd.h>
#include <grvl/platform/PosixApp.h>
#include <grvl/Queue.h>
#include <functional>

#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libinput.h>
#include <libudev.h>

#include <libdrm/drm_fourcc.h>

#include <xkbcommon/xkbcommon.h>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

class drm_screen;
class framebuffer_screen;

namespace grvl {

    struct NativeDisplayMode {
        std::string name;
        uint16_t width = 0;
        uint16_t height = 0;
        uint32_t refresh = 0;
        bool preferred = false;
    };

    struct NativeDisplay {
        std::string drm_path;
        uint32_t connector_id = 0;
        uint32_t connector_type = 0;
        uint32_t connector_type_id = 0;
        uint16_t width = 0;
        uint16_t height = 0;
        uint32_t refresh = 0;
        std::vector<NativeDisplayMode> modes;
        bool built_in = false;
    };

    // Linux "native" Application
    //
    // grvl configuration for "native" Linux applications running WITHOUT any display
    // server (X11 or Wayland). Such applications require root permissions and must be run in TTY.
    // Only available is GRVL_LINUX_NATIVE is enabled
    //
    // @remark
    // This class will terminate grvl and all application specific elements as soon as it is destroyed,
    // it is recomended to create the instance before the main render loop and only destroy it after the loop exits.
    class LinuxNativeApp : public PosixApp {
    private:
        NativeDisplay selected_display;
        bool has_selected_display = false;

        int fd = -1;
        int crtc_index = -1;

        drmModeResPtr resource = nullptr;
        drmModeConnectorPtr conn = nullptr;
        drmModeModeInfoPtr mode = nullptr;
        drmModeEncoderPtr encoder = nullptr;
        drmModeCrtcPtr crtc = nullptr;

        Queue<std::function<void()>> events;
        std::atomic<bool> thread_run;
        std::thread drm_thread, input_thread;

        struct {
            struct drm_mode_create_dumb dumb = {};
            uint32_t fb;
            uint32_t plane;
            struct {
                uint32_t fb{0}, crtc{0}, x{0}, y{0}, hotspot_x{0}, hotspot_y{0};
            } props;
            void* map = nullptr;
            uint32_t handles[4] = {};
            uint32_t pitches[4] = {};
            uint32_t offsets[4] = {};
        } primary, cursor;

        struct CursorState {
            std::atomic<int> x = 0;
            std::atomic<int> y = 0;
            std::atomic<bool> pending = false;
        } cursor_state;

        // to limit how often we draw we make sure the previous frame is shown before the next one starts rendering
        // this is done using this mutex, the Render() method wait for it, and the drm_thread signals it.
        std::mutex render_mutex;

        drmEventContext ev = {};
        struct libinput* li = nullptr;
        struct udev* ud = nullptr;
        uint32_t leds = 0;

        int x = 0;
        int y = 0;
        int abs_x = -1;
        int abs_y = -1;
        bool touch_down = false;
        bool left_mouse_pressed = false;
        bool draw_mouse_icon = true;

        struct xkb_context* xkb_ctx;
        struct xkb_keymap* xkb_keymap;
        struct xkb_state* xkb_state;

        static void PageFlipHandler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void* app);

        uint32_t GetPropertyId(uint32_t obj_id, uint32_t obj_type, const char* name);
        uint32_t GetPlaneType(uint32_t plane_id);
        uint32_t FindPlaneByType(uint32_t plane_type);

        void CloseDriver();
        bool InitDriver(int fd, uint16_t width, uint16_t height, uint32_t refresh, uint32_t connector_id = 0);
        bool TryUsingDriver(const char* path, uint16_t width, uint16_t height, uint32_t refresh, uint32_t connector_id = 0);

        void HandleKeycode(uint32_t xkb_keycode, uint32_t evdev_keycode, bool pressed);
        void HandleEvent(libinput_event* event);
        void HandleInput();
        void UpdateCursorPos();
        bool Setup() override;
        void CommitPlanes();
        void DRMWait();
    public:

        LinuxNativeApp(int width, int height, bool rotate_sideways = false);
        LinuxNativeApp(const NativeDisplay& display, bool rotate_sideways = false);
        ~LinuxNativeApp() override;

        static std::vector<NativeDisplay> EnumerateConnectedDisplays();

        void Render() override;
        void Swap() override;
        void Poll() override;

        // Decide whether a simple mouse icon should be drawn at the cursor position.
        //
        // @remark
        // Can be called both before and after this object is passed to Application::Init().
        void DrawMouseIcon(bool draw_mouse = true);

    };
}

#endif // defined(GRVL_LINUX_NATIVE_SUPPORT)
#endif // LINUXNATIVEAPP_H_
