//*****************************************************************************
//
//! @file itm_task.c
//!
//! @brief FreeRTOS task to manage output thruugh item registers.
//!
//! This task serves an itm register write queue. Queue entries can be
//! arbitrary ITM register writes or print strings. This task includes a
//! state machine to manage the power state of the ITM/TPIU/SWO pieces of this
//! output streaming mechansim.  When the ITM has been idle for 3 ticks then
//! everything is powered down. When a queue element arrives to a powered down
//! ITM flow then everything is powered up and it all starts over.
//!
//! The state machine also manages the output of strings of up to 128
//! characters. These 128 byte strings come attached to a single queue element.
//!
//! The goal is to provide thread safe print output from multiple tasks writing
//! to the single ITM output queue. While managing the ITM/TPIU/SWO power
//! state.
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

#include "freertos_sampler.h"
#include "rtos.h"

#include "itm_queue.h"
#include "itm_task.h"

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define STRING_SIZE 128
#define NUMBER_STRINGS 8

//*****************************************************************************
//
// Task handle.
//
//*****************************************************************************
TaskHandle_t xITMTask;



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
// Post an arbitrary stimulus register to the ITM.
// WARNING This call can BLOCK!!!
// WARNING Do not call this function from an ISR.
//
//*****************************************************************************
void
ITMWrite2Stimulus(uint32_t stim_reg, uint32_t stim_value)
{
    ITMQueueElement_t element;
    //
    //  load values in to the local stim structure
    //
    element.u.s.stim_register = stim_reg;
    element.u.s.stim_value = stim_value;

    //
    // pass the stimulus register number and value to the ITM task.
    //
    ITMQueue_send(&element); // blocking, but it can fail
//fixme change this so that the caller just builds the structure and passes it using noram queue interface calls.
}


//*****************************************************************************
//
// The ITM task Code Itself.
//
//*****************************************************************************
void
vITMTask(void *pvParameters)
{
    ITMQueueElement_t ITMQElement;

    //
    // stay in this loop forever
    //
    while(1)
    {
        //
        // wait for an element to arrive on the APP.
        //
        ITMQueue_read(&ITMQElement);

        // Go and handle the specific element that arrived
        switch(ITMQElement.ITMQueType)
        {
            case ITMQueue_ITM_REG:
                // fixme not implemented yet
                am_util_debug_printf("ITM Task  received ITM_REG queue element\n");
                break;
            case ITMQueue_PRINT_IT:
                am_util_debug_printf((char *)ITMQElement.u.pui32String);
                break;
            default:
                am_util_debug_printf("ITM Task ERROR received default: queue element\n");
                break;
        };
    }
}

//*****************************************************************************
//
// Initialize the ITM task.
//
//*****************************************************************************
void
ITMTaskSetup(void)
{

    //
    // Create the queue for this task to serve.
    //
    ITMQueue_create();

    //
    // Create the ITM task.
    //
    xTaskCreate(vITMTask, "ITM", 128, 0, 3, &xITMTask);
}
