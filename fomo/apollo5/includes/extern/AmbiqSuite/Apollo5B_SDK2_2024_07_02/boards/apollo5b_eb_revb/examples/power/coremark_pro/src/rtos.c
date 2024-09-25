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
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "coremark_pro.h"
#include "mpu_config.h"

//*****************************************************************************
//
// External prototypes
//
//*****************************************************************************
extern void linear_alg_mid_100x100_sp_main(int argc, char *argv[]);
extern void cjpeg_rose7_preset_main(int argc, char *argv[]);
extern void core_main(int argc, char *argv[]);
extern void loops_all_mid_10k_sp_main(int argc, char *argv[]);
extern void nnet_test_main(int argc, char *argv[]);
extern void parser_125k_main(int argc, char *argv[]);
extern void radix2_big_64k_main(int argc, char *argv[]);
extern void sha_test_main(int argc, char *argv[]);
extern void zip_test_main(int argc, char *argv[]);

#if defined(AM_BARE_METAL)
extern void al_start_time(void);
extern void al_stop_time(void);
#else
#define al_start_time()
#define al_stop_time()
#endif

void CoremarkTasks(void)
{
#ifdef ENABLE_PMU
    am_util_pmu_config_t pmu_config;
    am_util_pmu_profiling_t pmu_profiling;
    //
    // Select event types here
    //
    pmu_config.ui32Counters = VALID_PMU_COUNTERS;               // Enable all valid event counters

    pmu_config.ui32EventType[0] = ARM_PMU_L1I_CACHE_REFILL;
    pmu_config.ui32EventType[1] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[2] = ARM_PMU_L1I_CACHE;
    pmu_config.ui32EventType[3] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[4] = ARM_PMU_  L1D_CACHE_MISS_RD;
    pmu_config.ui32EventType[5] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[6] = ARM_PMU_L1D_CACHE_RD;
    pmu_config.ui32EventType[7] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    am_util_pmu_init(&pmu_config);
#endif

#define RUN_CJPEG   1
#define RUN_CORE    1
#define RUN_LIN_100 1
#define RUN_NNET    1
#define RUN_PARSER  1
#define RUN_RADIX2  1
#define RUN_SHA     1
#if defined ( HEAP_IN_PSRAM )
#define RUN_MID_10K 1
#define RUN_ZIP     1
#else
#define RUN_MID_10K 0
#define RUN_ZIP     0
#endif

    int argc = 4;
    char *argv[ 4 ];
    argv[ 0 ] = "-v0";
    argv[ 1 ] = "-i1";
    argv[ 2 ]  = "-c1";
    argv[ 3 ] = "-w1";

#if RUN_CJPEG
    #if ENABLE_CPU_HP_250
    argv[1] = "-i65";
    #else
    argv[1] = "-i25";
    #endif

    // The performance monitoring unit will be enabled right before each
    // workload is run in mith_main_loop(). Several of the workloads require
    // a fair amount of setup, so we don't want to record the cache info until
    // the actual workload runs.

    am_util_stdio_printf("Running cjpeg_rose7_preset tasks...\n");
    al_start_time();
    cjpeg_rose7_preset_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("cjpeg_rose7_preset tasks done...\n\n");
#endif

#if RUN_CORE
    argv[ 1 ] = "-i1";

    am_util_stdio_printf("Running core tasks...\n");
    al_start_time();
    core_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("core tasks done...\n\n");
#endif

#if RUN_LIN_100
  #if defined ( HEAP_IN_PSRAM )
    #if ENABLE_CPU_HP_250
    argv[1] = "-i35";
    #else
    argv[1] = "-i15";
    #endif
  #else
    #if ENABLE_CPU_HP_250
    argv[1] = "-i35";
    #else
    argv[1] = "-i15";
    #endif
  #endif

    am_util_stdio_printf("Running linear_alg_mid_100x100_sp tasks...\n");
    al_start_time();
    linear_alg_mid_100x100_sp_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("linear_alg_mid_100x100_sp tasks done...\n\n");
#endif

#if RUN_MID_10K
    argv[ 1 ] = "-i1";

    am_util_stdio_printf("Running loops_all_mid_10k_sp tasks...\n");
    al_start_time();
    loops_all_mid_10k_sp_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("loops_all_mid_10k_sp tasks done...\n");
#endif

#if RUN_NNET
  #if defined ( HEAP_IN_PSRAM )
    #if ENABLE_CPU_HP_250
    argv[1] = "-i2";
    #else
    argv[1] = "-i1";
    #endif
  #else
    #if ENABLE_CPU_HP_250
    argv[1] = "-i2";
    #else
    argv[1] = "-i1";
    #endif
  #endif

    am_util_stdio_printf("Running nnet_test tasks...\n");
    al_start_time();
    nnet_test_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("nnet_test tasks done...\n");
#endif

#if RUN_PARSER
    #if ENABLE_CPU_HP_250
    argv[1] = "-i2";
    #else
    argv[1] = "-i2";
    #endif

    am_util_stdio_printf("Running parser_125k tasks...\n");
    al_start_time();
    parser_125k_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("parser_125k tasks done...\n");
#endif

#if RUN_RADIX2
    #if ENABLE_CPU_HP_250
    argv[1] = "-i12";
    #else
    argv[1] = "-i12";
    #endif

    am_util_stdio_printf("Running radix2_big_64k tasks...\n");
    al_start_time();
    radix2_big_64k_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("radix2_big_64k tasks done...\n");
#endif

#if RUN_SHA
  #if defined ( HEAP_IN_PSRAM )
    #if ENABLE_CPU_HP_250
    argv[1] = "-i65";
    #else
    argv[1] = "-i25";
    #endif
  #else
    #if ENABLE_CPU_HP_250
    argv[1] = "-i50";
    #else
    argv[1] = "-i25";
    #endif
  #endif

    am_util_stdio_printf("Running sha_test tasks...\n");
    al_start_time();
    sha_test_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("sha_test tasks done...\n");
#endif

#if RUN_ZIP
    #if ENABLE_CPU_HP_250
    argv[1] = "-i12";
    #else
    argv[1] = "-i10";
    #endif

    am_util_stdio_printf("Running zip_test tasks...\n");
    al_start_time();
    zip_test_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("zip_test tasks done...\n");
#endif

    //
    // Disable performance monitoring unit
    //
    pmu_disable();
    am_util_stdio_printf("Coremark_pro tasks done...\n");
}

