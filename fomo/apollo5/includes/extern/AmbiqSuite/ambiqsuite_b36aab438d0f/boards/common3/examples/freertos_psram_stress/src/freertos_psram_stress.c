//*****************************************************************************
//
//! @file freertos_psram_stress.c
//!
//! @brief Example of the psram stress test running under FreeRTOS.
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
#include "freertos_psram_stress.h"
#include "rtos.h"

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
}


void
config_fastgpio(uint32_t pin)
{
    //
    // Configure the pins that are to be used for Fast GPIO.
    //
    am_hal_gpio_fastgpio_enable(pin);
    am_hal_gpio_fastgpio_clr(pin);
    //
    // Configure the pins that are to be used for Fast GPIO.
    //
    am_hal_gpio_fastgpio_disable(pin);
    am_hal_gpio_fastgpio_clr(pin);
    AM_HAL_GPIO_MASKCREATE(sGpioIntMask);
    am_hal_gpio_fast_pinconfig(AM_HAL_GPIO_MASKBIT(psGpioIntMask, pin),
                                         g_AM_HAL_GPIO_OUTPUT, 0);
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
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

#if 0
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

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
    am_util_debug_printf("FreeRTOS PSRAM Stress Example\n");

#ifdef CPU_SLEEP_GPIO
    // GPIO used to track the CPU sleeping
    //
    // Configure the pins that are to be used for Fast GPIO.
    //
    config_fastgpio(CPU_SLEEP_GPIO);
    config_fastgpio(TEST_GPIO);
    config_fastgpio(TEST_GPIO1);
    config_fastgpio(TEST_GPIO2);
#endif

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

