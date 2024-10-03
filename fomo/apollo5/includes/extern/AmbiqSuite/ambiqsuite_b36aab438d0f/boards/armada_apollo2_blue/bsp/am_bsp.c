//*****************************************************************************
//
//! @file am_bsp.c
//!
//! @brief Top level functions for performing board initialization.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo2_evk_btle BSP for the Apollo2 EVK BTLE board
//! @ingroup BSP
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_bsp.h"

#if 0
//*****************************************************************************
//
// LEDs
//
//*****************************************************************************
am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS] =
{
    {AM_BSP_GPIO_LED0, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED1, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED2, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED3, AM_DEVICES_LED_ON_LOW},
};
#endif

//*****************************************************************************
//
// Buttons.
//
//*****************************************************************************
am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS] =
{
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON0, AM_DEVICES_BUTTON_NORMAL_HIGH),
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON1, AM_DEVICES_BUTTON_NORMAL_HIGH),
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON2, AM_DEVICES_BUTTON_NORMAL_HIGH),
};
//*****************************************************************************
//
// Power tracking structures for IOM and UART
//
//*****************************************************************************
am_bsp_uart_pwrsave_t am_bsp_uart_pwrsave[AM_REG_UART_NUM_MODULES];
am_bsp_iom_pwrsave_t  am_bsp_iom_pwrsave[AM_REG_IOMSTR_NUM_MODULES];

//*****************************************************************************
//
//! @brief Enable the necessary pins for the given IOM module.
//!
//! @param ui32Module is the module number of the IOM interface to be used.
//!
//! Uses BSP pin definitions to configure the correct interface for the
//! selected IOM module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_spi_pins_enable(uint32_t ui32Module)
{
    switch(ui32Module)
    {
#ifdef AM_BSP_GPIO_IOM0_SCK
        case 0:
            am_bsp_pin_enable(IOM0_SCK);
            am_bsp_pin_enable(IOM0_MISO);
            am_bsp_pin_enable(IOM0_MOSI);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM1_SCK
        case 1:
            am_bsp_pin_enable(IOM1_SCK);
            am_bsp_pin_enable(IOM1_MISO);
            am_bsp_pin_enable(IOM1_MOSI);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM2_SCK
        case 2:
            am_bsp_pin_enable(IOM2_SCK);
            am_bsp_pin_enable(IOM2_MISO);
            am_bsp_pin_enable(IOM2_MOSI);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM5_SCK
        case 5:
            am_bsp_pin_enable(IOM5_SCK);
            am_bsp_pin_enable(IOM5_MISO);
            am_bsp_pin_enable(IOM5_MOSI);
            break;
#endif
        //
        // If we get here, the caller's selected IOM interface couldn't be
        // found in the BSP GPIO definitions. Halt program execution for
        // debugging.
        //
        default:
            while(1);
    }
}

//*****************************************************************************
//
//! @brief Disable the necessary pins for the given IOM module.
//!
//! @param ui32Module is the module number of the IOM interface to be used.
//!
//! Uses BSP pin definitions to configure the correct interface for the
//! selected IOM module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_spi_pins_disable(uint32_t ui32Module)
{
    switch(ui32Module)
    {
#ifdef AM_BSP_GPIO_IOM0_SCK
        case 0:
            am_bsp_pin_disable(IOM0_SCK);
            am_bsp_pin_disable(IOM0_MISO);
            am_bsp_pin_disable(IOM0_MOSI);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM1_SCK
        case 1:
            am_bsp_pin_disable(IOM1_SCK);
            am_bsp_pin_disable(IOM1_MISO);
            am_bsp_pin_disable(IOM1_MOSI);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM2_SCK
        case 2:
            am_bsp_pin_disable(IOM2_SCK);
            am_bsp_pin_disable(IOM2_MISO);
            am_bsp_pin_disable(IOM2_MOSI);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM5_SCK
        case 5:
            am_bsp_pin_disable(IOM5_SCK);
            am_bsp_pin_disable(IOM5_MISO);
            am_bsp_pin_disable(IOM5_MOSI);
            break;
#endif
        //
        // If we get here, the caller's selected IOM interface couldn't be
        // found in the BSP GPIO definitions. Halt program execution for
        // debugging.
        //
        default:
            while(1);
    }
}

