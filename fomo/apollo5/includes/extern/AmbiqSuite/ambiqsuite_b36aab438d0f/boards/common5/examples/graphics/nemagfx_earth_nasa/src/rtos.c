//*****************************************************************************
//
//! @file rtos.c
//!
//! @brief Essential functions to make the RTOS run correctly.
//!
//! These functions are required by the RTOS for ticking, sleeping, and basic
//! error checking.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_earth_nasa.h"
#include "event_task.h"
#include "render_task.h"
#include "display_task.h"
//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_HIGH              (configMAX_PRIORITIES -1)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_HIGH - 1)
#define TASK_PRIORITY_LOW               (TASK_PRIORITY_MIDDLE - 1)

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;

//*****************************************************************************
//
// Frame buffer lock.
//
//*****************************************************************************
SemaphoreHandle_t g_semDisplayStart = NULL;
SemaphoreHandle_t g_semDisplayEnd = NULL;

//*****************************************************************************
//
// Event semphore.
//
//*****************************************************************************
SemaphoreHandle_t g_semEventStart = NULL;
SemaphoreHandle_t g_semEventEnd = NULL;

//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t
am_freertos_sleep(uint32_t idleTime)
{
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void
am_freertos_wakeup(uint32_t idleTime)
{
    return;
}

//*****************************************************************************
//
// FreeRTOS debugging functions.
//
//*****************************************************************************
void
vApplicationMallocFailedHook(void)
{
    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    while (1);
}

void
vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    //
    // Run time stack overflow checking is performed if
    // configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    //
    while (1)
    {
        __asm("BKPT #0\n") ; // Break into the debugger
    }
}

#ifndef BAREMETAL

//*****************************************************************************
//
// High priority task to run immediately after the scheduler starts.
//
// This task is used for any global initialization that must occur after the
// scheduler starts, but before any functional tasks are running. This can be
// useful for enabling events, semaphores, and other global, RTOS-specific
// features.
//
//*****************************************************************************
void
setup_task(void *pvParameters)
{
    vTaskSuspendAll();

    //
    // Create the semaphone for event task
    //
    g_semEventStart = xSemaphoreCreateBinary();
    g_semEventEnd = xSemaphoreCreateBinary();

    //
    // Create the semaphone for buffer sync task
    //
    g_semDisplayStart = xSemaphoreCreateBinary();
    g_semDisplayEnd = xSemaphoreCreateBinary();

    //
    // Create the event tasks with the low priority.
    //
    xTaskCreate(EventTask, "EventTask", 1024, 0, TASK_PRIORITY_LOW, &EventTaskHandle);

    //
    // Create the render tasks with the high priority.
    //
    xTaskCreate(RenderTask, "RenderTask", 1024, 0, TASK_PRIORITY_HIGH, &RenderTaskHandle);

    //
    // Create the display tasks with the middle priority.
    //
    xTaskCreate(DisplayTask, "DisplayTask", 1024, 0, TASK_PRIORITY_HIGH, &DisplayTaskHandle);

    //
    // Resume task switch
    //
    xTaskResumeAll();

    //
    // The setup operations are complete, so suspend the setup task now.
    //
    vTaskDelete(NULL);

    while (1);
}

//*****************************************************************************
//
// Initializes all tasks
//
//*****************************************************************************
void
run_tasks(void)
{
    //
    // Set some interrupt priorities before we create tasks or start the scheduler.
    //
    // Note: Timer priority is handled by the FreeRTOS kernel, so we won't
    // touch it here.
    //

    //
    // Create essential tasks.
    //
    xTaskCreate(setup_task, "Setup", 512, 0, TASK_PRIORITY_HIGH, &xSetupTask);

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks to be created.  See the memory management section on the
     * FreeRTOS web site for more details (this is standard text that is not
     * really applicable to the Win32 simulator port). */
    while (1);
}

#endif
