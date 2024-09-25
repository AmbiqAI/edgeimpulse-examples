//*****************************************************************************
//
// file core_portme.c
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
/*
    File : core_portme.c
*/
/*
    Author : Shay Gal-On, EEMBC
    Legal : TODO!
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "coremark.h"
#include "am_mcu_apollo.h"

//
// This variable stores the value of the loop iteration
//
//
volatile e_loop_iter g_eAmbClkSpd;

#if VALIDATION_RUN
    volatile ee_s32 seed1_volatile = 0x3415;
    volatile ee_s32 seed2_volatile = 0x3415;
    volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PERFORMANCE_RUN
    volatile ee_s32 seed1_volatile = 0x0;
    volatile ee_s32 seed2_volatile = 0x0;
    volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PROFILE_RUN
    volatile ee_s32 seed1_volatile = 0x8;
    volatile ee_s32 seed2_volatile = 0x8;
    volatile ee_s32 seed3_volatile = 0x8;
#endif
    volatile ee_s32 seed4_volatile = ITERATIONS;
    volatile ee_s32 seed5_volatile = 0;
#if AM_PRINT_RESULTS
#define PRTBUFSIZE  768
    char am_prtbuf[PRTBUFSIZE];
    char *am_pcBuf;
    volatile unsigned am_bufcnt = 0;
#endif // AM_PRINT_RESULTS

/* Porting : Timing functions
    How to capture time and convert to seconds must be ported to whatever is supported by the platform.
    e.g. Read value from on board RTC, read value from cpu clock cycles performance counter etc.
    Sample implementation for standard time.h and windows.h definitions included.
*/
/* Define : TIMER_RES_DIVIDER
    Divider to trade off timer resolution and total time that can be measured.

    Use lower values to increase resolution, but make sure that overflow does not occur.
    If there are issues with the return value overflowing, increase this value.
    */
#ifdef TIME_64
#define CORETIMETYPE ee_u64
#else
#define CORETIMETYPE ee_u32
#endif
#define MYTIMEDIFF(fin, ini) ((fin) - (ini))
#define SAMPLE_TIME_IMPLEMENTATION 1
#if 1   // Defined in core_portme.h
#define NSECS_PER_SEC           AM_CORECLK_HZ
#define NSECS_PER_SEC_250MHZ    (250000000)
#ifdef TIME_64
#define GETMYTIME (0x00FFFFFF - am_hal_systick_count() + ((CORETIMETYPE)g_ui32SysTickWrappedTime * 0x1000000))
#else
#define GETMYTIME (0x00FFFFFF - am_hal_systick_count() + g_ui32SysTickWrappedTime)
#endif
#define START_PA_DUMP (*((volatile ee_u32 *)0x4ffff014))
#define TIMER_RES_DIVIDER 1
#define EE_TICKS_PER_SEC           (NSECS_PER_SEC / TIMER_RES_DIVIDER)
#define EE_TICKS_PER_SEC_250MHZ    (NSECS_PER_SEC_250MHZ / TIMER_RES_DIVIDER)
#endif

#if defined(AM_PART_APOLLO5A)
// Trim write enable/disable macros
#define TRIM_WR_ENABLE                         \
        AM_REGVAL(0x400401fc) = 0x2EF1543E;    \
        AM_REGVAL(0x400400f8) = 0xDF328C18;    \
        AM_REGVAL(0x40040144) = 0x8299B572;    \
        AM_REGVAL(0x40040278) = 0xA07DB9C8;

#define TRIM_WR_DISABLE                        \
        AM_REGVAL(0x400401fc) = 0;             \
        AM_REGVAL(0x400400f8) = 0;             \
        AM_REGVAL(0x40040144) = 0;             \
        AM_REGVAL(0x40040278) = 0;
#endif


/** Define Host specific (POSIX), or target specific global time variables. */
static CORETIMETYPE start_time_val, stop_time_val;

/* Function : start_time
    This function will be called right before starting the timed portion of the benchmark.

    Implementation may be capturing a system timer (as implemented in the example code)
    or zeroing some system parameters - e.g. setting the cpu clocks cycles to 0.
*/
void start_time(void)
{
    am_hal_systick_init(AM_HAL_SYSTICK_CLKSRC_INT);
    am_hal_systick_stop();
    am_hal_systick_load(0x00FFFFFF);
    am_hal_systick_int_enable();
    am_hal_interrupt_master_enable();
    am_hal_systick_start();
    start_time_val = GETMYTIME; // GETMYTIME could be used - but there should be very small change anyways, as we just started
    START_PA_DUMP = 0x1;
}

/* Function : stop_time
    This function will be called right after ending the timed portion of the benchmark.

    Implementation may be capturing a system timer (as implemented in the example code)
    or other system parameters - e.g. reading the current value of cpu cycles counter.
*/