//*****************************************************************************
//
//! @brief Enable the necessary pins for the given IOM module in I2C mode.
//!
//! @param ui32Module is the module number of the IOM interface to be used.
//!
//! Uses BSP pin definitions to configure the correct interface for the
//! selected IOM module to work in I2C mode.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_i2c_pins_enable(uint32_t ui32Module)
{
    switch(ui32Module)
    {
#ifdef AM_BSP_GPIO_IOM0_SCL
        case 0:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM0_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM0_SDA);

            am_bsp_pin_enable(IOM0_SDA);
            am_bsp_pin_enable(IOM0_SCL);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM1_SCL
        case 1:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM1_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM1_SDA);

            am_bsp_pin_enable(IOM1_SDA);
            am_bsp_pin_enable(IOM1_SCL);
            break;
#endif
        //
        // If we get here, the caller's selected IOM interface couldn't be
        // found in the BSP GPIO definitions. Halt program execution for
        // debugging.
        //
        default:
            while(1);
    }
}

//*****************************************************************************
//
//! @brief Disable the necessary pins for the given IOM module in I2C mode.
//!
//! @param ui32Module is the module number of the IOM interface to be used.
//!
//! Uses BSP pin definitions to configure the correct interface for the
//! selected IOM module to work in I2C mode.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_i2c_pins_disable(uint32_t ui32Module)
{
    switch(ui32Module)
    {
#ifdef AM_BSP_GPIO_IOM0_SCL
        case 0:
            am_bsp_pin_disable(IOM0_SDA);
            am_bsp_pin_disable(IOM0_SCL);
            break;
#endif

#ifdef AM_BSP_GPIO_IOM1_SCL
        case 1:
            am_bsp_pin_disable(IOM1_SDA);
            am_bsp_pin_disable(IOM1_SCL);
            break;
#endif
        //
        // If we get here, the caller's selected IOM interface couldn't be
        // found in the BSP GPIO definitions. Halt program execution for
        // debugging.
        //
        default:
            while(1);
    }
}

//*****************************************************************************
//
//! @brief Prepare the MCU for low power operation.
//!
//! This function enables several power-saving features of the MCU, and
//! disables some of the less-frequently used peripherals. It also sets the
//! system clock to 24 MHz.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_low_power_init(void)
{
    //
    // Enable internal buck converters.
    //
    am_hal_pwrctrl_bucks_enable();

    //
    // Initialize for low power in the power control block
    //
    am_hal_pwrctrl_low_power_init();

    //
    // Turn off the voltage comparator as this is enabled on reset.
    //
    am_hal_vcomp_disable();

    //
    // Run the RTC off the LFRC.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);

#if 0 // !MCU_VALIDATION Don't stop the XTAL for Armada testing
    //
    // Stop the XTAL.
    //
    am_hal_clkgen_osc_stop(AM_HAL_CLKGEN_OSC_XT);
#endif

    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();
}

