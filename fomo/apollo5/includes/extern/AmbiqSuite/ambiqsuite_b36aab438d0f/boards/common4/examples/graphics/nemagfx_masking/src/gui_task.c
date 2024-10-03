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
#include "nemagfx_masking.h"

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_graphics.h"

#include "psram.h"

#include "texture/mask_a8.h"
#include "texture/scotty_morton_rgba565.h"
#include "texture/scotty_xmas_morton_rgba565.h"
#include "texture/scotty_rgba565.h"
#include "texture/scotty_xmas_rgba565.h"
#include "texture/ambiq_240x76_rgba332.h"
#include "texture/mask240x76_a8.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//Address of the textures in PSRAM
#define PSRAM_MASK_454x454_A8                   (MSPI_XIP_BASE_ADDRESS + 0)
#define PSRAM_SCOTTY_454x454_RGB565             (((PSRAM_MASK_454x454_A8 + mask_a8_length + 8) >> 3) << 3)
#define PSRAM_SCOTTY_XMAS_454x454_RGB565        (((PSRAM_SCOTTY_454x454_RGB565 + scotty_rgba565_length + 8) >> 3) << 3)
#define PSRAM_SCOTTY_MORTON_454x454_RGB565      (((PSRAM_SCOTTY_XMAS_454x454_RGB565 + scotty_xmas_rgba565_length + 8) >> 3) << 3)
#define PSRAM_SCOTTY_XMAS_MORTON_454x454_RGB565        (((PSRAM_SCOTTY_MORTON_454x454_RGB565 + scotty_morton_rgba565_length + 8) >> 3) << 3)
#define PSRAM_AMBIQ_LOGO_240x76_RGB332          (((PSRAM_SCOTTY_XMAS_MORTON_454x454_RGB565 + scotty_xmas_morton_rgba565_length + 8) >> 3) << 3)
#define PSRAM_MASK_AMBIQ_LOGO_240x76_A8         (((PSRAM_AMBIQ_LOGO_240x76_RGB332 + ambiq_240x76_rgba332_length + 8) >> 3) << 3)

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
img_obj_t g_sMask =                     {{0},  454, 454, -1, 1, NEMA_A8,     NEMA_FILTER_BL};
img_obj_t g_sScottyImage =              {{0},  454, 454, -1, 1, NEMA_RGB565, NEMA_FILTER_BL};
img_obj_t g_sScottyXmasImage =          {{0},  454, 454, -1, 1, NEMA_RGB565, NEMA_FILTER_BL};
img_obj_t g_sScottyMortonImage =        {{0},  454, 454, -1, 1, NEMA_RGB565, NEMA_FILTER_BL | NEMA_TEX_MORTON_ORDER};
img_obj_t g_sScottyXmasMortonImage =    {{0},  454, 454, -1, 1, NEMA_RGB565, NEMA_FILTER_BL | NEMA_TEX_MORTON_ORDER};
img_obj_t g_sAmbiqLogo =                {{0},  240,  76, -1, 1, NEMA_RGB332, NEMA_FILTER_BL};
img_obj_t g_sMaskAmbiqLogo =            {{0},  240,  76, -1, 1, NEMA_A8    , NEMA_FILTER_BL};

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//Frame buffer
static img_obj_t g_sFrameBuffer[2] =
{
    {{0},  FB_RESX, FB_RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_BL},
    {{0},  FB_RESX, FB_RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_BL},
};

