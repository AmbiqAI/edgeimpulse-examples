//*****************************************************************************
//
//! @file freertos_3d0f.c
//!
//! @brief Example using freertos to talk to multiple sensors.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "freertos_3dof.h"

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;

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
    iom1_setup();
    LIS3MDLTaskSetup();

    //
    // Run the functional tasks
    //
    xTaskCreate(LIS3MDLTask, "LIS3MDLTask", 1024, 0, 3, &lis3mdl_task_handle);

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
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_IOMASTER1, configMAX_SYSCALL_INTERRUPT_PRIORITY);

    //
    // Enable GPIO interrupts.
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);

    //
    // Create essential tasks.
    //
    xTaskCreate(sleep_task, "Sleep", 64, 0, tskIDLE_PRIORITY, &xSleepTask);
    xTaskCreate(setup_task, "Setup", 256, 0, 3, &xSetupTask);

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();
}

//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
enable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();
    am_util_debug_printf_init(am_hal_itm_print);
    am_util_stdio_terminal_clear();
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clock frequency
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Initialize the board.
    //
    am_hal_pwrctrl_bucks_enable();
    am_hal_vcomp_disable();
    // am_hal_mcuctrl_bandgap_disable();

    //
    // Enable printing to the console.
    //
    enable_print_interface();

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS 3DOF Example\n");

    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

