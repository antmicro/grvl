#ifndef LINUXGENERICAPP_H_
#define LINUXGENERICAPP_H_

#include <grvl/platform/PosixApp.h>

namespace grvl {
    // Create generic Linux Application
    //
    // Will return either LinuxDesktopApp or LinuxNativeApp instance
    // depending on the runtime environment, ensuring the application will work in both.
    PosixApp* CreateGenericLinuxApp(int width, int height, bool rotate_sideways = false);
}

#endif // LINUXGENERICAPP_H_
