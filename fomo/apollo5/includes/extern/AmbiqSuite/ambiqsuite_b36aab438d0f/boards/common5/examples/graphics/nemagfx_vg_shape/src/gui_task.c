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
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_vg_shape.h"

#include "texture/ref.rgba.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
// Address of the textures in PSRAM
#define PSRAM_REF_80x100_RGBA (MSPI_XIP_BASE_ADDRESS + 0)

#define PAINT_TEXTURE_W (80)
#define PAINT_TEXTURE_H (100)

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
SemaphoreHandle_t g_semDCStart = NULL;
SemaphoreHandle_t g_semGPUStart = NULL;
SemaphoreHandle_t g_semDCEnd = NULL;
SemaphoreHandle_t g_semGPUEnd = NULL;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************
img_obj_t g_sRef = {{0}, PAINT_TEXTURE_W, PAINT_TEXTURE_H, -1,
                    0,   NEMA_RGBA8888, NEMA_FILTER_BL};

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

// Frame buffer
static img_obj_t g_sFrameBuffer[2] = {
    {{0}, FB_RESX, FB_RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_BL},
    {{0}, FB_RESX, FB_RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_BL},
};

// Pointer to frame buffer
img_obj_t *g_pFrameBufferGPU = &g_sFrameBuffer[0];
img_obj_t *g_pFrameBufferDC = &g_sFrameBuffer[1];

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern void *g_pPsramHandle;
extern void *g_pMSPIPsramHandle;

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
int framebufferCreate(void)
{
    int ret;

    // alloc textures buffer in psram
    g_sRef.bo.base_phys = PSRAM_REF_80x100_RGBA;
    g_sRef.bo.base_virt = (void *)PSRAM_REF_80x100_RGBA;
    g_sRef.bo.size = ref_rgba_len;

    // load image data
    nema_memcpy(g_sRef.bo.base_virt, ref_rgba, ref_rgba_len);

    // alloc framebuffer space in SSRAM
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[0]);
    if (ret < 0)
    {
        return ret;
    }

    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[1]);
    if (ret < 0)
    {
        return ret;
    }

    return 0;
}

static void framebufferDestroy(void)
{
    //Destroy memory objects
    nema_buffer_unmap(&g_sFrameBuffer[0].bo);
    nema_buffer_destroy(&g_sFrameBuffer[0].bo);

    nema_buffer_unmap(&g_sFrameBuffer[1].bo);
    nema_buffer_destroy(&g_sFrameBuffer[1].bo);
}

//*****************************************************************************
//
// Swap framebuffer
//
//*****************************************************************************
void frameBuffer_swap(void)
{
    img_obj_t *temp;

    temp = g_pFrameBufferGPU;
    g_pFrameBufferGPU = g_pFrameBufferDC;
    g_pFrameBufferDC = temp;
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
    ret = framebufferCreate();
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

        // Start DC/DSI
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_5);
#endif
        xSemaphoreGive(g_semDCStart);

        // wait GPU
        xSemaphoreTake(g_semGPUEnd, portMAX_DELAY);

        // wait DC
        xSemaphoreTake(g_semDCEnd, portMAX_DELAY);

        // Swap framebuffer
        frameBuffer_swap();
    }

    framebufferDestroy();
}
