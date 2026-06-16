
#include <grvl/grvl.h>

#if CONFIG_GRVL_USE_STM32_DMA
#include <grvl/platform/Dma2d.h>
#include <zephyr/logging/log.h>
#include <grvl/Blitter.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <vector>

#include <soc.h>

LOG_MODULE_REGISTER(dma2d, CONFIG_APP_LOG_LEVEL);

static DMA2D_HandleTypeDef hal_dma2d;
static bool dma_in_progress = true;
static int cluts[2] = {0};

static void Dma2dInitClut(int layer, int length)
{
    int rc;

    // attempts to disable the CLUT can be safely ignored
    if (length == 0) {
        return;
    }

    // if we already have the desired grayscale lookup loaded for this layer we can do nothing
    if (cluts[layer] == length) {
        return;
    }

    std::vector<uint32_t> grayscale;
    grayscale.resize(length);

    int step = 256 / length;

    DMA2D_CLUTCfgTypeDef clut;
    clut.pCLUT = grayscale.data();
    clut.CLUTColorMode = DMA2D_CCM_ARGB8888;
    clut.Size = length - 1;

    for (uint32_t i = 0; i < length; i ++) {
        uint32_t val = i * step;
        grayscale[i] = val | (val << 8u) | (val << 16u) | (0xff << 24);
    }

    if ((rc = HAL_DMA2D_CLUTLoad(&hal_dma2d, clut, layer)) != HAL_OK) {
        LOG_ERR("InitClut: Failed to configure CLUT for layer %d (err %d)\n", layer, HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    if (HAL_DMA2D_PollForTransfer(&hal_dma2d, 100) != HAL_OK) {
        LOG_ERR("InitClut: Failed to transfer CLUT for layer %d (err %d)\n", layer, HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    LOG_DBG("InitClut: Created CLUT for layer %d of size %d", layer, length);
    cluts[layer] = length;
}

static int Dma2dGetClutSize(grvl::Format format)
{
    if (format == grvl::Format::L8) return 256;
    if (format == grvl::Format::AL88) return 256;
    if (format == grvl::Format::AL44) return 16;

    return 0;
}

static void Dma2dWaitIdle(const char* where)
{
    if (dma_in_progress) {
        if (HAL_DMA2D_PollForTransfer(&hal_dma2d, 100) != HAL_OK) {
            LOG_ERR("WaitIdle: DMA transfer timeout in %s (err %d)", where, HAL_DMA2D_GetError(&hal_dma2d));
            return;
        }
    }

    dma_in_progress = false;
}

static void Dma2dBlit(uintptr_t fg_mem, uintptr_t bg_mem, uintptr_t out_mem,
                 uint32_t width, uint32_t height, uint32_t fg_off, uint32_t bg_off, uint32_t out_off,
                 grvl::Format fg_fmt, grvl::Format bg_fmt, grvl::Format out_fmt, uint32_t fnt_alpha)
{
    int rc;

    hal_dma2d.Init = {
        .Mode = bg_mem ? DMA2D_M2M_BLEND : DMA2D_M2M_PFC,
        .ColorMode = grvl::FormatToDma2d(out_fmt),
        .OutputOffset = out_off,
    };

    hal_dma2d.LayerCfg[0] = {
        .InputOffset = bg_off,
        .InputColorMode = grvl::FormatToDma2d(bg_fmt),
        .AlphaMode = DMA2D_NO_MODIF_ALPHA,
        .InputAlpha = 0xFF,
    };

    hal_dma2d.LayerCfg[1] = {
        .InputOffset = fg_off,
        .InputColorMode = grvl::FormatToDma2d(fg_fmt),
        .AlphaMode = DMA2D_NO_MODIF_ALPHA,
        .InputAlpha = fnt_alpha,
    };

    Dma2dWaitIdle("DmaBlit");

    Dma2dInitClut(0, Dma2dGetClutSize(bg_fmt));
    Dma2dInitClut(1, Dma2dGetClutSize(fg_fmt));

    /* DMA2D Initialization */
    if ((rc = HAL_DMA2D_Init(&hal_dma2d)) != HAL_OK) {
        LOG_ERR("DmaBlit: Failed to initialize DMA transfer (err %d)", HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    if ((rc = HAL_DMA2D_ConfigLayer(&hal_dma2d, 0)) != HAL_OK) {
        LOG_ERR("DmaBlit: Failed to configure DMA layer (err %d)", HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    if ((rc = HAL_DMA2D_ConfigLayer(&hal_dma2d, 1)) != HAL_OK) {
        LOG_ERR("DmaBlit: Failed to configure DMA layer (err %d)", HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    if ((rc = HAL_DMA2D_BlendingStart(&hal_dma2d, fg_mem, bg_mem, out_mem, width, height)) != HAL_OK) {
        LOG_ERR("DmaBlit: Failed to start DMA transfer (err %d)", HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    dma_in_progress = true;
}

static void Dma2dFill(uintptr_t out_mem, uint32_t width, uint32_t height, uint32_t off, uint32_t col, grvl::Format fmt)
{
    int rc;

    hal_dma2d.Init = {
        .Mode = DMA2D_R2M,
        .ColorMode = grvl::FormatToDma2d(fmt),
        .OutputOffset = off,
    };

    Dma2dWaitIdle("DmaFill");

    if ((rc = HAL_DMA2D_Init(&hal_dma2d)) != HAL_OK) {
        LOG_ERR("DmaFill: Failed to initialize DMA transfer (err=%d)", HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    if ((rc = HAL_DMA2D_Start(&hal_dma2d, col, out_mem, width, height)) != HAL_OK) {
        LOG_ERR("DmaFill: Failed to start DMA transfer (err=%d)", HAL_DMA2D_GetError(&hal_dma2d));
        return;
    }

    dma_in_progress = true;
}

namespace grvl {

    bool Dma2dInit()
    {
        __HAL_RCC_DMA2D_CLK_ENABLE();

        hal_dma2d.Instance = DMA2D;
        hal_dma2d.XferCpltCallback = nullptr;
        return true;
    }

    void Dma2dSetCallbacks(gui_callbacks_t& callbacks)
    {
        callbacks.fill = Dma2dFill;
        callbacks.blit = Dma2dBlit;
    }

}
#else
namespace grvl {

    bool Dma2dInit()
    {
        return true;
    }

    void Dma2dSetCallbacks(gui_callbacks_t& callbacks)
    {
        // do nothing
    }

}
#endif
