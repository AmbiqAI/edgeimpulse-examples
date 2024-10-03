//*****************************************************************************
//
//! @file ble_freertos_fcc_test.c
//!
//! @brief ARM Cordio BLE - FCC test example
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup ble_freertos_fcc_test BLE FreeRTOS FCC test Example
//! @ingroup ble_examples
//! @{
//!
//! Purpose: This example is used to put Bluetooth radio in Apollo4 into various
//! test mode on different channels on pressing BTN1 on the Apollo4 EVB.
//!
//! Printing takes place over the ITM at 1M Baud.
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

#include "am_util.h"
#include "am_bsp.h"

//*****************************************************************************
//
// Forward declarations.
//

//*****************************************************************************
//
// Timer for buttons.
//
//*****************************************************************************

extern void run_tasks(void);

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************

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
// Main function.
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
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    am_util_debug_printf("Apollo4 BLE FCC Test Example\n");

    run_tasks();

    am_util_debug_printf("Usage as below:\n");
    am_util_debug_printf("TX power is set to +3.0 dBm (max) \n");

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
