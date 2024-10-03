//*****************************************************************************
//
//! @file display_task.c
//!
//! @brief Task to handle DISPLAY operations.
//!
//! this task could support Display controller or MSPI driving display based on
//! different BSPs.
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
#include "usb_hs_stress_test.h"

volatile bool g_bDisplayTaskComplete = false ;
extern volatile bool g_bGuiTaskComplete ;

#if DISPLAY_TASK_ENABLE==1

//*****************************************************************************
//
// DISPLAY task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;


#include "graphics_power_control.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//#define DEBUG

#ifdef DEBUG
#define BUFFER_SIZE (FB_RESX*FB_RESY / SMALLFB_STRIPES * 3)
#endif


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************

void
DisplayTask(void *pvParameters)
{
    int ret;
    uint32_t ui32TaskCount = 0;
    am_util_stdio_printf("Display task start!\n");

    //Init display
    ret = am_devices_display_init(FB_RESX, FB_RESY, DISPLAY_FORMAT, false);
    if (ret != 0)
    {
        am_util_stdio_printf("display init failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

#ifdef DEBUG
    uint8_t* buffer = (uint8_t*)SSRAM1_BASEADDR;
#endif

#ifdef TASK_LOOP_NUM		
    while(ui32TaskCount < TASK_LOOP_NUM)
    {
        ui32TaskCount ++;
#else
    while(1)
    {
#endif

#ifdef DEBUG
        memset(buffer, 0x0+ g_intDisplayPart*0x3F, BUFFER_SIZE);
        g_pFrameBufferDC->bo.base_phys = (uintptr_t)buffer;
#endif

				
        //Wait start.
        xSemaphoreTake( g_semDCStart, portMAX_DELAY);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(25);
        am_hal_gpio_output_clear(26);
#endif


#ifdef SMALLFB
        //Set panel refresh region
        am_devices_display_set_region(FB_RESX,
                                      FB_RESY / SMALLFB_STRIPES,
                                      0,
                                      (FB_RESY / SMALLFB_STRIPES) * g_intDisplayPart);
#endif


        //frame transfer
        am_devices_display_transfer_frame(FB_RESX,
                                          FB_RESY / SMALLFB_STRIPES,
                                          g_pFrameBufferDC->bo.base_phys,
                                          NULL,
                                          NULL);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(23);
#endif

        //Wait frame transfer completion.
        am_devices_display_wait_transfer_done();

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(23);
#endif

        xSemaphoreGive(g_semDCEnd);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(25);
#endif

#ifdef DEBUG
        g_intDisplayPart ++;
        if(g_intDisplayPart > 3)
        {
            g_intDisplayPart = 0;
        }
#endif
    }
	
    //Wait for other tasks to complete
    vTaskDelay(1000);
		
	
    g_bDisplayTaskComplete = true;
	

    //Suspend and delete this task.
    vTaskDelete(NULL);
}

#endif
