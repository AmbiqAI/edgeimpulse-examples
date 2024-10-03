//*****************************************************************************
//
//! @file am_bsp.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup Apollo5 Engineering Board BSP Board Support Package (BSP)
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

#define APOLLO510_EVB
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

//*****************************************************************************
//
// Power Mode - if SIMOBUCK should be enabled, or stay in LDO mode
//
//*****************************************************************************
#ifndef AM_BSP_ENABLE_SIMOBUCK
#define AM_BSP_ENABLE_SIMOBUCK   1
#endif

extern const am_hal_gpio_pincfg_t g_AM_VALIDATION_GPIO;

//*****************************************************************************
//
// Definition of External Clock Sources for the board
//
//*****************************************************************************
#define AM_BSP_XTAL_HS_MODE         AM_HAL_CLKMGR_XTAL_HS_MODE_XTAL
#define AM_BSP_XTAL_HS_FREQ_HZ      (48000000U)
#define AM_BSP_XTAL_LS_MODE         AM_HAL_CLKMGR_XTAL_LS_MODE_XTAL
#define AM_BSP_XTAL_LS_FREQ_HZ      (32768U)
#define AM_BSP_EXTREF_CLK_FREQ_HZ   (0U)
//*****************************************************************************
//
// Definition of USBPHY Clock Source for High-Speed mode
//
// The Apollo510 EVB has a 48MHz Crystal, so we will use XTAL_HS / 2 as the
// reference clock. For full-speed, HFRC_24M can be used for lower power.
//
//*****************************************************************************
#define AM_BSP_USBPHY_CLK_SRC       AM_HAL_USB_PHYCLKSRC_XTAL_HS_DIV2
//#define AM_BSP_USBPHY_CLK_SRC       AM_HAL_USB_PHYCLKSRC_HFRC_24M
//*****************************************************************************
//
// The Apollo510 EVB uses external loadswitches to control USB power.
//
//*****************************************************************************
#define ENABLE_EXT_USB_PWR_RAILS


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
#define AM_BSP_NUM_BUTTONS                  2
#if AM_BSP_NUM_BUTTONS
extern am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS];
#endif

//*****************************************************************************
//
// LED definitions.
//
//*****************************************************************************
#define AM_BSP_NUM_LEDS                     3
#if AM_BSP_NUM_LEDS > 0
extern am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS];
#endif

//*****************************************************************************
//
// MSPI FLASH definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_FLASH_MODULE            1

//*****************************************************************************
//
// Touch interface definitions.
//
//*****************************************************************************
#define AM_BSP_TP_IOM_MODULE                0

//*****************************************************************************
//
// MSPI PSRAM definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE   AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0

#define USE_APS51216BA

//*****************************************************************************
//
// MSPI Display definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_CLKOND4(inst)  \
    ((inst == g_sDispCfg.ui32Module)?g_sDispCfg.bClockonD4:false)

//*****************************************************************************
//
// JDI timer pins definitions.
//
//*****************************************************************************
#define AM_BSP_JDI_TIMER_VA               0
#define AM_BSP_JDI_TIMER_VCOM             1
#define AM_BSP_JDI_TIMER_VB               2
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

//*****************************************************************************
//
// SDIO Bus Width definitions
//
//*****************************************************************************
#define AM_BSP_SDIO_INSTANCE    (0)
#define AM_BSP_SDIO_BUS_WIDTH   AM_HAL_HOST_BUS_WIDTH_8

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************


//*****************************************************************************
//
//! @brief Set up the display pins.
//!
//! This function configures reset,te,en,mode selection and clock/data pins
//
//*****************************************************************************
extern void am_bsp_disp_pins_enable(void);

//*****************************************************************************
//
//! @brief Deinit the display pins.
//!
//! This function de-configures reset,te,en,mode selection and clock/data pins
//
//*****************************************************************************
extern void am_bsp_disp_pins_disable(void);

//*****************************************************************************
//
//! @brief get jdi timer pins.
//!
//! This function could get pins number of VA,VCOM,VB
//
//*****************************************************************************
extern uint32_t am_bsp_disp_jdi_timer_pins(uint8_t ui8TimerPin);

//*****************************************************************************
//
//! @brief Set display reset pins.
//
//*****************************************************************************
extern void am_bsp_disp_reset_pins_set(void);

//*****************************************************************************
//
//! @brief Clear display reset pins.
//
//*****************************************************************************
extern void am_bsp_disp_reset_pins_clear(void);

