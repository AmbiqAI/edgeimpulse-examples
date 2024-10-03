//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
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
#include "nemagfx_enhanced_stress_test.h"

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"

#include "w0_360x360_rgba565.h"
#include "w0_360x196_rgba4444.h"
#include "w1_28x125_rgba.h"
#include "w2_28x174_rgba.h"
#include "w3_21x182_rgba.h"

#if defined( AM_PART_APOLLO4P ) || defined( AM_PART_APOLLO4L )
#define FB_FORMAT NEMA_RGB24
#pragma message("FB_FORMAT = NEMA_RGB24")
#else
#define FB_FORMAT NEMA_RGBA8888
#pragma message("FB_FORMAT = NEMA_RGBA8888")
#endif  //defined( AM_PART_APOLLO4P ) || defined( AM_PART_APOLLO4L )

#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)

#if (FB_FORMAT == NEMA_RGB24)
#define DC_LAYER_FORMAT NEMADC_RGB24
#pragma message("DC_LAYER_FORMAT = NEMADC_RGB24")
#elif (FB_FORMAT == NEMA_RGBA8888)
#define DC_LAYER_FORMAT NEMADC_RGBA8888
#pragma message("DC_LAYER_FORMAT = NEMADC_RGBA8888")
#else
#error unsupported DC layer format
#endif

#endif //(DISP_CTRL_IP == DISP_CTRL_IP_DC)
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//Address of the textures in PSRAM
#define PSRAM_W3_21x182_RGBA       (PSRAM_XIP_BASE_ADDRESS + PSRAM_TEXTURE_AREA_OFFSET)
#define PSRAM_W2_28x174_RGBA       (((PSRAM_W3_21x182_RGBA + w3_21x182_rgba_length + 8) >> 3) << 3)
#define PSRAM_W1_28x125_RGBA       (((PSRAM_W2_28x174_RGBA + w2_28x174_rgba_length + 8) >> 3) << 3)
#define PSRAM_W0_360x360_RGBA565   (((PSRAM_W1_28x125_RGBA + w1_28x125_rgba_length + 8) >> 3) << 3)
#define PSRAM_W0_360x196_RGBA4444  (((PSRAM_W0_360x360_RGBA565 + w0_360x360_rgba565_length + 8) >> 3) << 3)

//Address of the textures in PSRAM without XIP mapping
#define PSRAM_W3_21x182_RGBA_UNMAPED       (PSRAM_TEXTURE_AREA_OFFSET)
#define PSRAM_W2_28x174_RGBA_UNMAPED       (((PSRAM_W3_21x182_RGBA_UNMAPED + w3_21x182_rgba_length + 8) >> 3) << 3)
#define PSRAM_W1_28x125_RGBA_UNMAPED       (((PSRAM_W2_28x174_RGBA_UNMAPED + w2_28x174_rgba_length + 8) >> 3) << 3)
#define PSRAM_W0_360x360_RGBA565_UNMAPED   (((PSRAM_W1_28x125_RGBA_UNMAPED + w1_28x125_rgba_length + 8) >> 3) << 3)
#define PSRAM_W0_360x196_RGBA4444_UNMAPED  (((PSRAM_W0_360x360_RGBA565_UNMAPED + w0_360x360_rgba565_length + 8) >> 3) << 3)

//DSP ram region
#define DSP0_IRAM_ADDRESS (0x10160000)
#define DSP0_DRAM_ADDRESS (0x10180000)
#define DSP1_IRAM_ADDRESS (0x101C0000)
#define DSP1_DRAM_ADDRESS (0x101D0000)

//MSPI DMA timeout
#define PSRAM_DMA_TIMEOUT (100UL)

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
// Global Variables
//
//*****************************************************************************
//Textures
img_obj_t g_sHourHand =       {{0},  28, 125, -1, 1, NEMA_RGBA8888, NEMA_FILTER_BL};
img_obj_t g_sMinHand =        {{0},  28, 174, -1, 1, NEMA_RGBA8888, NEMA_FILTER_BL};
img_obj_t g_sSecHand =       {{0},  21, 182, -1, 1, NEMA_RGBA8888, NEMA_FILTER_BL};
//static img_obj_t g_sClockDial =   {{0},  360, 360, -1, 1, NEMA_RGB565, NEMA_FILTER_BL};
//static img_obj_t g_sSportsDial =  {{0},  360, 196, -1, 1, NEMA_RGBA4444, NEMA_FILTER_BL};

