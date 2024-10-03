//*****************************************************************************
//
//! @file nemagfx_watchface.c
//!
//! @brief NemaGFX example.
//! This example demonstrates the creation of a simple watch face comprising a 
//! background image, three clock hands, and an SVG format weather icon. 
//! The following features are showcased:
//! 1. Rotating clock hands using raw NemaGFX APIs.
//! 2. GPU power cycle process: 
//!    In render_task.c, the GPU powers off after command list execution is complete
//!    and restarts when it is scheduled to draw the next frame. 
//! 3. DC and DSI power cycle process: 
//!    In the display task, DC&DSI are powered up only when scheduled to transfer 
//!    a frame to the display panel and shut down immediately after 
//!    the transfer is complete."
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_watchface.h"
#include "rtos.h"

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    am_util_delay_ms(2000);
    //
    // External power on
    //
    am_bsp_external_pwr_on();

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print debug information.
    //
    am_util_debug_printf("FreeRTOS nemagfx_watchface Example\n");

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

