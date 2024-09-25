//*****************************************************************************
//
//! @file deepsleep.c
//!
//! @brief Example demonstrating how to enter deepsleep.
//!
//! Purpose: This example configures the device to go into a deep sleep mode. Once in
//! sleep mode the device has no ability to wake up. This example is merely to
//! provide the opportunity to measure deepsleep current without interrupts
//! interfering with the measurement.
//!
//! The example begins by printing out a banner announcement message through
//! the UART, which is then completely disabled for the remainder of execution.
//!
//! Text is output to the UART at 115,200 BAUD, 8 bit, no parity.
//! Please note that text end-of-line is a newline (LF) character only.
//! Therefore, the UART terminal must be set to simulate a CR/LF.
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

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

#define ALL_RETAIN 0 // 0 for min TCM retain (default), 1 for all retain

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    am_util_delay_ms(5000);

    am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
    {
         //
         //  In order to demonstrate the lowest possible power, this example enables the ROM automatic power down feature.
         //  This should not be used in general for most applications.
         //
        .eROMMode       = AM_HAL_PWRCTRL_ROM_AUTO,
#if defined(AM_PART_APOLLO5A)
        .bEnableCache   = false,
        .bRetainCache   = false,
#endif
#if defined(AM_PART_APOLLO5A)
#if ALL_RETAIN
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
#else
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
#endif
#elif defined(AM_PART_APOLLO5B)
#if ALL_RETAIN
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_RETAIN,
#else
        .eDTCMCfg       = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .eRetainDTCM    = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_RETAIN,
#endif
#endif
#if defined(AM_PART_APOLLO5A)
        .bEnableNVM     = true,
#else
        .eNVMCfg        = AM_HAL_PWRCTRL_NVM0_ONLY,
#endif
        .bKeepNVMOnInDeepSleep     = false
    };

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

    //
    // Initialize the printf interface for UART output.
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Deepsleep Example\n");

    //
    // To minimize power during the run, disable the UART.
    //
    am_bsp_uart_printf_disable();

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

   
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC); // Use LFRC instead of XT
    
    //
    // Disable XTAL 
    //
    MCUCTRL->XTALCTRL = 0;
    
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP, 0);

    // Disable RTC Oscillator
    am_hal_rtc_osc_disable();
    
    //
    // Disable all peripherals including Crypto
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL, 0);
    
    //
    // Disable Debug Subsystem
    //
    MCUCTRL->DBGCTRL = 0;

    am_hal_pwrctrl_mcu_memory_config(&McuMemCfg);
    
    //
    //
    // MRAM0 LP Setting affects the Latency to wakeup from Sleep. Hence, it should be configured in the application.
    //
    //

    // Configure the MRAM for low power mode.  Note that using MRAM LP mode has been shown to interact with
    // the Apollo5 RevA SW workaround for LP/HP mode transitions.  Thus, it is not recommended for general use
    // in customer applications.

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

    while (1)
    {
        //
        // Go to Deep Sleep and stay there.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