void stop_time(void)
{
    am_hal_systick_stop();
    stop_time_val = GETMYTIME;
    START_PA_DUMP = 0x0;
}
/* Function : get_time
    Return an abstract "ticks" number that signifies time on the system.

    Actual value returned may be cpu cycles, milliseconds or any other value,
    as long as it can be converted to seconds by <time_in_secs>.
    This methodology is taken to accomodate any hardware or simulated platform.
    The sample implementation returns millisecs by default,
    and the resolution is controlled by <TIMER_RES_DIVIDER>
*/
CORE_TICKS get_time(void)
{
    CORE_TICKS elapsed = (CORE_TICKS)(MYTIMEDIFF(stop_time_val, start_time_val));
    return elapsed;
}

/* Function : time_in_secs
    Convert the value returned by get_time to seconds.

    The <secs_ret> type is used to accomodate systems with no support for floating point.
    Default implementation implemented by the EE_TICKS_PER_SEC macro above.
*/
secs_ret time_in_secs(CORE_TICKS ticks)
{
    secs_ret retval = 0;

    if (g_eAmbClkSpd == AM_CLK_SPEED_250MHz)
    {
        retval = ((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC_250MHZ;
    }
    else
    {
        retval = ((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
    }

    return retval;
}

ee_u32 default_num_contexts = 1;

static void apollo5_cache_memory_config(void)
{
    am_hal_pwrctrl_sram_memcfg_t SRAMMemCfg =
    {
#if ALL_RETAIN
      .eSRAMCfg         = AM_HAL_PWRCTRL_SRAM_3M,
#else
      .eSRAMCfg         = AM_HAL_PWRCTRL_SRAM_NONE,
#endif
      .eActiveWithMCU   = AM_HAL_PWRCTRL_SRAM_NONE,
      .eActiveWithGFX   = AM_HAL_PWRCTRL_SRAM_NONE,
      .eActiveWithDISP  = AM_HAL_PWRCTRL_SRAM_NONE,
#if ALL_RETAIN
      .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_3M
#else
      .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_NONE
#endif
    };

    am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
    {

         
         //
         //  In order to demonstrate the lowest possible power, this example enables the ROM automatic power down feature.
         //  This should not be used in general for most applications.
         //

        .eROMMode       = AM_HAL_PWRCTRL_ROM_AUTO, 
#if defined(AM_PART_APOLLO5A)
        .bEnableCache   = true,
        .bRetainCache   = true,
        .bEnableNVM     = true,
#if ALL_RETAIN
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
#else
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
#endif
#elif defined(AM_PART_APOLLO5B)
        .eNVMCfg        = AM_HAL_PWRCTRL_NVM0_ONLY,
#if ALL_RETAIN
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_RETAIN,
#else
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_NORETAIN,
#endif
#else
#error AM_PART_APOLLO5x is not defined
#endif // AM_PART_APOLLO5A
        .bKeepNVMOnInDeepSleep     = false
    };

    am_hal_pwrctrl_mcu_memory_config(&McuMemCfg);

   
    //
    // MRAM0 LP Setting affects the Latency to wakeup from Sleep. Hence, it should be configured in the application.
    //

#if defined(AM_PART_APOLLO5A)
    MCUCTRL->MRAMPWRCTRL_b.MRAMLPREN = 1;
    MCUCTRL->MRAMPWRCTRL_b.MRAMSLPEN = 0;
    MCUCTRL->MRAMPWRCTRL_b.MRAMPWRCTRL = 1;
#else
    MCUCTRL->MRAMCRYPTOPWRCTRL_b.MRAM0LPREN = 1;
    MCUCTRL->MRAMCRYPTOPWRCTRL_b.MRAM0SLPEN = 0;
    MCUCTRL->MRAMCRYPTOPWRCTRL_b.MRAM0PWRCTRL = 1;
#endif

    //
    // Disable SRAM
    //
    am_hal_pwrctrl_sram_config(&SRAMMemCfg);

} // apollo5_cache_memory_config()

/* Function : portable_init
    Target specific initialization code
    Test for some common mistakes.
*/
void
portable_init(core_portable *p, int *argc, char *argv[])
{
    if (sizeof(ee_ptr_int) != sizeof(ee_u8 *))
    {
        ee_printf("ERROR! Please define ee_ptr_int to a type that holds a pointer!\n");
    }

    if (sizeof(ee_u32) != 4)
    {
        ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type!\n");
    }
    p->portable_id = 1;

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();
  

#if AM_PRINT_RESULTS
    int ix;

    //
    // Initialize our printf buffer.
    //
    for ( ix = 0; ix < PRTBUFSIZE; ix++ )
    {
        am_prtbuf[ix] = 0x00;
    }
    am_pcBuf = am_prtbuf;
    am_bufcnt = 0;

    //
    // Initialize the printf interface for UART output.
    //
    am_bsp_uart_printf_enable();

    if (g_eAmbClkSpd == AM_CLK_SPEED_250MHz)
    {
        am_util_stdio_printf("\n\n\n\nAmbiq Micro Coremark 250MHz test...\n\n");
    }
    else
    {
        am_util_stdio_printf("\n\n\n\nAmbiq Micro Coremark 96MHz test...\n\n");
    }

    //
    // To minimize power during the run, disable the UART.
    //
    am_bsp_uart_printf_disable();

#endif // AM_PRINT_RESULTS



    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC); // Use LFRC instead of XT
    
    //
    // Disable XTAL 
    //
    MCUCTRL->XTALCTRL = 0;
    
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP, 0);

    // Disable RTC Oscillator
    am_hal_rtc_osc_disable();
    

    // Disable Voltage comparator
    VCOMP->PWDKEY = VCOMP_PWDKEY_PWDKEY_Key;


    // Temporary fix to set DBGCTRL Register to 0
    MCUCTRL->DBGCTRL = 0;
    
    // Powering down various peripheral power domains
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DEBUG);
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP); 

#ifndef APOLLO5_FPGA
    //
    // Set memory configuration to minimum.
    //
    apollo5_cache_memory_config();
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

    if (g_eAmbClkSpd == AM_CLK_SPEED_250MHz)
    {
        uint32_t ui32retval = am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
        if ( ui32retval != AM_HAL_STATUS_SUCCESS )
        {
            ee_printf("Error with am_hal_pwrctrl_mcu_mode_select(), returned %d.\n", ui32retval);
        }
    }
    else
    {
        uint32_t ui32retval = am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER);
        if ( ui32retval != AM_HAL_STATUS_SUCCESS )
        {
            ee_printf("Error with am_hal_pwrctrl_mcu_mode_select(), returned %d.\n", ui32retval);
        }
    }

} // portable_init()

