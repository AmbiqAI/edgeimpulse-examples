//*****************************************************************************
//
//! @file am_bsp.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo2_evb_bsp BSP for the Apollo2 AMAPH1KK-KBR EVB Rev 1.1 board
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
// PWM_LED peripheral assignments.
//
//*****************************************************************************
//
// LED3 is pin 30
//
#define AM_BSP_GPIO_PWM_LED                 AM_BSP_GPIO_LED3
#define AM_BSP_GPIO_CFG_PWM_LED             AM_HAL_PIN_30_TCTB2

#define AM_BSP_PWM_LED_TIMER                2
#define AM_BSP_PWM_LED_TIMER_SEG            AM_HAL_CTIMER_TIMERB
#define AM_BSP_PWM_LED_TIMER_INT            AM_HAL_CTIMER_INT_TIMERB2

//*****************************************************************************
//
// UART definitions.
//
//*****************************************************************************
#define AM_BSP_UART_PRINT_INST              0
#define AM_BSP_UART_BOOTLOADER_INST         0

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
extern void am_bsp_uart_power_on_restore(uint32_t ui32Module);
extern void am_bsp_uart_power_off_save(uint32_t ui32Module);
extern void am_bsp_iom_power_on_restore(uint32_t ui32Module);
extern void am_bsp_iom_power_off_save(uint32_t ui32Module);
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
