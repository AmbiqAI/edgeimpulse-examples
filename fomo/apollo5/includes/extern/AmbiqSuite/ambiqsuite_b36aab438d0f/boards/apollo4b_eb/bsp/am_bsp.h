//*****************************************************************************
//
//  am_bsp.h
//! @file
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollob_eb_bsp BSP
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
#include "am_devices_led.h"
#include "am_devices_button.h"
#include "am_devices_display_generic.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined (DISP_CTRL_IP)
//*****************************************************************************
//
// Board level display hardware configurations
//
//*****************************************************************************
extern am_devices_display_hw_config_t g_sDispCfg;
#endif // DISP_CTRL_IP

// #### INTERNAL BEGIN ####
//
// Define MCU_VALIDATION for building tests for the validation group.
//
//#define MCU_VALIDATION  1
// #### INTERNAL END ####
//*****************************************************************************
//
// UART specifics for this board including assigned UART modules.
//
//*****************************************************************************
#define AM_BSP_UART_PRINT_INST  0   // UART COM module

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
#define AM_BSP_NUM_BUTTONS                  0
#if AM_BSP_NUM_BUTTONS
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
// MSPI FLASH definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_FLASH_DEVICE_ATXP032
#define AM_BSP_MSPI_FLASH_MODULE            2

//*****************************************************************************
//
// Touch interface definitions.
//
//*****************************************************************************
#define AM_BSP_TP_IOM_MODULE                 0

//*****************************************************************************
//
// MSPI PSRAM definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE   AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1

//*****************************************************************************
//
// Power Mode - if SIMOBUCK should be enabled, or stay in LDO mode
//
//*****************************************************************************
#ifndef AM_BSP_ENABLE_SIMOBUCK
#define AM_BSP_ENABLE_SIMOBUCK   1
#endif

//*****************************************************************************
//
// MSPI Display definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_CLKOND4(inst)  \
    ((inst == g_sDispCfg.ui32Module)?g_sDispCfg.bClockonD4:false)

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// Memory for a faked printf application
//
//*****************************************************************************
typedef struct
{
    uint8_t *pui8Buffer;
    uint32_t ui32Size;
    uint32_t ui32Index;
} am_bsp_memory_printf_state_t;
// #### INTERNAL END ####

#define SDIO_BUS_WIDTH_1 1
#define SDIO_BUS_WIDTH_4 4
#define SDIO_BUS_WIDTH_8 8

//*****************************************************************************
//
//! ADC pin config definitions
//
//*****************************************************************************
typedef enum
{
#ifdef AM_BSP_GPIO_ADCSE0
    eADCSE0 = AM_BSP_GPIO_ADCSE0,
#endif
#ifdef AM_BSP_GPIO_ADCSE1
    eADCSE1 = AM_BSP_GPIO_ADCSE1,
#endif
#ifdef AM_BSP_GPIO_ADCSE2
    eADCSE2 = AM_BSP_GPIO_ADCSE2,
#endif
#ifdef AM_BSP_GPIO_ADCSE3
    eADCSE3 = AM_BSP_GPIO_ADCSE3,
#endif
#ifdef AM_BSP_GPIO_ADCSE4
    eADCSE4 = AM_BSP_GPIO_ADCSE4,
#endif
#ifdef AM_BSP_GPIO_ADCSE5
    eADCSE5 = AM_BSP_GPIO_ADCSE5,
#endif
#ifdef AM_BSP_GPIO_ADCSE6
    eADCSE6 = AM_BSP_GPIO_ADCSE6,
#endif
#ifdef AM_BSP_GPIO_ADCSE7
    eADCSE7 = AM_BSP_GPIO_ADCSE7,
#endif
    //
    //! force enum to 32 bits, and need one item that is not conditional on #ifdef
    //
    eADC_x32 = 0x7FFFFFFF,
} am_bsp_adp_pins_e;



