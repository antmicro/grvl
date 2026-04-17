#ifndef LINUXGENERICAPP_H_
#define LINUXGENERICAPP_H_

#include "../Application.h"

namespace grvl {
    // Create generic Linux Application
    //
    // Will return either LinuxDesktopApp or LinuxNativeApp instance
    // depending on the runtime environment, ensuring the application will work in both.
    Application* CreateGenericLinuxApp(int width, int height, bool rotate_sideways = false);
}

#endif // LINUXGENERICAPP_H_
