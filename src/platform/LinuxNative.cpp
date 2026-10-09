#include <grvl/platform/LinuxNativeApp.h>
#include <grvl/Manager.h>
#include <grvl/platform/XrandrDrmLease.h>
#include <grvl/platform/FbtermDrmLease.h>

#include <algorithm>
#include <glob.h>
#include <regex>
#include <filesystem>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <pthread.h>
#include <sched.h>
#include <poll.h>
#include <chrono>

#include <poll.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <sys/mman.h>

namespace grvl {

    // Adawaita default cursor
    // CC-BY-SA
    // clang-format off
    static const uint32_t cursor_map[] = {
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x02000000, 0x45f0f0f0, 0x03000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x01000000, 0x09000000, 0xf7fdfdfd, 0x4bdddddd, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x02000000, 0x15000000, 0xffffffff, 0xf7f2f2f2, 0x4ed4d4d4, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1a000000, 0xffffffff, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff3c3c3c, 0xf7f1f1f1, 0x50d6d6d6, 0x04000000, 0x01000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xfdc3c3c3, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xf8fcfcfc, 0x4cdadada, 0x03000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff000000, 0xff414141, 0xff404040, 0xff000000, 0xff4d4d4d, 0xe8dfdfdf, 0x5f000000, 0x59000000, 0x54000000, 0x3f000000, 0x1a000000, 0x05000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff000000, 0xff414141, 0xfcf6f6f6, 0xf7c0c0c0, 0xff000000, 0xff010101, 0xf7dddddd, 0x65a4a4a4, 0x1e000000, 0x1a000000, 0x15000000, 0x09000000, 0x03000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xff3d3d3d, 0xf9efefef, 0x907c7c7c, 0xf4ebebeb, 0xff323232, 0xff000000, 0xfe626262, 0xcde1e1e1, 0x0e000000, 0x04000000, 0x02000000, 0x01000000, 0x00000000,
      0x00000000, 0x03000000, 0x1b000000, 0xffffffff, 0xf9f0f0f0, 0x827e7e7e, 0x43000000, 0xa5b2b2b2, 0xf9a9a9a9, 0xff000000, 0xff070707, 0xf8ebebeb, 0x3eb5b5b5, 0x03000000, 0x01000000, 0x00000000, 0x00000000,
      0x00000000, 0x03000000, 0x1a000000, 0xf9fafafa, 0x7f818181, 0x30000000, 0x1e000000, 0x46333333, 0xf8f2f2f2, 0xff1f1f1f, 0xff000000, 0xfd797979, 0xb7dedede, 0x09000000, 0x01000000, 0x00000000, 0x00000000,
      0x00000000, 0x02000000, 0x15000000, 0x6f939393, 0x2d000000, 0x11000000, 0x0a000000, 0x21000000, 0xb4c2c2c2, 0xfb909090, 0xff000000, 0xff101010, 0xf9f6f6f6, 0x19474747, 0x02000000, 0x00000000, 0x00000000,
      0x00000000, 0x01000000, 0x09000000, 0x19000000, 0x0d000000, 0x04000000, 0x03000000, 0x11000000, 0x52515151, 0xf9f2f2f2, 0xff1f1f1f, 0xff202020, 0xfaf5f5f5, 0x1e333333, 0x03000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x02000000, 0x05000000, 0x03000000, 0x01000000, 0x01000000, 0x07000000, 0x23000000, 0x99a5a5a5, 0xfaf5f5f5, 0xfaf5f5f5, 0x96a7a7a7, 0x17000000, 0x02000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02000000, 0x0f000000, 0x31000000, 0x530f0f0f, 0x52101010, 0x31000000, 0x0e000000, 0x01000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01000000, 0x04000000, 0x0e000000, 0x17000000, 0x17000000, 0x0e000000, 0x04000000, 0x00000000, 0x00000000, 0x00000000,
    };
    static const int cursor_map_width = 17;
    static const int cursor_map_height = 24;
    // pointer hostpot offset within the cursor_map
    static const int cursor_hotspot_x = 3;
    static const int cursor_hotspot_y = 3;
    // clang-format on

    static bool IsBuiltInConnector(uint32_t connector_type)
    {
        return connector_type == DRM_MODE_CONNECTOR_eDP ||
               connector_type == DRM_MODE_CONNECTOR_LVDS ||
               connector_type == DRM_MODE_CONNECTOR_DSI;
    }

    static bool IsUsableConnector(drmModeConnectorPtr connector)
    {
        return connector != nullptr &&
               connector->connection == DRM_MODE_CONNECTED &&
               connector->count_modes > 0;
    }

    static drmModeConnectorPtr PickConnector(int fd, drmModeResPtr resource, int requested_connector_id = -1)
    {
        std::vector<drmModeConnectorPtr> connectors;
        int selected = -1;

        for (int i = 0; i < resource->count_connectors; i++) {
            const auto connector = drmModeGetConnector(fd, resource->connectors[i]);
            connectors.push_back(connector);

            if (!IsUsableConnector(connector)) {
                continue;
            }

            if ((requested_connector_id >= 0) && (connector->connector_id != requested_connector_id)) {
                continue;
            }

            selected = connectors.size() - 1;
            break;
        }

        for (size_t i = 0; i < connectors.size(); i++) {
            if (i != selected) {
                drmModeFreeConnector(connectors[i]);
            }
        }

        if (selected == -1) {
            Log(ERROR, "No valid DRM connectors found!");
            return nullptr;
        }

        return connectors[selected];
    }

    static drmModeModeInfoPtr PickMode(drmModeConnectorPtr connector, const uint16_t width, const uint16_t height, const uint32_t refresh)
    {
        size_t pixels = 0;
        drmModeModeInfoPtr preferred = nullptr, highest = nullptr;

        for (int i = 0; i < connector->count_modes; i++) {
            const auto mode = &connector->modes[i];
            if (width == mode->hdisplay && height == mode->vdisplay) {
                if (refresh == -1 || refresh == mode->vrefresh) {
                    return mode;
                }
            }

            if (mode->type & DRM_MODE_TYPE_PREFERRED) {
                preferred = mode;
            }

            const size_t size = mode->vdisplay * mode->hdisplay;

            // pick the mode with the highest resolution
            if (size > pixels) {
                pixels = size;
                highest = mode;
            }
        }

        if (preferred) {
            return preferred;
        }

        if (highest) {
            return highest;
        }

        return nullptr;
    }

