#include <grvl/platform/PosixApp.h>
#include <grvl/Manager.h>

#include <chrono>
#include <cstdio>
#include <pthread.h>

// implementation

namespace grvl {

    void PosixApp::MarkUserActivity()
    {
        Manager::GetInstance().MarkUserActivity();
    }

    PosixApp::PosixApp(int width, int height, bool rotate_sideways)
        : Application(width, height, rotate_sideways)
    {
    }

    void PosixApp::SetCallbacks(gui_callbacks_t& callbacks)
    {
        Application::SetCallbacks(callbacks);
        callbacks.get_timestamp = PosixApp::CurrentTimestamp;
    }

    uint64_t PosixApp::CurrentTimestamp()
    {
        auto duration = std::chrono::steady_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    uint64_t PosixApp::GetLastUserActivityTimestamp() const
    {
        return Manager::GetInstance().GetLastInputTimestamp();
    }
}