//*****************************************************************************
//
//! @brief Enable the external display adaptor I2C (touch,als) pins
//
//*****************************************************************************
extern uint32_t am_bsp_touch_als_enable(void);

//*****************************************************************************
//
//! @brief Disable the external display adaptor I2C (touch,als) pins
//
//*****************************************************************************
extern uint32_t am_bsp_touch_als_disable(void);

//*****************************************************************************
//
//! @brief Set up the I2S pins based on module.
//!
//! @param ui32Module - I2S module
//! @param bBidirectionalData - Use Bidirectional Data for I2S Module
//
//*****************************************************************************
extern void am_bsp_i2s_pins_enable(uint32_t ui32Module, bool bBidirectionalData);

//*****************************************************************************
//
//! @brief Disable the I2S pins based on module.
//!
//! @param ui32Module - I2S module
//! @param bBidirectionalData - Use Bidirectional Data for I2S Module
//
//*****************************************************************************
extern void am_bsp_i2s_pins_disable(uint32_t ui32Module, bool bBidirectionalData);

//*****************************************************************************
//
//! @brief Set up the PDM pins based on module.
//!
//! @param ui32Module - PDM module
//
//*****************************************************************************
extern void am_bsp_pdm_pins_enable(uint32_t ui32Module);

//*****************************************************************************
//
//! @brief Disable the PDM pins based on module.
//!
//! @param ui32Module - PDM module
//
//*****************************************************************************
extern void am_bsp_pdm_pins_disable(uint32_t ui32Module);

extern void am_bsp_low_power_init(void);
extern void am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode);

//*****************************************************************************
//
//! @brief Disable the IOS pins based on mode and module.
//!
//! @param ui32Module - MSPI module
//! @param ui32IOSMode - IOS mode of the device
//
//*****************************************************************************
extern void am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode);

//*****************************************************************************
//
//! @brief Set up the MSPI pins based on the external flash device type with clock
//! on data pin 4.
//!
//! @param ui32Module - MSPI module
//! @param eMSPIDevice - MSPI device type
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//
//*****************************************************************************
extern void am_bsp_mspi_clkond4_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);

//*****************************************************************************
//
//! @brief Disable the MSPI pins based on the external flash device type with clock
//! on data pin 4.
//!
//! @param ui32Module - MSPI module
//! @param eMSPIDevice - MSPI device type
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//
//*****************************************************************************
extern void am_bsp_mspi_clkond4_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);

//*****************************************************************************
//
//! @brief Set up the MSPI pins based on the external flash device type.
//!
//! @param ui32Module - MSPI module
//! @param eMSPIDevice - MSPI device type
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
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
//
//*****************************************************************************
extern void am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice );

//*****************************************************************************
//
//! @brief Reset SDIO device via GPIO.
//!
//! This function reset SDIO device via GPIO
//
//*****************************************************************************
extern void am_bsp_sdio_reset(uint32_t ui32Module);

//*****************************************************************************
//
//! @brief Set up the SDIF's pins.
//!
//! @param ui8BusWidth - Bus Width of SDIF
//! @param ui8SdioNum - SDIO module number (0 or 1)
//!
//! This function configures SDIf's CMD, CLK, DAT0-7 pins
//
//*****************************************************************************
extern void am_bsp_sdio_pins_enable(uint8_t ui8SdioNum, uint8_t ui8BusWidth);

//*****************************************************************************
//
//! @brief Disable the SDIF's pins.
//!
//! @param ui8BusWidth - Bus Width of SDIF
//! @param ui8SdioNum - SDIO module number (0 or 1)
//!
//! This function deconfigures SDIF's CMD, CLK, DAT0-7 pins
//
//*****************************************************************************
extern void am_bsp_sdio_pins_disable(uint8_t ui8SdioNum, uint8_t ui8BusWidth);

//*****************************************************************************
//
//! @brief Power off eMMC card.
//!
//! @param ui8SdioNum - SDIO module number (0 or 1)
//!
//! This function disable VCCQ and VCC of eMMC
//
//*****************************************************************************
extern void am_bsp_emmc_power_off(uint8_t ui8SdioNum);

//*****************************************************************************
//
//! @brief Power on eMMC card.
//!
//! @param ui8SdioNum - SDIO module number (0 or 1)
//!
//! This function enable VCCQ and VCC of eMMC
//
//*****************************************************************************
extern void am_bsp_emmc_power_on(uint8_t ui8SdioNum);

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
//! @brief  Set up the SD's CD pin.
//!
//! This function configure SD's CD pin when enable or disable sd card detection.
//
//*****************************************************************************
extern void am_bsp_sd_cd_pin_enable(uint8_t ui8SdioNum, bool bEnable);

