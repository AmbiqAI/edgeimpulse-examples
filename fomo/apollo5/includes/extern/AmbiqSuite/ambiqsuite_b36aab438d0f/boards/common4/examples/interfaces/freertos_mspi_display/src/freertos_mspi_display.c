//*****************************************************************************
//
//! @file freertos_mspi_display.c
//!
//! @brief Example demonstrating MSPI display rendering
//!
//! This example demonstrates FB rendering to an MSPI connected display
//! Two predefined images are used. A configurable amount of frame sync's
//! (TE_DELAY) controls the frequency of flipping between two buffers
//!
//!
//! @verbatim
//! Pin connections:
//!
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
#include "freertos_mspi_display.h"
#include "rtos.h"

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
    am_bsp_itm_printf_enable();
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{

#if 1
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

#ifndef NOFPU
    //
    // Enable the floating point module, and configure the core for lazy
    // stacking.
    //
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS MSPI-MSPI Display Example\n");

    //
    // Debug GPIO
    //
    am_hal_gpio_pinconfig(DBG1_GPIO, am_hal_gpio_pincfg_output);
    DEBUG_GPIO_LOW(DBG1_GPIO);
    am_hal_gpio_pinconfig(DBG2_GPIO, am_hal_gpio_pincfg_output);
    DEBUG_GPIO_LOW(DBG2_GPIO);

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

