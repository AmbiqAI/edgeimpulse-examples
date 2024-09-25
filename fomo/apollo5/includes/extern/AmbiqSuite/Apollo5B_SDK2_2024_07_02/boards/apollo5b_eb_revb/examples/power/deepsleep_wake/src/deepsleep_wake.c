//*****************************************************************************
//
//! @file deepsleep_wake.c
//!
//! @brief Example that goes to deepsleep and wakes from either the RTC or GPIO.
//!
//! Purpose: This example configures the device to go into a deep sleep mode.
//! Once in deep sleep the RTC peripheral will wake the device every second.
//! Similarly, the GPIO peripheral wakes it up every time the button0 is pressed
//! or if the designated GPIO Toggles from Lo to Hi.
//!
//! The RTC Interrupt causes Led1 to toggle every 5sec if Leds are present
//! on the board. Else it prints ui32PntCnt through UART every 5sec and resets
//! to 0 once ui32PntCnt == 20.
//! Similarly, the GPIO Interrupt causes Led0 to toggle every time the button0
//! is pressed or the designated gpio is toggled from Lo2Hi.
//! If Leds aren't found on the board it sends out the GPIO Assertion Msg
//! through the UART.
//! The ALL_RETAIN == 0 case, we requires a custom system-config.yaml different
//! from the default system-config.yaml. The reason that is necessary is because
//! the TCM Memory Configuration for ALL_RETAIN == 0 is AM_HAL_PWRCTRL_ITCM32K_DTCM128K
//! and the start and end addresses of stack, heap and TCM falls within that addresses
//! range.
//! The example begins by printing out a banner annoucement message through
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
// RTC time structure.
am_hal_rtc_time_t g_sTime;
#define WAKEUP_GPIO_PIN         10
#define WAKEUP_RTC_PIN          60
#define INTERRUPT_GPIO 5

#define GPIO_INTERRUPT_ENABLE 1
#define RTC_INTERRUPT_ENABLE 1
#if AM_BSP_NUM_LEDS == 0
// Define PRINT_WAKE_LOOP for debug only, will use more power.
//#define PRINT_WAKE_LOOP

#ifdef PRINT_WAKE_LOOP
    uint32_t ui32PntCnt = 0;
#endif
#endif

//*****************************************************************************
//
// GPIO ISR
//
//*****************************************************************************
void
am_gpio0_001f_isr(void)
{
    am_hal_gpio_output_set(WAKEUP_GPIO_PIN);

    uint32_t ui32IntStatus;

    //
    // Delay for GPIO debounce.
    //
    am_util_delay_us(20000);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, true, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    AM_CRITICAL_END
#ifdef PRINT_WAKE_LOOP
    am_bsp_uart_printf_enable();
    am_util_stdio_printf("\n GPIO Interrupt is Asserted:  \n ");
    am_bsp_uart_printf_disable();
#endif

    //
    // Toggle LED 0.
    //
#if (AM_BSP_NUM_LEDS > 0)
    am_devices_led_toggle(am_bsp_psLEDs, 0);
#endif

}

//*****************************************************************************
//
// RTC ISR
//
//*****************************************************************************
static uint32_t g_RTCseconds = 0;

void
am_rtc_isr(void)
{
    am_hal_gpio_output_set(WAKEUP_RTC_PIN);
    //
    // Clear the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);


    if ( ++g_RTCseconds >= 5 )
    {
        //
        // Reset the seconds counter.
        //
        g_RTCseconds = 0;
#if (AM_BSP_NUM_LEDS > 0)
        //
        // Toggle LED 1.
        //
        am_devices_led_toggle(am_bsp_psLEDs, 1);
#else

#ifdef PRINT_WAKE_LOOP
        am_bsp_uart_printf_enable();
        am_util_stdio_printf(" %d", ui32PntCnt + 1);
        if ( (++ui32PntCnt % 20) == 0 )
        {
            ui32PntCnt = 0;
            am_util_stdio_printf("\nwake: ");
        }
        am_bsp_uart_printf_disable();
#endif
#endif
    }
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************

int
main(void)
{
    // Adding a 5 sec Delay
    am_util_delay_us(5000000);

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
      .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_3M,
#else
      .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_NONE
#endif
    };

#if GPIO_INTERRUPT_ENABLE
    am_hal_gpio_pinconfig(WAKEUP_GPIO_PIN, am_hal_gpio_pincfg_output); // Wakeup Flag GPIO
    am_hal_gpio_output_set(WAKEUP_GPIO_PIN);
#endif
#if RTC_INTERRUPT_ENABLE
    am_hal_gpio_pinconfig(WAKEUP_RTC_PIN, am_hal_gpio_pincfg_output); // Wakeup Flag RTC
    am_hal_gpio_output_set(WAKEUP_RTC_PIN);
#endif

    //
    // Initialize the printf interface for UART output.
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Deepsleep Wake Example\n");

    //
    // We are done printing.
    // Disable the UART
    //
    am_bsp_uart_printf_disable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
 

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

#if RTC_INTERRUPT_ENABLE

    //
    // Enable the XT for the RTC.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL, 0);

    //
    // Select XT for RTC clock source
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);

    //
    // Enable the RTC.
    //
    am_hal_rtc_osc_enable();