//Frame buffer
static img_obj_t g_sFrameBuffer[2] =
{
    {{0},  FB_RESX, FB_RESY / SMALLFB_STRIPES, -1, 1, FB_FORMAT, NEMA_FILTER_BL},
    {{0},  FB_RESX, FB_RESY / SMALLFB_STRIPES, -1, 1, FB_FORMAT, NEMA_FILTER_BL},
};

static img_obj_t g_sClockDialBuffer[2] =
{
    {{0},  FB_RESX, FB_RESY / SMALLFB_STRIPES, -1, 1, NEMA_RGB565, NEMA_FILTER_BL},
    {{0},  FB_RESX, FB_RESY / SMALLFB_STRIPES, -1, 1, NEMA_RGB565, NEMA_FILTER_BL},
};

static img_obj_t g_sSportsDialBuffer[2] =
{
    {{0},  FB_RESX, FB_RESY / SMALLFB_STRIPES, -1, 1, NEMA_RGBA4444, NEMA_FILTER_BL},
    {{0},  FB_RESX, FB_RESY / SMALLFB_STRIPES, -1, 1, NEMA_RGBA4444, NEMA_FILTER_BL},
};

//Pointer to frame buffer
img_obj_t* g_pFrameBufferGPU = &g_sFrameBuffer[0];
img_obj_t* g_pFrameBufferDC = &g_sFrameBuffer[1];

//Pointer to texture buffer
img_obj_t* g_pClockDialBufferDMA = &g_sClockDialBuffer[0];
img_obj_t* g_pClockDialBufferGPU = &g_sClockDialBuffer[1];

img_obj_t* g_pSportsDialBufferDMA = &g_sSportsDialBuffer[0];
img_obj_t* g_pSportsDialBufferGPU = &g_sSportsDialBuffer[1];

//flag to sync DMA finish
static SemaphoreHandle_t g_semDMAFinish = NULL;

//Each frame is divided into 4 parts, use the following varable to control
//the render_task and display_task to render or display the correct frame part.
int g_intTexturePart = -1;
int g_intRenderPart = -2;
int g_intDisplayPart = -3;

#ifdef USE_CRC_CHECK
//GPU CRC value
uint32_t g_GPUChecksumTable[SMALLFB_STRIPES] =
{
    0x48d3caab,
    0x18a5d905,
    0x6ba376e6,
    0x8c670bfe,
};
#endif

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern void *g_pPsramHandle;
extern void *g_pMSPIPsramHandle;
extern mspi_psram_device_func_t mspi_psram_device_func;
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

    // load textures to PSRAM
    memcpy((void*)PSRAM_W3_21x182_RGBA,      w3_21x182_rgba,      w3_21x182_rgba_length);
    memcpy((void*)PSRAM_W2_28x174_RGBA,      w2_28x174_rgba,      w2_28x174_rgba_length);
    memcpy((void*)PSRAM_W1_28x125_RGBA,      w1_28x125_rgba,      w1_28x125_rgba_length);
    memcpy((void*)PSRAM_W0_360x360_RGBA565,  w0_360x360_rgba565,  w0_360x360_rgba565_length);
    memcpy((void*)PSRAM_W0_360x196_RGBA4444, w0_360x196_rgba4444, w0_360x196_rgba4444_length);

    // alloc textures buffer in psram
    g_sSecHand.bo.base_phys = PSRAM_W3_21x182_RGBA;
    g_sSecHand.bo.base_virt = (void*)PSRAM_W3_21x182_RGBA;
    g_sSecHand.bo.size = w3_21x182_rgba_length;

    g_sMinHand.bo.base_phys = PSRAM_W2_28x174_RGBA;
    g_sMinHand.bo.base_virt = (void*)PSRAM_W2_28x174_RGBA;
    g_sMinHand.bo.size = w2_28x174_rgba_length;

    g_sHourHand.bo.base_phys = PSRAM_W1_28x125_RGBA;
    g_sHourHand.bo.base_virt = (void*)PSRAM_W1_28x125_RGBA;
    g_sHourHand.bo.size = w1_28x125_rgba_length;

