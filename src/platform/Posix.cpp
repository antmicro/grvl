
#include <platform/PosixApp.h>


#include <cstdio>
#include <chrono>
#include <pthread.h>

// callbacks

static void PrintfNewline(const char* text, va_list argList)
{
    vprintf(text, argList);
    printf("\n");
}

static void* PosixMutexCreate()
{
    auto* mutex = new pthread_mutex_t();
    pthread_mutex_init(mutex, nullptr);
    return mutex;
}

static void PosixMutexDestroy(void* mutex)
{
    delete (pthread_mutex_t*) mutex;
}

static int PosixMuteLock(void* mutex)
{
    return pthread_mutex_lock((pthread_mutex_t*)mutex);
}

static void PosixMutexUnlock(void* mutex)
{
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

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
        callbacks.gui_printf = PrintfNewline;

        callbacks.mutex_create = PosixMutexCreate;
        callbacks.mutex_lock = PosixMuteLock;
        callbacks.mutex_unlock = PosixMutexUnlock;
        callbacks.mutex_destroy = PosixMutexDestroy;
        callbacks.get_timestamp = ChronoGetTimestamp;
    }

}
