//*****************************************************************************
//
//! @file mindtree_txpower_ctrl.c
//!
//! @brief Tx power control example based on Mindtree stack
//!
//! Detailed description.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "stdio.h"
#include "stdarg.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "task.h"

extern int appl_init(void);

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint64_t ui64Status;

    //
    // Read and clear the GPIO interrupt status.
    //
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);

    //
    // Call the individual pin interrupt handlers for any pin that triggered an
    // interrupt.
    //
    am_hal_gpio_int_service(ui64Status);
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    //
    // Default setup.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);
    am_bsp_low_power_init();

    //
    // Enable ITM for debug.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
    am_bsp_pin_enable(ITM_SWO);
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();

    appl_init();

    vTaskStartScheduler();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
    }
}
