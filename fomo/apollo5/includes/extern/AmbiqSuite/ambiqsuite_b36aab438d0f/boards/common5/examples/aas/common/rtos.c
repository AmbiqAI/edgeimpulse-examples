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

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "libmetal/lib/thread.h"

#include "am_mcu_apollo.h"
#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "aust_mcps.h"

#include "setup.h"

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
metal_thread_t xSetupTask;

//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void am_timer_isr(void)
{
    uint32_t ui32Status;

    am_hal_timer_interrupt_status_get(false, &ui32Status);
    am_hal_timer_interrupt_clear(ui32Status);
}

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
    aust_mcps_count_begin(MCPS_SYS);

    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    aust_mcps_count_end(MCPS_SYS);

    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals
// etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    return;
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
void setup_task(void *pvParameters)
{
    //
    // Print a debug message.
    //
    am_util_debug_printf("Running setup tasks...\r\n");

    //
    // Run setup functions.
    //
    aust_task_setup();

    //
    // The setup operations are complete, so suspend the setup task now.
    //
    metal_thread_suspend(&xSetupTask);

    while (1)
        ;
}

//*****************************************************************************
//
// Initializes all tasks
//
//*****************************************************************************
void run_tasks(void)
{
    //
    // Set some interrupt priorities before we create tasks or start the
    // scheduler.
    //
    // Note: Timer priority is handled by the FreeRTOS kernel, so we won't
    // touch it here.
    //

    //
    // Create essential tasks.
    //
    metal_thread_init(&xSetupTask, setup_task, "Setup", 1024, 0, 3);

    //
    // Start the scheduler.
    //
    metal_scheduler_start();
}