//*****************************************************************************
//
//! @brief Enable the TPIU and ITM for debug printf messages.
//!
//! This function enables TPIU registers for debug printf messages and enables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_debug_printf_enable(void)
{
    am_hal_tpiu_config_t TPIUcfg;

    if ( g_ui32HALflags & AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_M )
    {
        return;
    }

    //
    // Write to the ITM control and status register.
    //
    AM_REGVAL(AM_REG_ITM_TCR_O) =
        AM_WRITE_SM(AM_REG_ITM_TCR_ATB_ID, 0x15)      |
        AM_WRITE_SM(AM_REG_ITM_TCR_TS_FREQ, 1)        |
        AM_WRITE_SM(AM_REG_ITM_TCR_TS_PRESCALE, 1)    |
        AM_WRITE_SM(AM_REG_ITM_TCR_SWV_ENABLE, 1)     |
        AM_WRITE_SM(AM_REG_ITM_TCR_DWT_ENABLE, 0)     |
        AM_WRITE_SM(AM_REG_ITM_TCR_SYNC_ENABLE, 0)    |
        AM_WRITE_SM(AM_REG_ITM_TCR_TS_ENABLE, 0)      |
        AM_WRITE_SM(AM_REG_ITM_TCR_ITM_ENABLE, 1);

    //
    // Enable the ITM and TPIU
    //
#if MCU_VALIDATION
    TPIUcfg.ui32SetItmBaud = AM_HAL_TPIU_BAUD_115200;
#else
    TPIUcfg.ui32SetItmBaud = AM_HAL_TPIU_BAUD_1M;
#endif
    am_hal_tpiu_enable(&TPIUcfg);
    am_bsp_pin_enable(ITM_SWO);

#if MCU_VALIDATION
    //
    // Enable HFRC Adjust, which requires the XTAL.
    // For safe operation, a 2 second delay is required AFTER enabling the XTAL.
    // We'll also select the XTAL for the RTC oscillator.
    //
//  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);
//  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL, 0);
    AM_BFW(CLKGEN, OCTRL, STOPXT, 0);
    AM_BFW(CLKGEN, OCTRL, OSEL, 0);     // RTC osc: 1=LFRC, 0=XTAL

    //
    // Set HFRC Adjust
    //
//  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE, 0);
    AM_REG(CLKGEN, HFADJ) =
        AM_REG_CLKGEN_HFADJ_HFADJ_GAIN_Gain_of_1        |   /* Default value (Apollo2) */
        AM_REG_CLKGEN_HFADJ_HFWARMUP_1SEC               |   /* Default value */
        AM_REG_CLKGEN_HFADJ_HFXTADJ(0x5B8)              |   /* Default value */
        AM_REG_CLKGEN_HFADJ_HFADJCK_4SEC                |   /* Default value */
        AM_REG_CLKGEN_HFADJ_HFADJEN_EN;

    //
    // For safe operation, a 2 second delay is required AFTER enabling the XTAL.
    //
    am_hal_flash_delay(FLASH_CYCLES_US(2000000));   // Give XTAL time to warm up

    //
    // Write to LeMans
    //
    am_hal_itm_stimulus_reg_word_write(0, 0xf8f8f8f8); // tell lemans to pay attention
#endif // MCU_VALIDATION

    am_hal_itm_stimulus_reg_word_write(0, 0xf8f8f8f8); // tell lemans to pay attention
}

//*****************************************************************************
//
//! @brief Enable the TPIU and ITM for debug printf messages.
//!
//! This function disables TPIU registers for debug printf messages and
//! enables ITM GPIO pin to GPIO mode and prepares the MCU to go to deep sleep.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_debug_printf_disable(void)
{
    if ( g_ui32HALflags & AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_M )
    {
        return;
    }

    //
    // Disable the TPIU
    //
    am_hal_itm_not_busy();
    am_hal_gpio_pin_config(AM_BSP_GPIO_ITM_SWO, AM_HAL_GPIO_OUTPUT);
    am_hal_tpiu_disable();
}

//*****************************************************************************
//
//! @brief ITM-based string print function.
//!
//! This function is used for printing a string via the ITM.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_itm_string_print(char *pcString)
{
    am_hal_itm_print(pcString);
}

//*****************************************************************************
//
//! @brief UART-based string print function.
//!
//! This function is used for printing a string via the UART, which for some
//! MCU devices may be multi-module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_uart_string_print(char *pcString)
{
    am_hal_uart_string_transmit_polled(AM_BSP_UART_PRINT_INST, pcString);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
