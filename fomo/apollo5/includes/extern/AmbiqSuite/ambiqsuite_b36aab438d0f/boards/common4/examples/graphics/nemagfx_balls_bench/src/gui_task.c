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
#include "nemagfx_balls_bench.h"

#ifndef BAREMETAL

TaskHandle_t gui_task_handle;

EventGroupHandle_t xGuiEventHandle;

//*****************************************************************************
//
// Perform initial setup for the gui task.
//
//*****************************************************************************
void
GuiTaskSetup(void)
{
    am_util_debug_printf("GuiTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xGuiEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xGuiEventHandle == NULL);
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    extern void balls_bench(void);
    balls_bench();
    while (1)
    {
        vTaskDelay(1000);
    }
}
#endif
