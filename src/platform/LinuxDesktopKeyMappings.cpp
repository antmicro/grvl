#include <grvl/Manager.h>
#include <grvl/platform/KeyMappings.h>

#include <SDL2/SDL_keycode.h>

namespace grvl {
    void InstallLinuxDesktopKeyMappings(Manager& manager)
    {
#define GRVL_KEY(id, sdl, evdev) manager.AddDefaultKeyMappingToContainer(sdl, #id, 100);
#include "KeyboardMappings.inc"
#undef GRVL_KEY
    }
}
