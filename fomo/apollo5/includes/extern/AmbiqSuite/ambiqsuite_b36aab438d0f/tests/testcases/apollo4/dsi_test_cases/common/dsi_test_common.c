//*****************************************************************************
//
//! @file dsi_test_common.c
//!
//! @brief DSI test cases common source code.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "dsi_test_common.h"

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
    am_hal_dsi_init();

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    CPU->DAXICFG_b.BUFFERENABLE = CPU_DAXICFG_BUFFERENABLE_ONE; // only enable a single buffer

    PWRCTRL->SSRAMPWREN = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL; // enable all
    while(PWRCTRL->SSRAMPWRST == 0);
    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 DSI Test Cases\n\n");
    
    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if(nemadc_init() != 0)
    {
        am_util_stdio_printf("DC init failed!\n");
    }
}

void
globalTearDown(void)
{
}

