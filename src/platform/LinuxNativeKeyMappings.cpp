#include <grvl/Manager.h>
#include <grvl/platform/KeyMappings.h>

#include <linux/input-event-codes.h>

namespace grvl {
    void InstallLinuxNativeKeyMappings(Manager& manager)
    {
#define GRVL_KEY(id, sdl, evdev) manager.AddDefaultKeyMappingToContainer(evdev, #id, 100);
#include "KeyboardMappings.inc"
#undef GRVL_KEY
    }
}
