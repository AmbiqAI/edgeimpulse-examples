//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief FreeRTOS task providing overall control of the Apollo application.
//!
//! This task supervises and manages the various task components. It interfaces
//! with either an IOS or Android cell phone over blue tooth low energy to
//! report fitness data.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"

#include "rtos.h"

#include "freertos_sampler.h"

#include "app_queue.h"
#include "app_task.h"

#include "buffer_pool.h"
#include "itm_queue.h"

//*****************************************************************************
//
// Macros
//
//*****************************************************************************

//*****************************************************************************
//
// Task handle and prototype.
//
//*****************************************************************************
TaskHandle_t xAppTask;
void AppTask(void *pvParameters);

//*****************************************************************************
//
// Global variables
//
//*****************************************************************************

//*****************************************************************************
//
// Event handles
//
//*****************************************************************************

//*****************************************************************************
//
// Private (static) Variables
//
//*****************************************************************************

//*****************************************************************************
//
// Use the ITM task to print a string.
// The ITM will free the buffer item when it has finished printing it.
//
//*****************************************************************************
static void
print_via_itm_task(volatile uint32_t *pui32Buffer)
{
    ITMQueueElement_t itm_msg;

    //
    // Enqueue it to the ITM task.
    //
    itm_msg.ITMQueType = ITMQueue_PRINT_IT;
    itm_msg.u.pui32String = pui32Buffer;
    ITMQueue_send(&itm_msg);

}

//*****************************************************************************
//
// Handle RTOS timer ISR derived events
//
//*****************************************************************************
static uint16_t seconds_count = 0;
static uint16_t ten_seconds_count = 0;
static void app_handle_rtos_tick(uint32_t event_code)
{
    static int action_picker = 0;
    volatile uint32_t *pui32Temp;
    char *cTemp;
    if ( event_code & RTOS_TICK_BUTTON0_RELEASED )
    {

        // prepare to print
        pui32Temp = buffer_pool_get();
        configASSERT(pui32Temp);
        cTemp = (char *) pui32Temp;
        am_util_stdio_sprintf(cTemp, "\t\t\tButton: released %d\n\n", action_picker );
        print_via_itm_task(pui32Temp);

        // prepare to print
        pui32Temp = buffer_pool_get();
        configASSERT(pui32Temp);
        cTemp = (char *) pui32Temp;
        if ( action_picker < 1 )
        {
                am_util_stdio_sprintf(cTemp, "\t\t\t\tButton: %d\n\n", action_picker );
                action_picker++;
        }
        else if ( action_picker == 1 )
        {
                am_util_stdio_sprintf(cTemp, "\t\t\t\tButton: %d\n\n", action_picker );
                action_picker++;
        }
        else
        {
                am_util_stdio_sprintf(cTemp, "\t\t\t\tButton: else %d\n\n", action_picker );
                action_picker++;
        }
        print_via_itm_task(pui32Temp);
    }

    if ( event_code & RTOS_TICK_1_SECOND )
    {
        am_devices_led_toggle(am_bsp_psLEDs, 0);    // dv*** 10242016

        //
        // Keep track of the seconds
        //
        seconds_count++;

        //
        // print the seconds count;
        //
        pui32Temp = buffer_pool_get();
        configASSERT(pui32Temp);
        cTemp = (char *) pui32Temp;
        am_util_stdio_sprintf(cTemp, "          app_handle_rtos_tick: seconds = %d\n", seconds_count);
        print_via_itm_task(pui32Temp);

    }
}



//*****************************************************************************
//
// The User Application task Code Itself.
//
//*****************************************************************************
void
AppTask(void *pvParameters)
{
    APPQueueElement_t    AppQElement;
    //fixme remove BaseType_t xResult;

    am_util_debug_printf("AppTask: starting\n\r");


    //
    // stay in this loop forever. Mostly we sleep!!!
    //
    while(1)
    {
        AppQElement.AppQueType = 0xff;
        //
        // wait for an element to arrive on the APP.
        //
        APPQueue_read(&AppQElement);

        //
        // We got an APP Queue Element so go process it
        //
        {
            //    am_util_debug_printf("AppQElement.key_press = 0x%x\n\r", AppQElement.key_press);
            //    AppQElement.key_press = 0;

            // Go and handle the specific element that arrived
            switch(AppQElement.AppQueType)
            {
                case APPQueueSF_RTOS_EVENT:
                    app_handle_rtos_tick(AppQElement.u.RTOS_event);
                    am_devices_led_toggle(am_bsp_psLEDs, 0);        // dv*** 10242016
                    break;
                default:
                    break;
            };
        }
    }
}



//*****************************************************************************
//
// Initialize the User Application Task.
//
//*****************************************************************************
void
AppTaskSetup(void)
{
    am_util_debug_printf("AppTask: setup\n\r");

    //
    // Create the User Application Task
    //
    xTaskCreate(AppTask, "App", 256 << 1, 0, configMAX_PRIORITIES - 2, &xAppTask);

    //
    // Create the User Application Task Main Queue.
    //
    APPQueue_create();

    //
    // check things out.
    //
    configASSERT( g_APPQueue );
}
