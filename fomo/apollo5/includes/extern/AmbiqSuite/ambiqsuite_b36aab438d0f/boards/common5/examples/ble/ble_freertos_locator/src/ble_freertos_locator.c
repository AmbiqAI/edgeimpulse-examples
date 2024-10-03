//*****************************************************************************
//
//! @file ble_freertos_locator.c
//!
//! @brief ARM Cordio BLE - Locator Application Example.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup ble_freertos_locator BLE FreeRTOS Locator Application Example
//! @ingroup ble_examples
//! @{
//!
//! Purpose: This example implements a BLE locater application within the FreeRTOS
//! framework. The locator will work as master to connect the asset tag device and
//! receive IQ data by sending CTE request.
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//! Press button0 of Apollo4 EVB to scan/connect the asset tag device.
//! Press button1 of Apollo4 EVB to start CTE request after connection.
//!
//! AM_DEBUG_PRINTF
//! WSF_TRACE_ENABLED=1
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
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
#include "ble_freertos_locator.h"
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

    //
    // Print the banner.
    //
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
    am_util_debug_printf("FreeRTOS Locator Example\n");

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

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