//    g_sClockDial.bo.base_phys = PSRAM_W0_360x360_RGBA565;
//    g_sClockDial.bo.base_virt = (void*)PSRAM_W0_360x360_RGBA565;
//    g_sClockDial.bo.size = w0_360x360_rgba565_length;
//
//    g_sSportsDial.bo.base_phys = PSRAM_W0_360x196_RGBA4444;
//    g_sSportsDial.bo.base_virt = (void*)PSRAM_W0_360x196_RGBA4444;
//    g_sSportsDial.bo.size = w0_360x196_rgba4444_length;

#ifdef USE_EXTENDED_RAM
    // alloc textures buffer in extended_ram
    g_sClockDialBuffer[0].bo.base_phys =  DSP0_IRAM_ADDRESS;
    g_sClockDialBuffer[0].bo.base_virt =  (void*)g_sClockDialBuffer[0].bo.base_phys;
    g_sClockDialBuffer[0].bo.size = nema_texture_size(g_sClockDialBuffer[0].format,
                                                    0,
                                                    g_sClockDialBuffer[0].w,
                                                    g_sClockDialBuffer[0].h);

    g_sSportsDialBuffer[1].bo.base_phys  = DSP1_IRAM_ADDRESS;
    g_sSportsDialBuffer[1].bo.base_virt  = (void*)g_sSportsDialBuffer[1].bo.base_phys;
    g_sSportsDialBuffer[1].bo.size = nema_texture_size(g_sSportsDialBuffer[1].format,
                                                    0,
                                                    g_sSportsDialBuffer[1].w,
                                                    g_sSportsDialBuffer[1].h);
#else
    // alloc textures buffer in SSRAM
    ret = SSRAM_buffer_alloc(&g_sClockDialBuffer[0]);
    if ( ret < 0 )
    {
        return ret;
    }

    ret = SSRAM_buffer_alloc(&g_sSportsDialBuffer[1]);
    if ( ret < 0 )
    {
        return ret;
    }
#endif

    // alloc textures buffer in SSRAM
    ret = SSRAM_buffer_alloc(&g_sClockDialBuffer[1]);
    if ( ret < 0 )
    {
        return ret;
    }

    ret = SSRAM_buffer_alloc(&g_sSportsDialBuffer[0]);
    if ( ret < 0 )
    {
        return ret;
    }

    // alloc framebuffer space in SSRAM
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[0]);
    if ( ret < 0 )
    {
        return ret;
    }

#ifdef USE_EXTENDED_RAM
    // alloc frame buffer in extended_ram.
    g_sFrameBuffer[1].bo.base_phys =  DSP0_DRAM_ADDRESS;
    g_sFrameBuffer[1].bo.base_virt =  (void*)g_sFrameBuffer[1].bo.base_phys;
    g_sFrameBuffer[1].bo.size = nema_texture_size(g_sFrameBuffer[1].format, 0, g_sFrameBuffer[1].w, g_sFrameBuffer[1].h);
#else
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[1]);
    if ( ret < 0 )
    {
        return ret;
    }
#endif

    return 0;
}

//*****************************************************************************
//
// Swap framebuffer
//
//*****************************************************************************
void
frameBuffer_swap(void)
{
    img_obj_t* temp;

    temp = g_pFrameBufferGPU;
    g_pFrameBufferGPU = g_pFrameBufferDC;
    g_pFrameBufferDC = temp;
}

//*****************************************************************************
//
// Swap texture buffer
//
//*****************************************************************************
void
textureBuffer_swap(void)
{
    img_obj_t* temp;

    temp = g_pSportsDialBufferDMA;
    g_pSportsDialBufferDMA = g_pSportsDialBufferGPU;
    g_pSportsDialBufferGPU = temp;

    temp = g_pClockDialBufferDMA;
    g_pClockDialBufferDMA = g_pClockDialBufferGPU;
    g_pClockDialBufferGPU = temp;
}

