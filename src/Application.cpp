#include "Application.h"

namespace grvl {

    Application::Application(int width, int height, bool rotate_sideways) {
        this->width = width;
        this->height = height;
        this->sideways = rotate_sideways;
    }

    void Application::SetFramebuffer(uintptr_t address) {
        instance->framebuffer = reinterpret_cast<uint8_t*>(address);
    }

    void Application::Init(Application* app) {
        instance = app;

        if (app == nullptr) {
            exit(1);
        }

        gui_callbacks_t callbacks {};
        app->SetCallbacks(callbacks);
        grvl::Init(&callbacks);

        if (!app->Setup()) {
            exit(2);
        }
    }

    Application* Application::GetInstance() {
        return instance;
    }

    void Application::SetCallbacks(gui_callbacks_t& callbacks) {
        callbacks.set_layer_pointer = Application::SetFramebuffer;
    }

    bool Application::ShouldRun() const
    {
        return should_run;
    }

}
