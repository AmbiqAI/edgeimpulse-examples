//*****************************************************************************
//
//! @file ble_freertos_amdtpc.c
//!
//! @brief ARM Cordio BLE - AMDTP Client (Master) Example.
//!
//! Purpose: This example is the client (master) for the BLE Ambiq Micro
//! Data Transfer Protocol. This example is meant to run on an Apollo4 EVB
//! along with another Apollo4 EVB running ble_freertos_amdtps as the server.
//! This example provides a UART command line interface with a simple menu
//! that allows the user to scan, connect and initiate data transfers from
//! either M->S or S->M direction.
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

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "ble_freertos_amdtpc.h"
#include "rtos.h"



//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
enable_print_interface(void)
{
#if 0
    //
    // Initialize the printf interface for UART output.
    //
    am_bsp_uart_printf_enable();
#else
    //
    // Initialize a debug printing interface.
    //
    am_bsp_itm_printf_enable();
#endif
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
    am_util_debug_printf("FreeRTOS AMDTPc 2.5ms Example\n");

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