#else
    //
    // Enable the LFRC for the RTC.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC, 0);

    //
    // Select LFRC for RTC clock source.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP, 0);
    MCUCTRL->XTALCTRL = 0;
    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();

#endif

#if GPIO_INTERRUPT_ENABLE
    am_hal_gpio_mask_t GpioIntMask = AM_HAL_GPIO_MASK_DECLARE_ZERO;

    //
    // Set the Input Pin Configuarions for Pin
    //

    am_hal_gpio_pincfg_t sInPinCfg = AM_HAL_GPIO_PINCFG_DEFAULT;
    sInPinCfg.GP.cfg_b.eIntDir = AM_HAL_GPIO_PIN_INTDIR_LO2HI;
    sInPinCfg.GP.cfg_b.eGPInput = AM_HAL_GPIO_PIN_INPUT_ENABLE;
    sInPinCfg.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X;

    //
    // Flag used in Interrupt Status api to get the enabled Interrupt in GPIO Interrupt Mask
    //

    bool bEnabledOnly;

    //
    // Interrupt Pin to be Tested
    //
#if (AM_BSP_NUM_BUTTONS > 0)  &&  (AM_BSP_NUM_LEDS > 0)
    uint32_t ui32PinTestIntrpt = AM_BSP_GPIO_BUTTON0;
#else
    uint32_t ui32PinTestIntrpt = INTERRUPT_GPIO;
#endif
    am_hal_gpio_pinconfig(ui32PinTestIntrpt, sInPinCfg);

    bEnabledOnly = true;

    //
    // Read the GPIO Interrupt Status
    //

    am_hal_gpio_interrupt_status_get(AM_HAL_GPIO_INT_CHANNEL_0, bEnabledOnly, &GpioIntMask);

    //
    // Clear the GPIO Interrupts
    //

    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, &GpioIntMask);

    //
    // Enable GPIO Interrupt
    //
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0, AM_HAL_GPIO_INT_CTRL_INDV_ENABLE, &ui32PinTestIntrpt);
#if (AM_BSP_NUM_BUTTONS > 0)  &&  (AM_BSP_NUM_LEDS > 0)
    //
    // Configure the LEDs.
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);

    //
    // Turn the LEDs off, but initialize LED1 on so user will see something.
    //
    for (int ix = 0; ix < AM_BSP_NUM_LEDS; ix++)
    {
        am_devices_led_off(am_bsp_psLEDs, ix);
    }

    am_devices_led_on(am_bsp_psLEDs, 1);
#endif
#endif
#ifdef PRINT_WAKE_LOOP
    am_bsp_uart_printf_enable();
    am_util_stdio_printf("\n\nwake: ");
    am_bsp_uart_printf_disable();
#endif
#if RTC_INTERRUPT_ENABLE
    //
    // Set the alarm repeat interval to be every second.
    //
    am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC);

    //
    // Clear the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);

    //
    // Enable the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_enable(AM_HAL_RTC_INT_ALM);

    NVIC_SetPriority(RTC_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(RTC_IRQn);
    //
    // Enable GPIO interrupts to the NVIC.
    //
#endif
#if GPIO_INTERRUPT_ENABLE
    NVIC_SetPriority(GPIO0_001F_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
#endif

    //
    // Enable interrupts to the core.
    //
    am_hal_interrupt_master_enable();

    while (1)
    {
#if GPIO_INTERRUPT_ENABLE
        am_hal_gpio_output_clear(WAKEUP_GPIO_PIN);
#endif
#if RTC_INTERRUPT_ENABLE
        am_hal_gpio_output_clear(WAKEUP_RTC_PIN);
#endif
        //
        // Go to Deep Sleep until wakeup.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
