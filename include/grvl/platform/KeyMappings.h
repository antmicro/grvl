#ifndef GRVL_PLATFORM_KEYMAPPINGS_H_
#define GRVL_PLATFORM_KEYMAPPINGS_H_

namespace grvl {
    class Manager;

    // Install defaults before loading XML. XML mappings override entries by code.
#if defined(GRVL_LINUX_DESKTOP_SUPPORT)
    void InstallLinuxDesktopKeyMappings(Manager& manager);
#endif
#if defined(GRVL_LINUX_NATIVE_SUPPORT)
    void InstallLinuxNativeKeyMappings(Manager& manager);
#endif
}

#endif
