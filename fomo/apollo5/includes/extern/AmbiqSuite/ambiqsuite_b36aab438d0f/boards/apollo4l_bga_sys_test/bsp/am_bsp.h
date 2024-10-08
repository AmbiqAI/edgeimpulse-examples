//*****************************************************************************
//
//  am_bsp.h
//! @file
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo4p_bga_sys_test BSP for the Apollo4p Sys Test Board
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
#define AM_BSP_UART_PRINT_INST  3   // UART COM module

//*****************************************************************************
//
// Button definitions.
//
//*****************************************************************************
#define AM_BSP_NUM_BUTTONS              2
#if AM_BSP_NUM_BUTTONS
extern am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS];
#endif

#define AM_BSP_NUM_RPI_BUTTONS          2
#if AM_BSP_RPI_NUM_BUTTONS
extern am_devices_button_t am_bsp_rpi_psButtons[AM_BSP_RPI_NUM_BUTTONS];
#endif

//*****************************************************************************
//
// MSPI FLASH definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_FLASH_MODULE        1

//*****************************************************************************
//
// Touch interface definitions.
//
//*****************************************************************************
#define AM_BSP_TP_IOM_MODULE             0

//*****************************************************************************
//
// FIFO Master interface definitions.
//
//*****************************************************************************
#define AM_BSP_FIFO_IOM_MODULE               1

//*****************************************************************************
//
// MSPI PSRAM definitions.
//
//*****************************************************************************
#define AM_BSP_MSPI_PSRAM_DEVICE_APS25616N
#define AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE   AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0

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

//*****************************************************************************
//
// SDIO Bus Width definitions
//
//*****************************************************************************
#define AM_BSP_SDIO_BUS_WIDTH   AM_HAL_HOST_BUS_WIDTH_8
//*****************************************************************************
//
// ETM Interafaces.
//
//*****************************************************************************
typedef enum
{
    AM_BSP_GRP0_ETM,
    AM_BSP_GRP1_ETM,
}
am_bsp_etm_interface_e;


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
extern void am_bsp_external_pwr_on(void);
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
extern void am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);
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
extern void am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);
//*****************************************************************************
//
//! @brief Set up the display pins.
//!
//! This function configures reset,te,en,mode selection and clock/data pins
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_disp_pins_enable(void);
//*****************************************************************************
//
//! @brief Deinit the display pins.
//!
//! This function de-configures reset,te,en,mode selection and clock/data pins
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_disp_pins_disable(void);
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
//! @brief Reset display reset pins.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_disp_reset_pins_clear(void);
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

extern void am_bsp_sdif_pins_enable(uint8_t ui8BusWidth);
extern void am_bsp_sdif_pins_disable(uint8_t ui8BusWidth);
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

extern void am_bsp_itm_string_print(char *pcString);
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
//! @brief Disable printing over ITM.
//
//*****************************************************************************
extern void am_bsp_itm_printf_disable(void);
//*****************************************************************************
//
//! @brief UART-based string print function.
//!
//! @param pcString - Pointer to chacter array to print
//!
//! This function is used for printing a string via the UART, which for some
//! MCU devices may be multi-module.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_uart_string_print(char *pcString);
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
//! @return None.
//
//*****************************************************************************
extern void am_bsp_uart_printf_disable(void);
//*****************************************************************************
//
//! @brief Initialize and configure the UART
//
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_buffered_uart_printf_enable(void);
//*****************************************************************************
//
//! @brief Disable the UART
//
//! @return None.
//
//*****************************************************************************
extern void am_bsp_buffered_uart_printf_disable(void);
//*****************************************************************************
//
// Interrupt routine for the buffered UART interface.
//
//*****************************************************************************
extern void am_bsp_buffered_uart_service(void);

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
//! @brief Enable print to memory
//!
//! @param psPrintfState - Pointer to printf state
//!
//! @return 0 on success.
//
//*****************************************************************************
extern int32_t am_bsp_memory_printf_enable(am_bsp_memory_printf_state_t *psPrintfState);
//*****************************************************************************
//
//! @brief Disable print to memory
//
//! @return None.
//
//*****************************************************************************
extern void am_bsp_memory_printf_disable(void);
// #### INTERNAL END ####
//*****************************************************************************
//
//! @brief Display device enable
//!
//! @param bEnable - enable or disable display.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_external_display_device_enable(bool bEnable);
//*****************************************************************************
//
//! @brief Accelerometer device enable
//!
//! @param bEnable - enable or disable accelemeter.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_external_accel_device_enable(bool bEnable);
//*****************************************************************************
//
//! @brief Enable the ETM pins.
//!
//! This function configures 6-pins for ETM Trace Signals
//!     If the user passes AM_BSP_PDM_ETM it will use GPIO50-55
//!     If the user passes AM_BSP_IOS_ETM it will use GPIO0-4
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_etm_pins_enable(am_bsp_etm_interface_e etm_interface);
//*****************************************************************************
//
//! @brief Disable the ETM pins.
//!
//! This function disbales the 6-pins for ETM Trace Signals
//!     If the user passes AM_BSP_GRP0_ETM it will use GPIO50-55
//!     If the user passes AM_BSP_IOS_ETM it will use GPIO0-4
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_etm_pins_disable(am_bsp_etm_interface_e etm_interface);
//*****************************************************************************
//
//! @brief Enable the RPi UART pins.
//!
//! This function configures 2-pins for RPi UART  Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_rpi_uart_pins_enable(void);
//*****************************************************************************
//
//! @brief Disable the RPi UART pins.
//!
//! This function disbales 2-pins for RPi UART  Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_rpi_uart_pins_disable(void);
//*****************************************************************************
//
//! @brief Enable the PDM0 pins.
//!
//! This function configures 2-pins for PDM0 Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_pdm0_pins_enable(void);
//*****************************************************************************
//
//! @brief Disable the PDM0 pins.
//!
//! This function de-configures 2-pins for PDM0 Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_pdm0_pins_disable(void);
//*****************************************************************************
//
//! @brief Enable the I2S0 pins.
//!
//! This function configures 3-pins for I2S0 Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_i2s0_pins_enable(void);
//*****************************************************************************
//
//! @brief Disable the I2S0 pins.
//!
//! This function disable 3-pins for I2S0 Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_i2s0_pins_disable(void);
//*****************************************************************************
//
//! @brief Enable the IOM PSRAM pins.
//!
//! This function configures 4-pins for SPI Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_iom_psram_pins_enable(void);
//*****************************************************************************
//
//! @brief Disable the IOM PSRAM pins.
//!
//! This function disable 4-pins for SPI Signals
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_iom_psram_pins_disable(void);

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
