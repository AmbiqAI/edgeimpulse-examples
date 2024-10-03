/*********************************************************
(C) 2014 EEMBC(R) and ULPBench(TM).  All rights reserved.

EEMBC ULPBench Software is a product of EEMBC and is provided
under the terms of the ULPBench License that is distributed
with the official EEMBC ULPBench Software release.
The Software is the proprietary intellectual property of EEMBC
and its members and is protected under all applicable laws,
including all applicable copyright laws.
If you received this EEMBC ULPBench Software
without the accompanying ULPBench License,
you must discontinue use and download the official release
from http://www.eembc.org/benchmark/ulp_sl.php.
**********************************************************/

//=============================================================================
// Platform.c
//
// Platform-specific declarations
//=============================================================================

//=============================================================================
// board/chip defines
#include "board.h"
#include "TesTPI.h"
#include "CoreProfile.h"
//=============================================================================

//*****************************************************************************
//
// Start the 1 second interrupt interval.
//
//*****************************************************************************
void RTC_Start( void )
{
    //
    // Enable the RTC.
    //
    am_hal_rtc_osc_enable();

    //
    // Enable the RTC 1 sec alarm interval.
    //
    am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC);
}

//*****************************************************************************
//
// Init function for the RTC
//
//*****************************************************************************
void ulp_pwrdown_memory(void)
{
#ifdef PART_apollo
    //
    // Power down all but the first SRAM banks.
    //
    am_hal_mcuctrl_sram_power_set(AM_HAL_MCUCTRL_SRAM_POWER_DOWN_1 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_2 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_3 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7,
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_1 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_2 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_3 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7);

    //
    // Power down flash bank 1
    //
    am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);
#endif // PART_apollo

}

//*****************************************************************************
//
// Enables hardware for debug printing inside the ULP CoreProfile.
// Note: This function if only called when DEBUG > 2 in the board.h
//
//*****************************************************************************
void ulp_debug_printing(void)
{
    //
    // Enable the ITM pin.
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Enable debug printf messages using ITM on SWO pin
    //
    am_bsp_debug_printf_enable();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("===== ULPBench =====\n\n");
    am_util_delay_ms(10);
}

//*****************************************************************************
//
// Initialize I/O for ULPBench.
//
//*****************************************************************************
void ulp_initialize_pins(void)
{
    //
    // Initialize LEDs.
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);

    //
    // Set up ULP status pin and clear it.
    //
    am_hal_gpio_pin_config(ULP_INDICATOR_PIN, AM_HAL_PIN_OUTPUT);
}

//*****************************************************************************
//
// Enable the RTC to generate the 1 second time base for the ULP CoreProfile.
//
//*****************************************************************************
void ulp_rtc_init(void)
{
	//
	// Wait for XT stable and clocking RTC.
	//
    while(AM_REG(CLKGEN,STATUS) & 0x00000003)
    {
        am_devices_led_toggle(am_bsp_psLEDs, 0);
        am_util_delay_ms(50);
    }

    //
    // Make sure LED is off.
    //
    am_devices_led_off(am_bsp_psLEDs, 0);

    //
    // Clear RTC interrupts.
    //
    am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);
    am_hal_rtc_int_clear(AM_HAL_RTC_INT_OF);

    //
    // Enable RTC Alarm interrupts
    //
    am_hal_rtc_int_enable(AM_HAL_RTC_INT_ALM);

    //
    // Enale the CLKGEN interupts in the NVIC
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CLKGEN);
}

//*****************************************************************************
//
// Setup up hardware for low-power.
//
//*****************************************************************************
void ulp_low_power_init(void)
{
#ifdef PART_apollo
    //
    // Enable internal buck converters.
    //
    am_hal_pwrctrl_bucks_enable();

    //
    // Turn off the voltage comparator as this is enabled on reset.
    //
    am_hal_vcomp_disable();

    //
    // Disable the bandgap.
    //
    am_hal_mcuctrl_bandgap_disable();
#endif // PART_apollo

    //
    // Power down unused sram and flash banks
    //
    ulp_pwrdown_memory();
}

//*****************************************************************************
//
// Setup up platform specific hardware part 1.
//
//*****************************************************************************
void hardware_setup_part1(void)
{
    //
    // Set the system clock to maximum frequency.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Low power initialization
    //
    ulp_low_power_init();

    //
    // Configure the IOs to work with ULPBench.
    //
    ulp_initialize_pins();

    //
    // Run ULP on RTC
    //
    ulp_rtc_init();

#if DEBUG>2
    //
    // Initialize system for debug printing
    //
    ulp_debug_printing();

#endif  // DEBUG

    //
    // Enable interrupts
    //
    am_hal_interrupt_master_enable();
}

//*****************************************************************************
//
// Setup up platform specific hardware part 2.
//
//*****************************************************************************
void hardware_setup_part2( void )
{
    RTC_Start();
}

//*****************************************************************************
//
// RTC ISR for 1 second timebase.
//
//*****************************************************************************
void am_clkgen_isr(void)
{
    //
    // Clear the RTC Alarm Interrupt.
    //
    AM_REG(RTC, INTCLR) = AM_HAL_RTC_INT_ALM;

    //
    // Run ULP bench
    //
    tesTimerInterrupt();
}
