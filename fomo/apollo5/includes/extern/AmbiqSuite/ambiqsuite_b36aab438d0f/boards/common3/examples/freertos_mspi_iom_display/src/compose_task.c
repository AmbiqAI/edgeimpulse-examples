//*****************************************************************************
//
//! @file compose_task.c
//!
//! @brief Task to handle composition operation.
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
#include "freertos_mspi_iom_display.h"

uint32_t         g_RxFrag[2][COMPOSE_FRAGMENT_SIZE / 4];
uint32_t         g_TxFrag[COMPOSE_FRAGMENT_SIZE / 4];
// Composition States: n is NUM_COMPOSE_FRAGMENTS
// 0: Initial State - Read two fragments
// 1: Compose frag # 0, Read Frag # 1
// 2: Compose frag # 1, Read Frag # 2
// 3: Compose frag # 2, Read Frag # 3
// 4: Compose frag # 3, Read Frag # 4
// n-1: Compose frag # n-1, Read Frag # 0
uint32_t        g_ComposeState = 0;
uint32_t        g_fragIdx = 0;
bool            g_bNewFB = false;
uint32_t        g_numCompose = 0;

// Memory for hiprio transactions
// Need to use hiprio transactions to insert in between a running sequence
uint8_t         iomDmaBuf[(AM_HAL_IOM_HIPRIO_ENTRY_SIZE)*64]; // Used only to send one transaction for CMD
uint8_t         mspiDmaBuf[(AM_HAL_MSPI_HIPRIO_ENTRY_SIZE)*128]; // We write two lines - which could be split into 4 transactions
//*****************************************************************************
//
// Composition task handle.
//
//*****************************************************************************
TaskHandle_t compose_task_handle;

//*****************************************************************************
//
// Handle for Compose-related events.
//
//*****************************************************************************
EventGroupHandle_t xComposeEventHandle;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
#ifdef DEBUG
uint32_t g_History[256];
uint8_t g_HistoryCount = 0;

#define ADD_HISTORY(hist)  \
do { \
  g_History[g_HistoryCount++] = (hist); \
} while(0)
#else

#define ADD_HISTORY(x)

#endif

void
frag_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nCompose# %d:Fragment %d Failed 0x%x\n", g_numCompose, g_ComposeState, transactionStatus);
    }
    else
    {
        DEBUG_PRINT_SUCCESS("\nCompose# %d:Fragment %d Done 0x%x\n", g_numCompose, g_ComposeState, transactionStatus);
        g_ComposeState++;
    }
    //
    // Send an event to the main LED task
    //
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xComposeEventHandle, COMPOSE_EVENT_NEW_FRAG_READY,
                                        &xHigherPriorityTaskWoken);

    //
    // If the LED task is higher-priority than the context we're currently
    // running from, we should yield now and run the radio task.
    //
    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

