#ifndef POSIXAPP_H_
#define POSIXAPP_H_

#include <atomic>

#include <grvl/platform/Application.h>

namespace grvl {

    class PosixApp : public Application {
    public:
        /// Returns the last user activity timestamp from std::chrono::steady_clock.
        /// The value is expressed in milliseconds and is set to "now" on start.
        uint64_t GetLastUserActivityTimestamp() const;

    protected:
        PosixApp(int width, int height, bool rotate_sideways);

        void SetCallbacks(gui_callbacks_t& callbacks) override;

        void MarkUserActivity();

    private:
        std::atomic<uint64_t> last_user_activity_ms { 0 };
    };
}

#endif // POSIXAPP_H_
