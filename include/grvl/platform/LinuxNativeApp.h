#ifndef LINUXNATIVEAPP_H_
#define LINUXNATIVEAPP_H_

#include <unistd.h>
#if defined(GRVL_LINUX_NATIVE_SUPPORT)
#include <grvl/platform/PosixApp.h>

#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <libdrm/drm_fourcc.h>

#include <xkbcommon/xkbcommon.h>
#include <unordered_set>
#include <atomic>

class drm_screen;
class framebuffer_screen;

namespace grvl {

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

        int fd = -1;
        int crtc_index = -1;

        drmModeResPtr resource = nullptr;
        drmModeConnectorPtr conn = nullptr;
        drmModeModeInfoPtr mode = nullptr;
        drmModeEncoderPtr encoder = nullptr;
        drmModeCrtcPtr crtc = nullptr;

        struct {
            struct drm_mode_create_dumb dumb = {};
            uint32_t fb;
            uint32_t plane;
            struct {
                uint32_t fb, crtc, x, y;
            } props;
            void *map = nullptr;
            uint32_t handles[4] = {};
            uint32_t pitches[4] = {};
            uint32_t offsets[4] = {};
        } primary, cursor;

        struct CursorState {
            std::atomic<int> x = 0;
            std::atomic<int> y = 0;
            std::atomic<bool> pending = false;
        } cursor_state;

        drmEventContext ev = {};

        int x = 0;
        int y = 0;
        int abs_x = -1;
        int abs_y = -1;
        int* inputs = nullptr;
        std::unordered_set<int> pointer_devices;
        int count = 0;
        bool touch_down = false;
        bool left_mouse_pressed = false;
        bool draw_mouse_icon = true;

        struct xkb_context* xkb_ctx;
        struct xkb_keymap* xkb_keymap;
        struct xkb_state* xkb_state;

        static void PageFlipHandler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void* app);

        uint32_t GetPropertyId(uint32_t obj_id, const char* name);
        uint32_t GetPlaneType(uint32_t plane_id);
        uint32_t FindPlaneByType(uint32_t plane_type);

        void CloseDriver();
        bool InitDriver(int fd, uint16_t width, uint16_t height, uint32_t refresh);
        bool TryUsingDriver(const char* path, uint16_t width, uint16_t height, uint32_t refresh);

        void LoadPointerDevices();
        void ClampCursor();
        bool Setup() override;
        void RenderCursor();
        void DRMWait();
    public:

        LinuxNativeApp(int width, int height, bool rotate_sideways = false);
        ~LinuxNativeApp() override;

        void Render() override;
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
