//*****************************************************************************
//
//! @file am_bsp.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo1_evb BSP for the Apollo1 APOLLO512-KBR Board Rev 1.0 board
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

#ifndef AM_BSP_H
#define AM_BSP_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_devices.h"
#include "am_bsp_gpio.h"

//*****************************************************************************
//
// PWM_LED peripheral assignments.
//
//*****************************************************************************
//
// LED3 is pin 26
//
#define AM_BSP_GPIO_PWM_LED                 AM_BSP_GPIO_LED3
#define AM_BSP_GPIO_CFG_PWM_LED             AM_HAL_PIN_26_TCTB0

#define AM_BSP_PWM_LED_TIMER                0
#define AM_BSP_PWM_LED_TIMER_SEG            AM_HAL_CTIMER_TIMERB
#define AM_BSP_PWM_LED_TIMER_INT            AM_HAL_CTIMER_INT_TIMERB0

//*****************************************************************************
//
// Button definitions.
//
//*****************************************************************************
#define AM_BSP_NUM_BUTTONS                  3
extern am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS];

//*****************************************************************************
//
// LED definitions.
//
//*****************************************************************************
#define AM_BSP_NUM_LEDS                     5
extern am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS];

//*****************************************************************************
//
// Display CS definitions.
//
//*****************************************************************************
//
// The Apollo IOM SPI interface supports only nCE (active low chip select)
//  signals. For displays that use a SPI interface using active-high CS, the
//  CS signaling must be driven via GPIO. The BSP therefore uses a GPIO to
//  drive it, which will work for any display whether active high or low.
// The nCE0 signal corresponds to GPIO12 (AM_BSP_GPIO_IOM1_DISPLAY_CS) in the
//  pin mapping.
// For the HAL calls that require a CS parameter, we'll give it 0 (for nCE0)
//  via AMBSP_IOM_DISPLAY_NCE, we'll select IOM1 using AM_BSP_IOM_DISPLAY_INST.
//
#define AM_BSP_IOM_DISPLAY_INST         1
#define AM_BSP_IOM_DISPLAY_NCE          0

//*****************************************************************************
//
// UART definitions.
//
//*****************************************************************************
//
// Apollo has a single UART instance.
//
#define AM_BSP_UART_PRINT_INST          0
#define AM_BSP_UART_BTLE_INST           0
#define AM_BSP_UART_BOOTLOADER_INST     0

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_bsp_iom_spi_pins_enable(uint32_t ui32Module);
extern void am_bsp_iom_spi_pins_disable(uint32_t ui32Module);
extern void am_bsp_iom_enable(uint32_t ui32Module);
extern void am_bsp_iom_disable(uint32_t ui32Module);
extern void am_bsp_iom_i2c_pins_enable(uint32_t ui32Module);
extern void am_bsp_iom_i2c_pins_disable(uint32_t ui32Module);
extern void am_bsp_low_power_init(void);
extern void am_bsp_debug_printf_enable(void);
extern void am_bsp_debug_printf_disable(void);
extern void am_bsp_itm_string_print(char *pcString);
extern void am_bsp_uart_string_print(char *pcString);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
