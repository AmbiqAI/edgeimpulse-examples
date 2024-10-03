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
    am_hal_gpio_pin_config(3, AM_HAL_PIN_3_PSOURCE);   // BLE_POWER
    am_hal_gpio_pin_config(4, AM_HAL_PIN_INPUT);       // ADXL364_POWER
    am_hal_gpio_pin_config(5, AM_HAL_PIN_INPUT);       // BLE_UART_RTS
    am_hal_gpio_pin_config(6, AM_HAL_PIN_INPUT);       // BLE_UART_CTS
    am_hal_gpio_pin_config(7, AM_HAL_PIN_INPUT);       // BLE_CLOCK
    am_hal_gpio_pin_config(8, AM_HAL_PIN_8_M1SCK);     //
    am_hal_gpio_pin_config(9, AM_HAL_PIN_9_M1MISO);    //
    am_hal_gpio_pin_config(10, AM_HAL_PIN_10_M1MOSI);  //
    am_hal_gpio_pin_config(11, AM_HAL_PIN_INPUT);      // LOAD_RESISTOR
    am_hal_gpio_pin_config(12, AM_HAL_PIN_12_M1nCE0);  // IO_EXPAND_nCE
    am_hal_gpio_pin_config(13, AM_HAL_PIN_INPUT);      // GYRO_INT2
    am_hal_gpio_pin_config(14, AM_HAL_PIN_14_M1nCE2);  // GYRO_nCE
    am_hal_gpio_pin_config(15, AM_HAL_PIN_15_M1nCE3);  // ADXL364_nCE
    am_hal_gpio_pin_config(16, AM_HAL_PIN_INPUT);      // GYRO_INT1
    am_hal_gpio_pin_config(17, AM_HAL_PIN_OUTPUT);     // BLE_RESET
    am_hal_gpio_pin_config(18, AM_HAL_PIN_INPUT);      //
    am_hal_gpio_pin_config(19, AM_HAL_PIN_INPUT);      // ADXL364_INT2
    am_hal_gpio_pin_config(29, AM_HAL_PIN_29_M1nCE6);  // SPIFLASH_nCE
    am_hal_gpio_pin_config(31, AM_HAL_PIN_INPUT);      // ADXL364_INT1
    am_hal_gpio_pin_config(35, AM_HAL_PIN_INPUT);      // COM_UART_TX
    am_hal_gpio_pin_config(36, AM_HAL_PIN_INPUT);      // COM_UART_RX
    am_hal_gpio_pin_config(41, AM_HAL_PIN_41_SWO);     //
}
