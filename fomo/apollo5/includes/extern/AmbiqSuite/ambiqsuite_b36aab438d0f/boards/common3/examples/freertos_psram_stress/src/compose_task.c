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
#include "freertos_psram_stress.h"

uint8_t         g_RxFrag[2][FB_SIZE / FB_FRAGMENT_SIZE][FB_FRAGMENT_SIZE];
uint8_t         g_TxFrag[FB_FRAGMENT_SIZE];
// Composition States:
// 0: Initial State - Read two fragments
// 1: Compose frag # 0, Read Frag # 1
// 2: Compose frag # 1, Read Frag # 2
// 3: Compose frag # 2, Read Frag # 3
// 4: Compose frag # 3, Read Frag # 0
uint32_t        g_ComposeState = 0;
uint32_t        g_fragIdx = 0;
bool            g_bNewFB = false;
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


void
frag_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nIter# %d:Fragment %d Read Failed 0x%x\n", g_numIter, g_ComposeState, transactionStatus);
    }
    else
    {
        DEBUG_PRINT_SUCCESS("\nIter# %d:Fragment %d Read Done 0x%x\n", g_numIter, g_ComposeState, transactionStatus);
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

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
ComposeTaskSetup(void)
{
    am_util_debug_printf("ComposeTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xComposeEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xComposeEventHandle == NULL);
    // Prepare the Fragments for the 1st iteration
    for (uint32_t i = 0; i < FB_FRAGMENT_SIZE; i++)
    {
       g_RxFrag[1][0][i] = (i & 0xFF) ^ 0xFF;
    }
    g_ComposeState = 1;
    // Notify compose task that the frame is ready
    xEventGroupSetBits(xComposeEventHandle, COMPOSE_EVENT_START_NEW_FRAME);

}

// Simulate the composition task using a state machine
uint32_t
compose(void)
{
    static uint32_t lastState = 0;
    uint32_t ui32Status;
    if ((g_ComposeState != lastState) && g_bNewFB)
    {
        uint32_t cur = g_numIter & 0x1;

#if 0
        // Compute the output FB fragment
        for (uint32_t i = 0; i < FB_FRAGMENT_SIZE; i++)
        {
            // Alternate between Src Buffers and add the iteration#
            g_TxFrag[i] = (g_RxFrag[cur][lastState][i] + g_numIter) & 0xFF;
        }
        // Write the fragment to next frame buffer
        ui32Status = am_devices_mspi_psram_write_hiprio(g_MSPIDevHdl, (uint8_t *)g_TxFrag, PSRAM_ACTFB_OFFSET + cur*FB_SIZE + lastState*FB_FRAGMENT_SIZE, FB_FRAGMENT_SIZE, NULL, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }
#else
        // Write the fragment to next frame buffer
        ui32Status = am_devices_mspi_psram_write_hiprio(g_MSPIDevHdl, (uint8_t *)g_RxFrag[cur][lastState], PSRAM_ACTFB_OFFSET + cur*FB_SIZE + lastState*FB_FRAGMENT_SIZE, FB_FRAGMENT_SIZE, NULL, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }
#endif
        if (g_ComposeState == (FB_SIZE / FB_FRAGMENT_SIZE))
        {
            // This FB is ready - now we can start for the next one
            g_ComposeState = 0;
            g_bNewFB = false;
        }
        lastState = g_ComposeState;
        // Read the two base fragments
        ui32Status = am_devices_mspi_psram_read_hiprio(g_MSPIDevHdl, (uint8_t *)g_RxFrag[0][lastState], PSRAM_SRCFB_OFFSET + g_ComposeState*FB_FRAGMENT_SIZE, FB_FRAGMENT_SIZE, NULL, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }
        ui32Status = am_devices_mspi_psram_read_hiprio(g_MSPIDevHdl, (uint8_t *)g_RxFrag[1][lastState], PSRAM_SRCFB_OFFSET + FB_SIZE + g_ComposeState*FB_FRAGMENT_SIZE, FB_FRAGMENT_SIZE, frag_read_complete, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }
        if (g_ComposeState == 0)
        {
            // Notify main task that the frame is ready
            xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_NEW_FRAME_READY);
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
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xComposeEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask != 0)
        {
            if (eventMask & COMPOSE_EVENT_START_NEW_FRAME)
            {
                g_bNewFB = true;
            }
            compose();
        }
    }
}
