//*****************************************************************************
//
//! @file power_down_test.c
//!
//! @brief Example of the AMBT53 Power Down Testing.
//!
//! Purpose: This example shows how to detect and process the power down
//! indication from AMBT53 and make the AMBT53 to boot up from power dwon mode.
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! this example can work on:
//! Apollo4p_evb + AMBT53 FPGA
//! Target hardware uses 1.8V power supply voltage.
//!
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
#include "power_down_test.h"
#include "rtos.h"

// Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
AM_SHARED_RW uint32_t axiScratchBuf[20];

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
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#if defined(AM_PART_APOLLO4B)
    //
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
#else
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_ENABLE, NULL);
#endif

    //
    // Configure the board for low power.
    //
    // TODO will be enable again after access the Scpm register successfully through I2C
    //am_bsp_low_power_init();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("AMBT53 Power Down Test\n");

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
