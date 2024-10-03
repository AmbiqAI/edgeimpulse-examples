//*****************************************************************************
//
//! @file graphic_task.c
//!
//! @brief Task to handle graphic operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "ipc_stress_test_main.h"

//*****************************************************************************
//
// Graphic task handle.
//
//*****************************************************************************
EventGroupHandle_t xGraphicStoreEventHandle;
EventGroupHandle_t xGraphicComposeEventHandle;

#define GRAPHIC_WRITE        0x1
#define GRAPHIC_COMPOSE      0x2

uint32_t g_numDisplay = 0;
uint32_t g_numCompose = 0;
// Pre-constructed Image(s)
extern const unsigned char g_ambiq_logo_bmp[];
extern const unsigned char g_ambiq_logo_bmp0[];
extern const unsigned char g_ambiq_logo_bmp1[];
extern const unsigned char g_ambiq_logo_bmp2[];
extern const unsigned char g_ambiq_logo_bmp_allblack[];
uint8_t                    *img0 = (uint8_t *)g_ambiq_logo_bmp0;
uint8_t                    *img1 = (uint8_t *)g_ambiq_logo_bmp1;

#define FB_SIZE                   0x52800   //330K
uint32_t fb1 = SRAM_BANK2_OFFSET;
uint32_t fb2 = SRAM_BANK2_OFFSET + FB_SIZE;
uint32_t act = SRAM_BANK2_OFFSET + 2 * FB_SIZE;

#define ROW_NUM                   400
#define COLUMN_NUM                400
#define PIXEL_SIZE                1
#define ROW_SIZE                  (COLUMN_NUM * PIXEL_SIZE)
#define FRAME_SIZE                ROW_NUM * ROW_SIZE

#define NUM_ROW_PER_COMPOSE_FRAGMENT      40
#define COMPOSE_FRAGMENT_SIZE             (NUM_ROW_PER_COMPOSE_FRAGMENT * ROW_SIZE)
#define NUM_COMPOSE_FRAGMENTS             ((ROW_NUM + NUM_ROW_PER_COMPOSE_FRAGMENT - 1) / NUM_ROW_PER_COMPOSE_FRAGMENT)

metal_thread_t graphic_store_task_handle;
metal_thread_t graphic_compose_task_handle;

#define BACKGROUND_COLOR          0x80 // 0x00
#define BAND_COLOR                0xfc // white
#define COLOR_MAX                 0xfc
#define BAND_WIDTH                16
// Extract color from a 4p word
#define COL(word, pix)            (((word) >> ((pix) << 3)) & COLOR_MAX)
#define WRAP_COL(num, max)        (((num) > (max)) ? 0 : (num))
// Increment color
#define INC_COL(color, inc)       WRAP_COL((color) + (inc))
// 4 pixel worth of data
#define COLOR_4P(color)           ((color) | ((color) << 8) | ((color) << 16) | ((color) << 24))
// Increment color for each pixel
#define INC_COL_4P(w, inc)        (INC_COL(COL((w), 0), (inc)) | (INC_COL(COL((w), 1), (inc)) << 8) | (INC_COL(COL((w), 2), (inc)) << 16) | (INC_COL(COL((w), 3), (inc)) << 24))

//*****************************************************************************
//
// Perform initial setup for the graphic task.
//
//*****************************************************************************
void
GraphicTaskSetup(void)
{
    am_util_debug_printf("GraphicTask: setup\r\n");

    //
    // Create an event handle for our wake-up events.
    //
    xGraphicStoreEventHandle = xEventGroupCreate();
    xGraphicComposeEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while ((xGraphicStoreEventHandle == NULL) || (xGraphicComposeEventHandle == NULL));

    // Initialize images
    am_devices_mspi_ambt53_dma_write(g_RpmsgDevHdl, img0, fb1, FRAME_SIZE, true);
    am_devices_mspi_ambt53_dma_write(g_RpmsgDevHdl, img1, fb2, FRAME_SIZE, true);

    // Notify compose task that the frame is ready
    xEventGroupSetBits(xGraphicComposeEventHandle, GRAPHIC_COMPOSE);
}

void
display_write_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nDisplay# %d:SRAM Write Failed 0x%x\n", g_numDisplay, transactionStatus);
    }
    else
    {
        am_util_stdio_printf("\nDisplay# %d:SRAM Write Done 0x%x\n", g_numDisplay, transactionStatus);
    }
    g_numDisplay++;
    // Signal main task that display is done
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xGraphicComposeEventHandle, GRAPHIC_COMPOSE,
                                        &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// Compose the next fragment
