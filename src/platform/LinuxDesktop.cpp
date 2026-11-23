
#include <SDL2/SDL_blendmode.h>
#include <grvl/Manager.h>
#include <grvl/platform/KeyMappings.h>
#include <grvl/platform/LinuxDesktopApp.h>
#include <grvl/render/SoftwareRasterizer.h>

#include <SDL2/SDL.h>

namespace grvl {

    static SDL_Keycode TranslateKeypad(SDL_Keycode key, SDL_Keymod mods)
    {
        // SDL reports keypad *physical* keys, even with NumLock disabled.
        // As on a typical PC keyboard, Shift temporarily reverses NumLock.
        if((bool(mods & KMOD_NUM)) == (bool(mods & KMOD_SHIFT))) {
            switch(key) {
                case SDLK_KP_4:
                    return SDLK_LEFT;
                case SDLK_KP_6:
                    return SDLK_RIGHT;
                case SDLK_KP_8:
                    return SDLK_UP;
                case SDLK_KP_2:
                    return SDLK_DOWN;
                case SDLK_KP_7:
                    return SDLK_HOME;
                case SDLK_KP_1:
                    return SDLK_END;
                case SDLK_KP_9:
                    return SDLK_PAGEUP;
                case SDLK_KP_3:
                    return SDLK_PAGEDOWN;
                case SDLK_KP_0:
                    return SDLK_INSERT;
                case SDLK_KP_PERIOD:
                    return SDLK_DELETE;
                default:
                    break;
            }
        }
        return key;
    }

    static int GetEnvInt(const char* name, int fallback)
    {
        const char* value = getenv(name);
        if(value == nullptr)
            return fallback;
        return atoi(value);
    }

    LinuxDesktopApp::LinuxDesktopApp(int width, int height, bool rotate_sideways)
        : PosixApp(width, height, rotate_sideways)
    {
    }

    LinuxDesktopApp::~LinuxDesktopApp()
    {
        if(texture)
            SDL_DestroyTexture(texture);
        if(renderer)
            SDL_DestroyRenderer(renderer);

        if(context)
            SDL_GL_DeleteContext(context);
        if(window)
            SDL_DestroyWindow(window);

        SDL_Quit();
        grvl::grvl::Destroy();
    }

    bool LinuxDesktopApp::SetupLegacy()
    {
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
        renderer = SDL_GetRenderer(window);

        if(!renderer) {
            renderer = SDL_CreateRenderer(window, -1, 0);
        }

        if(!renderer) {
            Log(ERROR, "SDL_CreateRenderer() Failed: %s", SDL_GetError());
            return false;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

        auto rasterizer = new SoftwareRasterizer();
        rasterizer->SetRotation(sideways);
        rasterizer->SetDisplaySize(width, height);
        rasterizer->CreateFramebuffersCollection(Format::XRGB8888, nullptr, nullptr);
        rasterizer->InitFramebuffersCollection();

        Manager::Initialize(width, height, rasterizer);
        InstallLinuxDesktopKeyMappings(Manager::GetInstance());
        Manager::GetInstance().SetIdleThrottle(true);
        return true;
    }

    bool LinuxDesktopApp::Setup()
    {
        if(SDL_Init(SDL_INIT_VIDEO) != 0) {
            Log(ERROR, "SDL_Init() Failed: %s", SDL_GetError());
            return false;
        }

        return SetupLegacy();
    }

    void LinuxDesktopApp::Render()
    {
        Manager& manager = Manager::GetInstance();
        this->frame_drawn = manager.ShouldRedraw();

        manager.MainLoopIteration(frame_drawn);
    }

    void LinuxDesktopApp::Swap()
    {
        Stopwatch watch {};

        if(frame_drawn) {

            void* pixels = 0;
            int pitch;

            if(SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
                memcpy(pixels, framebuffer, height * width * 4);
                SDL_UnlockTexture(texture);
            }

            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        Manager::GetInstance().perf.swap_times.put(watch.stop());
    }

    void LinuxDesktopApp::Poll()
    {
        SDL_Event event;
        SDL_zero(event);

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_MOUSEWHEEL:
                    Manager::GetInstance().MarkUserActivity();
                    break;

                case SDL_TEXTINPUT:
                    Manager::GetInstance().ProcessTextInput(event.text.text);
                    break;

                case SDL_QUIT:
                    should_run = false;
                    break;

                case SDL_KEYDOWN: {
                    const int physical = static_cast<int>(event.key.keysym.scancode);
                    auto it = pressed_keycodes.find(physical);
                    const uint32_t code = it != pressed_keycodes.end()
                        ? it->second
                        : static_cast<uint32_t>(TranslateKeypad(event.key.keysym.sym,
                                                                static_cast<SDL_Keymod>(event.key.keysym.mod)));
                    pressed_keycodes[physical] = code;
                    Manager::GetInstance().ProcessKeyInput(true, code, static_cast<uint16_t>(code));
                    break;
                }

                case SDL_KEYUP: {
                    const int physical = static_cast<int>(event.key.keysym.scancode);
                    const auto it = pressed_keycodes.find(physical);
                    const uint32_t code = it != pressed_keycodes.end()
                        ? it->second
                        : static_cast<uint32_t>(TranslateKeypad(event.key.keysym.sym,
                                                                static_cast<SDL_Keymod>(event.key.keysym.mod)));
                    if(it != pressed_keycodes.end()) {
                        pressed_keycodes.erase(it);
                    }
                    Manager::GetInstance().ProcessKeyInput(false, code, static_cast<uint16_t>(code));
                    break;
                }
            }
        }

        int x, y;
        bool state = SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT);
        Manager::GetInstance().ProcessTouchPoint(state, std::min(x, width), std::min(y, height));
    }

    void LinuxDesktopApp::SetTitle(const char* title)
    {
        this->title = title;
    }

    bool LinuxDesktopApp::IsNumLockEnabled() const
    {
        return SDL_GetModState() & KMOD_NUM;
    }
}
