//*****************************************************************************
//
//! @file am_dhry.c
//!
//! @brief Support functions for Dhrystone.
//!
//! Implements several support functions for Dhrystone.
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

#include "dhry.h"
#include "am_dhry.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile uint32_t g_ui32SysTickWrappedTime = 0;
#if AM_PRINT_RESULTS
#define PRTBUFSIZE  (2*1024)
    char am_prtbuf[PRTBUFSIZE];
    char *am_pcBuf;
    volatile unsigned am_bufcnt=0;
#endif // AM_PRINT_RESULTS


//*****************************************************************************
//
// Systick ISR.
//
//*****************************************************************************
void
am_systick_isr(void)
{
    //
    // Add enough cycles to account for one full cycle of systick.
    //
    g_ui32SysTickWrappedTime += 0x01000000;
}

#if 0
int
am_util_stdio_fprintf(FILE* fp, const char *pcFmt, ...)
{
    uint32_t ui32NumChars;

    va_list pArgs;
    va_start(pArgs, pcFmt);
#if 1
    ui32NumChars = am_sprintf(pcFmt, pArgs);
#else
    ui32NumChars = am_util_stdio_printf(pcFmt, pArgs);
#endif
    va_end(pArgs);

    return ui32NumChars;
}
#endif

uint32_t
am_clock(void)
{
    return am_hal_systick_count() + g_ui32SysTickWrappedTime;
}

void
am_dhrystone_init(void)
{
#if AM_PRINT_RESULTS
    int ix;

    // Initialize our printf buffer.
    for(ix = 0; ix < PRTBUFSIZE; ix++)
    {
        am_prtbuf[ix] = 0x00;
    }
    am_pcBuf = am_prtbuf;
    am_bufcnt = 0;
#endif // AM_PRINT_RESULTS

    //
    // Set the clock frequency.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable the floating point module, and configure the core for lazy
    // stacking.
    //
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);

#ifdef AM_PART_APOLLO
    //
    // SRAM bank power setting.
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
    // Flash bank power set.
    //
    am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);
#endif // AM_PART_APOLLO

#ifdef AM_PART_APOLLO2
    //
    // Buck enable
    //
    am_hal_pwrctrl_bucks_enable();

#if 0
    //
    // Enable only the needed flash and SRAM.
    //
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH512K);
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM8K);
#endif // 0

// #### INTERNAL BEGIN ####
#if 0
    // NOTE: GPIO modifications have no real effect (this section basically
    //       sets them to power up settings).

    //
    // Set GPIOs to a known state.
    //
    AM_REG(GPIO, PADKEY) = AM_REG_GPIO_PADKEY_KEYVAL;

    AM_REG(GPIO, PADREGA) = 0x18181818;     // 0-3
    AM_REG(GPIO, PADREGB) = 0x18181818;     // 4-7
    AM_REG(GPIO, PADREGC) = 0x18181818;     // 8-11
    AM_REG(GPIO, PADREGD) = 0x18181818;     // 12-15
    AM_REG(GPIO, PADREGE) = 0x18181818;     // 16-19
    AM_REG(GPIO, PADREGF) &= 0x0000FFFF;    // 20-23, SWD on 20 & 21
    AM_REG(GPIO, PADREGF) |= 0x18180000;    //  "
    AM_REG(GPIO, PADREGG) = 0x18181818;     // 24-27
    AM_REG(GPIO, PADREGH) = 0x18181818;     // 28-31
    AM_REG(GPIO, PADREGI) = 0x18181818;     // 32-35
    AM_REG(GPIO, PADREGJ) = 0x18181818;     // 36-39
    AM_REG(GPIO, PADREGK) &= 0x0000FF00;    // 40-43, SWO on 41
    AM_REG(GPIO, PADREGK) |= 0x18180018;    //  "
    AM_REG(GPIO, PADREGL) = 0x18181818;     // 44-47
    AM_REG(GPIO, PADREGM) = 0x00001818;     // 48-49

    AM_REG(GPIO, CFGA) = 0;                 // 0-7
    AM_REG(GPIO, CFGB) = 0;                 // 8-15
    AM_REG(GPIO, CFGC) &= 0x00FF0000;       // 16-23, SWD on 20 & 21
    AM_REG(GPIO, CFGD) = 0;                 // 24-31
    AM_REG(GPIO, CFGE) = 0;                 // 32-39
    AM_REG(GPIO, CFGF) &= 0x000000F0;       // 40-47, SWO on 41
    AM_REG(GPIO, CFGG) = 0;                 // 48-49

    AM_REG(GPIO, ENA) = 0;
    AM_REG(GPIO, ENB) = 0;

    AM_REG(GPIO, PADKEY) = 0;
