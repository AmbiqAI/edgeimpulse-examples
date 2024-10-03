//*****************************************************************************
//
//! @file am_hal_pwrctrl.h
//!
//! @brief Functions for enabling and disabling power domains.
//!
//! @addtogroup pwrctrl3 Pwrctrl - Power Control
//! @ingroup apollo3_hal
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

#ifndef AM_HAL_PWRCTRL_H
#define AM_HAL_PWRCTRL_H

//
//! Designate this peripheral.
//
#define AM_APOLLO3_PWRCTRL  1

typedef enum
{
    AM_HAL_PWRCTRL_PERIPH_NONE,
    AM_HAL_PWRCTRL_PERIPH_IOS,
    AM_HAL_PWRCTRL_PERIPH_IOM0,
    AM_HAL_PWRCTRL_PERIPH_IOM1,
    AM_HAL_PWRCTRL_PERIPH_IOM2,
    AM_HAL_PWRCTRL_PERIPH_IOM3,
    AM_HAL_PWRCTRL_PERIPH_IOM4,
    AM_HAL_PWRCTRL_PERIPH_IOM5,
    AM_HAL_PWRCTRL_PERIPH_UART0,
    AM_HAL_PWRCTRL_PERIPH_UART1,
    AM_HAL_PWRCTRL_PERIPH_ADC,
    AM_HAL_PWRCTRL_PERIPH_SCARD,
    AM_HAL_PWRCTRL_PERIPH_MSPI,
    AM_HAL_PWRCTRL_PERIPH_PDM,
    AM_HAL_PWRCTRL_PERIPH_BLEL,
    AM_HAL_PWRCTRL_PERIPH_MAX
} am_hal_pwrctrl_periph_e;

typedef enum
{
    AM_HAL_PWRCTRL_MEM_NONE,
    AM_HAL_PWRCTRL_MEM_SRAM_8K_DTCM,
    AM_HAL_PWRCTRL_MEM_SRAM_32K_DTCM,
    AM_HAL_PWRCTRL_MEM_SRAM_64K_DTCM,
    AM_HAL_PWRCTRL_MEM_SRAM_96K,
    AM_HAL_PWRCTRL_MEM_SRAM_128K,
    AM_HAL_PWRCTRL_MEM_SRAM_160K,
    AM_HAL_PWRCTRL_MEM_SRAM_192K,
    AM_HAL_PWRCTRL_MEM_SRAM_224K,
    AM_HAL_PWRCTRL_MEM_SRAM_256K,
    AM_HAL_PWRCTRL_MEM_SRAM_288K,
    AM_HAL_PWRCTRL_MEM_SRAM_320K,
    AM_HAL_PWRCTRL_MEM_SRAM_352K,
    AM_HAL_PWRCTRL_MEM_SRAM_384K,
    AM_HAL_PWRCTRL_MEM_FLASH_512K,
    AM_HAL_PWRCTRL_MEM_FLASH_1M,
    AM_HAL_PWRCTRL_MEM_CACHE,
    AM_HAL_PWRCTRL_MEM_ALL,
    AM_HAL_PWRCTRL_MEM_MAX
} am_hal_pwrctrl_mem_e;

#define AM_HAL_PWRCTRL_MEM_FLASH_MIN    AM_HAL_PWRCTRL_MEM_FLASH_512K
#define AM_HAL_PWRCTRL_MEM_FLASH_MAX    AM_HAL_PWRCTRL_MEM_FLASH_1M

#define AM_HAL_PWRCTRL_MEM_SRAM_MIN     AM_HAL_PWRCTRL_MEM_SRAM_8K_DTCM
#define AM_HAL_PWRCTRL_MEM_SRAM_MAX     AM_HAL_PWRCTRL_MEM_SRAM_384K

//*****************************************************************************
//
// Macros to check whether Apollo3 bucks are enabled.
//
//*****************************************************************************
#define am_hal_pwrctrl_simobuck_enabled_check()                             \
        (PWRCTRL->SUPPLYSTATUS_b.SIMOBUCKON)

#define am_hal_pwrctrl_blebuck_enabled_check()                              \
        (PWRCTRL->SUPPLYSTATUS_b.BLEBUCKON)


#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Enable power to a peripheral.
//!
//! @param ePeripheral - The peripheral to enable.
//!
//! This function enables power to the peripheral and waits for a
//! confirmation from the hardware.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_periph_enable(am_hal_pwrctrl_periph_e ePeripheral);

//*****************************************************************************
//
//! @brief Disable power to a peripheral.
//!
//! @param ePeripheral - The peripheral to disable.
//!
//! This function disables power to the peripheral and waits for a
//! confirmation from the hardware.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_periph_disable(am_hal_pwrctrl_periph_e ePeripheral);

//*****************************************************************************
//
//! @brief Determine whether a peripheral is currently enabled.
//!
//! @param ePeripheral  - The peripheral to enable.
//! @param pui32Enabled - Pointer to a ui32 that will return as 1 or 0.
//!
//! This function determines to the caller whether a given peripheral is
//! currently enabled or disabled.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_periph_enabled(
                am_hal_pwrctrl_periph_e ePeripheral, uint32_t *pui32Enabled);

//*****************************************************************************
//
//! @brief Enable a configuration of memory.
//!
//! @param eMemConfig - The memory configuration.
//!
//! This function establishes the desired configuration of flash, SRAM, ICache,
//! and DCache (DTCM) according to the desired Memory Configuration mask.
//!
//! @note Only the type of memory specified is affected. Therefore separate calls
//! are required to affect power settings for FLASH, SRAM, or CACHE.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_memory_enable(am_hal_pwrctrl_mem_e eMemConfig);

//*****************************************************************************
//
//! @brief Power down respective memory.
//!
//! @param eMemConfig - The memory power down enum.
//!
//! This function establishes the desired power down of flash, SRAM, ICache,
//! and DCache (DTCM) according to the desired enum.
//!
//! @note Only the type of memory specified is affected. Therefore separate calls
//! are required to affect power settings for FLASH, SRAM, or CACHE.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_memory_deepsleep_powerdown(am_hal_pwrctrl_mem_e eMemConfig);

//*****************************************************************************
//
//! @brief Apply retention voltage to respective memory.
//!
//! @param eMemConfig - The memory power down enum.
//!
//! This function establishes the desired power retain of flash, SRAM, ICache,
//! and DCache (DTCM) according to the desired enum.
//!
//! @note Only the type of memory specified is affected. Therefore separate calls
//! are required to affect power settings for FLASH, SRAM, or CACHE.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_memory_deepsleep_retain(am_hal_pwrctrl_mem_e eMemConfig);

//*****************************************************************************
//
//! @brief Initialize system for low power configuration.
//!
//! This function handles low power initialization.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_low_power_init(void);

//*****************************************************************************
//
//! @brief Initialize BLE Buck Trims for Lowest Power.
//!
//
//*****************************************************************************
extern void am_hal_pwrctrl_blebuck_trim(void);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_PWRCTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
