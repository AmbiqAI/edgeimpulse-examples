//*****************************************************************************
//
//! @file user_task.c
//!
//! @brief Task to handle user related operation.
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
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "rpmsg.h"

#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "user_task.h"

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void user_task(void *pvParameters);

//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************
metal_thread_t user_task_handle;
QueueHandle_t  g_UserQueue;

/**
*
* Initialize the user task queue
* Returns true for success.
*/
bool user_task_queue_create(void)
{
    // Create the user task Queue
    g_UserQueue = xQueueCreate(USER_TASK_QUEUE_NUM, sizeof(UserQueueElem_t));
    return (g_UserQueue != NULL);
}

/**
* Send a message to the SER Queue.
*
* This may block and if it does it blocks forever.
*
* Returns true if message successfully sent.
*/
bool user_task_queue_send(UserQueueElem_t *elem)
{
    if ( xPortIsInsideInterrupt() == pdTRUE )
    {
        BaseType_t xHigherPriorityTaskWoken, xResult;
        xHigherPriorityTaskWoken = 0;

        xResult = xQueueSendFromISR(g_UserQueue, elem, &xHigherPriorityTaskWoken);

        if (xResult != pdFAIL)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        return xResult != pdFAIL;
    }
    else
    {
        return xQueueSendToBack(g_UserQueue, elem, 0);
    }
    return true;
}

/**
* Read a message from the user task Queue.
*
* This may block and if it does it blocks forever.
*
* Returns true if message successfully read.
*/
bool user_task_queue_read(UserQueueElem_t *elem)
{
    return xQueueReceive(g_UserQueue, elem, portMAX_DELAY);
}

void user_task_setup(void)
{
    if (!user_task_queue_create())
    {
        am_util_debug_printf("user task queue crate failed\r\n");
    }

    int ret = metal_thread_init(&user_task_handle, user_task, "user task", 2 * 1024, 0, 3);
    if (ret)
    {
        am_util_debug_printf("metal_thread_init user task failure %d\n", ret);
        return;
    }
}

static void user_task(void *pvParameters)
{
    (void) pvParameters;

    am_util_debug_printf("user task start\r\n");

    while(1)
    {
        UserQueueElem_t element;

        element.type = 0xFFFFFFFF;

        if (user_task_queue_read(&element) == pdPASS)
        {
            ;
        }
        else
        {
            am_util_debug_printf("read usr queue failed\r\n");
        }
    }
}