void vStartCoremark( void *pvParameters )
{
    am_util_stdio_printf("Run Coremark_pro tasks with MPU disabled...\n");
    CoremarkTasks();
#ifdef MPU_CACHE_TEST // Set this MACRO in config_template.ini
    am_util_stdio_printf("\n\n\nRun Coremark_pro tasks with MPU enabled (Read-Allocation and Write-Allocation for SSRAM and PSRAM0)...\n");
    mpu_config (sMPUCfgTest1, sizeof(sMPUCfgTest1) / sizeof(am_hal_mpu_region_config_t));
    am_hal_mpu_enable(true, true);
    CoremarkTasks();
    am_hal_mpu_disable();
    am_util_stdio_printf("\n\n\nRun Coremark_pro tasks with MPU enabled (Non-cacheable for SSRAM and PSRAM0)...\n");
    mpu_config (sMPUCfgTest2, sizeof(sMPUCfgTest2) / sizeof(am_hal_mpu_region_config_t));
    am_hal_mpu_enable(true, true);
    CoremarkTasks();
    am_hal_mpu_disable();
#endif
    am_util_stdio_printf("All tests done...\n");
#if defined(AM_FREERTOS)
    vTaskSuspend(NULL);
#endif
}

#if defined(AM_FREERTOS)
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
    am_util_stdio_printf("Running setup tasks...\n");

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

    am_util_stdio_printf("Setup done...\n");

    //
    // The setup operations are complete, so suspend the setup task now.
    //
    vTaskDelete(NULL);
}

#define PSRAM_RTOS_HEAP_ADDRESS (MSPI_XIP_BASE_ADDRESS + MSPI_PSRAM_SIZE)

#if defined(HEAP_IN_PSRAM)
extern void prvHeapInit_external( void* address, size_t length );
#elif defined(HEAP_IN_SSRAM_AND_PSRAM)
extern void vPortDefineHeapRegions_external( const HeapRegion_t * const pxHeapRegions );
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
    // Heap init, this heap is used by coremark_pro workloads only.
    //
#if defined(HEAP_IN_PSRAM)
    prvHeapInit_external((void*)PSRAM_RTOS_HEAP_ADDRESS, MSPI_PSRAM_SIZE);
#elif defined(HEAP_IN_SSRAM_AND_PSRAM)
    HeapRegion_t xHeapRegions[] =
    {
        //The start part of ssram0 is used by HAL to hold several global data.
        { ( uint8_t * ) SSRAM_BASEADDR + 0x400, SSRAM_MAX_SIZE - 0x400 },
        { ( uint8_t * ) PSRAM_RTOS_HEAP_ADDRESS, MSPI_PSRAM_SIZE },
        { NULL, 0 }
    };

    vPortDefineHeapRegions_external(xHeapRegions);
#endif

    //
    // Set some interrupt priorities before we create tasks or start the scheduler.
    //
    // Note: Timer priority is handled by the FreeRTOS kernel, so we won't
    // touch it here.
    //

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
#endif
