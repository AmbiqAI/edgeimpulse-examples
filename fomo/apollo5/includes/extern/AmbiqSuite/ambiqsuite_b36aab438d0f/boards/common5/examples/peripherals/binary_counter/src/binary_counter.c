//*****************************************************************************
//
//! @file binary_counter.c
//!
//! @brief Example that displays the timer count on the LEDs.
//!
//! Purpose: This example increments a variable on every timer interrupt. The global
//! variable is used to set the state of the LEDs. The example sleeps otherwise.
//!
//! Printing takes place over the SWO/ITM at 1M Baud.
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#if AM_BSP_NUM_LEDS > 0
#define MAX_COUNT       ((1 << AM_BSP_NUM_LEDS) - 1)
#else
#define NUMLEDS         3       // Default to the expected number of LEDs
#define MAX_COUNT       ((1 << NUMLEDS) - 1)
#endif

#define TIMER_NUM       0       // Timer number to be used in the example

//#define PRINTF_DISABLE       // Defined inhibits all SWO printing (LEDs only)
#ifdef PRINTF_DISABLE
#define BC_PRINTF(...)
#else
#define BC_PRINTF(...)  am_util_stdio_printf(__VA_ARGS__)
#endif

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
volatile uint32_t g_ui32TimerCount = 0;

//*****************************************************************************
//
// Function to initialize the timer to interrupt once every second.
//
//*****************************************************************************
void
timer_init(void)
{
    am_hal_timer_config_t   TimerConfig;

    //
    // Set up the desired TIMER.
    // The default config parameters include:
    //  eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16
    //  eFunction = AM_HAL_TIMER_FN_EDGE
    //  Compare0 and Compare1 maxed at 0xFFFFFFFF
    //
    am_hal_timer_default_config_set(&TimerConfig);

    //
    // Modify the default parameters.
    // Configure the timer to a 1s period via ui32Compare1.
    //
    TimerConfig.eFunction        = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32Compare1     = 6000000;
#ifdef APOLLO5_FPGA
    TimerConfig.ui32Compare1 /= (96 / APOLLO5_FPGA);    // Adjust for FPGA frequency
#endif // APOLLO5_FPGA

    //
    // Configure the timer
    //
    if ( am_hal_timer_config(TIMER_NUM, &TimerConfig) != AM_HAL_STATUS_SUCCESS )
    {
        BC_PRINTF("Failed to configure TIMER%d.\n", TIMER_NUM);
    }

    //
    // Clear the timer and its interrupt
    //
    am_hal_timer_clear(TIMER_NUM);
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_NUM, AM_HAL_TIMER_COMPARE1));

} // timer_init()

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    //
    // Clear the timer Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_NUM, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(TIMER_NUM);

    //
    // Increment count and set limit based on the number of LEDs available.
    //
    g_ui32TimerCount = ( g_ui32TimerCount >= MAX_COUNT ) ? 0 : g_ui32TimerCount + 1;

} // am_ctimer_isr()

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Ret;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

#if AM_BSP_NUM_LEDS > 0
    //
    // Initialize the LED array
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
#endif

    //
    // Initialize the count so that the counting begins with 0
    // after the first trip to the ISR.
    //
    g_ui32TimerCount = MAX_COUNT;

#ifdef PRINTF_DISABLE
    //
    // Initialize timer.
    //
    timer_init();
#else
    //
    // Initialize the printf interface for SWO/ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Binary Counter Example for %s\n", AM_HAL_DEVICE_NAME);
    am_util_stdio_printf("---------------------------------------------------- \n");
    am_util_stdio_printf("The Timer clock source is HFRC_DIV16.\n");
    am_util_stdio_printf("The Timer wakes once per second to show a binary \n");
    am_util_stdio_printf(" value on the LEDs and a decimal value on SWO.\n");
    am_util_stdio_printf("---------------------------------------------------- \n");

#if defined(AM_PART_APOLLO5A)
    //
    // Checks if itm is busy and provides a delay to flush the fifo
    //
    am_bsp_debug_itm_printf_flush();
#endif // AM_PART_APOLLO5A

    //
    // Initialize timer.
    //
    timer_init();

    //
    // Disable debug printf messages on SWO.
    //
