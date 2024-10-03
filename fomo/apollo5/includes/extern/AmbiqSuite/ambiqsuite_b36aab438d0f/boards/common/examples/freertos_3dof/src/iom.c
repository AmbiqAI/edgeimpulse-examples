//*****************************************************************************
//
//! @file iom.c
//!
//! @brief Functions to handle IOM operation.
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
#include "freertos_3dof.h"

//*****************************************************************************
//
// Forward declaration for our custom IOM flush function.
//
//*****************************************************************************
void iom_freertos_flush(uint32_t ui32Module);

//*****************************************************************************
//
// Configuration structure for IOM1
//
//*****************************************************************************
am_hal_iom_config_t g_sIOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
    .ui32ClockFrequency = AM_HAL_IOM_1MHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};

//*****************************************************************************
//
// Memory for the IOM1 Queue.
//
//*****************************************************************************
am_hal_iom_queue_entry_t g_psIOMQueueArray[16];

//*****************************************************************************
//
// Event for the IOM1 queue.
//
//*****************************************************************************
EventGroupHandle_t xIOM1QueueEvent;

//*****************************************************************************
//
// Interrupt handler for IOM1 to suport queueing
//
//*****************************************************************************
void
am_iomaster1_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Read and clear the interrupt status.
    //
    ui32IntStatus = am_hal_iom_int_status_get(1, false);
    am_hal_iom_int_clear(1, ui32IntStatus);

    //
    // Service FIFO interrupts as necessary, and call IOM callbacks as
    // transfers are completed.
    //
    am_hal_iom_queue_service(1, ui32IntStatus);

    //
    // Check to see if the queue is empty and the IOM is idle. If both are
    // true, we should post an event to notify other tasks that the IOM has
    // finished all of its pending transfers.
    //
    if (AM_BFRn(IOMSTR, 1, STATUS, IDLEST) &&
        (am_hal_iom_queue_length_get(1) == 0))
    {
        BaseType_t xResult;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        //
        // Send an event to the main radio task
        //
        xResult = xEventGroupSetBitsFromISR(xIOM1QueueEvent, 1,
                                            &xHigherPriorityTaskWoken);

        //
        // If the radio task is higher-priority than the context we're currently
        // running from, we should yield now and run the radio task.
        //
        if (xResult != pdFAIL)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

//*****************************************************************************
//
// Setup routine for IOM1
//
//*****************************************************************************
void
iom1_setup(void)
{
    //
    // Create an event handle for our queue events, and make sure the event
    // creation succeeds.
    //
    xIOM1QueueEvent = xEventGroupCreate();
    while (xIOM1QueueEvent == NULL);

    //
    // Configure the IOM1 pins.
    //
    am_bsp_iom_spi_pins_enable(1);

    //
    // Configure IOM1 for talking with the LIS3MDL.
    //
    am_hal_iom_pwrctrl_enable(1);
    am_hal_iom_config(1, &g_sIOMConfig);

    //
    // Prepare IOM1 for queued operation.
    //
    am_hal_iom_queue_init(1, g_psIOMQueueArray, sizeof(g_psIOMQueueArray));

    //
    // Set the queue flush function to our RTOS-friendly version.
    //
    am_hal_iom_queue_flush = iom_freertos_flush;

    //
    // Set up threshold and command-complete interrupts.
    //
    am_hal_iom_int_clear(1, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
    am_hal_iom_int_enable(1, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);

    //
    // Enable IOM1 interrupts.
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOMASTER1);
}

//*****************************************************************************
//
// This function waits until the IOM queue is empty, then returns.
//
// This is necessary to allow "blocking" IOM calls to yield in an RTOS
// environment.
//
//*****************************************************************************
void
iom_freertos_flush(uint32_t ui32Module)
{
    //
    // Clear the event bits to make sure we don't react to a stale event.
    //
    xEventGroupClearBits(xIOM1QueueEvent, 1);

    //
    // Check to see if the queue is empty and the IOM is idle.
    //
    if (AM_BFRn(IOMSTR, ui32Module, STATUS, IDLEST) &&
        (am_hal_iom_queue_length_get(1) == 0))
    {
        //
        // If we don't have any pending IOM transactions, then we're done.
        //
        return;
    }
    else
    {
        //
        // If the IOM is in use, we'll yield and wait for a "queue empty"
        // event.
        //
        xEventGroupWaitBits(xIOM1QueueEvent, 1, pdTRUE, pdFALSE,
                            portMAX_DELAY);

        return;
    }

}

//*****************************************************************************
//
// Shut down IOM1 so we can sleep efficiently.
//
//*****************************************************************************
void
iom1_shutdown(void)
{
}

//*****************************************************************************
//
// Restart IOM1 so it can be used.
//
//*****************************************************************************
void
iom1_restart(void)
{
}
