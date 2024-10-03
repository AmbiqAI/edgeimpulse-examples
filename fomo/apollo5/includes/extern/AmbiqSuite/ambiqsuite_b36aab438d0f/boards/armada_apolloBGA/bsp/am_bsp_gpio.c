//*****************************************************************************
//
//! @file am_bsp_gpio.c
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! Structures and functions to support board-wide GPIO configuration.
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
#include "am_bsp_gpio.h"

//*****************************************************************************
//
// Choose a default state for all GPIO pins.
//
//*****************************************************************************
void
am_bsp_gpio_init(void)
{
    am_hal_gpio_pin_config(0, AM_HAL_PIN_0_UARTTX);    // BLE_UART_TX
    am_hal_gpio_pin_config(1, AM_HAL_PIN_1_UARTRX);    // BLE_UART_RX
    am_hal_gpio_pin_config(2, AM_HAL_PIN_INPUT);       //
    am_hal_gpio_pin_config(3, AM_HAL_PIN_INPUT);       //
    am_hal_gpio_pin_config(4, AM_HAL_PIN_INPUT);       // BLE_UART_RTS
    am_hal_gpio_pin_config(5, AM_HAL_PIN_5_M0SCK);     // BLE_UART_CTS
    am_hal_gpio_pin_config(6, AM_HAL_PIN_6_M0MISO);    //
    am_hal_gpio_pin_config(7, AM_HAL_PIN_7_M0MOSI);    //
    am_hal_gpio_pin_config(8, AM_HAL_PIN_8_M1SCK);     //
    am_hal_gpio_pin_config(9, AM_HAL_PIN_9_M1MISO);    //
    am_hal_gpio_pin_config(10, AM_HAL_PIN_10_M1MOSI);  //
    am_hal_gpio_pin_config(11, AM_HAL_PIN_OUTPUT);     // VBAT_LOAD
    am_hal_gpio_pin_config(12, AM_HAL_PIN_INPUT);      // ADXL364_INT1
    am_hal_gpio_pin_config(13, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(14, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(15, AM_HAL_PIN_INPUT);      // ADXL364_nCE
    am_hal_gpio_pin_config(16, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(17, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(18, AM_HAL_PIN_18_TCTA1);   // ADXL364_INT2
    am_hal_gpio_pin_config(19, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(22, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(23, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(24, AM_HAL_PIN_OUTPUT);     // ADXL362_nCE
    am_hal_gpio_pin_config(25, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(26, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(27, AM_HAL_PIN_OUTPUT);     //
    am_hal_gpio_pin_config(28, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(29, AM_HAL_PIN_OUTPUT);     // SPIFLASH_nCE
    am_hal_gpio_pin_config(30, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(31, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(32, AM_HAL_PIN_32_ADC7);    //
    am_hal_gpio_pin_config(33, AM_HAL_PIN_INPUT);      // BUTTON3
    am_hal_gpio_pin_config(34, AM_HAL_PIN_INPUT);      // BUTTON2 ADXL362_INT1
    am_hal_gpio_pin_config(35, AM_HAL_PIN_INPUT);      // BUTTON1 COM_UART_TX
    am_hal_gpio_pin_config(36, AM_HAL_PIN_INPUT);      // BUTTON0 COM_UART_RX
    am_hal_gpio_pin_config(37, AM_HAL_PIN_INPUT);      // COM_UART_RTS
    am_hal_gpio_pin_config(38, AM_HAL_PIN_INPUT);      // COM_UART_CTS
    am_hal_gpio_pin_config(39, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(40, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(41, AM_HAL_PIN_41_SWO);     //
    am_hal_gpio_pin_config(42, AM_HAL_PIN_OUTPUT);     //
    am_hal_gpio_pin_config(43, AM_HAL_PIN_OUTPUT);     // LED6
    am_hal_gpio_pin_config(44, AM_HAL_PIN_OUTPUT);     // LED5
    am_hal_gpio_pin_config(45, AM_HAL_PIN_OUTPUT);     // LED4 BLE_RESET
    am_hal_gpio_pin_config(46, AM_HAL_PIN_OUTPUT);     // LED3
    am_hal_gpio_pin_config(47, AM_HAL_PIN_OUTPUT);     // LED2
    am_hal_gpio_pin_config(48, AM_HAL_PIN_OUTPUT);     // LED1
    am_hal_gpio_pin_config(49, AM_HAL_PIN_OUTPUT);     // LED0 ADXL362_INT2
}
