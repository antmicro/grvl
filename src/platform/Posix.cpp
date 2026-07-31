#include <grvl/platform/PosixApp.h>

#include <cstdio>
#include <chrono>
#include <pthread.h>

// callbacks

static uint64_t ChronoGetTimestamp()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// implementation

namespace grvl {

    PosixApp::PosixApp(int width, int height, bool rotate_sideways)
        : Application(width, height, rotate_sideways)
    {
    }

    void PosixApp::SetCallbacks(gui_callbacks_t& callbacks)
    {
        Application::SetCallbacks(callbacks);
        callbacks.get_timestamp = ChronoGetTimestamp;
    }

}
