
#if defined(GRVL_LINUX_DESKTOP_SUPPORT)

#include <grvl/Manager.h>
#include <grvl/platform/LinuxDesktopApp.h>

#include <SDL2/SDL.h>

namespace grvl {

    LinuxDesktopApp::LinuxDesktopApp(int width, int height, bool rotate_sideways)
        : PosixApp(width, height, rotate_sideways)
    {
    }

    LinuxDesktopApp::~LinuxDesktopApp()
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();
        grvl::grvl::Destroy();
    }

    bool LinuxDesktopApp::Setup()
    {

        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("SDL_Init() Failed: %s\n", SDL_GetError());
            return false;
        }

        window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
        renderer = SDL_GetRenderer(window);

        if (!renderer) {
            renderer = SDL_CreateRenderer(window, -1, 0);
        }

        if (!renderer) {
            printf("SDL_CreateRenderer() Failed: %s\n", SDL_GetError());
            return false;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

        Manager::Initialize(width, height, 4, sideways);
        return true;
    }

    void LinuxDesktopApp::Render()
    {
        Manager::GetInstance().MainLoopIteration();

        void* pixels = 0;
        int pitch;

        if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
            memcpy(pixels, framebuffer, height * width * 4);
            SDL_UnlockTexture(texture);
        }

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void LinuxDesktopApp::Poll()
    {
        SDL_Event event;
        SDL_zero(event);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                should_run = false;
            }

            if (event.type == SDL_KEYDOWN) {
                Manager::GetInstance().ProcessKeyInput(true, event.key.keysym.sym);
            }

            if (event.type == SDL_KEYUP) {
                Manager::GetInstance().ProcessKeyInput(false, event.key.keysym.sym);
            }
        }

        int x, y;
        bool state = SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT);
        Manager::GetInstance().ProcessTouchPoint(state, min(x, width), min(y, height));

    }

    void LinuxDesktopApp::SetTitle(const char* title)
    {
        this->title = title;
    }

}

#endif
