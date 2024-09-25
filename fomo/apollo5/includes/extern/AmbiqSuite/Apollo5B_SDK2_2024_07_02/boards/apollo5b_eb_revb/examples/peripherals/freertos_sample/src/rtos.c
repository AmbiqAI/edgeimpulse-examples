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

#include <stdint.h>
#include <stdbool.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"

#include "freertos_sample.h"

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
extern volatile bool g_bHelloTaskComplete;
extern volatile bool g_bTimerTaskComplete;

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
    if (g_bHelloTaskComplete && g_bTimerTaskComplete)
    {
        // vTaskSuspend(NULL);
        // vTaskSuspendAll();
    }
    else
    {
        g_ui32IdleCycleCount++; // this var is reported & reset to 0 in HelloTask
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
    am_util_stdio_printf("\r\nRunning setup tasks...\r\n");

    //
    // Run setup functions.
    //
    HelloTaskSetup();
    TimerTaskSetup();

    //
    // Create the functional tasks
    //
    xTaskCreate(HelloTask, "HelloTask", 512, 0, TASK_PRIORITY_LOW, &hello_task_handle);
    xTaskCreate(TimerTask, "TimerTask", 512, 0, TASK_PRIORITY_LOW, &timer_task_handle);

    //
    // The setup operations are complete, so suspend the setup task now.
    //
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

