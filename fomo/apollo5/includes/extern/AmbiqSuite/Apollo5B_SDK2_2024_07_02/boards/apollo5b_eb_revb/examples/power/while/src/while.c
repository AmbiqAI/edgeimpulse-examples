//*****************************************************************************
//
//! @file while.c
//!
//! @brief Example to emulate a polling loop.
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup while While Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: This example provides a demonstration of the power required while<br>
//! executing in a tight loop on the Apollo5 MCU.<br>
//!<br>
//!   1) Initialize memory for performance and low power<br>
//!   2) Run while loop<br>
//!   3) Measure current draw<br>
//!
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

//*****************************************************************************
//
// Print option
//
//*****************************************************************************
#define PRINT_UART  1

#if 0
//
// The while loop, shown below in its C implementation, should be 32-byte aligned.
// To achieve this alignment easily across toolchains, we'll create the loop
// using an array of opcodes equivalent to the C code.
//
void while_loop(void)
{
    while(1)
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
}
#endif

//
// Prototype the assembly function.
//
typedef void (*whileloopfunc_t)(void);
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
#error The Arm5 compiler does not support M55
#define WHILE_ATTRIB
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
#define WHILE_ATTRIB   __attribute__ ((aligned (32)))
#elif defined(__GNUC_STDC_INLINE__)
#define WHILE_ATTRIB   __attribute__ ((aligned (32)))
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 32
#define WHILE_ATTRIB
#else
#error Unknown compiler.
#endif

//
// This is the aligned while loop, aka while_loop().
//

static
const
uint16_t whileloop[12] WHILE_ATTRIB =
{
    0xBF00, 0xBF00,
    0xBF00, 0xBF00,
    0xBF00, 0xBF00,
    0xBF00, 0xBF00,
    0xBF00, 0xBF00,
    0xE7F4,         // E7F4: b while_loop
    0x4770,         // 4770: bx lr (not actually needed and will never get here)
};
whileloopfunc_t while_loop = (whileloopfunc_t)((uint8_t *)whileloop + 1);

//*****************************************************************************
//
// Minimize power
//
//*****************************************************************************
static void apollo5_cache_memory_config(void)
{
    am_hal_pwrctrl_sram_memcfg_t SRAMMemCfg =
    {
        .eSRAMCfg               = AM_HAL_PWRCTRL_SRAM_NONE,
        .eActiveWithMCU         = AM_HAL_PWRCTRL_SRAM_NONE,
        .eActiveWithGFX         = AM_HAL_PWRCTRL_SRAM_NONE,
        .eActiveWithDISP        = AM_HAL_PWRCTRL_SRAM_NONE,
        .eSRAMRetain            = AM_HAL_PWRCTRL_SRAM_NONE
    };

#ifdef AM_PART_APOLLO5A
    am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
    {
        .eROMMode               = AM_HAL_PWRCTRL_ROM_AUTO,
        .bEnableCache           = true,
        .bRetainCache           = false,
        .eDTCMCfg               = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .eRetainDTCM            = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .bEnableNVM             = true,
        .bKeepNVMOnInDeepSleep  = false
    };
#else
    am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
    {
        .eROMMode               = AM_HAL_PWRCTRL_ROM_AUTO,
        .eDTCMCfg               = AM_HAL_PWRCTRL_ITCM32K_DTCM128K,
        .eRetainDTCM            = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_NORETAIN,
        .eNVMCfg                = AM_HAL_PWRCTRL_NVM0_ONLY,
        .bKeepNVMOnInDeepSleep  = false
    };
#endif // AM_PART_APOLLO5A

    am_hal_pwrctrl_mcu_memory_config(&McuMemCfg);

    //
    // Configure the cache, Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);


#ifdef AM_PART_APOLLO5A
    PWRCTRL->MEMRETCFG_b.CACHEPWDSLP = PWRCTRL_MEMRETCFG_CACHEPWDSLP_DIS;

    //
    // Configure the MRAM for low power mode
    //
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

//*****************************************************************************
//
// Main Function.
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Disable all peripherals including Crypto
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL, 0);

    //
    // Disable XTAL
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP, 0);

    //
    // Disable Debug Subsystem
    //
    MCUCTRL->DBGCTRL = 0;

    CLKGEN->OCTRL_b.OSEL = 1; // Used LFRC instead of XT
    CLKGEN->MISC = 0x08FC0000; // Enable all clock gating
    CLKGEN->CLKCTRL = 0x0; // Disable all unneccesary clocks including display controller clock

    //
    // Set memory configuration to minimum.
    //
    apollo5_cache_memory_config();

#ifdef AM_DEVICES_BLECTRLR_RESET_PIN
    //
    // For SiP packages, put the BLE Controller in reset.
    //
    am_hal_gpio_state_write(AM_DEVICES_BLECTRLR_RESET_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(AM_DEVICES_BLECTRLR_RESET_PIN,   am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(AM_DEVICES_BLECTRLR_RESET_PIN, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(AM_DEVICES_BLECTRLR_RESET_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
#endif // AM_DEVICES_BLECTRLR_RESET_PIN

#if (PRINT_UART == 1)
    am_bsp_uart_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Ambiq Micro 'while' example.\n\n");

    //
    // Print objective of the while example in SDK
    //
    am_util_stdio_printf("Used for measuring power in an infinite while loop.\n");

    //
    // Print the compiler version.
    //
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
    am_util_stdio_printf("HAL Compiler:    %s\n", g_ui8HALcompiler);
    am_util_stdio_printf("HAL SDK version: %d.%d.%d\n",
                         g_ui32HALversion.s.Major,
                         g_ui32HALversion.s.Minor,
                         g_ui32HALversion.s.Revision);
    am_util_stdio_printf("HAL compiled with %s-style registers\n",
                         g_ui32HALversion.s.bAMREGS ? "AM_REG" : "CMSIS");

    am_util_stdio_printf("\nEntering while loop...\n");

    //
    // To minimize power during the run, disable the UART.
    //
    am_hal_delay_us(10000);
    am_bsp_uart_printf_disable();


#endif // PRINT_UART

    while_loop();

}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