//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Prepare the MCU for low power operation.
//!
//! This function enables several power-saving features of the MCU, and
//! disables some of the less-frequently used peripherals.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_low_power_init(void);

//*****************************************************************************
//
//! @brief Set up the IOM pins based on mode and module.
//!
//! @param ui32Module - MSPI module
//! @param eIOMMode - IOS mode of the device
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);

//*****************************************************************************
//
//! @brief Disable the IOM pins based on mode and module.
//!
//! @param ui32Module - MSPI module
//! @param eIOMMode - IOS mode of the device
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);

//*****************************************************************************
//
//! @brief Set up the IOS pins based on mode and module.
//!
//! @param ui32Module - MSPI module
//! @param ui32IOSMode - IOS mode of the device
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode);

//*****************************************************************************
//
//! @brief Disable the IOS pins based on mode and module.
//!
//! @param ui32Module - MSPI module
//! @param ui32IOSMode - IOS mode of the device
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode);

//*****************************************************************************
//
//! @brief Set up the MSPI pins based on the external flash device type with clock
//! on data pin 4.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_mspi_clkond4_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);

//*****************************************************************************
//
//! @brief Set up the MSPI pins based on the external flash device type.
//!
//! @param ui32Module - MSPI module
//! @param eMSPIDevice - MSPI device type
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice );

//*****************************************************************************
//
//! @brief Disable the MSPI pins based on the external flash device type.
//!
//! @param ui32Module - MSPI module
//! @param eMSPIDevice - MSPI device type
//!
//! This function deconfigures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice );

//*****************************************************************************
//
//! @brief Set up the SDIF's pins.
//!
//! @param ui8BusWidth - Bus Width of SDIF
//!
//! This function configures SDIf's CMD, CLK, DAT0-7 pins
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_sdio_pins_enable(uint8_t ui8BusWidth);

//*****************************************************************************
//
//! @brief Disable the SDIF's pins.
//!
//! @param ui8BusWidth - Bus Width of SDIF
//!
//! This function deconfigures SDIF's CMD, CLK, DAT0-7 pins
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_sdio_pins_disable(uint8_t ui8BusWidth);

//*****************************************************************************
//
//! @brief Reset SDIO device via GPIO.
//!
//! This function reset SDIO device via GPIO
//
//*****************************************************************************
extern void am_bsp_sdio_reset(void);

//*****************************************************************************
//
//! @brief  Set up the SD's CD pin.
//!
//! This function configure SD's CD pin when enable or disable sd card detection.
//
//*****************************************************************************
extern void am_bsp_sd_cd_pin_enable(bool bEnable);

//*****************************************************************************
//
//! @brief  Set up the SD's WP pin.
//!
//! This function configure SD's WP pin to detect sd card write protection.
//
//*****************************************************************************
extern void am_bsp_sd_wp_pin_enable(bool bEnable);

//*****************************************************************************
//
//! @brief Enable the TPIU and ITM for debug printf messages.
//!
//! This function enables TPIU registers for debug printf messages and enables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
//!
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_debug_printf_enable(void);

//*****************************************************************************
//
//! @brief Disable the TPIU and ITM for debug printf messages.
//!
//! This function disables TPIU registers for debug printf messages and disables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_debug_printf_disable(void);

//*****************************************************************************
//
//! @brief Enable printing over ITM.
//
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_itm_printf_enable(void);

//*****************************************************************************
//
// @brief Disable printing over ITM.
//
//*****************************************************************************
extern void am_bsp_itm_printf_disable(void);

//*****************************************************************************
//
//! Initialize and configure the UART
//
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_uart_printf_enable(void);

//*****************************************************************************
//
//! Disable the UART
//
//*****************************************************************************
extern void am_bsp_uart_printf_disable(void);

//*****************************************************************************
//
//! @brief UART-based string print function.
//!
//! @param pcString - Pointer to character array to print
//!
//! This function is used for printing a string via the UART, which for some
//! MCU devices may be multi-module.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_uart_string_print(char *pcString);

