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

#include <stdint.h>
#include <stdbool.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"

#include "lvgl.h"

#include "complex_test_cases.h"


//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
extern volatile uint32_t g_ui32IdleCycleCount;
extern volatile bool g_bTimerTaskComplete;
extern volatile bool g_bPlaybackTaskComplete;
extern volatile bool g_bFileWriterTaskComplete;
extern volatile bool g_bCryptoTaskComplete;
extern volatile bool g_bWifiSourceTaskComplete;

//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t am_freertos_sleep(uint32_t idleTime)
{
    am_hal_gpio_output_toggle(GPIO_SLEEP_ENTRY);

    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    am_hal_gpio_output_toggle(GPIO_SLEEP_EXIT);

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

void
vApplicationIdleHook(void)
{
    am_hal_gpio_output_toggle(GPIO_IDLE_TASK);
    if (g_bPlaybackTaskComplete)
    {
        // Conclude the test and report the results
        if (TEST_PROTECT())
        {
            tearDown();
        }
        UnityConcludeTest();
        globalTearDown();
        UnityEnd();
        vTaskSuspendAll();
        vTaskSuspend(NULL);
    }
}

//*****************************************************************************
//
// FreeRTOS static allocator memory get callbacks
//
// Because we have static allocation turned on in FreeRTOS, we must define
// callbacks for the functions below. See
// https://www.freertos.org/a00110.html#configSUPPORT_STATIC_ALLOCATION */
//
//*****************************************************************************

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

//*****************************************************************************
//
// Get CPU occupation rate.
//
//*****************************************************************************
uint32_t getCpuOccupationRate(void)
{
    TaskHandle_t idleHandle = xTaskGetIdleTaskHandle();

    TaskStatus_t xTaskDetails;

    // Use the handle to obtain further information about the task.
    vTaskGetInfo( idleHandle,
                  &xTaskDetails,
                  pdTRUE, // Include the high water mark in xTaskDetails.
                  eSuspended ); // Include the task state in xTaskDetails.

    //Counter overflow has not been considered.
    uint32_t idleRunTime = xTaskDetails.ulRunTimeCounter;
    uint32_t totalRunTime = xTaskGetTickCount();

    static uint32_t lastIdleRunTime = 0;
    static uint32_t lastTotalRunTime = 0;

    uint32_t runTimeElaps;
    uint32_t totalTimeElaps;

    //If counter overflow
    if(totalRunTime < lastTotalRunTime)
    {
        totalTimeElaps = (uint64_t)totalRunTime + 0xffffffff - lastTotalRunTime;
    }
    else
    {
        totalTimeElaps = totalRunTime - lastTotalRunTime;
    }

    //If counter overflow
    if(idleRunTime < lastIdleRunTime)
    {
        runTimeElaps = (uint64_t)idleRunTime + 0xffffffff - lastIdleRunTime;
    }
    else
    {
        runTimeElaps = idleRunTime - lastIdleRunTime;
    }

    //Update record time
    lastIdleRunTime = idleRunTime;
    lastTotalRunTime = totalRunTime;


    //if not overflow
    if(runTimeElaps < totalTimeElaps)
        return 100 - runTimeElaps * 100 / totalTimeElaps ;
    else
        return 0;
}


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
    //
    // Print a debug message.
    //
    am_util_stdio_printf("\r\nRunning setup tasks...\r\n");

    //
    // Run setup functions.
    //
    GuiTaskSetup();
    WifiSourceTaskSetup();
    FileWriterTaskSetup();
    PlaybackTaskSetup();
    CryptoTaskSetup();
    //
    // Create the functional tasks
    //

    xTaskCreate(WifiSourceTask, "WifiSourceTask", 1024, 0, TASK_PRIORITY_LOW, &wifi_source_task_handle);
    xTaskCreate(FileWriterTask, "FileWriterTask", 1024, 0, TASK_PRIORITY_LOW, &file_writer_task_handle);
    xTaskCreate(PlaybackTask, "PlaybackTask", 2048, 0, TASK_PRIORITY_LOW, &playback_task_handle);
    xTaskCreate(CryptoTask, "CryptoTask", 1024, 0, TASK_PRIORITY_LOW, &crypto_task_handle);
    xTaskCreate(GuiTask, "GuiTask", 1024, 0, TASK_PRIORITY_MIDDLE, &GuiTaskHandle);
    xTaskCreate(DisplayTask, "DisplayTask", 1024, 0, TASK_PRIORITY_HIGH, &DisplayTaskHandle);

    vTaskSuspend(NULL);

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
    xTaskCreate(setup_task, "Setup", 512, 0, 7, &xSetupTask);

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();
}