/* Function : portable_fini
    Target specific final code
*/
void portable_fini(core_portable *p)
{
    p->portable_id = 0;

#if AM_PRINT_RESULTS
    int iCnt;
    char *pcBuf;

    //
    // Initialize the printf interface for UART output.
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_printf("\nAmbiq Micro Coremark run finished!\n\n");

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

    //
    // Disable the UART.
    //
    am_bsp_uart_printf_disable();
#endif // AM_PRINT_RESULTS

#if AM_BSP_NUM_LEDS
void set_leds(uint32_t mask, uint32_t delay);   // Function prototype
    //
    // Now for the grand finale, do a little something with the LEDs.
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);

    uint32_t ux, umask, umod;
    for (ux = 0; ux < (AM_BSP_NUM_LEDS * 4); ux++ )
    {
        umod = (ux % (AM_BSP_NUM_LEDS * 2));
        if ( umod < AM_BSP_NUM_LEDS )
        {
            // Walk up the LEDs sequentially.
            umask = 1 << umod;
        }
        else
        {
            // Go the other direction.
            umask = (1 << (AM_BSP_NUM_LEDS - 1)) >> (umod - AM_BSP_NUM_LEDS);
        }
        set_leds(umask, 200);
    }

    //
    // Flash the LED array 3 times.
    //
    for (ux = 0; ux < 3; ux++ )
    {
        set_leds((1 << AM_BSP_NUM_LEDS) - 1, 300);
        set_leds(0x00, 300);
    }
#endif // AM_BSP_NUM_LEDS
} // portable_fini()

#if AM_BSP_NUM_LEDS
void set_leds(uint32_t mask, uint32_t delay)
{
    am_devices_led_array_out(am_bsp_psLEDs, AM_BSP_NUM_LEDS, mask);
    am_util_delay_ms(delay);
}
#endif // AM_BSP_NUM_LEDS

#if AM_PRINT_RESULTS
int am_sprintf(char *pcFmt, ...)
{
    uint32_t ui32NumChars;
    int iRet = 0;

    va_list pArgs;

    if ( am_bufcnt < PRTBUFSIZE )
    {
        va_start(pArgs, pcFmt);
        ui32NumChars = am_util_stdio_vsprintf(am_pcBuf, pcFmt, pArgs);
        va_end(pArgs);

        if ( (am_bufcnt + ui32NumChars) >= PRTBUFSIZE )
        {
            //
            // This string is 40 chars (with the NULL terminator)
            //
            am_util_stdio_sprintf(&am_prtbuf[PRTBUFSIZE - (40 + 1)], "BUFFER OVERFLOWED! Increase PRTBUFSIZE\n");
            am_prtbuf[PRTBUFSIZE - 1] = 0x00;   // Double terminate the buffer
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
