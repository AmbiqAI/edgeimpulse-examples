//*****************************************************************************
//
//! @file serial_task.c
//!
//! @brief FreeRTOS task providing overall control of the Apollo application.
//!
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

#include "ser_queue.h"
#include "serial_task.h"

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
TaskHandle_t xSerialTask;
void SerialTask(void *pvParameters);

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
// Handler Serial Port ISR derived events
//
//*****************************************************************************
static uint16_t serial_count = 0;
static void serial_handler(uint32_t event_code)
{
    volatile uint32_t *pui32Temp;
    char *cTemp;

    if ( event_code & RTOS_TICK_1_SECOND )
    {
        //        am_devices_led_toggle(am_bsp_psLEDs,0);     // dv*** 10242016

        //
        // Keep track of the seconds
        //
        serial_count++;

        //
        // print the serial count;
        //
        pui32Temp = buffer_pool_get();
        configASSERT(pui32Temp);
        cTemp = (char *) pui32Temp;
        am_util_stdio_sprintf(cTemp, "          serial_handler: serial count = %d\n", serial_count);
        print_via_itm_task(pui32Temp);
    }

    if ( serial_count & 0x10 )
    {
        serial_count = 0;
        xTaskNotifyGive(xTaskNotifyTest);
    }
}

//*****************************************************************************
//
// The User Application task Code Itself.
//
//*****************************************************************************
void
SerialTask(void *pvParameters)
{
    SERQueueElement_t    SerQElement;
    //fixme remove BaseType_t xResult;

    am_util_debug_printf("SerialTask: starting\n\r");


    //
    // stay in this loop forever. Mostly we sleep!!!
    //
    while(1)
    {
        SerQElement.SerQueType = 0xff;

        //
        // wait for an element to arrive on the APP.
        //
        SERQueue_read(&SerQElement);
        {

            //    am_util_debug_printf("AppQElement.key_press = 0x%x\n\r", AppQElement.key_press);
            //    AppQElement.key_press = 0;

            // Go and handle the specific element that arrived
            switch ( SerQElement.SerQueType )
            {
                case SERQueueSF_RTOS_EVENT:
                    serial_handler(SerQElement.u.RTOS_event);
                    // am_devices_led_toggle(am_bsp_psLEDs,0);     // dv*** 10242016
                    break;
                default:
                    break;
            };
        }
    }
}



//*****************************************************************************
//
// Initialize the User Serial Task.
//
//*****************************************************************************
void
SerialTaskSetup(void)
{
    am_util_debug_printf("SerialTask: setup\n\r");

    //
    // Create the User serial Task
    //
    xTaskCreate(SerialTask, "Serial", 256 << 1, 0, configMAX_PRIORITIES - 2, &xSerialTask);

    //
    // Create the User Serial Task Main Queue.
    //
    SERQueue_create();

    //
    // check things out.
    //
    configASSERT( g_SERQueue );
}