#if !defined(AM_PART_APOLLO5A)
    am_bsp_debug_printf_disable();
#endif // !AM_PART_APOLLO5A
#endif // !PRINTF_DISABLE

    //
    // Enable the timer Interrupt.
    //
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(TIMER_NUM, AM_HAL_TIMER_COMPARE1));

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(TIMER_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Start the timer
    //
    am_hal_timer_start(TIMER_NUM);

    //
    // Loop forever.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

#if AM_BSP_NUM_LEDS > 0
        //
        // Set the LEDs.
        //
        am_devices_led_array_out(am_bsp_psLEDs, AM_BSP_NUM_LEDS, g_ui32TimerCount);
#endif // AM_BSP_NUM_LEDS

#ifndef PRINTF_DISABLE
        //
        // Enable debug printf messages using SWO.
        //
#if !defined(AM_PART_APOLLO5A)
        am_bsp_debug_printf_enable();
#endif // AM_PART_APOLLO5A
#endif // !PRINTF_DISABLE

        //
        // Output current count.
        //
        BC_PRINTF("%2d ", g_ui32TimerCount);

        if ( g_ui32TimerCount >= MAX_COUNT )
        {
#if !defined(AM_PART_APOLLO5A)
            am_hal_pwrctrl_mcu_mode_e eCurrentPowerMode;

            //
            // Take this opportunity to toggle TurboSPOT mode.
            //
            ui32Ret = am_hal_pwrctrl_mcu_mode_status(&eCurrentPowerMode);
            if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
            {
                BC_PRINTF("\n\nERROR (%d) getting current power mode status\n", ui32Ret);
            }

            if ( eCurrentPowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE )
            {
                ui32Ret = am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER);
                if ( ui32Ret == AM_HAL_STATUS_SUCCESS )
                {
                    BC_PRINTF("\n\nSwitched to LP Low Power Mode (%dMHZ)\n", AM_HAL_CLKGEN_FREQ_MAX_MHZ);
                }
                else
                {
                    BC_PRINTF("\n\nERROR (%d) while switching from TurboSPOT mode to Low Power.\n", ui32Ret);
                }
            }
            else if ( eCurrentPowerMode == AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER )
            {
                ui32Ret = am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
                if ( ui32Ret == AM_HAL_STATUS_SUCCESS )
                {
#if defined(AM_PART_APOLLO5A)
                    if (CLKGEN->MISC_b.CPUHPFREQSEL == CLKGEN_MISC_CPUHPFREQSEL_HP250MHz)
                    {
                        BC_PRINTF("\n\nSwitched to HP High Performance TurboSPOT mode (%dMHz)\n", AM_HAL_CLKGEN_FREQ_HP250_MHZ);
                    }
                    if (CLKGEN->MISC_b.CPUHPFREQSEL == CLKGEN_MISC_CPUHPFREQSEL_HP192MHz)
                    {
                        BC_PRINTF("\n\nSwitched to HP High Performance TurboSPOT mode (%dMHz)\n", AM_HAL_CLKGEN_FREQ_HP192_MHZ);
                    }
#else
                    BC_PRINTF("\n\nSwitched to HP High Performance TurboSPOT mode (%dMHz)\n", AM_HAL_CLKGEN_FREQ_HP250_MHZ);
#endif  // defined(AM_PART_APOLLO5A)
                }
                else
                {
                    BC_PRINTF("\n\nERROR (%d) while switching from Low Performance mode to TurboSPOT mode.\n", ui32Ret);
                }
            }
            else
            {
                BC_PRINTF("\n\nERROR, invalid current power mode = (%d).\n", (uint32_t)eCurrentPowerMode);
            }
#else
            BC_PRINTF("\n");
#endif

#if defined(AM_PART_APOLLO5A)
            //
            // Checks if itm is busy and provides a delay to flush the fifo
            //
            am_bsp_debug_itm_printf_flush();
#endif // !AM_PART_APOLLO5A
        }

#ifndef PRINTF_DISABLE
        //
        // Disable debug printf messages on SWO.
        //
#if !defined(AM_PART_APOLLO5A)
        am_bsp_debug_printf_disable();
#endif // !AM_PART_APOLLO5A
#endif // !PRINTF_DISABLE

    } // while()

} // main()