#endif
// #### INTERNAL END ####

    //
    // SRAM clkgate
    //
    AM_REG(PWRCTRL, SRAMCTRL) = AM_REG_PWRCTRL_SRAMCTRL_SRAM_MASTER_CLKGATE_M | AM_REG_PWRCTRL_SRAMCTRL_SRAM_CLKGATE_EN | AM_REG_PWRCTRL_SRAMCTRL_SRAM_LIGHT_SLEEP_DIS;

// #### INTERNAL BEGIN ####
    //
    // Apollo2 A0 note:
    // The following 2 settings, XTAL and VCOMP, seem to have varying effects
    // on current depending on the compiler.  For GCC and Keil, they seem to
    // save a couple of uA.  For IAR, they seem to add ~6 uA.
    // More investigation is needed.  REH 7/12/16
    //
// #### INTERNAL END ####
    //
    // Switch the RTC off of the XTAL
    //
    AM_BFW(CLKGEN, OCTRL, OSEL, 1);

    //
    // Turn off the voltage comparator
    //
    AM_REG(VCOMP, PWDKEY) = AM_REG_VCOMP_PWDKEY_KEYVAL;
#endif // AM_PART_APOLLO2

#if AM_PRINT_RESULTS
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Initialize the SWO GPIO pin
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
    // Clear the terminal.
    //
    am_util_stdio_terminal_clear();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Ambiq Micro Drystone test ...\n\n");
    am_util_delay_ms(2);

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
    am_hal_itm_disable();

#endif // AM_PRINT_RESULTS


    //
    // Start SysTick timer
    //
    am_hal_systick_load(0x00FFFFFF);
    am_hal_systick_int_enable();
    am_hal_systick_start();

} // am_dhrystone_init()

void am_dhrystone_fini(void)
{
    //
    // Stop SysTick timer
    //
    am_hal_systick_stop();

#if AM_PRINT_RESULTS
    int iCnt;
    char *pcBuf;

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Initialize the SWO GPIO pin
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
    // Clear the terminal.
    //
#if 0
    am_util_stdio_terminal_clear();
#endif

    //
    // Print the banner.
    //
    am_util_stdio_printf("\nAmbiq Micro Drystone run finished!\n\nResults:\n");

    //
    // Now, let's go parse the buffer and print it out!
    //
    pcBuf = am_prtbuf;
    iCnt = 0;
    while ( (*pcBuf != 0x00)  &&  (iCnt<PRTBUFSIZE) )
    {
        am_util_stdio_printf(pcBuf);
        while ( *pcBuf != 0x00 )
        {
            pcBuf++;
            iCnt++;
        }
        iCnt++;     // Account for the NULL terminator
        pcBuf++;    // Point after the NULL terminator to the next string
    }
#endif // AM_PRINT_RESULTS

    //
    // Enable the LEDs.
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);

    //
    // Turn on an LED.
    //
    am_devices_led_on(am_bsp_psLEDs, 0);

    //
    // Before exiting wait a little time to be sure all printing has
    // completed, and then disable the ITM.
    //
    am_util_delay_ms(2);

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
    am_hal_itm_disable();

#ifdef AM_PART_APOLLO2
    //
    // Re-enable flash and SRAM.
    //
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_ALL);
#endif // AM_PART_APOLLO2
}

#if AM_PRINT_RESULTS

int am_fprintf(FILE* fp, const char *pcFmt,...)
{
    uint32_t ui32NumChars;
    int iRet = 0;

    va_list pArgs;

    if ( am_bufcnt < PRTBUFSIZE )
    {
        va_start(pArgs, pcFmt);
        ui32NumChars = am_util_stdio_vsprintf(am_pcBuf, pcFmt, pArgs);
        va_end(pArgs);

        if ( (am_bufcnt+ui32NumChars) >= PRTBUFSIZE )
        {
            //
            // This string is 40 chars (with the NULL terminator)
            //
            am_util_stdio_sprintf(&am_prtbuf[PRTBUFSIZE-(40+1)], "BUFFER OVERFLOWED! Increase PRTBUFSIZE\n");
            am_prtbuf[PRTBUFSIZE-1] = 0x00;     // Double terminate the buffer
            am_pcBuf = &am_prtbuf[PRTBUFSIZE];  // Don't allow any further printing
            am_bufcnt = PRTBUFSIZE;             //  "
        }
        else
        {
            am_pcBuf += ui32NumChars;
            am_pcBuf++;                 // Skip NULL terminator
            am_bufcnt += ui32NumChars;
            am_bufcnt++;                // Include NULL terminator
            iRet = ui32NumChars;
        }
    } // if (am_bufcnt)

    return iRet;

} // am_sprintf()
#endif // AM_PRINT_RESULTS
