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
#include <stdarg.h>

#include "../../../../../../third_party/open-amp/libmetal/lib/thread.h"
#include "../../../../../../third_party/open-amp/libmetal/lib/log.h"
#include "../../../../../../third_party/open-amp/libmetal/lib/sys.h"

#include "codec_main.h"
#include "load_section_on_demand.h"
// #include "dynamic_load_codec_task.h"

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
metal_thread_t xSetupTask;

static void app_log_handler(enum metal_log_level level,
                            const char *format, ...)
{
    /*char msg[1024];
    va_list args;
    static const char *level_strs[] = {
        "metal: emergency: ",
        "metal: alert:     ",
        "metal: critical:  ",
        "metal: error:     ",
        "metal: warning:   ",
        "metal: notice:    ",
        "metal: info:      ",
        "metal: debug:     ",
    };

    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);

    if (level <= METAL_LOG_EMERGENCY || level > METAL_LOG_DEBUG)
        level = METAL_LOG_EMERGENCY;

    am_util_debug_printf("%s", msg);*/
}

//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void
am_timer_isr(void)
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
    am_util_debug_printf("Running setup tasks...\r\n");

    //
    // Run init functions.
    //
    codec_task_setup();

    metal_thread_suspend(&xSetupTask);

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
    metal_thread_init(&xSetupTask, setup_task, "Setup", 1024, 0, 3);

    struct metal_init_params metal_param =
    {
        .log_handler = app_log_handler,
        .log_level = METAL_LOG_DEBUG,
    };
    metal_init(&metal_param);

    //
    // Start the scheduler.
    //
    metal_scheduler_start();
}

