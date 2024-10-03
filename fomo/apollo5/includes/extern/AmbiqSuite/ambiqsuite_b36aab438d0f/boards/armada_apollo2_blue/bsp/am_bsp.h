//*****************************************************************************
//
//! @file am_bsp.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo2_evk_btle_bsp BSP for the Apollo2 EVK BTLE board
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

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Configure as a validation board
//
//*****************************************************************************
#define MCU_VALIDATION  1   // Force as validation

//*****************************************************************************
//
// EM9304 Peripheral Assignment
//
//*****************************************************************************
#define AM_BSP_EM9304_IOM                   5

//*****************************************************************************
//
// BMI160 peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_BMI160_IOM                   1
#define AM_BSP_BMI160_CS                    7

//*****************************************************************************
//
// ADXL362 peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_ADXL362_IOM                  1
#define AM_BSP_ADXL362_CS                   0
#define AM_BSP_ADXL362_TIMER                1
#define AM_BSP_ADXL362_TIMER_SEG            AM_HAL_CTIMER_TIMERB

//*****************************************************************************
//
// FLASH peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_FLASH_IOM                    1
#define AM_BSP_FLASH_CS                     6

//*****************************************************************************
//
// L3GD20H peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_L3GD20H_IOM                  1
#define AM_BSP_L3GD20H_CS                   2

//*****************************************************************************
//
// LIS3MDL peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_LIS3MDL_IOM                  1
#define AM_BSP_LIS3MDL_CS                   5

//*****************************************************************************
//
// BMI160 peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_BMI160_IOM                   1
#define AM_BSP_BMI160_CS                    7

//*****************************************************************************
//
// LIS2DH12 peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_LIS2DH12_IOM                 1
#define AM_BSP_LIS2DH12_CS                  1

//*****************************************************************************
//
// RTC peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_RTC_IOM                      1
#define AM_BSP_RTC_CS                       4

//*****************************************************************************
//
// PWM_LED peripheral assignments.
//
//*****************************************************************************
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
#define AM_BSP_NUM_LEDS                     4
extern am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS];

//*****************************************************************************
//
// UART definitions.
//
//*****************************************************************************
//
// Apollo2 has two UART instances.
//
#define AM_BSP_UART_BTLE_INST               0
#define AM_BSP_UART_PRINT_INST              1
#define AM_BSP_UART_BOOTLOADER_INST         1

//*****************************************************************************
//
// Structure for containing information about the UART configuration while
// it is powered down.
//
//*****************************************************************************
typedef struct
{
    bool     bSaved;
    uint32_t ui32TxPinNum;
    uint32_t ui32TxPinCfg;
}
am_bsp_uart_pwrsave_t;

//*****************************************************************************
//
// Structure for containing information about the IOM configuration while
// it is powered down.
// Each IOM can select up to 8 SPI devices.
//
//*****************************************************************************
typedef struct
{
    bool     bSaved[8];
    uint32_t ui32CsPinNum[8];
    uint32_t ui32CsPinCfg[8];
}
am_bsp_iom_pwrsave_t;

//*****************************************************************************
//
// External data definitions.
//
//*****************************************************************************
extern am_bsp_uart_pwrsave_t am_bsp_uart_pwrsave[AM_REG_UART_NUM_MODULES];
extern am_bsp_iom_pwrsave_t  am_bsp_iom_pwrsave[AM_REG_IOMSTR_NUM_MODULES];

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_bsp_iom_spi_pins_enable(uint32_t ui32Module);
extern void am_bsp_iom_spi_pins_disable(uint32_t ui32Module);
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
