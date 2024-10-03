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
#define AM_BSP_GPIO_ADXL362_INT1        34
#define AM_BSP_GPIO_ADXL362_INT2        49
#define AM_BSP_GPIO_ADXL362_nCE         24
#define AM_BSP_GPIO_ADXL364_INT1        12
#define AM_BSP_GPIO_ADXL364_INT2        18
#define AM_BSP_GPIO_ADXL364_nCE         15
#define AM_BSP_GPIO_BLE_RESET           45
#define AM_BSP_GPIO_BLE_UART_CTS        5
#define AM_BSP_GPIO_BLE_UART_RTS        4
#define AM_BSP_GPIO_BLE_UART_RX         1
#define AM_BSP_GPIO_BLE_UART_TX         0
#define AM_BSP_GPIO_BUTTON0             36
#define AM_BSP_GPIO_BUTTON1             35
#define AM_BSP_GPIO_BUTTON2             34
#define AM_BSP_GPIO_BUTTON3             33
#define AM_BSP_GPIO_COM_UART_CTS        38
#define AM_BSP_GPIO_COM_UART_RTS        37
#define AM_BSP_GPIO_COM_UART_RX         36
#define AM_BSP_GPIO_COM_UART_TX         35
#define AM_BSP_GPIO_LED0                49
#define AM_BSP_GPIO_LED1                48
#define AM_BSP_GPIO_LED2                47
#define AM_BSP_GPIO_LED3                46
#define AM_BSP_GPIO_LED4                45
#define AM_BSP_GPIO_LED5                44
#define AM_BSP_GPIO_LED6                43
#define AM_BSP_GPIO_SPIFLASH_nCE        29
#define AM_BSP_GPIO_VBAT_LOAD           11

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
