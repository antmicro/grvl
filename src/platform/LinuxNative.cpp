#if defined(GRVL_LINUX_NATIVE_SUPPORT)

#include <grvl/platform/LinuxNativeApp.h>
#include <grvl/Manager.h>

#include <core/framebuffer.hpp>
#include <core/drm.hpp>
#include <core/interrupt.hpp>

#include <regex>
#include <filesystem>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <cstdlib>

#include <atomic>
#include <poll.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <sys/mman.h>

// Adawaita default cursor
// CC-BY-SA
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

// input
static std::vector<std::string> GrepFiles(const std::string& path, const std::string& pattern)
{
    std::vector<std::string> paths;
    std::regex expr {pattern};

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string path = entry.path();

        if (std::regex_search(path, expr)) {
            paths.push_back(path);
        }
    }

    return paths;
}

void CloseFiles(int* fds, int count)
{
    for (int i = 0; i < count; i ++) {
        ioctl(fds[i], EVIOCGRAB, 0);
        close(fds[i]);
    }

    delete[] fds;
}

static int* OpenFiles(const std::vector<std::string>& paths)
{
    int* fds = new int[paths.size()];
    int i = 0;

    for (const auto& path : paths) {
        int fd = open(path.c_str(), O_RDONLY);

        if (fd == -1) {
            printf("Unable to open event source '%s'!\n", path.c_str());

            CloseFiles(fds, i);
            return nullptr;
        }

        int grab = 1;
        if (ioctl(fd, EVIOCGRAB, &grab) == -1) {
            printf("Warning: Unable to grab device '%s'. Console may still see input.\n", path.c_str());
        }
        fds[i] = fd;
        i ++;
    }

    return fds;
}

