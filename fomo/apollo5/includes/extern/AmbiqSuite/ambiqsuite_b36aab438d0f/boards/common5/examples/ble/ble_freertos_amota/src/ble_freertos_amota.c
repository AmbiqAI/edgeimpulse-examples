//*****************************************************************************
//
//! @file ble_freertos_amota.c
//!
//! @brief ARM Cordio BLE - Ambiq Micro Over the Air (AMOTA) Example.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup ble_freertos_amota BLE FreeRTOS Ambiq Micro Over the Air (AMOTA) Example
//! @ingroup ble_examples
//! @{
//!
//! Purpose: This example implements Ambiq Micro Over-the-Air (OTA) slave.  This
//! example is designed to allow loading of a binary software update from either
//! and iOS or Android phone running Ambiq's application.  This example works
//! with the Apollo4 Secure Bootloader (SBL) to place the image in flash and then
//! reset the Apollo4 to allow SBL to validate and install the image.
//!
//! AM_DEBUG_PRINTF
//! WSF_TRACE_ENABLED=1
//!
//! If enabled, debug messages will be sent over ITM at 1M Baud.
//!
//! Additional Information:
//! The directory \tools\apollo4_amota\scripts contains a Makefile which will
//! build the OTA binary.
//!
//! The directory \docs\app_notes\amota explains how to use the Ambiq iOS and
//! Android applications.
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
#include "ble_freertos_amota.h"
#include "rtos.h"
#include "amota_profile_config.h"

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
    am_util_debug_printf("FreeRTOS Fit AMOTA Example\n");

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