//*****************************************************************************
//
//! @brief  Set up the SD's WP pin.
//!
//! This function configure SD's WP pin to detect sd card write protection.
//
//*****************************************************************************
extern void am_bsp_sd_wp_pin_enable(uint8_t ui8SdioNum, bool bEnable);

//*****************************************************************************
//
//! @brief Disable the TPIU and ITM for debug printf messages.
//!
//! This function disables TPIU registers for debug printf messages and disables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
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
//! @brief Initialize and configure the UART
//
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_uart_printf_enable(void);

//*****************************************************************************
//
//! @brief Disable the UART
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
//
//*****************************************************************************
extern void am_bsp_uart_string_print(char *pcString);

#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
//! @brief Initialize and configure the UART
//!
//! @param pvHandle - Pointer to UART Handle
//!
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_buffered_uart_printf_enable(void *pvHandle);

//*****************************************************************************
//
//! @brief Disable the UART
//
//*****************************************************************************
extern void am_bsp_buffered_uart_printf_disable(void);

//*****************************************************************************
//
//! @brief Interrupt routine for the buffered UART interface.
//
//*****************************************************************************
extern void am_bsp_buffered_uart_service(void);
#endif // AM_BSP_DISABLE_BUFFERED_UART

// #### INTERNAL BEGIN ####
extern int32_t am_bsp_memory_printf_enable(am_bsp_memory_printf_state_t *psPrintfState);
extern void am_bsp_memory_printf_disable(void);

#if MCU_VALIDATION
extern void am_bsp_ckerr(int32_t i32ErrorCount);
#endif
// #### INTERNAL END ####
//*****************************************************************************
//
//! @brief External power on.
//!
//! This function turns on external power switch
//
//*****************************************************************************
extern void am_bsp_external_pwr_on(void);

//*****************************************************************************
//
//! @brief DSI VDD18 power switch.
//!
//! @param bEnable - Enable or disable the external VDD1.8V switch
//
//*****************************************************************************
extern void am_bsp_external_vdd18_switch(bool bEnable);

//*****************************************************************************
//
//! @brief VDDUSB33 power switch.
//!
//! @param bEnable - Enable or disable the external VDDUSB33 switch
//
//*****************************************************************************
extern void am_bsp_external_vddusb33_switch(bool bEnable);

//*****************************************************************************
//
//! @brief VDDUSB0P9 power switch.
//!
//! @param bEnable - Enable or disable the external VDDUSB0P9 switch
//
//*****************************************************************************
extern void am_bsp_external_vddusb0p9_switch(bool bEnable);

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
//
//*****************************************************************************
#define PINCFG_GET_DEPRECATED 0xDE9CA7ED
#define am_bsp_mspi_pincfg_get(eMSPIDevice, pPinnum, pPincfg) \
            am_bsp_mspi_ce_pincfg_get(PINCFG_GET_DEPRECATED, eMSPIDevice, pPinnum, pPincfg)

//*****************************************************************************
//
//! @brief MSPI CE Pin Config Get
//!
//! @param ui32Module - MSPI module 0/1/2/3
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
//! @brief MSPI Reset Pin Config Get
//!
//! @param ui32Module - MSPI module 0/1/2/3
//! @param eMSPIDevice - MSPI Device Type (HEX, OCTAL, ...)
//! @param pPinnum - MSPI pin number to return to the device driver
//! @param pPincfg - Pin configuration to return to the device driver
//!
//! This function returns the pinnum and pincfg for the Reset of MSPI requested.
//
//*****************************************************************************
extern void am_bsp_mspi_reset_pincfg_get( uint32_t ui32Module,
                                am_hal_mspi_device_e eMSPIDevice,
                                uint32_t * pPinnum,
                                am_hal_gpio_pincfg_t * pPincfg );

//*****************************************************************************
//
//! @brief Put the test errors to the TPIU and ITM.
//!
//! @param i32ErrorCount
//!
//! This function outputs the number of test errors to the TPIU/ITM.
//
//*****************************************************************************

extern void am_bsp_ckerr(int32_t i32ErrorCount);

//*****************************************************************************
//
//! @brief Checks if itm is busy and provides a delay to flush the fifo
//!
//! Return: true  = ITM not busy and no timeout occurred.
//!         false = Timeout occurred.
//
//*****************************************************************************
extern bool am_bsp_debug_itm_printf_flush(void);

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