int
init_mspi_psram_data(void)
{
    uint32_t ui32Status;
    uint32_t i, j;

    DEBUG_PRINT("Writing a known pattern to psram!\n");

    //
    // Generate raw color data into PSRAM frame buffer 1
    //
    for (i = 0; i < NUM_ROW_PER_FRAGMENT; i++)
    {
        uint32_t rowStart = i*(COLUMN_NUM / 4);
        for (j = 0; j < (COLUMN_NUM / 4); j++)
        {
            if (j < BAND_WIDTH / 4)
            {
                g_RxFrag[0][rowStart + j] = COLOR_4P(BAND_COLOR);
            }
            else
            {
                g_RxFrag[0][rowStart + j] = COLOR_4P(0xFF);
            }
            if (j >= (COLUMN_NUM - BAND_WIDTH) / 4)
            {
                g_RxFrag[1][rowStart + j] = COLOR_4P(COLOR_MAX ^ BAND_COLOR);
            }
            else
            {
                g_RxFrag[1][rowStart + j] = COLOR_4P(0xFF);
            }
        }
    }

    // Initialize Src Buf 1
    for (uint32_t address = 0; address < FRAME_SIZE; address += COMPOSE_FRAGMENT_SIZE)
    {
        uint32_t size = ((address + COMPOSE_FRAGMENT_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : COMPOSE_FRAGMENT_SIZE;
        //
        // Write the buffer into the target address in PSRAM
        //
        ui32Status = am_devices_mspi_psram_write(g_MSPIDevHdl, (uint8_t *)g_RxFrag[0], PSRAM_SRCFB1_OFFSET + address, size, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
            return -1;
        }
    }

    // Initialize Src Buf 2
    for (uint32_t address = 0; address < FRAME_SIZE; address += COMPOSE_FRAGMENT_SIZE)
    {
        uint32_t size = ((address + COMPOSE_FRAGMENT_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : COMPOSE_FRAGMENT_SIZE;
        //
        // Write the buffer into the target address in PSRAM
        //
        ui32Status = am_devices_mspi_psram_write(g_MSPIDevHdl, (uint8_t *)g_RxFrag[1], PSRAM_SRCFB2_OFFSET + address, size, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
            return -1;
        }
    }
#if (MODE_SRCBUF_READ == MODE_XIP)
#ifndef SRCBUF_XIP_UNCACHED
    ui32Status = am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_FLASH_CACHE_INVALIDATE, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to invalidate Cache!\n");
        return -1;
    }
#else
    //  Mark XIP SRCBUF as non-cached - to make sure we see its impact
    am_hal_cachectrl_nc_cfg_t ncCfg;
    ncCfg.bEnable = true;
    ncCfg.eNCRegion = AM_HAL_CACHECTRL_NCR1;
    ncCfg.ui32StartAddr = PSRAM_SRCFB_BASE;
    ncCfg.ui32EndAddr = PSRAM_SRCFB_BASE + PSRAM_SRCFB_SIZE;
    ui32Status = am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_NC_CFG, &ncCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to mark XIP SRCBUF region as non-cacheable\n");
        return -1;
    }
#endif
#endif
    return 0;
}
//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
ComposeTaskSetup(void)
{
    uint32_t iRet;
    am_util_debug_printf("ComposeTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xComposeEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xComposeEventHandle == NULL);
    // Initialize psram Data
    iRet = init_mspi_psram_data();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize MSPI psram data\n");
        while(1);
    }
    // Set up hiprio transactions which will be used for composition while Sequence is running
    am_hal_iom_hiprio_cfg_t iomHiPrioCfg;
    iomHiPrioCfg.pBuf = iomDmaBuf;
    iomHiPrioCfg.size = sizeof(iomDmaBuf);
    am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_INIT_HIPRIO, &iomHiPrioCfg);
    am_hal_mspi_hiprio_cfg_t mspiHiprioCfg;
    mspiHiprioCfg.pBuf = mspiDmaBuf;
    mspiHiprioCfg.size = sizeof(mspiDmaBuf);
    am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_INIT_HIPRIO, &mspiHiprioCfg);

    // Prepare the Fragments for the 1st iteration - TODO
    g_ComposeState = 1;
    // Notify compose task that the frame is ready
    xEventGroupSetBits(xComposeEventHandle, COMPOSE_EVENT_START_NEW_FRAME);
    // Notify main task that the frame is ready
//    xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_NEW_FRAME_READY);
}

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
#ifndef MINIMAL_CPU_COMPOSITIING
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
#else
            // Only Src2
            pDestBuf[rowStart + j] = pSrcBuf2[rowStart + (j + g_numCompose) % (COLUMN_NUM / 4)];
#endif
        }
    }
}

// Simulate the composition task using a state machine
uint32_t
compose(void)
{
    static uint32_t lastState = 0;
    uint32_t ui32Status;
    uint32_t *pSrcBuf1;
    uint32_t *pSrcBuf2;
    uint32_t *pDestBuf;

#ifdef SEQLOOP
    uint32_t actFbOffset = PSRAM_ACTFB1_OFFSET; // For SEQLOOP, we always need to use the same buffer
#else
    uint32_t actFbOffset = (g_numCompose & 0x1) ? PSRAM_ACTFB2_OFFSET: PSRAM_ACTFB1_OFFSET;
#endif


#if (MODE_SRCBUF_READ == MODE_XIP)
    pSrcBuf1 = (uint32_t *)(MSPI_XIP_BASE_ADDRESS + PSRAM_SRCFB1_OFFSET + (g_ComposeState - 1)*COMPOSE_FRAGMENT_SIZE);
    pSrcBuf2 = (uint32_t *)(MSPI_XIP_BASE_ADDRESS + PSRAM_SRCFB2_OFFSET + (g_ComposeState - 1)*COMPOSE_FRAGMENT_SIZE);
#elif (MODE_SRCBUF_READ == MODE_XIPMM)
    pSrcBuf1 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS + PSRAM_SRCFB1_OFFSET + (g_ComposeState - 1)*COMPOSE_FRAGMENT_SIZE);
    pSrcBuf2 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS + PSRAM_SRCFB2_OFFSET + (g_ComposeState - 1)*COMPOSE_FRAGMENT_SIZE);
