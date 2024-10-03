//*****************************************************************************
//
//! @file bts_task.c
//!
//! @brief Bring to See task 
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "sensor_hub_test_cases.h"
#include "bts_task.h"

//*****************************************************************************
//
// Bts task handle.
//
//*****************************************************************************
TaskHandle_t BtsTaskHandle;

//*****************************************************************************
//
// Global variables and templates 
//
//*****************************************************************************
volatile bool g_bBtsTaskComplete = false;
extern volatile bool display_watchface;
extern TaskHandle_t GuiTaskHandle;
extern bool g_bTestEnd;
SemaphoreHandle_t g_semBtsEvent = NULL;

//*****************************************************************************
//
// Perform initial setup for the bts task.
//
//*****************************************************************************
void
BtsTaskSetup(void)
{
    am_util_stdio_printf("Bts task: setup\r\n");
}

//*****************************************************************************
//
// BtsTask
//
//*****************************************************************************

void
BtsTask(void *pvParameters)
{
    uint32_t i = 0;
    const TickType_t xDelay = 10000 / portTICK_PERIOD_MS; // 10 second delay

    g_semBtsEvent = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semBtsEvent!= NULL);
    if ( g_semBtsEvent == NULL )
    {
        am_util_stdio_printf("Create semphore failed!\n");
        vTaskDelete(NULL);
    }
    
    am_util_stdio_printf("\r\nRunning BtsTask\r\n");

    while (1)
    {
        // Print out number of iterations
        am_util_stdio_printf("\r\nBtsTask Iteration: %d, Unity Test Failures: %d\r\n", i++, Unity.CurrentTestFailed);

        xSemaphoreGive(g_semBtsEvent);

        // Delay (block) the task for a specified period
        // Eventually we will replace this with a semaphore which
        // we will wait for from a "bring-to-see" interrupt
        vTaskDelay(xDelay);

        if(g_bTestEnd)
        {
            break;
        }


    }
    // Indicate the BtsTask is complete
    g_bBtsTaskComplete = true;
    am_util_stdio_printf("\r\nBtsTask Complete!\r\n");
    // Suspend and delete the task
    vTaskDelete(NULL);
}
