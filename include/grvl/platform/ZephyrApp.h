#ifndef ZEPHYRAPP_H_
#define ZEPHYRAPP_H_

#include <grvl/platform/Application.h>

#include <cstdint>

#include <zephyr/input/input.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/linker/devicetree_regions.h>

namespace grvl {

    // Zephyr Application
    //
    // grvl configuration for Zephyr applications. It draws to the given display device and uses DMA2D blitter for
    // performance. Support for touch input is included.
    //
    // @remark
    // This class will terminate grvl and all application specific elements as soon as it is destroyed,
    // it is recomended to create the instance before the main render loop and only destroy it after the loop exits.
    class ZephyrApp : public Application {

        struct {
            bool pressed;
            int32_t x;
            int32_t y;
        } mouse;

        int bpp;
        const device* display_device;
        display_buffer_descriptor display_descriptor;

        bool InitDisplay(const device* display);

        void SetCallbacks(gui_callbacks_t& callbacks) override;
        bool Setup() override;

    public:

        void HandleInput(input_event *evt);

        ZephyrApp(const device* display, bool rotate_sideways = false);
        ~ZephyrApp() override;

        void Render() override;
        void Swap() override;
        void Poll() override;

    };

}

#endif
