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

#ifndef BAREMETAL
#include "nemagfx_transition_effects.h"
//*****************************************************************************
//
// gui task handle.
//
//*****************************************************************************
TaskHandle_t gui_task_handle;

//*****************************************************************************
//
// Handle for Radio-related events.
//
//*****************************************************************************
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
    while ( xGuiEventHandle == NULL );
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    transition_effects();

    while(1)
    {
        vTaskDelay(1000);
    }
}
#endif