#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
//! Initialize and configure the UART
//!
//! @param pvHandle - Pointer to UART Handle
//!
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_buffered_uart_printf_enable(void);

//*****************************************************************************
//
//! Disable the UART
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_buffered_uart_printf_disable(void);

//*****************************************************************************
//
//! Interrupt routine for the buffered UART interface.
//
//*****************************************************************************
extern void am_bsp_buffered_uart_service(void);
#endif // AM_BSP_DISABLE_BUFFERED_UART

// #### INTERNAL BEGIN ####
extern int32_t am_bsp_memory_printf_enable(am_bsp_memory_printf_state_t *psPrintfState);
extern void am_bsp_memory_printf_disable(void);
// #### INTERNAL END ####

//*****************************************************************************
//
//! @brief disable the display pins.
//!
//! This function de-configures reset,te,en,mode select sometimes with clock/data pins
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_disp_pins_disable(void);

//*****************************************************************************
//
//! @brief Set up the display pins.
//!
//! This function configures reset,te,en,mode select sometimes with clock/data pins
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_disp_pins_enable(void);

//*****************************************************************************
//
//! @brief Set display reset pins.
//!
//! @return None.
//
//*****************************************************************************

extern void am_bsp_disp_reset_pins_set(void);
//*****************************************************************************
//
//! @brief clear display reset pins.
//!
//! @return None.
//
//*****************************************************************************

extern void am_bsp_disp_reset_pins_clear(void);

//*****************************************************************************
//
//! @brief External power on.
//!
//! This function turns on external power switch
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_external_pwr_on(void);

//*****************************************************************************
//
//! @brief DSI VDD18 power switch.
//!
//! @param bEnable - Enable or disable the external VDD1.8V switch
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_external_vdd18_switch(bool bEnable);

//*****************************************************************************
//
//! @brief MSPI Pin Config Get
//!
//! @param eMSPIDevice - MSPI Device Type (HEX, OCTAL, ...)
//! @param pPinnum - MSPI pin number to return to the device driver
//! @param pPincfg - Pin configuration to return to the device driver
//!
//! This function is specific to the am_devices_mspi_psram_aps25616n.c
//! It was written to get the APS256 CE pinout configuration.
//! The more generic function is am_bsp_mspi_ce_pincfg_get
//!
//! @return None.
//
//*****************************************************************************
#define PINCFG_GET_DEPRECATED 0xDE9CA7ED
#define am_bsp_mspi_pincfg_get(eMSPIDevice, pPinnum, pPincfg) \
            am_bsp_mspi_ce_pincfg_get(PINCFG_GET_DEPRECATED, eMSPIDevice, pPinnum, pPincfg)

//*****************************************************************************
//
//! @brief MSPI CE Pin Config Get
//!
//! @param ui32Module - MSPI module 0/1/2
//! @param eMSPIDevice - MSPI Device Type (HEX, OCTAL, ...)
//! @param pPinnum - MSPI pin number to return to the device driver
//! @param pPincfg - Pin configuration to return to the device driver
//!
//! This function returns the pinnum and pincfg for the CE of MSPI requested.
//
//*****************************************************************************
extern void am_bsp_mspi_ce_pincfg_get( uint32_t ui32Module,
                        am_hal_mspi_device_e eMSPIDevice,
                        uint32_t * pPinnum,
                        am_hal_gpio_pincfg_t * pPincfg );

//*****************************************************************************
//
//! @brief configure or de-configure adc pins
//!
//! @param tADCPin   adc ping number
//! @param bPinADCModeEnable enable adc fucntion on pin when true
//!
//! @return standard Hal status code
//
//*****************************************************************************
extern uint32_t am_bsp_adc_pin_config( am_bsp_adp_pins_e tADCPin,
                                       bool bPinADCModeEnable ) ;


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