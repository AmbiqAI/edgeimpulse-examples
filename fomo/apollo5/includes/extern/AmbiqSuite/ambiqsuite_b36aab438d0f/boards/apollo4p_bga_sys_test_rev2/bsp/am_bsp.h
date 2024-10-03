//*****************************************************************************
//
//  am_bsp.h
//! @file
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo4_bga_sys_test_bsp BSP for the Apollo4 Sys Test Board
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
#define APOLLO4P_BGA_SYS_TEST

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

//
// Define MCU_VALIDATION for building tests for the validation group.
//
// Test Result/Progress
#define MCU_VALIDATION_DEBUG_REG    0x4FFFF000 // TB Debug Register

// Test GPIO
// If defined, test will keep toggling the pin to indicate progress
#define MCU_VALIDATION_GPIO        30

extern const am_hal_gpio_pincfg_t g_AM_VALIDATION_GPIO;

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
#define AM_BSP_MSPI_FLASH_MODULE            1

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
extern void am_bsp_low_power_init(void);
extern void am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode);
extern void am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode);
extern void am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);
extern void am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);

extern void am_bsp_sdio_pins_enable(uint8_t ui8BusWidth);
extern void am_bsp_sdio_pins_disable(uint8_t ui8BusWidth);
extern void am_bsp_sdio_reset(void);
extern void am_bsp_sd_cd_pin_enable(bool bEnable);
extern void am_bsp_sd_wp_pin_enable(bool bEnable);

extern void am_bsp_sdif_pins_enable(uint8_t ui8BusWidth);
extern void am_bsp_sdif_pins_disable(uint8_t ui8BusWidth);

extern int32_t am_bsp_debug_printf_enable(void);
extern void am_bsp_debug_printf_disable(void);

extern void am_bsp_itm_string_print(char *pcString);
extern int32_t am_bsp_itm_printf_enable(void);
extern void am_bsp_itm_printf_disable(void);

extern void am_bsp_uart_string_print(char *pcString);

extern int32_t am_bsp_uart_printf_enable(void);
extern void am_bsp_uart_printf_disable(void);

extern int32_t am_bsp_buffered_uart_printf_enable(void);
extern void am_bsp_buffered_uart_printf_disable(void);
extern void am_bsp_buffered_uart_service(void);

// #### INTERNAL BEGIN ####
extern int32_t am_bsp_memory_printf_enable(am_bsp_memory_printf_state_t *psPrintfState);
extern void am_bsp_memory_printf_disable(void);
// #### INTERNAL END ####

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
//! @brief clear display reset pins.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_disp_reset_pins_clear(void);

extern void am_bsp_external_display_device_enable(bool bEnable);
extern void am_bsp_external_accel_device_enable(bool bEnable);

extern void am_bsp_external_vdd18_switch(bool bEnable);

extern void am_bsp_external_vddusb33_switch(bool bEnable);
extern void am_bsp_external_vddusb0p9_switch(bool bEnable);

extern void am_bsp_etm_pins_enable(am_bsp_etm_interface_e etm_interface);
extern void am_bsp_etm_pins_disable(am_bsp_etm_interface_e etm_interface);

extern void am_bsp_rpi_uart_pins_enable(void);
extern void am_bsp_rpi_uart_pins_disable(void);

extern void am_bsp_pdm0_pins_enable(void);
extern void am_bsp_pdm0_pins_disable(void);
extern void am_bsp_pdm1_pins_enable(void);
extern void am_bsp_pdm1_pins_disable(void);
extern void am_bsp_pdm2_pins_enable(void);
extern void am_bsp_pdm2_pins_disable(void);
extern void am_bsp_pdm3_pins_enable(void);
extern void am_bsp_pdm3_pins_disable(void);

extern void am_bsp_i2s0_pins_enable(void);
extern void am_bsp_i2s0_pins_disable(void);
extern void am_bsp_i2s1_pins_enable(void);
extern void am_bsp_i2s1_pins_disable(void);
extern void am_bsp_iom_psram_pins_enable(void);
extern void am_bsp_iom_psram_pins_disable(void);

//*****************************************************************************
//
//! @brief Put the test errors to the TPIU and ITM.
//!
//! @param i32ErrorCount
//!
//! This function outputs the number of test errors to the TPIU/ITM.
//!
//! @return None.
//
//*****************************************************************************
extern void am_bsp_ckerr(int32_t i32ErrorCount);

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