    static std::vector<std::string> GetDrmCardPaths()
    {
        std::vector<std::string> paths;

        const char* override_path = std::getenv("DRM_PATH");
        if (override_path && override_path[0] != '\0') {
            paths.emplace_back(override_path);
        }

        glob_t result = {};
        if (glob("/dev/dri/card*", 0, nullptr, &result) == 0) {
            for (size_t i = 0; i < result.gl_pathc; ++i) {
                paths.emplace_back(result.gl_pathv[i]);
            }
        }
        globfree(&result);

        std::sort(paths.begin(), paths.end());
        paths.erase(std::unique(paths.begin(), paths.end()), paths.end());
        return paths;
    }

    const static struct libinput_interface interface = {
        .open_restricted = [] (const char* path, int flags, void* user) {
            const int fd = open(path, flags);
            if (fd < 0) {
                Log(WARN, "Can't open '%s': %s", path, strerror(errno));
                return -errno;
            }

            /* Never take synthetic devices
               TODO: make it a configurable flag */
            struct input_id id;
            if (ioctl(fd, EVIOCGID, &id) == 0 && id.bustype == BUS_VIRTUAL) {
                Log(INFO, "Ignoring virtual device '%s'", path);
                close(fd);
                return -EACCES;
            }

            constexpr size_t BITS_PER_LONG = sizeof(unsigned long) * 8;
            constexpr size_t KEYMAP_SIZE = KEY_MAX / BITS_PER_LONG + 1;
            unsigned long keys[KEYMAP_SIZE] = {0};

            if(ioctl(fd, EVIOCGKEY(sizeof(keys)), keys) < 0) {
                Log(WARN, "Call to ioctl EVIOCGKEY failed for file descriptor %d: %s", fd, strerror(errno));
            }

            // Release all pressed keys before grabbing the device, to avoid stuck keys
            for(int code = 0; code < KEY_CNT; ++code) {
                if(!((keys[code / BITS_PER_LONG] >> (code % BITS_PER_LONG)) & 1UL)) {
                    continue;
                }

                struct input_event ev = {.type = EV_KEY, .code = (uint16_t)code, .value = 0};
                if (write(fd, &ev, sizeof(ev)) < 0) {
                    Log(WARN, "Failed to write key release event for code %d: %s", code, strerror(errno));
                }
            }

            struct input_event syn = {.type = EV_SYN, .code = SYN_REPORT, .value = 0};
            if(write(fd, &syn, sizeof(syn)) < 0) {
                Log(WARN, "Failed to write SYN_REPORT event: %s", strerror(errno));
            }

            if (ioctl(fd, EVIOCGRAB, 1) < 0) {
                Log(WARN, "Call to ioctl EVIOCGRAB failed for file descriptor %d: %s", fd, strerror(errno));
            }

            return fd;
        },
        .close_restricted = [] (int fd, void* user) {
            ioctl(fd, EVIOCGRAB, 0);
            close(fd);
        }
    };

// implementation

    static NativeDisplayMode MakeNativeDisplayMode(const drmModeModeInfo& mode)
    {
        NativeDisplayMode display_mode;
        display_mode.name = std::string(mode.name, std::min(strlen(mode.name), static_cast<size_t>(DRM_DISPLAY_MODE_LEN)));
        display_mode.width = mode.hdisplay;
        display_mode.height = mode.vdisplay;
        display_mode.refresh = mode.vrefresh;
        display_mode.preferred = (mode.type & DRM_MODE_TYPE_PREFERRED) != 0;
        return display_mode;
    }


    std::vector<NativeDisplay> LinuxNativeApp::EnumerateConnectedDisplays()
    {
        std::vector<NativeDisplay> displays;

        for (const auto& path : GetDrmCardPaths()) {
            const int display_fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
            if (display_fd < 0) {
                continue;
            }

            drmModeResPtr resources = drmModeGetResources(display_fd);
            if (!resources) {
                close(display_fd);
                continue;
            }

            for (int i = 0; i < resources->count_connectors; ++i) {
                drmModeConnectorPtr connector = drmModeGetConnector(display_fd, resources->connectors[i]);

                if (!IsUsableConnector(connector)) {
                    drmModeFreeConnector(connector);
                    continue;
                }

                const drmModeModeInfoPtr selected_mode = PickMode(connector, 0, 0, uint32_t(-1));

                NativeDisplay display;
                display.drm_path = path;
                display.connector_id = connector->connector_id;
                display.connector_type = connector->connector_type;
                display.connector_type_id = connector->connector_type_id;
                display.built_in = IsBuiltInConnector(connector->connector_type);

                for (int mode_index = 0; mode_index < connector->count_modes; ++mode_index) {
                    display.modes.push_back(MakeNativeDisplayMode(connector->modes[mode_index]));
                }

                if (selected_mode != nullptr) {
                    display.width = selected_mode->hdisplay;
                    display.height = selected_mode->vdisplay;
                    display.refresh = selected_mode->vrefresh;
                    displays.push_back(std::move(display));
                }

                drmModeFreeConnector(connector);
            }

            drmModeFreeResources(resources);
            close(display_fd);
        }

        return displays;
    }

