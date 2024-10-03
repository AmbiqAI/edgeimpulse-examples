//*****************************************************************************
//
//! @file main_task.c
//!
//! @brief Task to handle main operation.
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
#include "freertos_psram_stress.h"

// Ping Pong Temp Buffer in SRAM to read Flash data to, and write FRAM data from
uint32_t        g_TempBuf[2][TEMP_BUFFER_SIZE / 4];

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t render_task_handle;

//*****************************************************************************
//
// Handle for Render-related events.
//
//*****************************************************************************
EventGroupHandle_t xRenderEventHandle;

void
flash_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
      DEBUG_PRINT("\nIter# %d:Flash Read Failed 0x%x\n", g_numIter, transactionStatus);
    }
    else
    {
        DEBUG_PRINT_SUCCESS("\nIter# %d:Flash Read Done 0x%x\n", g_numIter, transactionStatus);
    }
}

void
fram_write_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nIter# %d:FRAM Write Failed 0x%x\n", g_numIter, transactionStatus);
    }
    else
    {
        DEBUG_PRINT_SUCCESS("\nIter# %d:FRAM Write Done 0x%x\n", g_numIter, transactionStatus);
    }
    // Signal main task that display is done
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xMainEventHandle, MAIN_EVENT_DISPLAY_DONE,
                                        &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


int
initiate_mspi_iom_xfer(void)
{
    uint32_t      ui32Status = 0;
    uint32_t      u32Arg;
    am_hal_iom_callback_t   iomCb = 0;
    am_hal_mspi_callback_t  mspiCb = 0;

#ifdef SEQLOOP
    bool          bBool;
#endif

#ifndef RUN_AUTONOMOUS
    iomCb = fram_write_complete;
    mspiCb = flash_read_complete;
#endif

    // Clear flags
    u32Arg = 0x001F0000;  // clear all flags

    ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_FLAG_SETCLR, &u32Arg);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to clear flags\n");
        return ui32Status;
    }
    ui32Status = am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_FLAG_SETCLR, &u32Arg);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to clear flags\n");
        return ui32Status;
    }
    // Link MSPI and IOM
    u32Arg = FRAM_IOM_MODULE;
    ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_LINK_IOM, &u32Arg);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to link IOM & MSPI\n");
        return ui32Status;
    }

    DEBUG_PRINT_SUCCESS("\nInitiating MSP -> IOM Transfer\n");
#ifdef SEQLOOP
    // Set in Sequence mode
    bBool = true;
    ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_SET_SEQMODE, &bBool);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to Set Sequence mode\n");
        return ui32Status;
    }
    ui32Status = am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_SET_SEQMODE, &bBool);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to Set Sequence mode\n");
        return ui32Status;
    }
#endif

    // Queue up FRAM Writes and Flash Reads
    for (uint32_t address = 0, bufIdx = 0; address < FB_SIZE; address += SPI_TXN_SIZE)
    {
        uint32_t bufOdd = bufIdx % 2;
        uint32_t psramFbOffset = PSRAM_ACTFB_OFFSET + ((g_numIter - 1) & 0x1)*FB_SIZE;
        ui32Status = am_devices_mspi_psram_read_adv(g_MSPIDevHdl, (uint8_t *)g_TempBuf[bufOdd], psramFbOffset + address,
                                       (((address + SPI_TXN_SIZE) >= FB_SIZE) ? (FB_SIZE - address) : SPI_TXN_SIZE),
                                       (bufOdd ? MSPI_WAIT_FOR_IOM_BUFFER1 : MSPI_WAIT_FOR_IOM_BUFFER0),
                                       (bufOdd ? MSPI_SIGNAL_IOM_BUFFER1 : MSPI_SIGNAL_IOM_BUFFER0),
                                       (((address + SPI_TXN_SIZE) >= FB_SIZE) ? mspiCb : 0),
                                       0);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up MSPI Read transaction\n");
            break;
        }
        ui32Status = fram_func.fram_nonblocking_write_adv(g_IomDevHdl, (uint8_t *)g_TempBuf[bufOdd], address,
                                       (((address + SPI_TXN_SIZE) >= FB_SIZE) ? (FB_SIZE - address) : SPI_TXN_SIZE),
                                       (bufOdd ? IOM_WAIT_FOR_MSPI_BUFFER1 : IOM_WAIT_FOR_MSPI_BUFFER0),
                                       (bufOdd ? IOM_SIGNAL_MSPI_BUFFER1 : IOM_SIGNAL_MSPI_BUFFER0),
                                       (((address + SPI_TXN_SIZE) >= FB_SIZE) ? iomCb : 0),
                                       0);
        bufIdx++;
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up SPI Write transaction\n");
             break;
        }
    }
    if (ui32Status == 0)
    {
#ifdef SEQLOOP
        // End the sequence and start
        am_hal_iom_seq_end_t iomLoop;
        am_hal_mspi_seq_end_t mspiLoop;
        iomLoop.bLoop = true;
        // Let IOM be fully controlled by MSPI
        iomLoop.ui32PauseCondition = iomLoop.ui32StatusSetClr = 0;
        ui32Status = am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_SEQ_END, &iomLoop);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to Close Sequence\n");
            return ui32Status;
        }
        mspiLoop.bLoop = true;
#ifndef RUN_AUTONOMOUS
        // Let MSPI be controlled by a flag
        mspiLoop.ui32PauseCondition = 0x4;
        mspiLoop.ui32StatusSetClr = 0x4 << 16;
#else
        mspiLoop.ui32PauseCondition = 0;
        mspiLoop.ui32StatusSetClr = 0;
#endif
        ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_SEQ_END, &mspiLoop);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to Close Sequence\n");
            return ui32Status;
        }
#endif
    }
    else
    {
        while(1);
    }
    return ui32Status;
}

void
RenderTaskSetup(void)
{
    am_util_debug_printf("RenderTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xRenderEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xRenderEventHandle == NULL);

}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RenderTask(void *pvParameters)
{
    uint32_t eventMask;

    while (1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xRenderEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask != 0)
        {
            if (eventMask & RENDER_EVENT_START_NEW_FRAME)
            {
                initiate_mspi_iom_xfer();
            }
        }
    }
}

