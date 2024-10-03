//*****************************************************************************
//
//! @file app_queue.c
//!
//! @brief APP queue interface routines.
//!
//! These functions and definitions provide the interface for both the writer
//! and the reader of the APP queue.
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

#include "app_queue.h"

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
QueueHandle_t g_APPQueue;


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
// Initialize the APP Queue
// Returns true for failure.
//
//*****************************************************************************
bool
APPQueue_create(void)
{
    // Create the APP Queue
    g_APPQueue = xQueueCreate ( APP_QUEUE_NUM_ELEMENTS,
                               sizeof(APPQueueElement_t) );

    if ( g_APPQueue == NULL )
    {
        while(1);
    }

    return (g_APPQueue == NULL);
}

//*****************************************************************************
//
// Send a message to the APP Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool
APPQueue_send(APPQueueElement_t *Element)
{
    return xQueueSendToBack(g_APPQueue, Element, 0 );
}

//*****************************************************************************
//
// Send a message to the APP Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool
APPQueue_sendFromISR(APPQueueElement_t *Element)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = 0;

    xResult = xQueueSendFromISR(g_APPQueue, Element, &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return xResult != pdFAIL;
}

//*****************************************************************************
//
// Read a message from the APP Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully read.
//
//*****************************************************************************
bool
APPQueue_read(APPQueueElement_t *Element)
{
     return xQueueReceive(g_APPQueue, Element, portMAX_DELAY );
}

//*****************************************************************************
//
// Check the APP Queue to see if it is too full to accept a message (non-blocking).
// Returns true if queue is full.
//
//*****************************************************************************
bool
APPQueue_is_full(void)
{
    UBaseType_t NumAvailable;

    //
    // Query the space available in the queue.
    //
    NumAvailable = uxQueueSpacesAvailable(g_APPQueue);

    //
    // Figure out if it is full
    //
    if ( NumAvailable >= APP_QUEUE_NUM_ELEMENTS)
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
// Check the APP queue to see if it contains any messages (non-blocking).
// Returns true if queue is empty.
//
//*****************************************************************************
bool
APPQueue_is_empty(void)
{
    UBaseType_t NumAvailable;

    //
    // Query the space available in the queue.
    //
    NumAvailable = uxQueueSpacesAvailable(g_APPQueue);

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

