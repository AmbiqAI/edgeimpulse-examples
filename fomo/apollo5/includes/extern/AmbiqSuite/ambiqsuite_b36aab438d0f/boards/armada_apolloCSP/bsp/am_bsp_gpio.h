//*****************************************************************************
//
//! @file am_bsp_gpio.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_BSP_GPIO_H
#define AM_BSP_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_BSP_GPIO_ADXL364_INT1        31
#define AM_BSP_GPIO_ADXL364_INT2        19
#define AM_BSP_GPIO_ADXL364_POWER       4
#define AM_BSP_GPIO_ADXL364_nCE         15
#define AM_BSP_GPIO_BLE_CLOCK           7
#define AM_BSP_GPIO_BLE_POWER           3
#define AM_BSP_GPIO_BLE_RESET           17
#define AM_BSP_GPIO_BLE_UART_CTS        6
#define AM_BSP_GPIO_BLE_UART_RTS        5
#define AM_BSP_GPIO_BLE_UART_RX         1
#define AM_BSP_GPIO_BLE_UART_TX         0
#define AM_BSP_GPIO_COM_UART_RX         36
#define AM_BSP_GPIO_COM_UART_TX         35
#define AM_BSP_GPIO_GYRO_INT1           16
#define AM_BSP_GPIO_GYRO_INT2           13
#define AM_BSP_GPIO_GYRO_nCE            14
#define AM_BSP_GPIO_IO_EXPAND_nCE       12
#define AM_BSP_GPIO_LOAD_RESISTOR       11
#define AM_BSP_GPIO_SPIFLASH_nCE        29

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_bsp_gpio_init(void);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_GPIO_H
