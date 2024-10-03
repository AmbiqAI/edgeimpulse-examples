//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle XIP operations.
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

#define MAX_XIP_TASK_DELAY    (100 * 2)
 

//*****************************************************************************
//
// XIP task handle.
//
//*****************************************************************************
TaskHandle_t XipTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
extern void loops_all_mid_10k_sp_main(int argc, char *argv[]);
extern uint32_t ui32LoadFlag;


//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
XipTask(void *pvParameters)
{
    int argc = 4;
    char* argv[4];
    argv[0] = "loops_all_mid_10k_sp";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v1";
    
    uint32_t randomDelay;
    
    while ( ui32LoadFlag != 0xdeadbeef )
    {
        am_util_stdio_printf("XIP: waiting for PSRAM loaded...\r\n");
        vTaskDelay(100);
    }

    while (1)
    {
        am_util_stdio_printf("Running loops_all_mid_10k_sp tasks...\r\n");

        loops_all_mid_10k_sp_main(argc, argv);

        am_util_stdio_printf("loops_all_mid_10k_sp tasks done...\r\n");
        
        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_XIP_TASK_DELAY;
        vTaskDelay(randomDelay);
        if (eTaskGetState(GuiTaskHandle) == eDeleted)
        {
            break;
        }
    }
    am_util_stdio_printf("\nXIP task end!\n");

    vTaskDelete(NULL);
    while (1);
}
