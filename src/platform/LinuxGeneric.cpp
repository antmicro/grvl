
#include <platform/LinuxNativeApp.h>
#include <platform/LinuxDesktopApp.h>
#include <platform/LinuxGenericApp.h>


#if defined(GRVL_LINUX_NATIVE_SUPPORT) || defined(GRVL_LINUX_DESKTOP_SUPPORT)

static bool IsEnvSet(const char* name) {
    char* val = getenv(name);

    if (val == NULL || strlen(val) == 0) {
        return false;
    }

    return true;
}

#endif

namespace grvl {

    Application* CreateGenericLinuxApp(int width, int height, bool rotate_sideways) {
#if defined(GRVL_LINUX_NATIVE_SUPPORT) || defined(GRVL_LINUX_DESKTOP_SUPPORT)
        if (IsEnvSet("DISPLAY") || IsEnvSet("WAYLAND_DISPLAY")) {
#if defined(GRVL_LINUX_DESKTOP_SUPPORT)
            return new LinuxDesktopApp(width, height, rotate_sideways);
#endif
        } else {
#if defined(GRVL_LINUX_NATIVE_SUPPORT)
            return new LinuxNativeApp(width, height, rotate_sideways);
#endif
        }

#endif
        return nullptr;
    }

}