static void
composeFragment(uint32_t *pSrcBuf1, uint32_t *pSrcBuf2, uint32_t *pDestBuf, uint32_t numRows)
{
    uint32_t i, j;
    // Compute the output FB fragment
    for (i = 0; i < numRows; i++)
    {
        uint32_t rowStart = i*(COLUMN_NUM / 4);
        for (j = 0; j < (COLUMN_NUM / 4); j++)
        {
            if (pSrcBuf2[rowStart + (j + g_numCompose) % (COLUMN_NUM / 4)] != COLOR_4P(0xFF))
            {
                if (pSrcBuf1[rowStart + ((j + (COLUMN_NUM / 4) - g_numCompose % (COLUMN_NUM / 4)) % (COLUMN_NUM / 4))] != COLOR_4P(0xFF))
                {
                    // Overlap region
                    pDestBuf[rowStart + j] = COLOR_4P(COLOR_MAX ^ (g_numCompose % COLOR_MAX));
                }
                else
                {
                    // Only Src2
                    pDestBuf[rowStart + j] = pSrcBuf2[rowStart + (j + g_numCompose) % (COLUMN_NUM / 4)];
                }
            }
            else
            {
                if (pSrcBuf1[rowStart + ((j + (COLUMN_NUM / 4) - g_numCompose % (COLUMN_NUM / 4)) % (COLUMN_NUM / 4))] != COLOR_4P(0xFF))
                {
                    // Only Src1
                    pDestBuf[rowStart + j] = pSrcBuf1[rowStart + ((j + (COLUMN_NUM / 4) - g_numCompose % (COLUMN_NUM / 4)) % (COLUMN_NUM / 4))];
                }
                else
                {
                    // None
                    pDestBuf[rowStart + j] = COLOR_4P(g_numCompose % COLOR_MAX);
                }
            }
        }
    }
}

void
GraiphicComposeTask(void *pvParameters)
{
    uint32_t eventMask;
    uint32_t *pSrcBuf1;
    uint32_t *pSrcBuf2;
    uint32_t *pDestBuf;

    while (1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xGraphicComposeEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask & GRAPHIC_COMPOSE)
        {
            pSrcBuf1 = (uint32_t *)AMBT53_ADDR(fb1 + g_numCompose * COMPOSE_FRAGMENT_SIZE);
            pSrcBuf2 = (uint32_t *)AMBT53_ADDR(fb2 + g_numCompose * COMPOSE_FRAGMENT_SIZE);
            pDestBuf = (uint32_t *)AMBT53_ADDR(act + g_numCompose * COMPOSE_FRAGMENT_SIZE);
            composeFragment(pSrcBuf1, pSrcBuf2, pDestBuf, NUM_ROW_PER_COMPOSE_FRAGMENT);
            if ( ++g_numCompose > NUM_COMPOSE_FRAGMENTS )
            {
                g_numCompose = 0;
                am_util_stdio_printf("\nCompose Done\n");
                xEventGroupSetBits(xGraphicStoreEventHandle, GRAPHIC_WRITE);
            }
            else
            {
                xEventGroupSetBits(xGraphicComposeEventHandle, GRAPHIC_COMPOSE);
            }
        }
    }
}

void
GraiphicStoreTask(void *pvParameters)
{
    uint32_t eventMask;
    uint32_t      ui32Status = 0;
    am_hal_mspi_callback_t  mspiSinkCb = display_write_complete;
    uint8_t *pui8Src = NULL;
    uint32_t ui32Dst = 0;

    while (1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xGraphicStoreEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask & GRAPHIC_WRITE)
        {
            pui8Src = (g_numDisplay & 0x1) ? img0 : img1;
            ui32Dst = (g_numDisplay & 0x1) ? fb1 : fb2;
            ui32Status = am_devices_mspi_ambt53_dma_write_adv(g_RpmsgDevHdl,
                                                       pui8Src, ui32Dst,
                                                       FRAME_SIZE,
                                                       0,
                                                       0,
                                                       mspiSinkCb,
                                                       0);
            if (ui32Status)
            {
                am_util_stdio_printf("\nFailed to queue up MSPI Write transaction\n");
            }
        }
    }
}

