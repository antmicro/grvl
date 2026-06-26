
#include <grvl/platform/Dma2d.h>
#include <grvl/platform/ZephyrApp.h>
#include <zephyr/logging/log.h>
#include <grvl/Manager.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

// TODO: rename to 'grvl' - can't do it now as it conflicts with the demo
LOG_MODULE_REGISTER(lib_grvl, CONFIG_APP_LOG_LEVEL);

static void ZephyrInputCallback(input_event *evt, void *user_data) {
    static_cast<grvl::ZephyrApp*>(grvl::Application::GetInstance())->HandleInput(evt);
}

INPUT_CALLBACK_DEFINE(nullptr, ZephyrInputCallback, nullptr);

static uint64_t GetTimestamp()
{
    return static_cast<uint64_t>(k_uptime_get());
}

namespace grvl {

    void ZephyrApp::HandleInput(input_event* evt)
    {
        switch (evt->code) {
        case INPUT_BTN_TOUCH:
            mouse.pressed = evt->value;
            break;

        case INPUT_ABS_X:
            mouse.x = evt->value;
            break;

        case INPUT_ABS_Y:
            mouse.y = evt->value;
            break;

        default:
            LOG_WRN_ONCE("Unknown input event %d", evt->code);
            break;
        }
    }

    bool ZephyrApp::InitDisplay(const device* dev)
    {
        struct display_capabilities caps;

        if (!device_is_ready(dev)) {
            LOG_ERR("Display %s initialization failed", dev->name);
            return false;
        }

        display_get_capabilities(dev, &caps);

        this->width = caps.x_resolution;
        this->height = caps.y_resolution;
        this->bpp = DISPLAY_BITS_PER_PIXEL(caps.current_pixel_format) / 8;

        if (width == 0 || height == 0 || bpp == 0) {
            return false;
        }

        display_descriptor = {};
        display_descriptor.buf_size = width * height * bpp;
        display_descriptor.pitch = width;
        display_descriptor.width = width;
        display_descriptor.height = height;

        LOG_INF("Using display %s, (width: %d, height: %d, bpp: %d, size: %d)", dev->name, width, height, bpp, display_descriptor.buf_size);

        display_blanking_off(dev);
        return true;
    }

    ZephyrApp::ZephyrApp(const device* display, bool sideways) : Application(0, 0, sideways)
    {
        display_device = display;
    }

    ZephyrApp::~ZephyrApp()
    {

    }

    void ZephyrApp::SetCallbacks(gui_callbacks_t& callbacks)
    {
        Dma2dSetCallbacks(callbacks);

        callbacks.set_layer_pointer = Application::SetFramebuffer;
        callbacks.get_timestamp = GetTimestamp;

        callbacks.gui_printf = [] (const char *text, va_list argList) {
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), text, argList);
            LOG_INF("%s", buffer);
        };
    }

    bool ZephyrApp::Setup()
    {
        if (InitDisplay(display_device) == false) {
            return false;
        }

        Dma2dInit();
        Manager::Initialize(width, height, bpp, sideways);
        return true;
    }

    void ZephyrApp::Render()
    {
        Manager::GetInstance().MainLoopIteration();
    }

    void ZephyrApp::Swap()
    {
       display_write(display_device, 0, 0, &display_descriptor, framebuffer);
    }

    void ZephyrApp::Poll()
    {
        Manager::GetInstance().ProcessTouchPoint(mouse.pressed, mouse.x, mouse.y);
    }

}
