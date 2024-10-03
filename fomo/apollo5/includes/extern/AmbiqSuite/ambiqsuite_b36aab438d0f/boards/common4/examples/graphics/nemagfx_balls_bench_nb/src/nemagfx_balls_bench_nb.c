//*****************************************************************************
//
//! @file nemagfx_balls_bench_nb.c
//!
//! @brief NemaGFX Balls Bench NB Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_balls_bench_nb NemaGFX Balls Bench NB Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example created some random balls with alpha blending and number of
//! pictures on the screen,
//! use
//! #define MAX_CIRCLE_NUM               (15)
//! #define MAX_PICTURE_NUM              (15)

//! to choose how many balls rendering on the screen
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
//! This example creates random balls with alpha blending on the screen, and
//! supports both bare metal and RTOS modes. The bare metal mode in this examples
//! supports a state machine algorithm. The advantage of using this state machine
//! is that users can easily add their own applications to the program without
//! affecting the FPS.support both bare metal and RTOS supported mode, bare metal
//! mode use state machine this is state machine algorithm can not get higher FPS,
//! The advantage of using a state machine is that users can easily add their own
//! applications to the program without affecting the FPS.
//!
//! In the baremetal mode, the CPU are running in the following states
//! STATE_FIRST_ROUND,
//! STATE_SECOND_ROUND,
//! STATE_THIRD_ROUND,
//! STATE_DURING_WAIT_TE,
//! STATE_DURING_WAIT_TE_COMPLETE,
//! STATE_DURING_WAIT_VSYNC,
//! STATE_DURING_WAIT_VSYNC_COMPLETE,
//! STATE_COMPLETE

//! In the RTOS mode, use two semaphore
//! g_sSemRenderComplete
//! g_sSemFlushComplete
//! to mutex two individual task
//! task_rending
//! task_flush

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
#include "nemagfx_balls_bench_nb.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "nema_hal.h"
#include "main.h"

#ifndef BAREMETAL
    #include "rtos.h"
#endif

//#define AM_DEBUG_PRINTF

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
    am_bsp_debug_printf_enable();
}

//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
disable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_disable();
    am_hal_itm_disable();
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
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

#if !defined(APOLLO4_FPGA)
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

    //
    // If you want to change display configurations in example, please change them here as below.
    //
    // g_eDispType = RM67162_DSI;
    // g_sDsiCfg.ui8NumLanes = 1;
    // g_sDsiCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDsiCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
    if ((g_sDispCfg[g_eDispType].eInterface == DISP_IF_DSI) || (g_sDispCfg[g_eDispType].bUseDPHYPLL == true))
    {
        //
        // VDD18 control callback function
        //
        am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
        //
        // Enable DSI power and configure DSI clock.
        //
        am_hal_dsi_init();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();
    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif
    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }

#ifdef BAREMETAL
    am_util_stdio_printf("nemafgx_balls_bench Example\n");
    am_balls_bench();
#else // BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS nemafgx_balls_bench Example\n");

    //
    // Run the application.
    //
    run_tasks();
#endif // BAREMETAL

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

