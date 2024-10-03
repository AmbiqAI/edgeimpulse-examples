//*****************************************************************************
//
//! @file itm_queue.c
//!
//! @brief Queue interface to ITM task for handling printf outputs.
//!
//! These functions and definitions provide the interface for both the writer
//! and the reader of the ITM queue.
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

//*****************************************************************************
//
// Macros
//
//*****************************************************************************

//*****************************************************************************
//
// Queue handle
//
//*****************************************************************************
QueueHandle_t g_ITMQueue;


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
// Initialize the ITM Queue
// Returns true for failure.
//
//*****************************************************************************
bool
ITMQueue_create(void)
{
    // Create the ITM Queue
    g_ITMQueue = xQueueCreate ( ITM_QUEUE_NUM_ELEMENTS,
                               sizeof(ITMQueueElement_t)
                             );
    return (g_ITMQueue == NULL);
}

//*****************************************************************************
//
// Send a message to the ITM Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool
ITMQueue_send(ITMQueueElement_t *Element)
{
    return xQueueSendToBack(g_ITMQueue, Element, 0 );
}

//*****************************************************************************
//
// Send a message to the ITM Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool
ITMQueue_sendFromISR(ITMQueueElement_t *Element)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = 0;

    xResult = xQueueSendFromISR(g_ITMQueue, Element, &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return xResult != pdFAIL;
}

//*****************************************************************************
//
// Read a message from the ITM Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully read.
//
//*****************************************************************************
bool
ITMQueue_read(ITMQueueElement_t *Element)
{
     return xQueueReceive(g_ITMQueue, Element, portMAX_DELAY );
}

//*****************************************************************************
//
// Check the ITM Queue to see if it is too full to accept a message (non-blocking).
// Returns true if queue is full.
//
//*****************************************************************************
bool
ITMQueue_is_full(void)
{
    UBaseType_t NumAvailable;

    //
    // Query the space available in the queue.
    //
    NumAvailable = uxQueueSpacesAvailable(g_ITMQueue);

    //
    // Figure out if it is full
    //
    if ( NumAvailable >= ITM_QUEUE_NUM_ELEMENTS )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//*****************************************************************************
//
// Check the ITM queue to see if it contains any messages (non-blocking).
// Returns true if queue is empty.
//
//*****************************************************************************
bool
ITMQueue_is_empty(void)
{
    UBaseType_t NumAvailable;

    //
    // Query the space available in the queue.
    //
    NumAvailable = uxQueueSpacesAvailable(g_ITMQueue);

    //
    // Figure out if it is empty
    //
    if ( NumAvailable == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

