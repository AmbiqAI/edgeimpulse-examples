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
    am_bsp_low_power_init();
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Assign a fixed value to display type.
    //
    g_sDispCfg.eInterface = DISP_IF_DSI;
    //
    // VDD18 control callback function
    //
    am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
    am_hal_dsi_init();

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
    
    // Set GPIO102 high to enable VDD18 load switch
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDD18_SWITCH, g_AM_BSP_GPIO_VDD18_SWITCH);
    am_hal_gpio_output_set(AM_BSP_GPIO_VDD18_SWITCH);
    
    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4p DSI Test Cases\n\n");

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
    // Clear GPIO102 to disable VDD18 load switch
    am_hal_gpio_output_clear(AM_BSP_GPIO_VDD18_SWITCH);
}

