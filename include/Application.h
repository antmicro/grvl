
#ifndef GRVL_APPLICATION_H_
#define GRVL_APPLICATION_H_

#include <grvl.h>
#include <xkbcommon/xkbcommon.h>

class drm_screen;
class InputSystem;
class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

namespace grvl {

    // higher level abstraction over grvl setup
    //
    // This class (and its sub-classes) can be used to more easly configure grvl
    // for specific usecases. Without the need to handle events and rendering yourself.
    //
    // @remark
    // This class will terminate grvl and all application specific elements as soon as it is destroyed,
    // it is recomended to create the instance before the main render loop and only destroy it after the loop exits.
    class Application {
    protected:

        int width = 100;
        int height = 100;
        bool sideways = false;
        bool should_run = true;

        static inline Application* instance;
        uint8_t* framebuffer;

        static void SetFramebuffer(uintptr_t address);

        virtual void SetCallbacks(gui_callbacks_t& callbacks);
        virtual bool Setup() = 0;

    public:

        static void Init(Application* app);
        static Application* GetInstance();

        Application(int width, int height, bool rotate_sideways);
        virtual ~Application() = default;

        // Draw the next frame.
        // This method should be called in the main render loop.
        virtual void Render() = 0;

        // Pool user inputs.
        // This method should be called in the main render loop.
        virtual void Poll() = 0;

        // Check if the application should exit the main loop and terminate
        // for example due to the user closing the application window.
        virtual bool ShouldRun() const;

    };

    class PosixApp : public Application {
    protected:

        PosixApp(int width, int height, bool rotate_sideways);

        void SetCallbacks(gui_callbacks_t& callbacks) override;

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

        SDL_Texture* texture = nullptr;
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        const char* title = "Untitled";

        bool Setup() override;

    public:

        LinuxDesktopApp(int width, int height, bool rotate_sideways = false);
        ~LinuxDesktopApp() override;

        void Render() override;
        void Poll() override;

        // Set title for the window.
        //
        // @remark
        // Must be called before this object is passed to Application::Init().
        void SetTitle(const char* title);

    };

    // Create generic Linux Application
    //
    // Will return either LinuxDesktopApp or LinuxNativeApp instance
    // depending on the runtime environment, ensuring the application will work in both.
    Application* CreateGenericLinuxApp(int width, int height, bool rotate_sideways = false);

}

#endif /* GRVL_APPLICATION_H_ */
