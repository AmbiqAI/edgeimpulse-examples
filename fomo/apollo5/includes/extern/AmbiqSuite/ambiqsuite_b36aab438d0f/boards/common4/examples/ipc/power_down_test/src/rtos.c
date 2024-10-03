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
#include "power_down_test.h"

//*****************************************************************************
//
    // Macro definitions.
    //
//*****************************************************************************

#define EXT_WAKEUP_TIMER_ID            (0)
#define EXT_WAKEUP_TIMER_PERIOD_MS     (5000)

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;
static TimerHandle_t ext_wakeup_tmr_handle;
static uint32_t ext_wakeup_tmr_period = EXT_WAKEUP_TIMER_PERIOD_MS;
static uint32_t ext_wakeup_tmr_id = EXT_WAKEUP_TIMER_ID;
static uint32_t force_active_test_cnt;

//force the Ambt53 not enter power down per 2 times external wake up test
#define FORCE_ACTIVE_TEST_PERIOD      (2)
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

    // we don't have this function in new hal
    //  am_hal_ctimer_int_service(ui32Status);
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

static void vTimerCallback(TimerHandle_t pxTimer)
{
    int32_t tmr_idx;

    // Optionally do something if the pxTimer parameter is NULL.
    configASSERT(pxTimer);

    // Which timer expired?
    tmr_idx = (int32_t)pvTimerGetTimerID(pxTimer);

    if (tmr_idx == (int32_t)ext_wakeup_tmr_id)
    {
        if (bIsPowerDown)
        {
            // The timer was not created.
            am_util_stdio_printf("External wake up\r\n");
            am_devices_ambt53_wakeup_from_pwrdown();

            force_active_test_cnt++;
            // Have a force active test PER FORCE_ACTIVE_TEST_PERIOD
            if ( force_active_test_cnt >= FORCE_ACTIVE_TEST_PERIOD )
            {
                force_active_test_cnt = 0;
                // Simulate the mcu force the AMBT53 not enter power down mode
                am_devices_force_ambt53_active_req(IPC_WRITE);
                am_util_delay_ms(10);
                am_devices_force_ambt53_active_release(IPC_WRITE);
            }
        }

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
    am_util_debug_printf("Running setup tasks...\r\n");

    //
    // Run setup functions.
    //
    int status = am_devices_ambt53_boot();
    am_util_debug_printf("boot ambt53, status:%d\r\n", status);

    am_devices_ambt53_pwrdown_setup(true);
        // Create one timer to wakeup the ambt53 core externally
    if ((ext_wakeup_tmr_handle = xTimerCreate("Timer",
                            pdMS_TO_TICKS(ext_wakeup_tmr_period),
                            pdTRUE,
                            (void *)ext_wakeup_tmr_id,
                            vTimerCallback)) == NULL)
    {
        // The timer was not created.
        am_util_stdio_printf("ext_wakeup_tmr_handler create failed\r\n");
        return;
    }

    if (xTimerStart(ext_wakeup_tmr_handle, 0) != pdPASS)
    {
        // The timer could not be set into the Active state.
        am_util_stdio_printf("ext_wakeup_tmr_handle start failed\r\n");
    }

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
    xTaskCreate(setup_task, "Setup", 512, 0, 3, &xSetupTask);

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();
}

