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

#include "sensor_hub_test_cases.h"
#include "lvgl.h"

//*****************************************************************************
//
// Task Config.
//
//*****************************************************************************
#define TEST_NUM 1

static uint32_t g_ui32TestCnt;
bool g_bTestEnd = false;

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
bool bBackToHpMode = false;
extern volatile bool g_bProcessSensorsTaskComplete;
extern volatile bool g_bGuiTaskComplete;
extern volatile bool g_bEmmcTaskComplete;
extern volatile bool g_bBtsTaskComplete;


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
    am_hal_gpio_output_set(GPIO_SLEEP_WAKE);
#ifndef APOLLO5_FPGA
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

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
    am_hal_gpio_output_clear(GPIO_SLEEP_WAKE);
    return;
}

TaskHandle_t WakeUpTaskHandle;

void
WakeUpTask(void *pvParameters)
{
    while(1)
    {
#ifdef DEEPSLEEP_DEBUG
        //am_util_stdio_printf("WakeUpTask.\r\n");
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
    //am_util_stdio_printf("Heap malloc failed!\r\n");

    pTaskName = pcTaskGetName(NULL);

    //am_util_stdio_printf("Suspend %s task...\r\n", pTaskName);

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
    if(g_bTestEnd)
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
MessageBufferHandle_t g_xSensorDataMessageBuffer;

void
setup_task(void *pvParameters)
{
    uint32_t i;
    uint32_t ui32Status;
    //
    // Print a debug message.
    //
    am_util_stdio_printf("Running setup tasks...\r\n");

    //
    // Stop task switch
    //
    vTaskSuspendAll();
    
    
    //xTaskCreate(WakeUpTask, "WakeUpTask", 512, 0, 3, &WakeUpTaskHandle);

    g_xSensorDataMessageBuffer = xMessageBufferCreate(MSG_BUF_BYTES + 4); // add 4 bytes to send buf len 

#if ( PROCESS_SENSORS_TASK_ENABLE == 1 )
    //
    // Create the Process Sensors task, which will process SPI data from external Sensor Hub
    //
    xTaskCreate(ProcessSensorsTask, "ProcessSensorsTask", 1024, 0, 4, &ProcessSensorsTaskHandle);
    
#endif

    //
    // Create the Bring To See task
    //
#if ( BTS_TASK_ENABLE == 1 )
        xTaskCreate(BtsTask, "BtsTask", 512, 0, 3, &BtsTaskHandle);
#endif

#if ( GUI_TASK_ENABLE == 1 )
    //
    // Create the Gui tasks, this task will control render task and display task
    //
    xTaskCreate(GuiTask, "GuiTask", 1024, 0, 4, &GuiTaskHandle);

    //
    // Create the display tasks, put at the highest priority to save power.
    //
    xTaskCreate(DisplayTask, "DisplayTask", 1024, 0, 5, &DisplayTaskHandle);

#endif

    //
    // Create the emmc tasks
    //
#if ( EMMC_TASK_ENABLE == 1 )
        xTaskCreate(EmmcTask, "EmmcTask", 512, 0, 1, &EmmcTaskHandle);
#endif


    //
    // Resume task switch
    //
    xTaskResumeAll();

    //
    // The setup operations are complete, so suspend the setup task now.
    //
    vTaskDelete(NULL);

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
    // Set some interrupt priorities before we create tasks or start the scheduler.
    //
    // Note: Timer priority is handled by the FreeRTOS kernel, so we won't
    // touch it here.
    //
#if (GUI_TASK_ENABLE == 1)
    prvHeapInit_external((void*)(MSPI_XIP_BASE_ADDRESS + PSRAM_HEAP_AREA_OFFSET),
                         (MSPI_PSRAM_SIZE-PSRAM_HEAP_AREA_OFFSET));
#endif

    //
    // Create essential tasks.
    //
    xTaskCreate(setup_task, "Setup", 512, 0, TASK_PRIORITY_HIGH, &xSetupTask);

    //PWRCTRL->CPUPWRCTRL_b.SLEEPMODE = 1;

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

