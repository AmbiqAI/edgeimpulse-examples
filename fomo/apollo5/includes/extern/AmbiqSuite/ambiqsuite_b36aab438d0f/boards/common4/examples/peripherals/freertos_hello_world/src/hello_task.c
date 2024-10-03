//*****************************************************************************
//
//! @file radio_task.c
//!
//! @brief Task to handle radio operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "freertos_hello_world.h"
#include "hello_task.h"

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
TaskHandle_t hello_task_handle;

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
HelloTaskSetup(void)
{
    am_util_debug_printf("Hello task: setup\r\n");
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
HelloTask(void *pvParameters)
{
    uint32_t i = 0;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    am_util_debug_printf("Running task\r\n");

    while (1)
    {
        am_util_debug_printf("Iteration: %d\r\n", i++);
        vTaskDelay(xDelay);
    }
}
