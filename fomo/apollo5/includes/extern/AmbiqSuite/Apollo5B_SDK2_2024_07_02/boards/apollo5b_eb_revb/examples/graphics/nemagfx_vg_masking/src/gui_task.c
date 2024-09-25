//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_vg_masking.h"

#include "nema_core.h"
#include "nema_event.h"
#include "nema_graphics.h"
#include "nema_utils.h"

#include "texture/boy_rgba565.h"
#include "texture/girl_rgba565.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
// Address of the textures in PSRAM
#define PSRAM_BOY_454x454_RGB565                                               \
    (((MSPI_XIP_BASE_ADDRESS + 8) >> 3) << 3)
#define PSRAM_GIRL_454x454_RGB565                                              \
    (((PSRAM_BOY_454x454_RGB565 + boy_rgba565_length + 8) >> 3) << 3)

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Semaphores used to sync Gui task with render task, display task.
//
//*****************************************************************************
SemaphoreHandle_t g_semDCStart  = NULL;
SemaphoreHandle_t g_semGPUStart = NULL;
SemaphoreHandle_t g_semDCEnd    = NULL;
SemaphoreHandle_t g_semGPUEnd   = NULL;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************
img_obj_t g_sMask      = {{0}, 454, 454, -1, 1, NEMA_A8, NEMA_FILTER_BL};
img_obj_t g_sBoyImage  = {{0}, 454, 454, -1, 1, NEMA_RGB565, NEMA_FILTER_BL};
img_obj_t g_sGirlImage = {{0}, 454, 454, -1, 1, NEMA_RGB565, NEMA_FILTER_BL};
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

// Frame buffer
img_obj_t g_sFrameBuffer = {{0}, FB_RESX,     FB_RESY,       -1,
                            1,   NEMA_RGB565, NEMA_FILTER_BL};

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************

//*****************************************************************************
//
// Allocate buffer in SSRAM using tsi_malloc
//
//*****************************************************************************
int SSRAM_buffer_alloc(img_obj_t *img)
{
    uint32_t size;

    size = nema_texture_size(img->format, 0, img->w, img->h);

    img->bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);

    if (img->bo.base_virt == NULL)
    {
        am_util_stdio_printf("TSI buffer Malloc failed!\n");
        return -1;
    }

    memset(img->bo.base_virt, 0, size);

    return 0;
}

//*****************************************************************************
//
// Init texture
//
//*****************************************************************************
int texture_framebuffer_init(void)
{
    int ret;

    // alloc textures buffer in psram
    uint32_t size = nema_texture_size(g_sMask.format, NEMA_TEX_MIRROR,
                                        g_sMask.w, g_sMask.h);
    g_sMask.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, size);

    g_sBoyImage.bo.base_phys = PSRAM_BOY_454x454_RGB565;
    g_sBoyImage.bo.base_virt = (void *)PSRAM_BOY_454x454_RGB565;
    g_sBoyImage.bo.size      = boy_rgba565_length;

    g_sGirlImage.bo.base_phys = PSRAM_GIRL_454x454_RGB565;
    g_sGirlImage.bo.base_virt = (void *)PSRAM_GIRL_454x454_RGB565;
    g_sGirlImage.bo.size      = girl_rgba565_length;

    // load image data
    nema_memcpy(g_sBoyImage.bo.base_virt, boy_rgba565, boy_rgba565_length);
    nema_memcpy(g_sGirlImage.bo.base_virt, girl_rgba565, girl_rgba565_length);

    // alloc framebuffer space in SSRAM
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer);
    if (ret < 0)
    {
        return ret;
    }

    return 0;
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void GuiTask(void *pvParameters)
{
    int ret;

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
    am_hal_gpio_pinconfig(
        DEBUG_PIN_1, am_hal_gpio_pincfg_output); // GPIODEBUG_PIN_1 is used to
                                                 // measure the GPU work time.
    am_hal_gpio_pinconfig(
        DEBUG_PIN_2, am_hal_gpio_pincfg_output); // GPIODEBUG_PIN_2 is used to
                                                 // measure the DC work time.
    am_hal_gpio_pinconfig(
        DEBUG_PIN_3, am_hal_gpio_pincfg_output); // GPIODEBUG_PIN_3 is used to
                                                 // measure the GPU sleep time.
    am_hal_gpio_pinconfig(
        DEBUG_PIN_4, am_hal_gpio_pincfg_output); // GPIODEBUG_PIN_4 is used to
                                                 // measure the DC sleep time.
    am_hal_gpio_pinconfig(
        DEBUG_PIN_5,
        am_hal_gpio_pincfg_output); // GPIODEBUG_PIN_5 is the DC start signal.
    am_hal_gpio_pinconfig(
        DEBUG_PIN_6,
        am_hal_gpio_pincfg_output); // GPIODEBUG_PIN_6 is the GPU start signal.
#endif

    // Allocate memory for framebuffer and textures
    ret = texture_framebuffer_init();
    if (ret < 0)
    {
        am_util_stdio_printf("frame buffer init failed!\n");
        vTaskDelete(NULL);
    }

    am_util_stdio_printf("GUI task start!\n");

    while (1)
    {
        // Start GPU
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_6);
#endif
        xSemaphoreGive(g_semGPUStart);

        // wait GPU
        xSemaphoreTake(g_semGPUEnd, portMAX_DELAY);

        // Start DC/DSI
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_5);
#endif
        xSemaphoreGive(g_semDCStart);

        // wait DC
        xSemaphoreTake(g_semDCEnd, portMAX_DELAY);
    }
}
