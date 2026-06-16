#ifndef DMA2D_H_
#define DMA2D_H_

#include <grvl/grvl.h>

namespace grvl {

    bool Dma2dInit();
    void Dma2dSetCallbacks(gui_callbacks_t& callbacks);

}

#endif