    LinuxNativeApp::LinuxNativeApp(int width, int height, bool rotate_sideways)
        : PosixApp(width, height, rotate_sideways)
    {
        xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

        xkb_keymap = xkb_keymap_new_from_names(xkb_ctx, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (!xkb_keymap) {
            // @TODO: make configurable
            struct xkb_rule_names names = {
                .rules = NULL,
                .model = "pc105",
                .layout = "us",
                .variant = "",
                .options = NULL
            };
            xkb_keymap = xkb_keymap_new_from_names(xkb_ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
        }
        xkb_state = xkb_state_new(xkb_keymap);

    }

    LinuxNativeApp::LinuxNativeApp(const NativeDisplay& display, bool rotate_sideways)
        : LinuxNativeApp::LinuxNativeApp(display.width, display.height, rotate_sideways)
    {
        selected_display = display;
        has_selected_display = true;
    }

    LinuxNativeApp::~LinuxNativeApp()
    {
        thread_run = false;
        frame_signal.Post();
        first_frame_signal.Post();

        input_thread.join();

        libinput_unref(li);
        udev_unref(ud);

        xkb_state_unref(xkb_state);
        xkb_keymap_unref(xkb_keymap);
        xkb_context_unref(xkb_ctx);

        drm_thread.join();
        CloseDriver();

        grvl::grvl::Destroy();
    }

    int LinuxNativeApp::AcquireDrmLease(int fd)
    {
      Log(INFO, "DRM lease: trying XRandR lease for fd=%d", fd);
      int lease_fd = AcquireXrandrLease(fd);
      if (lease_fd >= 0) {
        Log(INFO, "DRM lease: acquired XRandR lease fd=%d", lease_fd);
        drm_access_type = DrmAccessType::XrandrLease;
        return lease_fd;
      }

      lease_fd = AcquireFbtermLease();
      if (lease_fd >= 0) {
        drm_access_type = DrmAccessType::FbtermLease;
      }

      return lease_fd;
    }


    uint32_t LinuxNativeApp::GetPropertyId(uint32_t obj_id, uint32_t obj_type, const char* name)
    {
        uint32_t prop_id = 0;

        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, obj_id, obj_type);
        if (!props) return 0;

        for (uint32_t i = 0; i < props->count_props; i++) {
            drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[i]);
            if (!prop) continue;

            if (std::strcmp(prop->name, name) == 0) {
                prop_id = prop->prop_id;
                drmModeFreeProperty(prop);
                break;
            }
            drmModeFreeProperty(prop);
        }

        drmModeFreeObjectProperties(props);
        return prop_id;
    }

    uint64_t LinuxNativeApp::GetPropertyValue(uint32_t obj_id, uint32_t obj_type, const char* name)
    {
        uint64_t prop_value = 0;

        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, obj_id, obj_type);
        if (!props) return 0;

        for (uint32_t i = 0; i < props->count_props; i++) {
            drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[i]);
            if (!prop) continue;

