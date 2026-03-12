# Quickstart

## Linking (CMAKE)

### Standalone

Pull grvl and link it in your project:

```cmake
add_subdirectory(GRVL_SOURCE_DIR)
target_link_libraries(app PRIVATE grvl)
```

### Zephyr

```cmake
set(GRVL_ZEPHYR ON)
add_subdirectory(GRVL_SOURCE_DIR)
target_link_libraries(app PRIVATE grvl)
```

## Minimal example

```cpp
#include <grvl.h>
#include <Manager.h>
...
int main(){
    ...
    // initialize callbacks
    gui_callbacks_t callbacks;
    callbacks.malloc = malloc;
    callbacks.free = free;
    callbacks.dma_operation = dma_operation;
    callbacks.dma_operation_clt = dma_operation_clt;
    callbacks.dma_fill = dma_fill;
    callbacks.wait_for_vsync = wait_for_vsync;
    callbacks.flipping_completed = flipping_completed;
    callbacks.set_layer_pointer = set_layer_poiner;
    callbacks.gui_printf = gui_printf;
    callbacks.mutex_create = mutex_create;
    callbacks.mutex_lock = mutex_lock;
    callbacks.mutex_unlock = mutex_unlock;
    callbacks.mutex_destroy = mutex_destroy;
    callbacks.get_timestamp = get_timestamp;
    grvl::Init(&callbacks);

    // initalize manager
    Manager::Initialize(WIDTH, HEIGHT, BPP, IS_FLIPPED);

    // fill the gui media
    Manager::GetInstance()
        .AddFontToFontContainer(FONT_NAME_USED_IN_XML, new Font(FONT_PATH))
        .AddImageContentToContainer(IMAGE_NAME_USED_IN_XML, new ImageContent(ImageContent::FromPNG(IMAGE_PATH)))
        .AddCallbackToContainer(CALLBACK_NAME_USED_IN_XML, (grvl::Event::CallbackPointer) callback_function)
        .BuildFromXML(XML_PATH)
        .InitializationFinished();

    while(running){
        // update gui
        Manager::GetInstance().MainLoopIteration();
        ...
        platform_specific_wait(); // e.g. SDL_WaitEventTimeout or k_msleep
        ...

        // update specific components
        ProgressBar* progressBar = dynamic_cast<ProgressBar*>(displayManager->GetActiveScreen()->GetElement("progress_bar"));
        progressBar->SetProgressValue(counter % 100);
        ...
        // handle events
        Manager::GetInstance().ProcessTouchPoint(state, x, y);
    }
}
```

For more in-detail examples see [the Examples chapter](examples.md).