//*****************************************************************************
//
// MSPI DMA finish callback
//
//*****************************************************************************
void
MspiTransferCallback(void *pCallbackCtxt, uint32_t status)
{
    if ( status == AM_HAL_STATUS_SUCCESS )
    {
        xSemaphoreGiveFromISR(g_semDMAFinish, NULL);
    }
    else
    {
        am_util_stdio_printf("GUI: mspi0 DMA failed! status=0x%8X\n", status);
    }
}

//*****************************************************************************
//
// Load texture by DMA
//
//*****************************************************************************
void
clock_dial_texture_load(uint32_t parts)
{
    uint32_t offset;
    uint32_t length;
    uint32_t ui32Ret = AM_HAL_STATUS_SUCCESS;

    offset = g_sClockDialBuffer[0].bo.size * parts;
    length = g_sClockDialBuffer[0].bo.size;

#if defined( AM_PART_APOLLO4P ) ||  defined( AM_PART_APOLLO4L )
    ui32Ret = mspi_psram_device_func.mspi_nonblocking_read(g_pPsramHandle,
                                                         g_pClockDialBufferDMA->bo.base_virt,
                                                         PSRAM_W0_360x360_RGBA565_UNMAPED + offset,
                                                         length,
                                                         MspiTransferCallback,
                                                         NULL);
#else
    (void)length; // Avoid warning!
    // !!! DON'T USE DMA on RevB due to Fifo full issue and FALC-486
    //
    //memcpy(g_pClockDialBufferDMA->bo.base_virt,
    //       (void *)(PSRAM_W0_360x360_RGBA565 + offset),
    //       length);
    g_pClockDialBufferDMA->bo.base_virt = (void*)(PSRAM_W0_360x360_RGBA565 + offset);
    g_pClockDialBufferDMA->bo.base_phys = (PSRAM_W0_360x360_RGBA565 + offset);
#endif

    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("GUI:mspi0 DMA START failed!\n");
    }
}

//*****************************************************************************
//
// Load texture by CPU memory copy
//
//*****************************************************************************
void
sports_dial_texture_load(uint32_t parts)
{
    uint32_t offset;
    int32_t length;
    int32_t left;
    int32_t stride;

    offset = g_sSportsDialBuffer[0].bo.size * parts;
    left = w0_360x196_rgba4444_length - offset;

    if ( left < 0 )
    {
        left = 0;
    }

    length = (left < g_sSportsDialBuffer[0].bo.size) ? left : g_sSportsDialBuffer[0].bo.size;

    memcpy(g_pSportsDialBufferDMA->bo.base_virt, (void*)(PSRAM_W0_360x196_RGBA4444 + offset), length);

    stride = nema_stride_size(g_sSportsDialBuffer[0].format, 0, g_sSportsDialBuffer[0].w);

    g_pSportsDialBufferDMA->h = length / stride;

}

#ifdef USE_CRC_CHECK
//*****************************************************************************
//
// Calculate CRC
//
//*****************************************************************************
static inline void
crc32_start(uint32_t init, void* buffer, uint32_t length)
{
    // Program the CRC engine to compute the crc
    //
    SECURITY->RESULT          = init;
    SECURITY->SRCADDR         = (uint32_t)buffer;
    SECURITY->LEN             = length;
    SECURITY->CTRL_b.FUNCTION = SECURITY_CTRL_FUNCTION_CRC32;
    //
    // Start the CRC
    //
    SECURITY->CTRL_b.ENABLE = 1;
}

static inline void
crc32_wait(void)
{
    while ( (SECURITY->CTRL & SECURITY_CTRL_ENABLE_Msk) != 0 );
}

