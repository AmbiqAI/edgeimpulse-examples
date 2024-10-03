//*****************************************************************************
//
//! @file freertos_amdtp.c
//!
//! @brief AMDTP example.
//!
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
#include "SEGGER_SYSVIEW.h"
#include "freertos_stability_test.h"
#include "rtos.h"

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
void
am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(0, true);

    am_hal_iom_int_clear(0, ui32Status);

    am_hal_iom_int_service(0, ui32Status);
}

void
am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(1, true);

    am_hal_iom_int_clear(1, ui32Status);

    am_hal_iom_int_service(1, ui32Status);
}

void
am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(2, true);

    am_hal_iom_int_clear(2, ui32Status);

    am_hal_iom_int_service(2, ui32Status);
}

void
am_iomaster3_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(3, true);

    am_hal_iom_int_clear(3, ui32Status);

    am_hal_iom_int_service(3, ui32Status);
}

void
am_iomaster4_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(4, true);

    am_hal_iom_int_clear(4, ui32Status);

    am_hal_iom_int_service(4, ui32Status);
}

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
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();
    am_util_debug_printf_init(am_hal_itm_print);
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
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

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

    // Turn off unused Flash & SRAM

#ifdef AM_PART_APOLLO
    //
    // SRAM bank power setting.
    // Need to match up with actual SRAM usage for the program
    // Current usage is between 32K and 40K - so disabling upper 3 banks
    //
    am_hal_mcuctrl_sram_power_set(AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7,
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7);

#if 0 // Not turning off the Flash as it may be needed to download the image
    //
    // Flash bank power set.
    //
    am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);
#endif
#endif // AM_PART_APOLLO
#ifdef AM_PART_APOLLO2
#if 0 // Not turning off the Flash as it may be needed to download the image
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH512K);
#endif
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM256K);
#endif

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#else
    am_hal_itm_enable();
#endif

    // Configure the SEGGER SystemView Interface.
    //
    SEGGER_SYSVIEW_Conf();

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS AMDTP Example\n");

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

