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

#include "ser_queue.h"

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
QueueHandle_t g_SERQueue;


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
// Initialize the SERIAL Queue
// Returns true for failure.
//
//*****************************************************************************
bool
SERQueue_create(void)
{
    // Create the SER Queue
    g_SERQueue = xQueueCreate ( SER_QUEUE_NUM_ELEMENTS,
                       sizeof(SERQueueElement_t) );
    return (g_SERQueue == NULL);
}

//*****************************************************************************
//
// Send a message to the SER Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool
SERQueue_send(SERQueueElement_t *Element)
{
    return xQueueSendToBack(g_SERQueue, Element, 0 );
}

//*****************************************************************************
//
// Send a message to the SER Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool
SERQueue_sendFromISR(SERQueueElement_t *Element)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = 0;

    xResult = xQueueSendFromISR(g_SERQueue, Element, &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    return xResult != pdFAIL;
}

//*****************************************************************************
//
// Read a message from the SER Queue.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully read.
//
//*****************************************************************************
bool
SERQueue_read(SERQueueElement_t *Element)
{
     return xQueueReceive(g_SERQueue, Element, portMAX_DELAY );
}

//*****************************************************************************
//
// Check the SER Queue to see if it is too full to accept a message (non-blocking).
// Returns true if queue is full.
//
//*****************************************************************************
bool
SERQueue_is_full(void)
{
    UBaseType_t NumAvailable;

    //
    // Query the space available in the queue.
    //
    NumAvailable = uxQueueSpacesAvailable(g_SERQueue);

    //
    // Figure out if it is full
    //
    if ( NumAvailable >= SER_QUEUE_NUM_ELEMENTS )
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
// Check the SER queue to see if it contains any messages (non-blocking).
// Returns true if queue is empty.
//
//*****************************************************************************
bool
SERQueue_is_empty(void)
{
    UBaseType_t NumAvailable;

    //
    // Query the space available in the queue.
    //
    NumAvailable = uxQueueSpacesAvailable(g_SERQueue);

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