#else
    pSrcBuf1 = g_RxFrag[0];
    pSrcBuf2 = g_RxFrag[1];
#endif

#if (MODE_DESTBUF_WRITE == MODE_XIPMM)
    pDestBuf = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS + actFbOffset + lastState*COMPOSE_FRAGMENT_SIZE);
#else
    pDestBuf = (uint32_t *)g_TxFrag;
#endif

    if ((g_ComposeState != lastState) && g_bNewFB)
    {
        composeFragment(pSrcBuf1, pSrcBuf2, pDestBuf, NUM_ROW_PER_FRAGMENT);
        if (xSemaphoreTake(xMSPIMutex, portMAX_DELAY) != pdTRUE)
        {
            am_util_stdio_printf("Failed to get the semaphore\n");
            return AM_HAL_STATUS_FAIL;
        }
        // Write the fragment to next frame buffer
#if (MODE_DESTBUF_WRITE != MODE_XIPMM)
#if (MODE_SRCBUF_READ == MODE_DMA)
        ui32Status = am_devices_mspi_psram_write_hiprio(g_MSPIDevHdl, (uint8_t *)g_TxFrag, actFbOffset + lastState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, NULL, 0);
#else
        ui32Status = am_devices_mspi_psram_write_hiprio(g_MSPIDevHdl, (uint8_t *)g_TxFrag, actFbOffset + lastState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, frag_complete, 0);
#endif
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to write buffer to PSRAM Device!\n");
            xSemaphoreGive(xMSPIMutex);
            return ui32Status;
        }
#endif

        if (g_ComposeState == NUM_COMPOSE_FRAGMENTS)
        {
            // This FB is ready - now we can start for the next one
            g_ComposeState = 0;
        }
        lastState = g_ComposeState;
#if (MODE_SRCBUF_READ == MODE_DMA)
        // Read the two base fragments
#ifndef MINIMAL_CPU_COMPOSITIING
        ui32Status = am_devices_mspi_psram_read_hiprio(g_MSPIDevHdl, (uint8_t *)pSrcBuf2, PSRAM_SRCFB2_OFFSET + g_ComposeState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, NULL, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
            xSemaphoreGive(xMSPIMutex);
            return ui32Status;
        }
#endif
        ui32Status = am_devices_mspi_psram_read_hiprio(g_MSPIDevHdl, (uint8_t *)pSrcBuf1, PSRAM_SRCFB1_OFFSET + g_ComposeState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, frag_complete, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
            xSemaphoreGive(xMSPIMutex);
            return ui32Status;
        }
#endif
        xSemaphoreGive(xMSPIMutex);
        if (g_ComposeState == 0)
        {
            DEBUG_PRINT("Done composing ActFB @ 0x%x\n", actFbOffset);
            g_bNewFB = false;
            g_numCompose++;
            DEBUG_GPIO_LOW(TEST_GPIO2);
            // Notify main task that the frame is ready
            xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_NEW_FRAME_READY);
#if (MODE_DESTBUF_WRITE == MODE_XIPMM) && (MODE_SRCBUF_READ != MODE_DMA)
            g_ComposeState++;
        }
        else
        {
            // No DMA - self signal completion
            g_ComposeState++;
            xEventGroupSetBits(xComposeEventHandle, COMPOSE_EVENT_NEW_FRAG_READY);
#endif
        }
    }
    return 0;
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
ComposeTask(void *pvParameters)
{
    uint32_t eventMask;

    while (1)
    {
        //
        // Wait for an event to be posted to the Compose Event Handle.
        //
        eventMask = xEventGroupWaitBits(xComposeEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask != 0)
        {
            if (eventMask & COMPOSE_EVENT_START_NEW_FRAME)
            {
                g_bNewFB = true;
                DEBUG_GPIO_HIGH(TEST_GPIO2);
            }
            else
            {
            }
#ifndef NO_COMPOSITING
            if (compose())
            {
                while(1);
            }
#else
            // No Compositing - Just indicate Frame Ready
            xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_NEW_FRAME_READY);

#endif
        }
    }
}
