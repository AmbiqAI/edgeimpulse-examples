//*****************************************************************************
//
//! @file
//!
//! @brief Coremark running in SRAM.
//!
//! This example runs the official EEMBC COREMARK test.
//! It uses a prebuilt image to run coremark out of SRAM instead of Flash.
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
#include "am_devices.h"
#include "am_bsp.h"
#include "am_util.h"
#include "string.h"

#include "coremark_sram_image.h"

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
__asm void
am_run_coremark(uint32_t stack, uint32_t addr)
{
    MOV SP,R0
    BX  R1
}
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION > 6000000)
void __attribute__((naked))
am_run_coremark(uint32_t stack, uint32_t addr)
{
    __asm("mov sp,r0\n");
    __asm("bx r1\n");
}
#elif defined(__GNUC_STDC_INLINE__)
void __attribute__((naked))
am_run_coremark(uint32_t stack, uint32_t addr)
{
    __asm("mov sp,r0\n");
    __asm("bx r1\n");
}
#elif defined(__IAR_SYSTEMS_ICC__)
void
am_run_coremark(uint32_t stack, uint32_t addr)
{
    asm("MOV SP,R0");
    asm("BX  R1");
}
#endif

//*****************************************************************************
//
// Main.
//
//*****************************************************************************
int
main(void)
{
    volatile uint32_t *stack, *addr;

    //
    // Set the clock frequency.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Initialize the BSP.
    //
    am_bsp_low_power_init();

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
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Loading Coremark into SRAM!\n\n");

    //
    // Transfer Cormark image to SRAM
    //
    memcpy((void *)0x10002000, (void *)g_pui8CoremarkSramImage, COREMARK_SRAM_IMAGE_LENGTH);

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
    // am_bsp_debug_printf_disable();

    //
    // Set Vector table to SRAM
    //
    AM_REG(SYSCTRL, VTOR) = 0x10002000;

    //
    // Get stack and reset addresses
    //
    stack = (uint32_t *)&g_pui8CoremarkSramImage[0];
    addr = (uint32_t *)&g_pui8CoremarkSramImage[4];

    //
    // Run Coremark from SRAM
    //
    am_run_coremark(*stack, *addr);

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
//        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
