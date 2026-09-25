#include <grvl/platform/PosixApp.h>

#include <cstdio>
#include <chrono>
#include <pthread.h>

// callbacks

static uint64_t ChronoGetTimestamp()
{
    auto duration = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// implementation

namespace grvl {
    void PosixApp::MarkUserActivity()
    {
        last_user_activity_ms.store(ChronoGetTimestamp());
    }

    PosixApp::PosixApp(int width, int height, bool rotate_sideways)
        : Application(width, height, rotate_sideways)
    {
        MarkUserActivity();
    }

    void PosixApp::SetCallbacks(gui_callbacks_t& callbacks)
    {
        Application::SetCallbacks(callbacks);
        callbacks.get_timestamp = ChronoGetTimestamp;
    }


    uint64_t PosixApp::GetLastUserActivityTimestamp() const
    {
        return last_user_activity_ms.load();
    }
}
