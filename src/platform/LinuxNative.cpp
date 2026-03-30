
#if defined(GRVL_LINUX_NATIVE_SUPPORT)

#include <Application.h>
#include <Manager.h>

#include <core/framebuffer.hpp>
#include <core/drm.hpp>
#include <core/interrupt.hpp>

#include <regex>
#include <filesystem>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <cstdlib>

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

void CloseFiles(int* fds, int count) {
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

static bool ReadEvent(struct input_event* event, const int* fds, int file_count, bool wait)
{
    const int fd = FindReadable(fds, file_count, wait);

    if (fd == -1) {
        return false;
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
        // @TODO: make configurable
        struct xkb_rule_names names = {
            .rules = NULL,
            .model = "pc105",
            .layout = "pl",
            .variant = "",
            .options = NULL
        };
        xkb_keymap = xkb_keymap_new_from_names(xkb_ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
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

    void LinuxNativeApp::DrawSimpleMouse()
    {
        int bw = 12;
        int bh = 16;
        uint64_t bitmap[16] = {
            0b100000000000,
            0b110000000000,
            0b111000000000,
            0b111100000000,
            0b111110000000,
            0b111111000000,
            0b111111100000,
            0b111111110000,
            0b111111111000,
            0b111111111100,
            0b111111111110,
            0b111111111111,
            0b111111111100,
            0b111111100000,
            0b111110000000,
            0b110000000000,
        };

        const int max_x = std::min(x + bw, width - 1);
        const int max_y = std::min(y + bh, height - 1);

        for (int by = 0, iy = y; iy < max_y; iy ++) {
            for (int bx = 0, ix = x; ix < max_x; ix ++) {
                bool bit = bitmap[by] & (1 << (bw - bx));

                if (bit) {
                    bgra_buffer[ix + width * iy] = 0xFFFFFFFF;
                }

                bx ++;
            }

            by ++;
        }
    }

    bool LinuxNativeApp::Setup()
    {
        if (geteuid() != 0) {
            printf("This application requires root privelages to access kernel APIs!\n");
            return false;
        }

        output = new drm_screen("");

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

        Manager::Initialize(width, height, 4, sideways);
        setup_interrupt_handlers();
        return true;
    }

    void LinuxNativeApp::Render()
    {
        Manager::GetInstance().MainLoopIteration();
        UpdateBgraBuffer();

        image img {(uint8_t*) bgra_buffer, width, height};

        // center vertically and horizontally
        img.sx = 0.5;
        img.sy = 0.5;

        if (draw_mouse_icon) {
            DrawSimpleMouse();
        }

        output->blit(img);
        usleep(4000);
    }

    void LinuxNativeApp::Poll()
    {
        struct input_event event;

        while (ReadEvent(&event, inputs, count, false)) {

            switch (event.type) {
                case EV_REL:
                {
                    // relative mouse movement
                    // event.value contains relative offset
                    if (event.code == 0) x += event.value;
                    if (event.code == 1) y += event.value;
                    break;
                }

                case EV_ABS:
                {
                    // digitizers and touch controls
                    // event.value contains absolute position
                    if (event.code == 0) x = event.value;
                    if (event.code == 1) y = event.value;
                    break;
                }

                case EV_KEY:
                {
                    if (event.code == BTN_LEFT) {
                        left_mouse_pressed = event.value;
                    }
                    
                    xkb_keycode_t keycode = event.code + 8;
                    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, keycode);
                    if (event.value > 0) {
                        switch (keysym) {
                            case XKB_KEY_BackSpace: Manager::GetInstance().ProcessBackspace(); break;
                            case XKB_KEY_Return: Manager::GetInstance().ProcessEnter(); break;

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
                                // @TODO: Manager::GetInstance().....
                                break;

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
                                // @TODO: Manager::GetInstance().ProcessFunctionKey(kysym - XKB_KEY_F1 + 1);
                                break;

                            default:
                                char* buffer;
                                size_t size = xkb_state_key_get_utf8(xkb_state, keycode, nullptr, 0) + 1;
                                if (size > 0) {
                                    buffer = (char*)malloc(size);
                                    if (buffer) {
                                        if (xkb_state_key_get_utf8(xkb_state, keycode, buffer, size) > 0) {
                                            Manager::GetInstance().ProcessTextInput(buffer);
                                        }
                                        free(buffer);
                                    }
                                    
                                }
                        }
                        
                    }
                    
                    // always update the state
                    xkb_state_update_key(xkb_state, keycode, event.value ? XKB_KEY_DOWN : XKB_KEY_UP);
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
