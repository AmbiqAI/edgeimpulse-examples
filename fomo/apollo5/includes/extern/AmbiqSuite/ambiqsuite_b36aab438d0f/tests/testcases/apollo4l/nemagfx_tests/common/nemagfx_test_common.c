//*****************************************************************************
//
//! @file nemagfx_test_common.c
//!
//! @brief NemaGFX test cases common source code.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************


//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    am_bsp_low_power_init();
    //
    // Enable printing through the ITM interface.
    //
    am_bsp_itm_printf_enable();
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_util_stdio_printf("Apollo4L NemaGFX Test Cases\n\n");
    //
    // Initialize NemaGFX
    //
    if(nema_init() != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");
    }
}

void
globalTearDown(void)
{
}

