//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
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
#include "lvgl_benchmark.h"

#include "lv_demo_benchmark.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;


//*****************************************************************************
//
// GUI functions
//
//*****************************************************************************


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("GUI task start!\n");

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    lv_demo_benchmark_set_max_speed(true);
    lv_demo_benchmark();
    //lv_demo_benchmark_run_scene(40);


    while (1)
    {
        lv_timer_handler();

        //vTaskDelay(1);
    }
}
