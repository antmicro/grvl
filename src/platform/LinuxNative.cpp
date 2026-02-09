
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
    }

    LinuxNativeApp::~LinuxNativeApp()
    {
        CloseFiles(inputs, count);

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

            // relative mouse movement
            // event.value contains relative offset
            if (event.type == EV_REL) {
                if (event.code == 0) x += event.value;
                if (event.code == 1) y += event.value;
            }

            // digitizers and touch controls
            // event.value contains absolute position
            if (event.type == EV_ABS) {
                if (event.code == 0) x = event.value;
                if (event.code == 1) y = event.value;
            }

            // mouse and keyboard buttons
            // event.value contains 0, 1 or 2 (2 for "repeat")
            if (event.type == EV_KEY) {
                if (event.code == BTN_LEFT) {
                    left_mouse_pressed = event.value;
                }

                Manager::GetInstance().ProcessKeyInput(event.value != 0, event.code);
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
