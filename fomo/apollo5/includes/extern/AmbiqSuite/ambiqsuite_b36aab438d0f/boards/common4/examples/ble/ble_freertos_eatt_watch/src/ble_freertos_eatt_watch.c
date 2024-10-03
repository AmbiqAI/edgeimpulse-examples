//*****************************************************************************
//
//! @file ble_freertos_eatt_watch.c
//!
//! @brief ARM Cordio BLE - Concurrent Master/Slave Example.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup ble_freertos_eatt_watch BLE FreeRTOS Concurrent Master/Slave Example
//! @ingroup ble_examples
//! @{
//!
//! Purpose: This example similar to ble_freertos_watch but performs the EATT
//! connection on PSM and CID.
//! demonstrates an BLE application in the Central role.
//! That is the BLE application operates as a slave to phone master and as the
//! master of subordinate slave devices running freertos_eatt_fit example in this SDK.
//!
//! Additional Information:
//! 1. Printing takes place over the ITM at 1M Baud.
//! 2. When the example powers up,
//!    2.A. it enters advertising mode by default to wait for connection from
//!         smart phone with Time profile, Alert Notification profile and Phone
//!         Alert Status profile supported.
//!    2.B. when BTN1 on Apollo4 EVB is short-pressed, if advertising is on, it
//!         stops advertising first and then starts scanning when advertising is
//!         stopped; if scanning is on, it stops scanning and re-start advertising
//!         when scanning stops.
//!    2.C. During scanning, the device (if discovered) running freertos_fit
//!         example in this SDK will be connected and scanning will be stopped.
//!    2.D. Repeat 2.B. and 2.C. above to connect to a new slave device running
//!         freertos_fit example (max slaves is 3).
//! 3. when phone (iPhone is used) connects to this example, the services of Time
//!    profile, Alert Notification profile and Phone Alert Status profile will be
//!    subscribed by the example.
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
#include "ble_freertos_eatt_watch.h"
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
    am_util_debug_printf("FreeRTOS Watch Example\n");

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
