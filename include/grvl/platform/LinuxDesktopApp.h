#ifndef LINUXDESKTOPAPP_H_
#define LINUXDESKTOPAPP_H_

#if defined(GRVL_LINUX_DESKTOP_SUPPORT)

#include <unordered_map>

#include <grvl/platform/PosixApp.h>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

namespace grvl {

    // Linux "Desktop" Application
    //
    // grvl configuration for "desktop" Linux applications running on either
    // X11 or Wayland desktop.
    // Only available is GRVL_LINUX_DESKTOP is enabled
    //
    // @remark
    // This class will terminate grvl and all application specific elements as soon as it is destroyed,
    // it is recomended to create the instance before the main render loop and only destroy it after the loop exits.
    class LinuxDesktopApp : public PosixApp {
    private:
        SDL_Window* window = nullptr;
        void* context = nullptr;

        // used only when OpenGL is NOT used
        SDL_Texture* texture = nullptr;
        SDL_Renderer* renderer = nullptr;

        const char* title = "Untitled";
        bool frame_drawn = true;

        // SDL scancode -> keycode originally sent to Manager on KEYDOWN.
        std::unordered_map<int, uint32_t> pressed_keycodes;

        bool SetupLegacy();

        bool Setup() override;

    public:
        LinuxDesktopApp(int width, int height, bool rotate_sideways = false);
        ~LinuxDesktopApp() override;

        void Render() override;
        void Swap() override;
        void Poll() override;

        // Set title for the window.
        //
        // @remark
        // Must be called before this object is passed to Application::Init().
        void SetTitle(const char* title);

        bool IsNumLockEnabled() const override;
    };

}

#endif // defined(GRVL_LINUX_DESKTOP_SUPPORT)
#endif // LINUXDESKTOPAPP_H_
