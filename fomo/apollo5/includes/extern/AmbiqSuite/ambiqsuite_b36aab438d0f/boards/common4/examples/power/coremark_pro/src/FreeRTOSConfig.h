//*****************************************************************************
//
//! @file FreeRTOSConfig.h
//!
//! @brief Configuration options for FreeRTOS
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

#define configCPU_CLOCK_HZ                      96000000UL
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    16
#define configMINIMAL_STACK_SIZE                (256)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  0
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0

#define configUSE_POSIX_ERRNO                   1
#define configUSE_APPLICATION_TASK_TAG          1

#define configTOTAL_HEAP_SIZE                   (288 * 1024)

/* Allow RTOS objects to be created using RAM provided by the application writer. */
#define configSUPPORT_STATIC_ALLOCATION         1

/*  Create RTOS objects using dynamically allocated RAM */
#define configSUPPORT_DYNAMIC_ALLOCATION		1

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                20
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE*2)

/* Interrupt nesting behaviour configuration. */
#define configKERNEL_INTERRUPT_PRIORITY         (0x7 << 5)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (0x4 << 5)
#define NVIC_configKERNEL_INTERRUPT_PRIORITY        (0x7)
#define NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY   (0x4)

/* Define to trap errors during development. */
#define configASSERT(x)     if (( x ) == 0) while(1);

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  0
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          0
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_pxTaskGetStackStart             1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  0
#define INCLUDE_pcTaskGetTaskName               0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xSemaphoreGetMutexHolder        1

#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION 1 // Enable non-SysTick based Tick
#define configUSE_TICKLESS_IDLE                   2 // Ambiq specific implementation for Tickless

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
extern uint32_t am_freertos_sleep(uint32_t);
extern void am_freertos_wakeup(uint32_t);

#define configPRE_SLEEP_PROCESSING( time ) \
    do { \
        (time) = am_freertos_sleep(time); \
    } while (0);

#define configPOST_SLEEP_PROCESSING(time)    am_freertos_wakeup(time)
#endif
/*-----------------------------------------------------------*/
#ifndef AM_PART_APOLLO
#define AM_FREERTOS_USE_STIMER_FOR_TICK
#endif

#ifdef AM_FREERTOS_USE_STIMER_FOR_TICK
#ifdef APOLLO4_FPGA
#define configSTIMER_CLOCK_HZ                     1500000
#define configSTIMER_CLOCK                        AM_HAL_STIMER_HFRC_6MHZ
#else
#define configSTIMER_CLOCK_HZ                     32768
#define configSTIMER_CLOCK                        AM_HAL_STIMER_XTAL_32KHZ
#endif
#else // Use TIMER
#define configTIMER_NUM                          3
#define configTIMER_CLOCK_HZ                     32768
#define configTIMER_CLOCK                        AM_HAL_TIMER_CLOCK_XT
#endif

#ifdef __cplusplus
}
#endif

#endif // FREERTOS_CONFIG_H
