//*****************************************************************************
//
//  am_bsp.h
//! @file
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo3_fpga_bsp BSP for the Apollo3 Hotshot FPGA
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
#include "am_bsp_pins.h"

//
// Make individual includes to not require full port before usage.
//#include "am_devices.h"
//
#include "am_devices_led.h"
#include "am_devices_button.h"

#ifdef __cplusplus
extern "C"
{
#endif

// #### INTERNAL BEGIN ####
//
// Define MCU_VALIDATION for building tests for the validation group.
//
//#define MCU_VALIDATION  1
// #### INTERNAL END ####
//*****************************************************************************
//
// Print interface type
//
//*****************************************************************************
#define AM_BSP_PRINT_INFC_NONE              0
#define AM_BSP_PRINT_INFC_SWO               1
#define AM_BSP_PRINT_INFC_UART0             2
#define AM_BSP_PRINT_INFC_BUFFERED_UART0    3

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
#if 1   // Engineering board has no native buttons.
#undef  AM_BSP_NUM_BUTTONS
#else
#define AM_BSP_NUM_BUTTONS                  2
extern am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS];
#endif

//*****************************************************************************
//
// LED definitions.
//
//*****************************************************************************
#if 1   // FPGAs and EBs generally have no native LEDs.
#undef  AM_BSP_NUM_LEDS
#else
#define AM_BSP_NUM_LEDS                     4
extern am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS];
#endif

//*****************************************************************************
//
// UART definitions.
//
//*****************************************************************************
//
// Apollo3 has two UART instances.
// AM_BSP_UART_PRINT_INST should correspond to COM_UART.
//
#define AM_BSP_UART_IOS_INST                0
#define AM_BSP_UART_PRINT_INST              0
#define AM_BSP_UART_BOOTLOADER_INST         1

//*****************************************************************************
//
//! Structure containing UART configuration information while it is powered down.
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
// MSPI definitions.
//
//*****************************************************************************
//
//
#define AM_BSP_MSPI_PSRAM_INST              0
#define AM_BSP_MSPI_DISPLAY_INST            1

//*****************************************************************************
//
// External data definitions.
//
//*****************************************************************************
extern am_bsp_uart_pwrsave_t am_bsp_uart_pwrsave[AM_REG_UART_NUM_MODULES];

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_bsp_low_power_init(void);
extern void am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);
extern void am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);

extern void am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode);
extern void am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode);

extern void am_bsp_debug_printf_enable(void);
extern void am_bsp_debug_printf_disable(void);

extern void am_bsp_itm_string_print(char *pcString);
extern void am_bsp_itm_printf_enable(void);
extern void am_bsp_itm_printf_disable(void);

extern void am_bsp_uart_string_print(char *pcString);
extern void am_bsp_uart_printf_enable(void);
extern void am_bsp_uart_printf_disable(void);

extern void am_bsp_buffered_uart_printf_enable(void);
extern void am_bsp_buffered_uart_service(void);

extern uint32_t am_bsp_com_uart_transfer(const am_hal_uart_transfer_t *psTransfer);

// #### INTERNAL BEGIN ####
extern void am_bsp_internal_bucks_enable(void);
#if 1//MCU_VALIDATION  // TODO: Should this be MCU_VALIDATION?
extern void am_bsp_ckerr(int32_t i32ErrorCount);
#endif
#if 0
extern void am_bsp_uart_power_on_restore(uint32_t ui32Module);
extern void am_bsp_uart_power_off_save(uint32_t ui32Module);
#endif
// #### INTERNAL END ####
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