//Pointer to frame buffer
img_obj_t* g_pFrameBufferA = &g_sFrameBuffer[0];
img_obj_t* g_pFrameBufferB = &g_sFrameBuffer[1];

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
int
SSRAM_buffer_alloc(img_obj_t* img)
{
    uint32_t size;

    size = nema_texture_size(img->format, 0, img->w, img->h);

    img->bo = nema_buffer_create(size);

    if ( img->bo.base_virt == NULL )
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
int
texture_framebuffer_init(void)
{
    int ret;

    // alloc textures buffer in psram
    g_sMask.bo.base_phys = PSRAM_MASK_454x454_A8;
    g_sMask.bo.base_virt = (void*)PSRAM_MASK_454x454_A8;
    g_sMask.bo.size = mask_a8_length;

    g_sScottyImage.bo.base_phys = PSRAM_SCOTTY_454x454_RGB565;
    g_sScottyImage.bo.base_virt = (void*)PSRAM_SCOTTY_454x454_RGB565;
    g_sScottyImage.bo.size = scotty_rgba565_length;

    g_sScottyXmasImage.bo.base_phys = PSRAM_SCOTTY_XMAS_454x454_RGB565;
    g_sScottyXmasImage.bo.base_virt = (void*)PSRAM_SCOTTY_XMAS_454x454_RGB565;
    g_sScottyXmasImage.bo.size = scotty_xmas_rgba565_length;

    g_sScottyMortonImage.bo.base_phys = PSRAM_SCOTTY_MORTON_454x454_RGB565;
    g_sScottyMortonImage.bo.base_virt = (void*)PSRAM_SCOTTY_MORTON_454x454_RGB565;
    g_sScottyMortonImage.bo.size = scotty_morton_rgba565_length;

    g_sScottyXmasMortonImage.bo.base_phys = PSRAM_SCOTTY_XMAS_MORTON_454x454_RGB565;
    g_sScottyXmasMortonImage.bo.base_virt = (void*)PSRAM_SCOTTY_XMAS_MORTON_454x454_RGB565;
    g_sScottyXmasMortonImage.bo.size = scotty_xmas_morton_rgba565_length;

    g_sAmbiqLogo.bo.base_phys = PSRAM_AMBIQ_LOGO_240x76_RGB332;
    g_sAmbiqLogo.bo.base_virt = (void*)PSRAM_AMBIQ_LOGO_240x76_RGB332;
    g_sAmbiqLogo.bo.size = ambiq_240x76_rgba332_length;

    g_sMaskAmbiqLogo.bo.base_phys = PSRAM_MASK_AMBIQ_LOGO_240x76_A8;
    g_sMaskAmbiqLogo.bo.base_virt = (void*)PSRAM_MASK_AMBIQ_LOGO_240x76_A8;
    g_sMaskAmbiqLogo.bo.size = mask240x76_a8_length;

    // load image data
    memcpy(g_sMask.bo.base_virt, mask_a8, mask_a8_length);
    memcpy(g_sScottyImage.bo.base_virt, scotty_rgba565, scotty_rgba565_length);
    memcpy(g_sScottyXmasImage.bo.base_virt, scotty_xmas_rgba565, scotty_xmas_rgba565_length);
    memcpy(g_sScottyMortonImage.bo.base_virt, scotty_morton_rgba565, scotty_morton_rgba565_length);
    memcpy(g_sScottyXmasMortonImage.bo.base_virt, scotty_xmas_morton_rgba565, scotty_xmas_morton_rgba565_length);
    memcpy(g_sAmbiqLogo.bo.base_virt, ambiq_240x76_rgba332, ambiq_240x76_rgba332_length);
    memcpy(g_sMaskAmbiqLogo.bo.base_virt, mask240x76_a8, mask240x76_a8_length);

    // alloc framebuffer space in SSRAM
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[0]);
    if ( ret < 0 )
    {
        return ret;
    }

    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[1]);
    if ( ret < 0 )
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
void
GuiTask(void *pvParameters)
{
    int ret;

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output); //GPIODEBUG_PIN_1 is used to measure the GPU work time.
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output); //GPIODEBUG_PIN_2 is used to measure the DC work time.
    am_hal_gpio_pinconfig(DEBUG_PIN_3, am_hal_gpio_pincfg_output); //GPIODEBUG_PIN_3 is used to measure the GPU sleep time.
    am_hal_gpio_pinconfig(DEBUG_PIN_4, am_hal_gpio_pincfg_output); //GPIODEBUG_PIN_4 is used to measure the DC sleep time.
    am_hal_gpio_pinconfig(DEBUG_PIN_5, am_hal_gpio_pincfg_output); //GPIODEBUG_PIN_5 is the DC start signal.
    am_hal_gpio_pinconfig(DEBUG_PIN_6, am_hal_gpio_pincfg_output); //GPIODEBUG_PIN_6 is the GPU start signal.
#endif

    // Allocate memory for framebuffer and textures
    ret = texture_framebuffer_init();
    if ( ret < 0 )
    {
        am_util_stdio_printf("frame buffer init failed!\n");
        vTaskDelete(NULL);
    }

    am_util_stdio_printf("GUI task start!\n");

    while (1)
    {
        g_ui32FrameTime = xTaskGetTickCount() % SWITCH_SCALE_ROTATE;

        //Start GPU
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_6);
#endif
        xSemaphoreGive(g_semGPUStart);

        //wait GPU
        xSemaphoreTake( g_semGPUEnd, portMAX_DELAY);

        //Start DC/DSI
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_5);
#endif
        xSemaphoreGive(g_semDCStart);

        //wait DC
        xSemaphoreTake( g_semDCEnd, portMAX_DELAY);

    }
}

