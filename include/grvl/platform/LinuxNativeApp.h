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

        drm_screen* output = nullptr;

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
        drmEventContext ev = {};

        int x = 0;
        int y = 0;
        int* inputs = nullptr;
        int count = 0;
        bool left_mouse_pressed = false;
        bool draw_mouse_icon = true;

        uint32_t* bgra_buffer;

        void ClampCursor();
        void UpdateBgraBuffer();

        bool Setup() override;

        struct xkb_context* xkb_ctx;
        struct xkb_keymap* xkb_keymap;
        struct xkb_state* xkb_state;
    public:

        LinuxNativeApp(int width, int height, bool rotate_sideways = false);
        ~LinuxNativeApp() override;

        void Render() override;
        void RenderCursor() override;
        void Poll() override;
        void DRMWait() override;

        // Decide whether a simple mouse icon should be drawn at the cursor position.
        //
        // @remark
        // Can be called both before and after this object is passed to Application::Init().
        void DrawMouseIcon(bool draw_mouse = true);

    };
}

#endif // defined(GRVL_LINUX_NATIVE_SUPPORT)
#endif // LINUXNATIVEAPP_H_
