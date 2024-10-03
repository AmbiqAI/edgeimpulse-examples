//*****************************************************************************
//
//! @file hello_task.c
//!
//! @brief Simple task with a while() loop that implements a vTaskDelay of 
//!     1 second on every iteration.  After HELLO_TASK_COUNT iterations (set 
//!     in src/freertos_sample_test_cases.h), the task breaks out of the while(1)
//!     loop, sets bHelloTaskComplete to "true", and suspends/deletes itself.
//!     The bHelloTaskComplete variable is monitored by the ApplicationIdleTask.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "freertos_sample_test_cases.h"
#include "hello_task.h"

//*****************************************************************************
//
// Hello task handle.
//
//*****************************************************************************
TaskHandle_t hello_task_handle;

//*****************************************************************************
//
// Global variables and templates 
//
//*****************************************************************************
volatile uint32_t g_ui32IdleCycleCount = 0;
volatile bool g_bHelloTaskComplete = false;

//*****************************************************************************
//
// Perform initial setup for the hello task.
//
//*****************************************************************************
void
HelloTaskSetup(void)
{
    am_util_stdio_printf("Hello task: setup\r\n");
}

//*****************************************************************************
//
// HelloTask - will delete itself after HELLO_TASK_COUNT loops complete
//
//*****************************************************************************

void
HelloTask(void *pvParameters)
{
    uint32_t i = 0;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS; // 1 second delay
    am_util_stdio_printf("\r\nRunning HelloTask\r\n");

    while (i < HELLO_TASK_COUNT)
    {
        // Toggle an observation GPIO
        am_hal_gpio_output_toggle(GPIO_HELLO_TASK);
        // Print out number of iterations
        am_util_stdio_printf("\r\nHelloTask Iteration: %d\r\n", i++);
        // Print out number of Idle Cycles since the last iteration
        am_util_stdio_printf("HelloTask Idle Cycle Count: %d\r\n", g_ui32IdleCycleCount);
        // Reset the Idle Cycle Count
        g_ui32IdleCycleCount = 0;
        // Delay (block) the task for a specified period
        vTaskDelay(xDelay);
    }
    // Indicate the HelloTask loops are complete
    g_bHelloTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}