static int FindReadable(const int* fds, int file_count, bool wait)
{
    fd_set rdset;
    FD_ZERO(&rdset);

    int max_fd = 0;

    for (int i = 0; i < file_count; i ++) {
        const int fd = fds[i];
        FD_SET(fd, &rdset);

        if (fd > max_fd) {
            max_fd = fd;
        }
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int ret = select(max_fd + 1, &rdset, nullptr, nullptr, wait ? nullptr : &timeout);

    if (ret > 0) {
        for (int i = 0; i < file_count; i ++) {
            int fd = fds[i];
            if (FD_ISSET(fd, &rdset)) {
                return fd;
            }
        }
    }

    return -1;
}

static void FileRead(int fd, char* element, int size)
{
    while (size != 0) {
        int ret = read(fd, element, size);

        if (ret < 0) {
            printf("Failed to read from open file descriptor %d!\n", ret);
            exit(1);
        }

        size -= ret;
        element += ret;
    }
}

static bool ReadEvent(struct input_event* event, const int* fds, int file_count, bool wait, int* read_from = nullptr)
{
    const int fd = FindReadable(fds, file_count, wait);

    if (fd == -1) {
        return false;
    }

    if (read_from != nullptr) {
        *read_from = fd;
    }

    FileRead(fd, (char*) event, sizeof(*event));
    return true;
}

// implementation

namespace grvl {

    LinuxNativeApp::LinuxNativeApp(int width, int height, bool rotate_sideways)
        : PosixApp(width, height, rotate_sideways)
    {
        bgra_buffer = new uint32_t [width * height];
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
        CloseFiles(inputs, count);

        xkb_state_unref(xkb_state);
        xkb_keymap_unref(xkb_keymap);
        xkb_context_unref(xkb_ctx);

        delete output;
        delete[] bgra_buffer;

        grvl::grvl::Destroy();
    }

    void LinuxNativeApp::UpdateBgraBuffer()
    {
        struct Channels {
            uint8_t r, g, b, a;
        };

        union Pixel {
            Channels channels;
            uint32_t data;
        };

        uint32_t* source = reinterpret_cast<uint32_t*>(framebuffer);

        for (int iy = 0; iy < height; iy ++) {
            for (int ix = 0; ix < width; ix ++) {
                Pixel pixel;

                pixel.data = source[iy * width + ix];
                std::swap(pixel.channels.r, pixel.channels.b);
                bgra_buffer[iy * width + ix] = pixel.data;
            }
        }
    }

    void LinuxNativeApp::ClampCursor()
    {
        if (x < 0) x = 0;
        if (x >= width) x = width - 1;
        if (y < 0) y = 0;
        if (y > height) y = height - 1;
    }

    uint32_t get_prop_id(int fd, uint32_t obj_id, const char *name) {
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

    uint32_t get_plane_type(int fd, uint32_t plane_id) {
        uint32_t type_val = uint32_t(-1);

        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);

        uint32_t type_prop_id = get_prop_id(fd, plane_id, "type");

        for (uint32_t i = 0; i < props->count_props; i++) {
            if (props->props[i] == type_prop_id) {
                type_val = (uint32_t)props->prop_values[i];
                break;
            }
        }

        drmModeFreeObjectProperties(props);
        return type_val;
    }

    uint32_t find_plane_by_type(int drm_fd, uint32_t plane_type, uint32_t crtc_id) {
        drmModePlaneRes *plane_res = drmModeGetPlaneResources(drm_fd);
        if (!plane_res) return 0;

        uint32_t found_id = 0;

        for (uint32_t i = 0; i < plane_res->count_planes; i++) {
            uint32_t plane_id = plane_res->planes[i];
            drmModePlane *plane =
              drmModeGetPlane(drm_fd, plane_id);

            if (!(plane->possible_crtcs & (1 << crtc_id))) {
                drmModeFreePlane(plane);
                continue;
            }
            drmModeFreePlane(plane);

            if (get_plane_type(drm_fd, plane_id) == plane_type) {
                found_id = plane_id;
                break;
            }
        }

        drmModeFreePlaneResources(plane_res);
        return found_id;
    }

    struct CursorState {
        std::atomic<int> latest_x{0};
        std::atomic<int> latest_y{0};

        std::atomic<bool> dirty{false};
        std::atomic<bool> pending{false};

        int displayed_x = 0;
        int displayed_y = 0;
    };

    static CursorState g_cursor;

    static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
    {
        g_cursor.pending = false;
    }

    bool LinuxNativeApp::Setup()
    {
        output = new drm_screen("", width, height);

        // Linux kernel input API
        auto paths = GrepFiles("/dev/input/", "event\\d+");
        inputs = OpenFiles(paths);
        count = paths.size();

        if (count == 0 || inputs == nullptr) {
            printf("Unable to access Linux kernel input API!\n");

            // make sure we don't exit without closing YAV screen
            delete output;
            output = nullptr;

            return false;
        }

        // Check for trackpads which return absolute position,
        // but use pointer (relative move)
        unsigned long props;
        for(int i = 0; i < count; ++i) {
            if(ioctl(inputs[i], EVIOCGPROP(sizeof(props)), &props) >= 0) {
                if(props & (1UL << INPUT_PROP_POINTER)) {
                    pointer_devices.insert(inputs[i]);
                }
            }
        }

        Manager::Initialize(width, height, 4, sideways);
        setup_interrupt_handlers();

        // set drm caps
        int ret = drmSetClientCap(output->fd(), DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
        if (ret != 0) {
            perror("Failed to enable Universal Planes");
        }

        drmSetClientCap(output->fd(), DRM_CLIENT_CAP_ATOMIC, 1);

        // Prepare DRM buffers for the mouse cursor
        cursor.dumb.width = 64;
        cursor.dumb.height = 64;
        cursor.dumb.bpp = 32;
        drmIoctl(output->fd(), DRM_IOCTL_MODE_CREATE_DUMB, &cursor.dumb);

        cursor.handles[0] = cursor.dumb.handle;
        cursor.pitches[0] = cursor.dumb.pitch;

        struct drm_mode_map_dumb mreq = {};
        mreq.handle = cursor.dumb.handle; // The handle from your CREATE_DUMB call
        if (drmIoctl(output->fd(), DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
            perror("Failed to prepare dumb buffer for mapping");
        }

        cursor.map = (uint32_t *)mmap(0, cursor.dumb.size, PROT_READ | PROT_WRITE,
                                      MAP_SHARED, output->fd(), mreq.offset);

        drmModeAddFB2(output->fd(), 64, 64, DRM_FORMAT_ARGB8888,
                      cursor.handles, cursor.pitches,
                      cursor.offsets, &cursor.fb, 0);

        drmModeRes *res = drmModeGetResources(output->fd());
        if (!res) {
          fprintf(stderr, "drmModeGetResources failed\n");
          return 1;
        }

        drmModeConnector *conn = NULL;

        for (int i = 0; i < res->count_connectors; i++) {

          conn = drmModeGetConnector(output->fd(), res->connectors[i]);

          if (conn && conn->connection == DRM_MODE_CONNECTED)
            break;

          drmModeFreeConnector(conn);
          conn = NULL;
        }

        if (!conn) {
          fprintf(stderr, "no connected connector\n");
          return 1;
        }

        drmModeModeInfo mode = conn->modes[0];

        drmModeEncoder *enc =
          drmModeGetEncoder(output->fd(), conn->encoder_id);

        if (!enc) {
          fprintf(stderr, "no encoder\n");
          return 1;
        }

        uint32_t crtc_id = enc->crtc_id;

        uint32_t crtc_index = 0;

        for (int i = 0; i < res->count_crtcs; i++) {
          if (res->crtcs[i] == crtc_id) {
            crtc_index = i;
            break;
          }
        }

        cursor.plane = find_plane_by_type(output->fd(), DRM_PLANE_TYPE_CURSOR, crtc_index);

        // Prepare primary plane buffers
        primary.dumb.width = width;
        primary.dumb.height = height;
        primary.dumb.bpp = 32;

        drmIoctl(output->fd(), DRM_IOCTL_MODE_CREATE_DUMB, &primary.dumb);

        primary.handles[0] = primary.dumb.handle;
        primary.pitches[0] = primary.dumb.pitch;

        drmModeAddFB2(output->fd(), width, height, DRM_FORMAT_XRGB8888,
                      primary.handles,
                      primary.pitches,
                      primary.offsets,
                      &primary.fb,
                      0);

        // Map it to CPU memory
        struct drm_mode_map_dumb dmap = {};
        dmap.handle = primary.dumb.handle;
        drmIoctl(output->fd(), DRM_IOCTL_MODE_MAP_DUMB, &dmap);

        primary.map = mmap(0, primary.dumb.size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, output->fd(), dmap.offset);

        // Clear the buffer (black)
        memset(primary.map, 0, primary.dumb.size);

        // this is common so just call it once
        primary.plane = find_plane_by_type(output->fd(), DRM_PLANE_TYPE_PRIMARY, crtc_index);

        // draw the cursor (once)
        for (int by = 0, iy = y; iy < cursor_map_height; iy ++) {
            for (int bx = 0, ix = x; ix < cursor_map_width; ix ++) {
                ((uint32_t*)cursor.map)[ix + 64 * iy] =
                    cursor_map[ix + cursor_map_width * iy];
                bx ++;
            }

            by ++;
        }

        int drm_fd = output->fd();

        drmModeAtomicReq *req = drmModeAtomicAlloc();

        // Set up cursor request
        cursor.props.fb = get_prop_id(drm_fd, cursor.plane, "FB_ID");
        cursor.props.crtc = get_prop_id(drm_fd, cursor.plane, "CRTC_ID");
        cursor.props.x = get_prop_id(drm_fd, cursor.plane, "CRTC_X");
        cursor.props.y = get_prop_id(drm_fd, cursor.plane, "CRTC_Y");

        primary.props.fb = get_prop_id(drm_fd, primary.plane, "FB_ID");
        primary.props.crtc = get_prop_id(drm_fd, primary.plane, "CRTC_ID");

        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.fb, cursor.fb);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.crtc, crtc_id);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.x, 0);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.y, 0);

        uint32_t crtc_x, crtc_y, crtc_w, crtc_h, src_w, src_h, src_x, src_y;

        crtc_w = get_prop_id(drm_fd, cursor.plane, "CRTC_W");
        crtc_h = get_prop_id(drm_fd, cursor.plane, "CRTC_H");
        src_w = get_prop_id(drm_fd, cursor.plane, "SRC_W");
        src_h = get_prop_id(drm_fd, cursor.plane, "SRC_H");
        src_x = get_prop_id(drm_fd, cursor.plane, "SRC_X");
        src_y = get_prop_id(drm_fd, cursor.plane, "SRC_Y");

        drmModeAtomicAddProperty(req, cursor.plane, crtc_w, 64);
        drmModeAtomicAddProperty(req, cursor.plane, crtc_h, 64);

        drmModeAtomicAddProperty(req, cursor.plane, src_x, 0);
        drmModeAtomicAddProperty(req, cursor.plane, src_y, 0);

        drmModeAtomicAddProperty(req, cursor.plane, src_w, 64 << 16);
        drmModeAtomicAddProperty(req, cursor.plane, src_h, 64 << 16);

        // add primary plane to the same request
        crtc_x = get_prop_id(drm_fd, primary.plane, "CRTC_X");
        crtc_y = get_prop_id(drm_fd, primary.plane, "CRTC_Y");
        crtc_w = get_prop_id(drm_fd, primary.plane, "CRTC_W");
        crtc_h = get_prop_id(drm_fd, primary.plane, "CRTC_H");
        src_w = get_prop_id(drm_fd, primary.plane, "SRC_W");
        src_h = get_prop_id(drm_fd, primary.plane, "SRC_H");
        src_x = get_prop_id(drm_fd, primary.plane, "SRC_X");
        src_y = get_prop_id(drm_fd, primary.plane, "SRC_Y");

        drmModeAtomicAddProperty(req, primary.plane, primary.props.crtc, crtc_id);
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
        int rc = drmModeAtomicCommit(drm_fd, req, flags, NULL);
        drmModeAtomicFree(req);


        ev.version = DRM_EVENT_CONTEXT_VERSION;
        ev.page_flip_handler = page_flip_handler;

        return true;
    }

    void LinuxNativeApp::Render()
    {
        Manager::GetInstance().MainLoopIteration();
        UpdateBgraBuffer();

        // copy framebuffer to primary plane buffer
        memcpy(primary.map, bgra_buffer, width * height * 4);

        g_cursor.dirty = true;
    }
    void LinuxNativeApp::RenderCursor()
    {
        if (g_cursor.pending){
            return;
        }

        if (!g_cursor.dirty) {
            return;
        }

        int x = g_cursor.latest_x.load();
        int y = g_cursor.latest_y.load();

        int drm_fd = output->fd();
        uint32_t crtc_id = output->fb->crtc->crtc_id;
        drmModeAtomicReq *req = drmModeAtomicAlloc();
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.fb, cursor.fb);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.crtc, crtc_id);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.x, x);
        drmModeAtomicAddProperty(req, cursor.plane, cursor.props.y, y);
        drmModeAtomicAddProperty(req, primary.plane, primary.props.fb, primary.fb);

        uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT; //DRM_MODE_ATOMIC_ALLOW_MODESET;
        static int last_rc = 0;
        int rc = drmModeAtomicCommit(drm_fd, req, flags, NULL);

        drmModeAtomicFree(req);

         if (rc == 0) {
             g_cursor.pending = true;
             g_cursor.dirty = false;

             g_cursor.displayed_x = x;
             g_cursor.displayed_y = y;

             return;
        }
    }

    void LinuxNativeApp::DRMWait()
    {
        int drm_fd = output->fd();
        pollfd pfd = {};
        pfd.fd = drm_fd;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 16);

        if (ret > 0 && (pfd.revents & POLLIN)) {
            drmHandleEvent(drm_fd, &ev);
        }
    }

    void LinuxNativeApp::Poll()
    {
        struct input_event event;
        int fd;

            while(ReadEvent(&event, inputs, count, false, &fd)) {

            switch (event.type) {
                case EV_REL:
                {
                    // relative mouse movement
                    // event.value contains relative offset
                    if (event.code == 0) x += event.value;
                    if (event.code == 1) y += event.value;

                    g_cursor.latest_x = x;
                    g_cursor.latest_y = y;
                    g_cursor.dirty = true;

                    break;
                }

                case EV_ABS:
                {
                    // Touchpad device - translate absolute position
                    // to relative offset
                    if (pointer_devices.find(fd) != pointer_devices.end()) {
                        if (!touch_down) break;

                        if (abs_x == -1 || abs_y == -1) {
                            if (event.code == 0) abs_x = event.value;
                            if (event.code == 1) abs_y = event.value;
                            break;
                        }

                        if (event.code == 0) {
                            x += (event.value - abs_x);
                            abs_x = event.value;
                        }
                        if (event.code == 1) {
                            y += (event.value - abs_y);
                            abs_y = event.value;
                        }
                    } else {
                        // digitizers and touch controls
                        // event.value contains absolute position
                        if (event.code == 0) x = event.value;
                        if (event.code == 1) y = event.value;
                    }

                    g_cursor.latest_x = x;
                    g_cursor.latest_y = y;
                    g_cursor.dirty = true;

                    break;
                }

                case EV_KEY:
                {
                    if (event.code == BTN_LEFT) {
                        left_mouse_pressed = event.value;
                    } else if(event.code == BTN_TOUCH) {
                        touch_down = event.value;
                        if(!event.value) {
                            abs_x = -1;
                            abs_y = -1;
                        }
                    }

                    xkb_keycode_t keycode = event.code + 8;
                    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, keycode);
                    if (event.value > 0) {
                        switch (keysym) {
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
                                    std::vector<char>buffer(size + 1);
                                    if (xkb_state_key_get_utf8(xkb_state, keycode, buffer.data(), buffer.size()) > 0) {
                                        Manager::GetInstance().ProcessTextInput(buffer.data());
                                    }

                                }
                        }

                    }

                    // always update the state
                    xkb_state_update_key(xkb_state, keycode, event.value ? XKB_KEY_DOWN : XKB_KEY_UP);
                    Manager::GetInstance().ProcessKeyInput(event.value, event.code);
                }
            }

            if (was_interrupted()) {
                should_run = false;
            }

            Manager::GetInstance().ProcessTouchPoint(left_mouse_pressed, x, y);
            ClampCursor();
        }
    }

    void LinuxNativeApp::DrawMouseIcon(bool flag) {
        draw_mouse_icon = flag;
    }

}

#endif