static inline uint32_t
crc32_get(void)
{
    return SECURITY->RESULT;
}
#endif

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    int ret;
    int parts = 0;

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
    am_hal_gpio_pinconfig(22, am_hal_gpio_pincfg_output); //GPIO22 is used to measure the GPU work time.
    am_hal_gpio_pinconfig(23, am_hal_gpio_pincfg_output); //GPIO23 is used to measure the DC work time.
    am_hal_gpio_pinconfig(24, am_hal_gpio_pincfg_output); //GPIO24 is used to measure the GPU sleep time.
    am_hal_gpio_pinconfig(25, am_hal_gpio_pincfg_output); //GPIO25 is used to measure the DC sleep time.
    am_hal_gpio_pinconfig(26, am_hal_gpio_pincfg_output); //GPIO26 is the DC start signal.
    am_hal_gpio_pinconfig(27, am_hal_gpio_pincfg_output); //GPIO27 is the GPU start signal.
#endif

    // Allocate memory for framebuffer and textures
    ret = texture_framebuffer_init();
    if ( ret < 0 )
    {
        am_util_stdio_printf("frame buffer init failed!\n");
        vTaskDelete(NULL);
    }

    g_semDMAFinish = xSemaphoreCreateBinary();
    if ( g_semDMAFinish == NULL )
    {
        am_util_stdio_printf("Create semphone failed!\n");
        vTaskDelete(NULL);
    }

    am_util_stdio_printf("GUI task start!\n");

    while (1)
    {

#ifdef SMALLFB
        for (parts = 0; parts < SMALLFB_STRIPES; parts++)
#endif
        {

            //Step the pipeline
            g_intDisplayPart = g_intRenderPart;
            g_intRenderPart = g_intTexturePart;
            g_intTexturePart = parts;

            //DC workflow
            if ( g_intDisplayPart >= 0 )
            {
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
                am_hal_gpio_output_set(26);
#endif
                xSemaphoreGive(g_semDCStart);
            }

#ifdef USE_CRC_CHECK
            int crcPart = g_intDisplayPart;
            //Use CRC do the framebuffer check
            if ( crcPart >= 0 )
            {
                //Start CRC
                crc32_start(0xFFFFFFFF, g_pFrameBufferDC->bo.base_virt, g_pFrameBufferDC->bo.size);
            }
#endif

            //GPU workflow
            if ( g_intRenderPart >= 0 )
            {
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
                am_hal_gpio_output_set(27);
#endif
                xSemaphoreGive(g_semGPUStart);
            }

            //DMA workflow
            if ( g_intTexturePart >= 0 )
            {
                //Start DMA to load texture to texture buffer
                clock_dial_texture_load(g_intTexturePart);

                //Copy texture 2, it's a block operation, so we start it after DMA, GPU and DC are triggered.
                sports_dial_texture_load(g_intTexturePart);
            }

            //wait DC
            if ( g_intDisplayPart >= 0 )
            {
                xSemaphoreTake( g_semDCEnd, portMAX_DELAY);
            }

#ifdef USE_CRC_CHECK
            uint32_t crc_result;
            //Wait CRC
            if ( crcPart >= 0 )
            {
                crc32_wait();
                crc_result = crc32_get();
                if (crc_result != g_GPUChecksumTable[crcPart])
                {
                    am_util_stdio_printf("crc check error! part=%d\n", crcPart);
                }
            }
#endif

            //wait GPU
            if ( g_intRenderPart >= 0 )
            {
                xSemaphoreTake( g_semGPUEnd, portMAX_DELAY);
            }

#if defined( AM_PART_APOLLO4P ) ||  defined( AM_PART_APOLLO4L )
            //wait DMA
            if ( g_intTexturePart >= 0 )
            {
                BaseType_t ret = xSemaphoreTake( g_semDMAFinish, PSRAM_DMA_TIMEOUT);
                if ( ret == pdFALSE )
                {
                    am_util_stdio_printf("mspi DMA timeout!\n");
                }
            }
#endif

            //swap texture buffer
            if ( g_intTexturePart >= 0 )
            {
                textureBuffer_swap();
            }

            //swap frame buffer
            if ( g_intRenderPart >= 0 )
            {
                frameBuffer_swap();
            }

        }

        //print fps
        nema_calculate_fps();
    }
}

