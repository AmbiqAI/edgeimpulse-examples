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
#include "portmacro.h"
#include "portable.h"

#include "rtos.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
TaskHandle_t xSleepTask;

//*****************************************************************************
//
// RTOS Tick events
//
//*****************************************************************************
void
rtos_tick(void)
{
    //
    // If this was a timer a0 interrupt, perform the necessary functions
    // for the tick ISR.
    //
    (void) portSET_INTERRUPT_MASK_FROM_ISR();
    {
        //
        // Increment RTOS tick
        //
        if ( xTaskIncrementTick() != pdFALSE )
        {
            //
            // A context switch is required.  Context switching is
            // performed in the PendSV interrupt. Pend the PendSV
            // interrupt.
            //
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(0);
}

//*****************************************************************************
//
// Configures and enables the timer interrupt used by FreeRTOS
//
//*****************************************************************************
void
vPortSetupTimerInterrupt(void)
{
    am_hal_ctimer_config_t sTimer0Config =
    {
        .ui32Link = 0,

        .ui32TimerAConfig = (AM_HAL_CTIMER_HFRC_12KHZ |
                             AM_HAL_CTIMER_FN_REPEAT |
                             AM_HAL_CTIMER_INT_ENABLE),

        .ui32TimerBConfig = 0
    };

    //
    // Configure the timer frequency and mode.
    //
    am_hal_ctimer_config(0, &sTimer0Config);

    //
    // Set the timeout interval
    //
    am_hal_ctimer_compare_set(0, AM_HAL_CTIMER_TIMERA, 0, 12);

    //
    // Enable the interrupt for timer A0
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    //
    // Enable the timer interrupt in the NVIC, making sure to use the
    // appropriate priority level.
    //
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configKERNEL_INTERRUPT_PRIORITY);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

    //
    // Enable the timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
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
    while (1);
}

//*****************************************************************************
//
// Sets the "idle" action.
//
//*****************************************************************************
void
sleep_task(void *pvParameters)
{
    while(1)
    {
        //
        // Enter deep-sleep.
        //
        //am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}