            if (std::strcmp(prop->name, name) == 0) {
                prop_value = props->prop_values[i];
                drmModeFreeProperty(prop);
                break;
            }
            drmModeFreeProperty(prop);
        }

        drmModeFreeObjectProperties(props);
        return prop_value;
    }

    uint32_t LinuxNativeApp::GetPlaneType(uint32_t plane_id)
    {
        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);
        if (!props) {
            Log(ERROR, "Failed to get properties for DRM plane %u!", plane_id);
            return uint32_t(-1);
        }

        uint32_t type_prop_id = GetPropertyId(plane_id, DRM_MODE_OBJECT_PLANE, "type");

        for (uint32_t i = 0; i < props->count_props; i++) {
            if (props->props[i] == type_prop_id) {
                uint32_t type_val = (uint32_t)props->prop_values[i];
                drmModeFreeObjectProperties(props);
                return type_val;
            }
        }

        drmModeFreeObjectProperties(props);
        return uint32_t(-1);
    }

    uint32_t LinuxNativeApp::FindPlaneByType(uint32_t plane_type)
    {
        drmModePlaneResPtr plane_res = drmModeGetPlaneResources(fd);
        if (!plane_res) return 0;

        uint32_t found_id = 0;
        bool selected = false;

        for (uint32_t i = 0; i < plane_res->count_planes; i++) {
            uint32_t plane_id = plane_res->planes[i];

            if (GetPlaneType(plane_id) != plane_type) {
                continue;
            }

            drmModePlanePtr plane = drmModeGetPlane(fd, plane_id);
            if (!plane) {
              Log(ERROR, "Failed to get DRM plane %u!", plane_id);
              continue;
            }

            uint32_t crtcs = plane->possible_crtcs;
            drmModeFreePlane(plane);

            // Only select planes compatible with our CRTC
            if (!(crtcs & (1 << crtc_index))) {
                continue;
            }

            found_id = plane_id;
            selected = true;
            break;
        }

        drmModeFreePlaneResources(plane_res);

        if (!selected) {
            Log(ERROR, "Failed to select DRM plane matching type %x!", plane_type);
        }

        return found_id;
    }

    void LinuxNativeApp::CloseDriver()
    {
        if (fd < 0) {
            return;
        }

        DestroyDumbBuffer(cursor);
        DestroyDumbBuffer(primary);

        if (resource) {
            drmModeFreeResources(resource);
            resource = nullptr;
        }

        if (conn) {
            drmModeFreeConnector(conn);
            conn = nullptr;
        }

        if (encoder) {
            drmModeFreeEncoder(encoder);
            encoder = nullptr;
        }

        if (crtc) {
            drmModeFreeCrtc(crtc);
            crtc = nullptr;
        }

        mode = nullptr;
        crtc_index = -1;

        if (fd >= 0) {
            close(fd);
            fd = -1;
        }

        if (drm_access_type == DrmAccessType::FbtermLease) {
            if (!ReleaseFbtermLease()) {
                Log(ERROR, "Failed to release fbterm lease");
            }
        }

        drm_access_type = DrmAccessType::None;
    }

    bool LinuxNativeApp::InitDriver(int driver, uint16_t width, uint16_t height, uint32_t refresh, int requested_connector_id)
    {
        this->fd = driver;

        this->resource = drmModeGetResources(fd);
        if (!resource) {
            Log(ERROR, "Unable to get DRM resources!");
            CloseDriver();
            return false;
        }

        Log(INFO, "DRM init: initial fd=%d resources connectors=%d CRTCs=%d encoders=%d",
            fd, resource->count_connectors, resource->count_crtcs, resource->count_encoders);

        Log(INFO, "DRM init: attempting drmSetMaster(fd=%d)", fd);
        if (drmSetMaster(fd) == 0) {
            Log(INFO, "DRM init: DRM master acquired on fd=%d", fd);
            drm_access_type = DrmAccessType::Master;
        } else {
            Log(WARN, "Unable to aquire DRM master control on fd=%d: %s", fd, strerror(errno));
            const int lease_fd = AcquireDrmLease(fd);

            drmModeFreeResources(resource);
            resource = nullptr;

            close(fd);
            this->fd = lease_fd;

            if (this->fd < 0) {
                return false;
            }

            this->resource = drmModeGetResources(fd);
            if (!resource) {
                Log(ERROR, "Unable to get DRM resources!");
                CloseDriver();
                return false;
            }
            Log(INFO, "DRM init: lease fd=%d resources connectors=%d CRTCs=%d encoders=%d",
                fd, resource->count_connectors, resource->count_crtcs, resource->count_encoders);
        }

        // Pick the display only after DRM access has been established. The exact same
        // connector selection policy is therefore used for DRM master and XRandR lease.
        Log(INFO, "DRM selection: requested_connector=%d access_type=%d candidates=%d",
            requested_connector_id, static_cast<int>(drm_access_type), resource->count_connectors);
        for(int i = 0; i < resource->count_connectors; i++) {
            drmModeConnectorPtr candidate = drmModeGetConnector(fd, resource->connectors[i]);
            if(!candidate) {
                Log(WARN, "DRM selection: connector[%d] id=%u query failed", i, resource->connectors[i]);
                continue;
            }
            Log(INFO, "DRM selection: connector[%d] id=%u type=%u type_id=%u connection=%u modes=%d encoder=%u",
                i, candidate->connector_id, candidate->connector_type, candidate->connector_type_id,
                candidate->connection, candidate->count_modes, candidate->encoder_id);
            drmModeFreeConnector(candidate);
        }
        this->conn = PickConnector(fd, resource, requested_connector_id);
        if (!conn) {
            Log(ERROR, "Unable to pick DRM connection!");
            CloseDriver();
            return false;
        }

        Log(INFO, "DRM selection: SELECTED connector=%u type=%u type_id=%u encoder=%u modes=%d",
            conn->connector_id, conn->connector_type, conn->connector_type_id, conn->encoder_id, conn->count_modes);

        this->mode = PickMode(conn, width, height, refresh);
        if (!mode) {
            Log(ERROR, "Unable to pick DRM mode!");
            CloseDriver();
            return false;
        }

        if (drm_access_type == DrmAccessType::XrandrLease) {
            // Resolve the selected connector's CRTC through its encoder, just like
            // the DRM master path does. Connector CRTC_ID may read as 0 on a lease FD.
            this->encoder = drmModeGetEncoder(fd, conn->encoder_id);
            if (!encoder) {
                Log(ERROR, "DRM selection: unable to get encoder=%u for leased connector=%u",
                    conn->encoder_id, conn->connector_id);
            } else {
                const uint32_t selected_crtc_id = encoder->crtc_id;
                Log(INFO, "DRM selection: leased connector=%u encoder=%u current_CRTC=%u",
                    conn->connector_id, encoder->encoder_id, selected_crtc_id);

                for (int i = 0; i < resource->count_crtcs; i++) {
                    Log(INFO, "DRM selection: lease CRTC[%d]=%u%s", i, resource->crtcs[i],
                        resource->crtcs[i] == selected_crtc_id ? " [selected]" : "");
                    if (resource->crtcs[i] == selected_crtc_id) {
                        crtc_index = i;
                        break;
                    }
                }

                if (crtc_index >= 0) {
                    this->crtc = drmModeGetCrtc(fd, resource->crtcs[crtc_index]);
                    Log(INFO, "DRM selection: selected leased CRTC index=%d id=%u", crtc_index, resource->crtcs[crtc_index]);
                } else {
                    Log(ERROR, "DRM selection: encoder CRTC=%u not present in lease", selected_crtc_id);
                }
            }
        } else if (drm_access_type == DrmAccessType::FbtermLease) {
            if (resource->count_crtcs != 1) {
                Log(ERROR, "Expected exactly one CRTC in DRM lease, found %d!", resource->count_crtcs);
                CloseDriver();
                return false;
            }

            crtc_index = 0;
            this->crtc = drmModeGetCrtc(fd, resource->crtcs[crtc_index]);
        } else {
            this->encoder = drmModeGetEncoder(fd, conn->encoder_id);
            if (!encoder) {
                Log(ERROR, "Unable to get DRM encoder!");
                CloseDriver();
                return false;
            }

            // Find index of the CRTC we are using
            for (int i = 0; i < resource->count_crtcs; i++) {
                if (resource->crtcs[i] == encoder->crtc_id) {
                    crtc_index = i;
                    break;
                }
            }

            if (crtc_index >= 0) {
                this->crtc = drmModeGetCrtc(fd, resource->crtcs[crtc_index]);
            }
        }

        if (!crtc || crtc_index < 0) {
            Log(ERROR, "Unable to get DRM CRTC!");
            CloseDriver();
            return false;
        }

        return true;
    }

    bool LinuxNativeApp::TryUsingDriver(const char* path, uint16_t width, uint16_t height, uint32_t refresh, int requested_connector_id)
    {
        if (path != nullptr && (strlen(path) != 0)) {
            int fh = open(path, O_RDWR);

            if (fh < 0) {
                Log(ERROR, "Failed to open device '%s'!", path);
                return false;
            }

            if (!InitDriver(fh, width, height, refresh, requested_connector_id)) {
                Log(ERROR, "Failed to initialize DRM using device '%s'!", path);
                return false;
            }

            return true;
        }

        return false;
    }

    void LinuxNativeApp::UpdateCursorPos()
    {
        if (x < 0) x = 0;
        if (x >= width) x = width - 1;
        if (y < 0) y = 0;
        if (y >= height) y = height - 1;

        cursor_state.x = x;
        cursor_state.y = y;
    }

    void LinuxNativeApp::PageFlipHandler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void* app)
    {
        reinterpret_cast<LinuxNativeApp*>(app)->cursor_state.pending = false;
    }

    void LinuxNativeApp::CommitPlanes()
    {
        if (cursor_state.pending){
            return;
        }

        int x = cursor_state.x.load();
        int y = cursor_state.y.load();

        drmModeAtomicReq *req = drmModeAtomicAlloc();

        if (cursor.plane) {
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.fb, cursor.fb);
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.crtc, crtc->crtc_id);
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.x, x);
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.y, y);

            if (cursor.props.hotspot_x) {
                drmModeAtomicAddProperty(req, cursor.plane, cursor.props.hotspot_x, cursor_hotspot_x);
            }

            if (cursor.props.hotspot_y) {
                drmModeAtomicAddProperty(req, cursor.plane, cursor.props.hotspot_y, cursor_hotspot_y);
            }
        }

        drmModeAtomicAddProperty(req, primary.plane, primary.props.fb, primary.fb);

        uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT;
        int rc = drmModeAtomicCommit(fd, req, flags, this);

        drmModeAtomicFree(req);

        if (rc == 0) {
             cursor_state.pending = true;
             return;
        }
    }

    void LinuxNativeApp::DRMWait()
    {
        pollfd pfd = {};
        pfd.fd = fd;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, drm_event_timeout);

        if (ret > 0 && (pfd.revents & POLLIN)) {
            drmHandleEvent(fd, &ev);
        }
    }

    bool LinuxNativeApp::CreateDumbBuffer(DumbBuffer &buffer,
                                       uint32_t buffer_width,
                                       uint32_t buffer_height,
                                       uint32_t bpp)
    {
        buffer = {};
        buffer.dumb.width = buffer_width;
        buffer.dumb.height = buffer_height;
        buffer.dumb.bpp = bpp;

        if (drmIoctl(fd,
                    DRM_IOCTL_MODE_CREATE_DUMB,
                    &buffer.dumb) != 0) {
            Log(ERROR,
                    "Failed to create %ux%u dumb buffer: %s",
                    buffer_width,
                    buffer_height,
                    strerror(errno));
            return false;
        }

        buffer.handles[0] = buffer.dumb.handle;
        buffer.pitches[0] = buffer.dumb.pitch;
        buffer.offsets[0] = 0;

        drm_mode_map_dumb map_request = {};
        map_request.handle = buffer.dumb.handle;

        if (drmIoctl(fd,
                    DRM_IOCTL_MODE_MAP_DUMB,
                    &map_request) != 0) {
            Log(ERROR,
                    "Failed to map %ux%u dumb buffer: %s",
                    buffer_width,
                    buffer_height,
                    strerror(errno));
            return false;
        }

        buffer.map = mmap(nullptr,
                buffer.dumb.size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd,
                map_request.offset);

        if (buffer.map == MAP_FAILED) {
            buffer.map = nullptr;

            Log(ERROR,
                    "Failed to mmap %ux%u dumb buffer: %s",
                    buffer_width,
                    buffer_height,
                    strerror(errno));
            return false;
        }

        return true;
    }

    bool LinuxNativeApp::AddFramebuffer(DumbBuffer &buffer,
                                        uint32_t buffer_width,
                                        uint32_t buffer_height,
                                        uint32_t format)
    {
        if (drmModeAddFB2(fd,
                    buffer_width,
                    buffer_height,
                    format,
                    buffer.handles,
                    buffer.pitches,
                    buffer.offsets,
                    &buffer.fb,
                    0) != 0) {
            Log(ERROR,
                    "Failed to create framebuffer for %ux%u dumb buffer: %s",
                    buffer_width,
                    buffer_height,
                    strerror(errno));
            return false;
        }

        return true;
    }

    void LinuxNativeApp::DestroyDumbBuffer(DumbBuffer &buffer)
    {
        Log(ERROR, "Destroying dumb buffer %u\n", buffer.fb);
        if (buffer.fb != 0) {
            if (drmModeRmFB(fd, buffer.fb) != 0) {
                Log(ERROR,
                        "drmModeRmFB(%u) failed: %s",
                        buffer.fb,
                        strerror(errno));
            }

            buffer.fb = 0;
        }

        if (buffer.map != nullptr && buffer.map != MAP_FAILED) {
            munmap(buffer.map, buffer.dumb.size);
            buffer.map = nullptr;
        }

        if (buffer.dumb.handle != 0) {
            drm_mode_destroy_dumb destroy = {};
            destroy.handle = buffer.dumb.handle;

            if (drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy) != 0) {
                Log(ERROR,
                        "DRM_IOCTL_MODE_DESTROY_DUMB(%u) failed: %s",
                        buffer.dumb.handle,
                        strerror(errno));
            }

            buffer.dumb.handle = 0;
        }

        buffer.dumb.size = 0;
    }

    bool LinuxNativeApp::Setup()
    {
        bool driver_found = false;

        if (has_selected_display) {
            if (TryUsingDriver(selected_display.drm_path.c_str(), selected_display.width, selected_display.height, selected_display.refresh, selected_display.connector_id)) {
                driver_found = true;
            }
        } else for (const auto& path : GetDrmCardPaths()) {
            if (TryUsingDriver(path.c_str(), width, height, -1)) {
                driver_found = true;
                break;
            }
        }

        if (!driver_found) {
            Log(ERROR, "No usable DRM device found!");
            return false;
        }

        ud = udev_new();
        li = libinput_udev_create_context(&interface, nullptr, ud);
        libinput_udev_assign_seat(li, "seat0");

        Manager::Initialize(width, height, 4, sideways);

        // set drm caps
        int ret = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
        if (ret != 0) {
            Log(ERROR, "Failed to enable Universal Planes!");
            return false;
        }

        ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
        if (ret != 0) {
            Log(ERROR, "Failed to enable Atomic Modesetting!");
            return false;
        }

        /*
         * On virtualized drivers, e.g. virtio-gpu, cursor planes may be hidden
         * unless userspace explicitly advertises that it understands cursor
         * hotspots. Non-virtualized drivers may return EOPNOTSUPP that is fine,
         * keep running and use the normal cursor-plane probing below.
         */
#ifdef DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT
        ret = drmSetClientCap(fd, DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT, 1);
        if (ret != 0 && errno != EOPNOTSUPP) {
            Log(ERROR, "Failed to enable cursor plane hotspot client cap: %s", strerror(errno));
        }
#else
        Log(ERROR, "DRM cursor plane hotspot not supported!");
#endif

        if (!CreateDumbBuffer(cursor, 64, 64, 32))
            return false;

        if (!AddFramebuffer(cursor, 64, 64, DRM_FORMAT_ARGB8888))
            return false;

        cursor.plane = FindPlaneByType(DRM_PLANE_TYPE_CURSOR);

        if (!cursor.plane) {
            Log(ERROR, "Failed to find DRM cursor plane!");
        }

        if (!CreateDumbBuffer(primary, width, height, 32))
            return false;

        if (!AddFramebuffer(primary, width, height, DRM_FORMAT_XRGB8888))
            return false;

        // Clear the buffer (black)
        memset(primary.map, 0, primary.dumb.size);

        // this is common so just call it once
        primary.plane = FindPlaneByType(DRM_PLANE_TYPE_PRIMARY);

        // draw the cursor (once)
        for (int by = 0, iy = y; iy < cursor_map_height; iy ++) {
            for (int bx = 0, ix = x; ix < cursor_map_width; ix ++) {
                ((uint32_t*)cursor.map)[ix + 64 * iy] =
                    cursor_map[ix + cursor_map_width * iy];
                bx ++;
            }

            by ++;
        }

        drmModeAtomicReqPtr req = drmModeAtomicAlloc();

        if (drm_access_type == DrmAccessType::XrandrLease) {
            Log(INFO, "XRandR blank: selected connector=%u CRTC=%u; blank all other leased outputs",
                conn->connector_id, crtc->crtc_id);

            // Xorg deliberately leaves leased primary planes scanning out their last
            // framebuffer. Detach every non-selected connector/CRTC/plane so no X
            // content remains visible on the other leased outputs.
            for (int i = 0; i < resource->count_connectors; i++) {
                const uint32_t connector_id = resource->connectors[i];
                if (connector_id == conn->connector_id) {
                    continue;
                }

                const uint64_t old_crtc = GetPropertyValue(connector_id, DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID");
                const uint32_t connector_crtc = GetPropertyId(connector_id, DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID");
                Log(INFO, "XRandR blank: connector=%u old_CRTC=%llu -> CRTC_ID=0",
                    connector_id, static_cast<unsigned long long>(old_crtc));
                if (connector_crtc) {
                    drmModeAtomicAddProperty(req, connector_id, connector_crtc, 0);
                } else {
                    Log(WARN, "XRandR blank: connector=%u missing CRTC_ID property", connector_id);
                }
            }

            drmModePlaneResPtr plane_res = drmModeGetPlaneResources(fd);
            if (plane_res) {
                for (uint32_t i = 0; i < plane_res->count_planes; i++) {
                    const uint32_t plane_id = plane_res->planes[i];
                    drmModePlanePtr plane = drmModeGetPlane(fd, plane_id);
                    if (!plane) {
                        continue;
                    }

                    Log(INFO, "XRandR blank: plane=%u type=%u CRTC=%u FB=%u%s",
                        plane_id, GetPlaneType(plane_id), plane->crtc_id, plane->fb_id,
                        plane->crtc_id == crtc->crtc_id ? " [selected CRTC]" : "");
                    if (plane->crtc_id != 0 && plane->crtc_id != crtc->crtc_id) {
                        Log(INFO, "XRandR blank: detach plane=%u from CRTC=%u FB=%u",
                            plane_id, plane->crtc_id, plane->fb_id);
                        const uint32_t plane_fb = GetPropertyId(plane_id, DRM_MODE_OBJECT_PLANE, "FB_ID");
                        const uint32_t plane_crtc = GetPropertyId(plane_id, DRM_MODE_OBJECT_PLANE, "CRTC_ID");
                        if (plane_fb) drmModeAtomicAddProperty(req, plane_id, plane_fb, 0);
                        if (plane_crtc) drmModeAtomicAddProperty(req, plane_id, plane_crtc, 0);
                    }

                    drmModeFreePlane(plane);
                }
                drmModeFreePlaneResources(plane_res);
            }

            for (int i = 0; i < resource->count_crtcs; i++) {
                const uint32_t crtc_id = resource->crtcs[i];
                if (crtc_id == crtc->crtc_id) {
                    continue;
                }

                const uint64_t old_active = GetPropertyValue(crtc_id, DRM_MODE_OBJECT_CRTC, "ACTIVE");
                const uint64_t old_mode = GetPropertyValue(crtc_id, DRM_MODE_OBJECT_CRTC, "MODE_ID");
                Log(INFO, "XRandR blank: CRTC=%u ACTIVE=%llu MODE_ID=%llu -> disable",
                    crtc_id, static_cast<unsigned long long>(old_active), static_cast<unsigned long long>(old_mode));
                const uint32_t active = GetPropertyId(crtc_id, DRM_MODE_OBJECT_CRTC, "ACTIVE");
                const uint32_t mode_id = GetPropertyId(crtc_id, DRM_MODE_OBJECT_CRTC, "MODE_ID");
                if (active) drmModeAtomicAddProperty(req, crtc_id, active, 0);
                if (mode_id) drmModeAtomicAddProperty(req, crtc_id, mode_id, 0);
            }
        }

        Log(INFO, "DRM modeset: enable selected connector=%u CRTC=%u", conn->connector_id, crtc->crtc_id);

        uint32_t connector_crtc = GetPropertyId(conn->connector_id, DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID");
        uint32_t crtc_active = GetPropertyId(crtc->crtc_id, DRM_MODE_OBJECT_CRTC, "ACTIVE");
        uint32_t crtc_mode_id = GetPropertyId(crtc->crtc_id, DRM_MODE_OBJECT_CRTC, "MODE_ID");

        uint32_t mode_blob = 0;
        if (drmModeCreatePropertyBlob(fd, mode, sizeof(*mode), &mode_blob) != 0) {
            Log(ERROR, "Failed to create mode blob");
            return false;
        }

        drmModeAtomicAddProperty(req, conn->connector_id, connector_crtc, crtc->crtc_id);
        drmModeAtomicAddProperty(req, crtc->crtc_id, crtc_active, 1);
        drmModeAtomicAddProperty(req, crtc->crtc_id, crtc_mode_id, mode_blob);

        primary.props.fb = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "FB_ID");
        primary.props.crtc = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "CRTC_ID");

        // Set up cursor request
        if (cursor.plane) {
            cursor.props.fb = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "FB_ID");
            cursor.props.crtc = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "CRTC_ID");
            cursor.props.x = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "CRTC_X");
            cursor.props.y = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "CRTC_Y");
            cursor.props.hotspot_x = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "HOTSPOT_X");
            cursor.props.hotspot_y = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "HOTSPOT_Y");

            // Keep the cursor hidden until the first initialized frame is ready.
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.fb, 0);
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.crtc, 0);
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.x, 0);
            drmModeAtomicAddProperty(req, cursor.plane, cursor.props.y, 0);
            if (cursor.props.hotspot_x) {
                drmModeAtomicAddProperty(req, cursor.plane, cursor.props.hotspot_x, cursor_hotspot_x);
            }
            if (cursor.props.hotspot_y) {
                drmModeAtomicAddProperty(req, cursor.plane, cursor.props.hotspot_y, cursor_hotspot_y);
            }

            uint32_t crtc_w = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "CRTC_W");
            uint32_t crtc_h = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "CRTC_H");
            uint32_t src_w = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "SRC_W");
            uint32_t src_h = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "SRC_H");
            uint32_t src_x = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "SRC_X");
            uint32_t src_y = GetPropertyId(cursor.plane, DRM_MODE_OBJECT_PLANE, "SRC_Y");

            drmModeAtomicAddProperty(req, cursor.plane, crtc_w, 64);
            drmModeAtomicAddProperty(req, cursor.plane, crtc_h, 64);

            drmModeAtomicAddProperty(req, cursor.plane, src_x, 0);
            drmModeAtomicAddProperty(req, cursor.plane, src_y, 0);

            drmModeAtomicAddProperty(req, cursor.plane, src_w, 64 << 16);
            drmModeAtomicAddProperty(req, cursor.plane, src_h, 64 << 16);
        }

        // add primary plane to the same request
        uint32_t crtc_x = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "CRTC_X");
        uint32_t crtc_y = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "CRTC_Y");
        uint32_t crtc_w = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "CRTC_W");
        uint32_t crtc_h = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "CRTC_H");
        uint32_t src_w = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "SRC_W");
        uint32_t src_h = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "SRC_H");
        uint32_t src_x = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "SRC_X");
        uint32_t src_y = GetPropertyId(primary.plane, DRM_MODE_OBJECT_PLANE, "SRC_Y");

        drmModeAtomicAddProperty(req, primary.plane, primary.props.crtc, crtc->crtc_id);
        drmModeAtomicAddProperty(req, primary.plane, primary.props.fb, primary.fb);
        drmModeAtomicAddProperty(req, primary.plane, crtc_x, 0);
        drmModeAtomicAddProperty(req, primary.plane, crtc_y, 0);
        drmModeAtomicAddProperty(req, primary.plane, crtc_w, width);
        drmModeAtomicAddProperty(req, primary.plane, crtc_h, height);
        drmModeAtomicAddProperty(req, primary.plane, src_x, 0);
        drmModeAtomicAddProperty(req, primary.plane, src_y, 0);
        drmModeAtomicAddProperty(req, primary.plane, src_w, width << 16);
        drmModeAtomicAddProperty(req, primary.plane, src_h, height << 16);

        uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_ATOMIC_ALLOW_MODESET;
        ret = drmModeAtomicCommit(fd, req, flags, nullptr);
        if (ret) {
            Log(ERROR, "Atomic commit failed: %s", strerror(errno));
            return false;
        }
        drmModeAtomicFree(req);
        drmModeDestroyPropertyBlob(fd, mode_blob);

        ev.version = DRM_EVENT_CONTEXT_VERSION;
        ev.page_flip_handler = PageFlipHandler;

        thread_run = true;
        drm_thread = std::thread([this] () -> void {
            first_frame_signal.Wait();
            while (thread_run) {
                CommitPlanes();
                DRMWait();
                frame_signal.Post();
            }
        });

        input_thread = std::thread([this] () -> void {
            while (thread_run) {
                HandleInput();
            }
        });

        return true;
    }

    static uint64_t NowMs()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    void LinuxNativeApp::SetIdleThrottle(bool enabled)
    {
        idle_throttle = enabled;

        if (idle_throttle) {
            // this could be -1 (blocking), but let's make it a long drm_event_timeout
            // so we never block indefinitely, e.g. be easy on exit
            drm_event_timeout = 300;
        } else {
            drm_event_timeout = 0; // no wait
        }
    }

    void LinuxNativeApp::RequestRedraw()
    {
        last_event_ms = NowMs();
    }

    bool LinuxNativeApp::ShouldDrawFrame()
    {
        // keep rendering at full rate this long after the last input event
        static constexpr uint64_t active_window_ms = 1000;
        // otherwise still refresh now and then, for things changing without input (clock, JS, etc.)
        static constexpr uint64_t idle_interval_ms = 250;

        if (!idle_throttle) {
            return true;
        }

        const uint64_t now = NowMs();

        return now - last_event_ms < active_window_ms
            || now - last_draw_ms >= idle_interval_ms
            || Manager::GetInstance().NeedsFrames();
    }

    void LinuxNativeApp::Render()
    {
        if (!thread_run) {
            frame_drawn = false;
            return;
        }

        frame_signal.Wait();

        frame_drawn = !first_frame_ready || ShouldDrawFrame();
        if (!frame_drawn) {
            return;
        }

        last_draw_ms = NowMs();
        Manager::GetInstance().perf.status = idle_throttle ? "[throttle: on]" : "[throttle: off]";
        Manager::GetInstance().MainLoopIteration();
    }

    void LinuxNativeApp::Swap()
    {
        // nothing was rendered, the previous frame is still being displayed
        if (!frame_drawn) {
            return;
        }

        Stopwatch watch {};

        const uint32_t row_bytes = width * 4;
        auto* dst = static_cast<uint8_t*>(primary.map);
        auto* src = reinterpret_cast<const uint8_t*>(framebuffer);

        // copy framebuffer to primary plane buffer
        // DRM dumb buffers may have pitch != width * 4, so do not copy the
        // whole logical image as one tightly packed block.
        if (row_bytes == primary.dumb.pitch) {
            memcpy(dst, src, row_bytes * height);
        } else {
            for (int y = 0; y < height; ++y, src += row_bytes, dst += primary.dumb.pitch) {
                memcpy(dst, src, row_bytes);
            }
        }

        Manager::GetInstance().perf.swap_times.put(watch.stop());

        if(!first_frame_ready) {
            first_frame_ready = true;
            first_frame_signal.Post();
        }
    }

    void LinuxNativeApp::Poll()
    {
        while (true) {
            auto event = events.pop();

            if (!event) {
                break;
            }

            (*event)();
        }

        auto x = cursor_state.x.load();
        auto y = cursor_state.y.load();

        Manager::GetInstance().ProcessTouchPoint(left_mouse_pressed, x, y);
    }

    void LinuxNativeApp::HandleKeycode(uint32_t xkb_keycode, uint32_t evdev_keycode, bool pressed)
    {
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, xkb_keycode);
        const bool ctrl_pressed = xkb_state_mod_name_is_active(xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);

        if (pressed) switch (keysym) {
            case XKB_KEY_BackSpace:
            case XKB_KEY_Return:

            case XKB_KEY_Tab:
            case XKB_KEY_Escape:
            case XKB_KEY_Delete:
            case XKB_KEY_Home:
            case XKB_KEY_End:
            case XKB_KEY_Left:
            case XKB_KEY_Right:
            case XKB_KEY_Up:
            case XKB_KEY_Down:
            case XKB_KEY_Page_Up:
            case XKB_KEY_Page_Down:

            case XKB_KEY_F1:
            case XKB_KEY_F2:
            case XKB_KEY_F3:
            case XKB_KEY_F4:
            case XKB_KEY_F5:
            case XKB_KEY_F6:
            case XKB_KEY_F7:
            case XKB_KEY_F8:
            case XKB_KEY_F9:
            case XKB_KEY_F10:
            case XKB_KEY_F11:
            case XKB_KEY_F12:
                break;

            default:
                if (ctrl_pressed) {
                    break;
                }

                char* buffer;
                size_t size = xkb_state_key_get_utf8(xkb_state, xkb_keycode, nullptr, 0);

                if (size > 0) {
                    std::vector<char> buffer (size + 1);
                    if (xkb_state_key_get_utf8(xkb_state, xkb_keycode, buffer.data(), buffer.size()) > 0) {

                        // move the buffer to the handler
                        events.push([buffer = std::move(buffer)] () {
                            Manager::GetInstance().ProcessTextInput(buffer.data());
                        });
                    }
                }
        }

        xkb_state_update_key(xkb_state, xkb_keycode, pressed ? XKB_KEY_DOWN : XKB_KEY_UP);

        events.push([pressed, evdev_keycode] () {
            Manager::GetInstance().ProcessKeyInput(pressed, evdev_keycode);
        });
    }

    void LinuxNativeApp::HandleEvent(libinput_event* event)
    {
        last_event_ms = NowMs();

        auto type = libinput_event_get_type(event);

        switch (type) {

            // Relative movement (e.g. mouse)
            case LIBINPUT_EVENT_POINTER_MOTION:
            {
                libinput_event_pointer* pointer = libinput_event_get_pointer_event(event);

                const double dx = libinput_event_pointer_get_dx(pointer);
                const double dy = libinput_event_pointer_get_dy(pointer);

                if (dx != 0 || dy != 0) {
                    MarkUserActivity();
                }

                x += dx;
                y += dy;

                UpdateCursorPos();
                break;
            }

            // Digitizers and touch controls (e.g. drawing tablets)
            case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
            {
                libinput_event_pointer* pointer = libinput_event_get_pointer_event(event);

                const int new_x = libinput_event_pointer_get_absolute_x_transformed(pointer, width);
                const int new_y = libinput_event_pointer_get_absolute_y_transformed(pointer, height);

                if (new_x != x || new_y != y) {
                    MarkUserActivity();
                }

                x = new_x;
                y = new_y;

                UpdateCursorPos();
                break;
            }

            case LIBINPUT_EVENT_POINTER_BUTTON:
            {
                libinput_event_pointer* pointer = libinput_event_get_pointer_event(event);
                uint32_t button = libinput_event_pointer_get_button(pointer);
                bool pressed = (libinput_event_pointer_get_button_state(pointer) == LIBINPUT_BUTTON_STATE_PRESSED);

                if (pressed) {
                    MarkUserActivity();
                }

                if (button == BTN_LEFT) {
                    left_mouse_pressed = pressed;
                }
                break;
            }

            case LIBINPUT_EVENT_KEYBOARD_KEY:
            {
                libinput_event_keyboard* keyboard = libinput_event_get_keyboard_event(event);
                uint32_t evdev_key = libinput_event_keyboard_get_key(keyboard);
                bool pressed = libinput_event_keyboard_get_key_state(keyboard) == LIBINPUT_KEY_STATE_PRESSED;

                if (pressed) {
                    MarkUserActivity();

                    if (evdev_key == KEY_NUMLOCK) leds ^= LIBINPUT_LED_NUM_LOCK;
                    if (evdev_key == KEY_CAPSLOCK) leds ^= LIBINPUT_LED_CAPS_LOCK;
                    if (evdev_key == KEY_SCROLLLOCK) leds ^= LIBINPUT_LED_SCROLL_LOCK;

                    libinput_device* dev = libinput_event_get_device(event);
                    libinput_device_led_update(dev, static_cast<libinput_led>(leds));
                }

                // The '+8' is here to convert from the kernel/libinput's
                // keycodes to the ones expected by xkbcommon
                HandleKeycode(evdev_key + 8, evdev_key, pressed);
                Manager::GetInstance().ProcessKeyInput(pressed, evdev_key);
                break;
            }

        }
    }

    void LinuxNativeApp::HandleInput()
    {
        struct pollfd pfd;
        pfd.fd = libinput_get_fd(li);
        pfd.events = POLLIN;

        while (true) {
            // wait for up to 5ms
            poll(&pfd, 1, 5);

            libinput_dispatch(li);
            struct libinput_event* event = libinput_get_event(li);

            if (event == nullptr) {
                break;
            }

            HandleEvent(event);
            libinput_event_destroy(event);
        }
    }

    void LinuxNativeApp::DrawMouseIcon(bool flag) {
        draw_mouse_icon = flag;
    }
}
