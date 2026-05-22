#if defined(GRVL_LINUX_NATIVE_SUPPORT)

#include <grvl/platform/LinuxNativeApp.h>
#include <grvl/Manager.h>

#include <regex>
#include <filesystem>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <pthread.h>
#include <sched.h>

#include <poll.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <sys/mman.h>

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
// clang-format on

static drmModeConnectorPtr PickConnector(int fd, drmModeResPtr resource)
{
    std::vector<drmModeConnectorPtr> connectors;
    int selected = -1;

    for (int i = 0; i < resource->count_connectors; i++) {
        const auto connector = drmModeGetConnectorCurrent(fd, resource->connectors[i]);
        connectors.push_back(connector);

        if (connector == nullptr) {
            continue;
        }

        if (connector->count_modes == 0) {
            continue;
        }

        if (connector->connection == DRM_MODE_DISCONNECTED) {
            continue;
        }

        // we always select the "last" connector (first valid)
        selected = connectors.size() - 1;
        break;
    }

    for (size_t i = 0; i < connectors.size(); i++) {
        if (i != selected) {
            drmModeFreeConnector(connectors[i]);
        }
    }

    if (selected == -1) {
        printf("No valid DRM connectors found!\n");
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

const static struct libinput_interface interface = {
    .open_restricted = [] (const char* path, int flags, void* user) {
        const int fd = open(path, flags);
        return fd < 0 ? -errno : fd;
    },
    .close_restricted = [] (int fd, void* user) {
        close(fd);
    }
};


// implementation

namespace grvl {

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

    LinuxNativeApp::~LinuxNativeApp()
    {
        thread_run = false;

        input_thread.join();

        libinput_unref(li);
        udev_unref(ud);

        xkb_state_unref(xkb_state);
        xkb_keymap_unref(xkb_keymap);
        xkb_context_unref(xkb_ctx);

        cursor_thread.join();
        CloseDriver();

        grvl::grvl::Destroy();
    }

    uint32_t LinuxNativeApp::GetPropertyId(uint32_t obj_id, const char* name)
    {
        uint32_t prop_id = 0;

        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, obj_id, DRM_MODE_OBJECT_PLANE);
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

    uint32_t LinuxNativeApp::GetPlaneType(uint32_t plane_id)
    {
        uint32_t type_val = uint32_t(-1);

        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);

        uint32_t type_prop_id = GetPropertyId(plane_id, "type");

        for (uint32_t i = 0; i < props->count_props; i++) {
            if (props->props[i] == type_prop_id) {
                type_val = (uint32_t)props->prop_values[i];
                break;
            }
        }

        drmModeFreeObjectProperties(props);
        return type_val;
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
            printf("Failed to select DRM plane!\n");
        }

        return found_id;
    }

    void LinuxNativeApp::CloseDriver()
    {
        if (fd == -1) {
            return;
        }

        if (resource) {
            drmModeFreeResources(resource);
        }

        if (conn) {
            drmModeFreeConnector(conn);
        }

        if (encoder) {
            drmModeFreeEncoder(encoder);
        }

        close(fd);
    }

    bool LinuxNativeApp::InitDriver(int fd, uint16_t width, uint16_t height, uint32_t refresh)
    {
        this->fd = fd;

        this->resource = drmModeGetResources(fd);
        if (!resource) {
            printf("Unable to get DRM resources!\n");
            return false;
        }

        this->conn = PickConnector(fd, resource);
        if (!conn) {
            printf("Unable to pick DRM connection!\n");
            return false;
        }

        this->mode = PickMode(conn, width, height, refresh);
        if (!mode) {
            printf("Unable to pick DRM mode!\n");
            return false;
        }

        this->encoder = drmModeGetEncoder(fd, conn->encoder_id);
        if (!encoder) {
            printf("Unable to get DRM encoder!\n");
            return false;
        }

        // Find index of the CRTC we are using
        for (int i = 0; i < resource->count_crtcs; i++) {
            if (resource->crtcs[i] == encoder->crtc_id) {
                crtc_index = i;
                break;
            }
        }

        return true;
    }

    bool LinuxNativeApp::TryUsingDriver(const char* path, uint16_t width, uint16_t height, uint32_t refresh)
    {
        if (path != nullptr && (strlen(path) != 0)) {
            int fh = open(path, O_RDWR);
            if (fh > 0) {
                return InitDriver(fh, width, height, refresh);
            }

            printf("Failed to open '%s'!\n", path);
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
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.fb, cursor.fb);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.crtc, encoder->crtc_id);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.x, x);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.y, y);
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

        int ret = poll(&pfd, 1, 16);

        if (ret > 0 && (pfd.revents & POLLIN)) {
            drmHandleEvent(fd, &ev);
        }
    }

    bool LinuxNativeApp::Setup()
    {
        const char* drivers[] = {
            std::getenv("DRM_PATH"), "/dev/dri/card0", "/dev/dri/card1"
        };

        for (const char* path : drivers) {
            if (TryUsingDriver(path, width, height, -1)) break;
        }

        ud = udev_new();
        li = libinput_udev_create_context(&interface, nullptr, ud);
        libinput_udev_assign_seat(li, "seat0");

        Manager::Initialize(width, height, 4, sideways);

        // set drm caps
        int ret = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
        if (ret != 0) {
            printf("Failed to enable Universal Planes!\n");
            return false;
        }

        drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

        // Prepare DRM buffers for the mouse cursor
        cursor.dumb.width = 64;
        cursor.dumb.height = 64;
        cursor.dumb.bpp = 32;
        drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &cursor.dumb);

        cursor.handles[0] = cursor.dumb.handle;
        cursor.pitches[0] = cursor.dumb.pitch;

        struct drm_mode_map_dumb mreq = {};
        mreq.handle = cursor.dumb.handle;
        if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
            printf("Failed to prepare dumb buffer for mapping!\n");
            return false;
        }

        cursor.map = (uint32_t *)mmap(0, cursor.dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);

        drmModeAddFB2(fd, 64, 64, DRM_FORMAT_ARGB8888,
                      cursor.handles, cursor.pitches,
                      cursor.offsets, &cursor.fb, 0);

        cursor.plane = FindPlaneByType(DRM_PLANE_TYPE_CURSOR);

        // Prepare primary plane buffers
        primary.dumb.width = width;
        primary.dumb.height = height;
        primary.dumb.bpp = 32;

        drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &primary.dumb);

        primary.handles[0] = primary.dumb.handle;
        primary.pitches[0] = primary.dumb.pitch;

        drmModeAddFB2(fd, width, height, DRM_FORMAT_XRGB8888,
                      primary.handles,
                      primary.pitches,
                      primary.offsets,
                      &primary.fb,
                      0);

        // Map it to CPU memory
        struct drm_mode_map_dumb dmap = {};
        dmap.handle = primary.dumb.handle;
        drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &dmap);

        primary.map = mmap(0, primary.dumb.size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, dmap.offset);

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

        // Set up cursor request
        cursor.props.fb = GetPropertyId(cursor.plane, "FB_ID");
        cursor.props.crtc = GetPropertyId(cursor.plane, "CRTC_ID");
        cursor.props.x = GetPropertyId(cursor.plane, "CRTC_X");
        cursor.props.y = GetPropertyId(cursor.plane, "CRTC_Y");

        primary.props.fb = GetPropertyId(primary.plane, "FB_ID");
        primary.props.crtc = GetPropertyId(primary.plane, "CRTC_ID");

        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.fb, cursor.fb);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.crtc, encoder->crtc_id);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.x, 0);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.y, 0);

        uint32_t crtc_x, crtc_y, crtc_w, crtc_h, src_w, src_h, src_x, src_y;

        crtc_w = GetPropertyId(cursor.plane, "CRTC_W");
        crtc_h = GetPropertyId(cursor.plane, "CRTC_H");
        src_w = GetPropertyId(cursor.plane, "SRC_W");
        src_h = GetPropertyId(cursor.plane, "SRC_H");
        src_x = GetPropertyId(cursor.plane, "SRC_X");
        src_y = GetPropertyId(cursor.plane, "SRC_Y");

        drmModeAtomicAddProperty(req, cursor.plane, crtc_w, 64);
        drmModeAtomicAddProperty(req, cursor.plane, crtc_h, 64);

        drmModeAtomicAddProperty(req, cursor.plane, src_x, 0);
        drmModeAtomicAddProperty(req, cursor.plane, src_y, 0);

        drmModeAtomicAddProperty(req, cursor.plane, src_w, 64 << 16);
        drmModeAtomicAddProperty(req, cursor.plane, src_h, 64 << 16);

        // add primary plane to the same request
        crtc_x = GetPropertyId(primary.plane, "CRTC_X");
        crtc_y = GetPropertyId(primary.plane, "CRTC_Y");
        crtc_w = GetPropertyId(primary.plane, "CRTC_W");
        crtc_h = GetPropertyId(primary.plane, "CRTC_H");
        src_w = GetPropertyId(primary.plane, "SRC_W");
        src_h = GetPropertyId(primary.plane, "SRC_H");
        src_x = GetPropertyId(primary.plane, "SRC_X");
        src_y = GetPropertyId(primary.plane, "SRC_Y");

        drmModeAtomicAddProperty(req, primary.plane, primary.props.crtc, encoder->crtc_id);
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
        drmModeAtomicCommit(fd, req, flags, nullptr);
        drmModeAtomicFree(req);

        ev.version = DRM_EVENT_CONTEXT_VERSION;
        ev.page_flip_handler = PageFlipHandler;

        thread_run = true;
        cursor_thread = std::thread([this] () -> void {
            while (thread_run) {
                CommitPlanes();
                DRMWait();
            }
        });

        input_thread = std::thread([this] () -> void {
            while (thread_run) {
                HandleInput();
            }
        });

        return true;
    }

    void LinuxNativeApp::Render()
    {
        Manager::GetInstance().MainLoopIteration();
    }

    void LinuxNativeApp::Swap()
    {
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

    void LinuxNativeApp::HandleKeycode(uint32_t keycode, bool pressed)
    {
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, keycode);

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
                char* buffer;
                size_t size = xkb_state_key_get_utf8(xkb_state, keycode, nullptr, 0);

                if (size > 0) {
                    std::vector<char> buffer (size + 1);
                    if (xkb_state_key_get_utf8(xkb_state, keycode, buffer.data(), buffer.size()) > 0) {

                        // move the buffer to the handler
                        events.push([buffer = std::move(buffer)] () {
                            Manager::GetInstance().ProcessTextInput(buffer.data());
                        });
                    }
                }
        }

        xkb_state_update_key(xkb_state, keycode, pressed ? XKB_KEY_DOWN : XKB_KEY_UP);

        events.push([pressed, keycode] () {
            Manager::GetInstance().ProcessKeyInput(pressed, keycode);
        });
    }

    void LinuxNativeApp::HandleInput()
    {
        struct libinput_event* event;

        libinput_dispatch(li);

        while ((event = libinput_get_event(li)) != nullptr) {
            auto type = libinput_event_get_type(event);

            switch (type) {

                // Relative movement (e.g. mouse)
                case LIBINPUT_EVENT_POINTER_MOTION:
                {
                    libinput_event_pointer* pointer = libinput_event_get_pointer_event(event);

                    x += libinput_event_pointer_get_dx(pointer);
                    y += libinput_event_pointer_get_dy(pointer);

                    UpdateCursorPos();
                    break;
                }

                // Digitizers and touch controls (e.g. drawing tablets)
                case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
                {
                    libinput_event_pointer* pointer = libinput_event_get_pointer_event(event);

                    x = libinput_event_pointer_get_absolute_x_transformed(pointer, width);
                    y = libinput_event_pointer_get_absolute_y_transformed(pointer, height);

                    UpdateCursorPos();
                    break;
                }

                case LIBINPUT_EVENT_POINTER_BUTTON:
                {
                    libinput_event_pointer* pointer = libinput_event_get_pointer_event(event);
                    uint32_t button = libinput_event_pointer_get_button(pointer);
                    bool pressed = (libinput_event_pointer_get_button_state(pointer) == LIBINPUT_BUTTON_STATE_PRESSED);

                    if (button == BTN_LEFT) {
                        left_mouse_pressed = pressed;
                    }
                    break;
                }

                case LIBINPUT_EVENT_KEYBOARD_KEY:
                {
                    libinput_event_keyboard* keyboard = libinput_event_get_keyboard_event(event);
                    uint32_t key = libinput_event_keyboard_get_key(keyboard);
                    libinput_key_state state = libinput_event_keyboard_get_key_state(keyboard);

                    // The '+8' is here to convert from the kernel/libinput's
                    // keycodes to the ones expected by xkbcommon
                    HandleKeycode(key + 8, state == LIBINPUT_KEY_STATE_PRESSED);
                    break;
                }

            }

            libinput_event_destroy(event);
            libinput_dispatch(li);
        }

    }

    void LinuxNativeApp::DrawMouseIcon(bool flag) {
        draw_mouse_icon = flag;
    }

}

#endif
