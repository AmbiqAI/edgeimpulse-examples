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

#include "sdio_rs9116_emmc_multiple_instances_test_cases.h"
#include "emmc_task.h"
#include "sdio_rs9116_wifi_interface_task.h"
//*****************************************************************************
//
// Task Config.
//
//*****************************************************************************

static uint32_t g_ui32TestCnt;
static bool g_bReportedResults = false;

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;
TaskHandle_t emmc_task_handle;
TaskHandle_t sdio_task_handle;

bool bBackToHpMode = false;

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
// #### INTERNAL BEGIN ####
#ifndef APOLLO5_FPGA
// #### INTERNAL END ####
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
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
    return;
}

TaskHandle_t WakeUpTaskHandle;

void
WakeUpTask(void *pvParameters)
{
    while(1)
    {
#ifdef DEEPSLEEP_DEBUG
        am_util_stdio_printf("WakeUpTask.\r\n");
#endif
        vTaskDelay(100);
    }
}

//*****************************************************************************
//
// FreeRTOS debugging functions.
//
//*****************************************************************************
void
vApplicationMallocFailedHook(void)
{
    const char* pTaskName;

    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    am_util_stdio_printf("Heap malloc failed!\r\n");

    pTaskName = pcTaskGetName(NULL);

    am_util_stdio_printf("Suspend %s task...\r\n", pTaskName);

    vTaskSuspend(NULL);
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
    if( (eTaskGetState(emmc_task_handle) == eDeleted) && (eTaskGetState(sdio_task_handle) == eDeleted) && !g_bReportedResults)
    {
        am_util_stdio_printf("Test loop - %d done!\r\n", g_ui32TestCnt);
        g_ui32TestCnt++;

        if (g_ui32TestCnt < TEST_NUM)
        {
            vTaskDelete(WakeUpTaskHandle);
            vTaskResume(xSetupTask);
        }
        else
        {
            if (!g_bReportedResults)
            {
                if (TEST_PROTECT())
                {
                    tearDown();
                }
                UnityConcludeTest();
                globalTearDown();
                UnityEnd();
                g_bReportedResults = true;
            }
        }
    }
}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
     *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
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
    am_util_stdio_printf("Running sdio multiple test cases tasks...\r\n");
    g_ui32TestCnt = 0;

    //
    // Prepare differern sdio mode configuration.
    //
    emmc_test_cfg_init();

    while(1)
    {
        am_util_stdio_printf("\n*--------------------------------*\r\n");
        am_util_stdio_printf("Test loop - %d start...\r\n", g_ui32TestCnt);

        //
        // Stop task switch
        //
        vTaskSuspendAll();

        //
        // Create the semaphone used to sync 2 EMMC tasks
        //
        g_semEMMCWrite = xSemaphoreCreateBinary();
        TEST_ASSERT_TRUE(g_semEMMCWrite != NULL);
        if ( g_semEMMCWrite == NULL )
        {
            am_util_stdio_printf("Create semphone failed!\n");
            vTaskDelete(NULL);
        }

        g_semEMMCRead = xSemaphoreCreateBinary();
        TEST_ASSERT_TRUE(g_semEMMCRead != NULL);
        if ( g_semEMMCRead == NULL )
        {
            am_util_stdio_printf("Create semphone failed!\n");
            vTaskDelete(NULL);
        }
        g_semSdioWrite = xSemaphoreCreateBinary();
        TEST_ASSERT_TRUE(g_semSdioWrite != NULL);
        if ( g_semSdioWrite == NULL )
        {
            am_util_stdio_printf("Create semphone failed!\n");
            vTaskDelete(NULL);
        }

        g_semSdioRead = xSemaphoreCreateBinary();
        TEST_ASSERT_TRUE(g_semSdioRead != NULL);
        if ( g_semSdioRead == NULL )
        {
            am_util_stdio_printf("Create semphone failed!\n");
            vTaskDelete(NULL);
        }

        //
        // Create the emmc tasks
        //
        xTaskCreate(EmmcTask, "EmmcTask", 512, 0, 4, &emmc_task_handle);

        xTaskCreate(Sdio_RS116_WiFi_interface_Task, "Sdio_RS116_WiFi_interface_Task", 512, 0, 4, &sdio_task_handle);

        xTaskCreate(WakeUpTask, "WakeUpTask", 512, 0, 4, &WakeUpTaskHandle);

        //
        // Resume task switch
        //
        xTaskResumeAll();

        //
        // The setup operations are complete, so suspend the setup task now.
        //
        vTaskSuspend(NULL);
    }
}

extern void prvHeapInit_external( void* address, size_t length );
//*****************************************************************************
//
// Initializes all tasks
//
//*****************************************************************************
void
run_tasks(void)
{
    //
    // Create essential tasks.
    //
    xTaskCreate(setup_task, "Setup", 512, 0, 4, &xSetupTask);

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
