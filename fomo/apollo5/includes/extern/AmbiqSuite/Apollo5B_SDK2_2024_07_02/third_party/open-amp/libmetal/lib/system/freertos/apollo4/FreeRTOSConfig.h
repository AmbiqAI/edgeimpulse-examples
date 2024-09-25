//*****************************************************************************
//
//! @file FreeRTOSConfig.h
//!
//! @brief Configuration options for FreeRTOS
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
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#define configCPU_CLOCK_HZ                      ( 96000000UL )
#define configTICK_RATE_HZ                      ( 1000 )
#define configMAX_PRIORITIES                    ( 7 )
#define configMINIMAL_STACK_SIZE                ( 512 )

#define configTOTAL_HEAP_SIZE                   ( ( size_t ) 0x10000 )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0
#define configQUEUE_REGISTRY_SIZE               20
#define configUSE_QUEUE_SETS                    1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_TIME_SLICING                  0
#define configSUPPORT_STATIC_ALLOCATION         0
#define configINITIAL_TICK_COUNT                ( ( TickType_t ) 0 ) /* For test. */
#define configSTREAM_BUFFER_TRIGGER_LEVEL_TEST_MARGIN 1 /* As there are a lot of tasks running. */

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1



/* Run time stats gathering configuration options. */
//unsigned long ulGetRunTimeCounterValue( void ); /* Prototype of function that returns run time counter. */
//void vConfigureTimerForRunTimeStats( void );  /* Prototype of function that initialises the run time counter. */
#define configGENERATE_RUN_TIME_STATS           0
//#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
//#define portGET_RUN_TIME_COUNTER_VALUE() ulGetRunTimeCounterValue()

#define configUSE_TRACE_FACILITY                0

/* Co-routine related configuration options. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* Stats formatting functions:
Format the raw data provided by the uxTaskGetSystemState() function into human
readable ASCII form.  See the notes in the implementation of vTaskList() within
FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Software timer related configuration options. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                20
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/* Interrupt nesting behaviour configuration. */
#define configKERNEL_INTERRUPT_PRIORITY         (0x7 << 5)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (0x4 << 5)
#define NVIC_configKERNEL_INTERRUPT_PRIORITY        (0x7)
#define NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY   (0x4)

/* Define to trap errors during development. */
#define configASSERT(x)     if (( x ) == 0) while(1);

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function.  In most cases the linker will remove unused
functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xSemaphoreGetMutexHolder        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1

/* It is a good idea to define configASSERT() while developing.  configASSERT()
uses the same semantics as the standard C assert() macro. */
//extern void vAssertCalled( unsigned long ulLine, const char * const pcFileName );
//#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __LINE__, __FILE__ )

/* Include the FreeRTOS+Trace FreeRTOS trace macro definitions. */
//#include "trcRecorder.h"

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
#define configSTIMER_CLOCK_HZ                     32768
#define configSTIMER_CLOCK                        AM_HAL_STIMER_XTAL_32KHZ
#else // Use CTimer
#define configTIMER_NUM                           3
#define configTIMER_CLOCK_HZ                      32768
#define configTIMER_CLOCK                         AM_HAL_TIMER_CLOCK_XT
#endif

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_CONFIG_H */
