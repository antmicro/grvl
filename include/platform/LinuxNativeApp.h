#ifndef LINUXNATIVEAPP_H_
#define LINUXNATIVEAPP_H_

#if defined(GRVL_LINUX_NATIVE_SUPPORT)
#include <xkbcommon/xkbcommon.h>
#include "platform/PosixApp.h"

class drm_screen;

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

        int x = 0;
        int y = 0;
        int* inputs = nullptr;
        int count = 0;
        bool left_mouse_pressed = false;
        bool draw_mouse_icon = true;

        uint32_t* bgra_buffer;

        void ClampCursor();
        void UpdateBgraBuffer();
        void DrawSimpleMouse();

        bool Setup() override;

        struct xkb_context* xkb_ctx;
        struct xkb_keymap* xkb_keymap;
        struct xkb_state* xkb_state;
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
