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

#include "daxi_test_common.h"
#include "coremark_pro.h"
//#include "posix_demo.h"

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;

#define WORKLOAD_TASK_DELAY                (1000*1000)
#define WORKLOAD_BOOTUP_TASK_PRIORITY      (4)
#define WORKLOAD_BOOTUP_TASK_STACK_SIZE    (1024)

#if 0
//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    //
    // Check the timer interrupt status.
    //
    ui32Status = am_hal_ctimer_int_status_get(false);
    am_hal_ctimer_int_clear(ui32Status);

    //
    // Run handlers for the various possible timer events.
    //
    am_hal_ctimer_int_service(ui32Status);
}
#endif

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
#ifndef APOLLO4_FPGA
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
    am_util_stdio_printf("Heap malloc failed!\r\n");
    am_util_stdio_printf("Suspend current task...\r\n");

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

extern void cjpeg_rose7_preset_main(int argc, char *argv[]);

void vStartCjpegRose7Preset( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "cjpeg-rose7-preset";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running cjpeg_rose7_preset tasks...\r\n");

        cjpeg_rose7_preset_main(argc, argv);

        am_util_stdio_printf("cjpeg_rose7_preset tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void core_main(int argc, char *argv[]);

void vStartCore( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "core";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running core tasks...\r\n");

        core_main(argc, argv);

        am_util_stdio_printf("core tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void linear_alg_mid_100x100_sp_main(int argc, char *argv[]);

void vStartLinearAlgMid100x100Sp( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "linear_alg_mid_100x100_sp";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running linear_alg_mid_100x100_sp tasks...\r\n");

        linear_alg_mid_100x100_sp_main(argc, argv);

        am_util_stdio_printf("linear_alg_mid_100x100_sp tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void loops_all_mid_10k_sp_main(int argc, char *argv[]);

void vStartLoopsAllMid10kSp( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "loops_all_mid_10k_sp";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running loops_all_mid_10k_sp tasks...\r\n");

        loops_all_mid_10k_sp_main(argc, argv);

        am_util_stdio_printf("loops_all_mid_10k_sp tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void nnet_test_main(int argc, char *argv[]);

void vStartNnetTest( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "nnet_test";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running nnet_test tasks...\r\n");

        nnet_test_main(argc, argv);

        am_util_stdio_printf("nnet_test tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void parser_125k_main(int argc, char *argv[]);

void vStartParser125k( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "parser_125k";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running parser_125k tasks...\r\n");

        parser_125k_main(argc, argv);

        am_util_stdio_printf("parser_125k tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void radix2_big_64k_main(int argc, char *argv[]);

void vStartRadix2Big64k( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "radix2_big_64k";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running radix2_big_64k tasks...\r\n");

        radix2_big_64k_main(argc, argv);

        am_util_stdio_printf("radix2_big_64k tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void sha_test_main(int argc, char *argv[]);

void vStartShaTest( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "sha_test";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running sha_test tasks...\r\n");

        sha_test_main(argc, argv);

        am_util_stdio_printf("sha_test tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

extern void zip_test_main(int argc, char *argv[]);

void vStartZipTest( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    argv[0] = "zip_test";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v0";

    while (1)
    {
        am_util_stdio_printf("Running zip_test tasks...\r\n");

        zip_test_main(argc, argv);

        am_util_stdio_printf("zip_test tasks done...\r\n");

        vTaskDelay(WORKLOAD_TASK_DELAY);
    }
}

void vStartCoremark( void *pvParameters )
{
    int argc = 4;
    char* argv[4];
    uint32_t i = 0, j = 0, ui32Status;

    am_util_stdio_printf("Running Coremark_pro tasks...\r\n");
#ifdef CSV_PRINT
#ifdef ONLY_RUN_JPEG
    am_util_stdio_printf("Test Num,CPU->CACHECFG,CPU->DAXICFG,CacheEnable,CacheLRU,CacheIEnable,CacheDEnable,CacheDescr,PASSTHROUGH,AGINGSENABLE,BUFFER,AGINGCOUNTER,FLUSHLEVEL(Free Buffers),WorkLoad,Time (secs)\n");
#else
    am_util_stdio_printf("Test Num,CPU->CACHECFG,CPU->DAXICFG,CacheEnable,CacheLRU,CacheIEnable,CacheDEnable,CacheDescr,PASSTHROUGH,AGINGSENABLE,BUFFER,AGINGCOUNTER,FLUSHLEVEL(Free Buffers),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs),WorkLoad,Time (secs)\n");
#endif
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#ifndef CSV_PRINT
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", CPU->CACHECFG, CPU->DAXICFG);
#else
            uint32_t ui32BuffNum[] = {1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 15, 16, 29, 30, 31, 32};
#if defined(CACHE_TEST) && !defined(DAXI_TEST)
            am_util_stdio_printf("%d,0x%08X,0x%08X,", j, CPU->CACHECFG, CPU->DAXICFG);
#else
            am_util_stdio_printf("%d,0x%08X,0x%08X,", j * TEST_NUM + i, CPU->CACHECFG, CPU->DAXICFG);
#endif
            am_util_stdio_printf("%d,%d,%d,%d,%d,", CPU->CACHECFG_b.ENABLE, CPU->CACHECFG_b.LRU, CPU->CACHECFG_b.IENABLE, CPU->CACHECFG_b.DENABLE, CPU->CACHECFG_b.CONFIG);
            am_util_stdio_printf("%d,%d,%d,%d,%d,", CPU->DAXICFG_b.DAXIPASSTHROUGH, CPU->DAXICFG_b.AGINGSENABLE, ui32BuffNum[CPU->DAXICFG_b.BUFFERENABLE], 1UL << CPU->DAXICFG_b.AGINGCOUNTER, CPU->DAXICFG_b.FLUSHLEVEL + 2);
#endif
#ifdef CSV_PRINT
            am_util_stdio_printf("cjpeg-rose7-preset,");
#endif
            argv[0] = "cjpeg-rose7-preset";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            cjpeg_rose7_preset_main(argc, argv);
#ifndef ONLY_RUN_JPEG
#ifdef CSV_PRINT
            am_util_stdio_printf("core,");
#endif
            argv[0] = "core";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            core_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("linear_alg_mid_100x100_sp,");
#endif
            argv[0] = "linear_alg_mid_100x100_sp";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            linear_alg_mid_100x100_sp_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("loops_all_mid_10k_sp,");
#endif
            argv[0] = "loops_all_mid_10k_sp";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            loops_all_mid_10k_sp_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("nnet_test,");
#endif
            argv[0] = "nnet_test";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            nnet_test_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("parser_125k,");
#endif
            argv[0] = "parser_125k";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            parser_125k_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("radix2_big_64k,");
#endif
            argv[0] = "radix2_big_64k";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            radix2_big_64k_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("sha_test,");
#endif
            argv[0] = "sha_test";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            sha_test_main(argc, argv);
#ifdef CSV_PRINT
            am_util_stdio_printf("zip_test,");
#endif
            argv[0] = "zip_test";
            argv[1] = "-c1";
            argv[2] = "-w1";
            argv[3] = "-v0";
            zip_test_main(argc, argv);
#endif
#ifdef CSV_PRINT
            am_util_stdio_printf("\n");
#endif
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    am_util_stdio_printf("Coremark_pro tasks done...\r\n");

    vTaskSuspend(NULL);
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
    am_util_stdio_printf("Running setup tasks...\r\n");

    xTaskCreate( vStartCjpegRose7Preset,
                 "cjpeg-rose7-preset",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    xTaskCreate( vStartCore,
                 "core",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );
    xTaskCreate( vStartLinearAlgMid100x100Sp,
                 "linear_alg_mid_100x100_sp",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    xTaskCreate( vStartLoopsAllMid10kSp,
                 "loops_all_mid_10k_sp",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    xTaskCreate( vStartNnetTest,
                 "nnet_test",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    xTaskCreate( vStartRadix2Big64k,
                 "radix2_big_64k",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    xTaskCreate( vStartShaTest,
                 "sha_test",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    xTaskCreate( vStartZipTest,
                 "zip_test",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );

    am_util_stdio_printf("Setup done...\r\n");

    //
    // The setup operations are complete, so suspend the setup task now.
    //
    vTaskDelete(NULL);
}

extern void prvHeapInit_external( void* address, size_t length );
#ifdef SSRAM_ONLY
    #define SSRAM_HEAP_SIZE 0x80000
    AM_SHARED_RW uint8_t ui8SsramHeap[SSRAM_HEAP_SIZE];
#endif
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
#ifdef SSRAM_ONLY
    prvHeapInit_external((void*)ui8SsramHeap, SSRAM_HEAP_SIZE);
#else
    prvHeapInit_external((void*)MSPI_XIP_BASE_ADDRESS, MSPI_PSRAM_SIZE);
#endif
    /* Start the task to run POSIX demo */
    //xTaskCreate( vStartPOSIXDemo,
    //             "posix",
    //             configMINIMAL_STACK_SIZE,
    //             NULL,
    //             3,
    //             NULL );

    //
    // Create essential tasks.
    //
#if RUN_IN_SINGLE_TASK == 1
    xTaskCreate( vStartCoremark,
                 "coremark_pro",
                 WORKLOAD_BOOTUP_TASK_STACK_SIZE,
                 NULL,
                 WORKLOAD_BOOTUP_TASK_PRIORITY,
                 NULL );
#else
    xTaskCreate(setup_task, "Setup", 512, 0, 3, &xSetupTask);
#endif

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

