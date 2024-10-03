//*****************************************************************************
//
//! @file ble_freertos_throughput.c
//!
//! @brief ARM Cordio BLE - Ambiq Micro Throughput Example.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup ble_freertos_throughput BLE FreeRTOS Throughput Example
//! @ingroup ble_examples
//! @{
//!
//! Purpose: This example implements is designed to test the data transmitting
//! and receiving thourghput. The example needs to be connected with the Ambiq BLE
//! Test APP (only Android now), then sends data to the APP by Notify method (Uplink)
//! or receives data from APP by Write Command method (Downlink) to check the rate
//! showed in the APP screen. The default MTU is 23 and PHY is 1M, you can request
//! the MTU exchange and PHY change through the settings of APP.
//!
//! AM_DEBUG_PRINTF
//! WSF_TRACE_ENABLED=1
//!
//! If enabled, debug messages will be sent over ITM at 1M Baud.
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
#include "ble_freertos_throughput.h"
#include "rtos.h"

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

#ifdef AM_DEBUG_PRINTF
    //
    // Initialize the printf interface for ITM output
    //

    am_bsp_debug_printf_enable();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_stdio_terminal_clear();
    am_util_debug_printf("FreeRTOS Throughput Example\n");

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
