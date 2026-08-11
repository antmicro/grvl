#include <cstdint>
#include <cstdlib>
#include <ctime>

#include <grvl/grvl.h>
#include <grvl/Manager.h>

#include <grvl/platform/Application.h>
#include <grvl/Manager.h>

#ifndef ROMFS_PATH
#   define ROMFS_PATH "/romfs"
#endif

void run(grvl::Application& app)
{
    grvl::Application::Init(&app);

    auto mgr = &grvl::Manager::GetInstance();

    mgr->AddFontToFontContainer("normal", new grvl::GrvlBakedFont(ROMFS_PATH "/fonts/mona16.gbf"));
    mgr->perf.overlay = grvl::Performance::FULL;

    mgr->BuildFromXML(ROMFS_PATH "/empty.xml");
    mgr->InitializationFinished();
    mgr->SetActiveScreen("home", 0);

    while (app.ShouldRun()) {
        app.Render();
        app.Swap();
        app.Poll();
    }
}

#ifdef __ZEPHYR__
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include <zephyr/sys/errno_private.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#include <zephyr/fs/ext2.h>
#include <zephyr/fs/fs.h>

#include <grvl/platform/ZephyrApp.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(calendar, LOG_LEVEL_INF);

static constexpr auto GRVL_THRD_STACK_SIZE = KB(10);
static constexpr auto GRVL_THRD_PRIORITY = 1;

static void setup_romfs()
{
    int rc;
    static struct fs_mount_t mount;

    // this struct needs to stay static - it must not go out of scope
    // while we are still using the filesystem.
    mount.type = FS_EXT2;
    mount.mnt_point = ROMFS_PATH;
    mount.storage_dev = (void*) "SD";
    mount.flags = FS_MOUNT_FLAG_NO_FORMAT | FS_MOUNT_FLAG_READ_ONLY;

    if (rc = fs_mount(&mount)) {
        LOG_ERR("Failed to mount sdcard (err: %d)", rc);
    }
}

static void grvl_thread(void *a1, void *a2, void *a3)
{
    const device* display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    setup_romfs();
    grvl::ZephyrApp app {display};

    run(app);

    LOG_ERR("How did we get here?");
    k_sleep(K_FOREVER);
}

K_THREAD_DEFINE(grvl_tid, GRVL_THRD_STACK_SIZE, grvl_thread, nullptr, nullptr, nullptr, GRVL_THRD_PRIORITY, 0, 0);

#else /* LINUX */

#include <grvl/platform/LinuxGenericApp.h>

int main()
{
    grvl::Application* app = grvl::CreateGenericLinuxApp(940, 800);

    run(*app);

    delete app;
}

#endif
