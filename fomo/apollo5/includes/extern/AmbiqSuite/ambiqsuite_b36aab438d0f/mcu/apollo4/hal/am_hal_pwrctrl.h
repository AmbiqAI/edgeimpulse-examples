//*****************************************************************************
//
//  am_hal_pwrctrl.h
//! @file
//!
//! @brief Functions for enabling and disabling power domains.
//!
//! @addtogroup pwrctrl4 Power Control
//! @ingroup apollo4hal
//! @{

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

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Performace mode enums.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PWRCTRL_MCU_MODE_ULTRA_LOW_POWER,  // 24 MHz
    AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER,        // 96 MHz
    AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE, // 192 MHz
} am_hal_pwrctrl_mcu_mode_e;

typedef enum
{
    AM_HAL_PWRCTRL_DSP_MODE_ULTRA_LOW_POWER,  // 48 MHz
    AM_HAL_PWRCTRL_DSP_MODE_LOW_POWER,        // 192 MHz
    AM_HAL_PWRCTRL_DSP_MODE_HIGH_PERFORMANCE, // 384 MHz
} am_hal_pwrctrl_dsp_mode_e;

//*****************************************************************************
//
// Peripheral power enums.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PWRCTRL_PERIPH_IOS,
    AM_HAL_PWRCTRL_PERIPH_IOM0,
    AM_HAL_PWRCTRL_PERIPH_IOM1,
    AM_HAL_PWRCTRL_PERIPH_IOM2,
    AM_HAL_PWRCTRL_PERIPH_IOM3,
    AM_HAL_PWRCTRL_PERIPH_IOM4,
    AM_HAL_PWRCTRL_PERIPH_IOM5,
    AM_HAL_PWRCTRL_PERIPH_IOM6,
    AM_HAL_PWRCTRL_PERIPH_IOM7,
    AM_HAL_PWRCTRL_PERIPH_UART0,
    AM_HAL_PWRCTRL_PERIPH_UART1,
    AM_HAL_PWRCTRL_PERIPH_UART2,
    AM_HAL_PWRCTRL_PERIPH_UART3,
    AM_HAL_PWRCTRL_PERIPH_ADC,
    AM_HAL_PWRCTRL_PERIPH_MSPI0,
    AM_HAL_PWRCTRL_PERIPH_MSPI1,
    AM_HAL_PWRCTRL_PERIPH_MSPI2,
    AM_HAL_PWRCTRL_PERIPH_GFX,
    AM_HAL_PWRCTRL_PERIPH_DISP,
    AM_HAL_PWRCTRL_PERIPH_DISPPHY,
    AM_HAL_PWRCTRL_PERIPH_CRYPTO,
    AM_HAL_PWRCTRL_PERIPH_SDIO,
    AM_HAL_PWRCTRL_PERIPH_USB,
    AM_HAL_PWRCTRL_PERIPH_USBPHY,
    AM_HAL_PWRCTRL_PERIPH_DEBUG,
    AM_HAL_PWRCTRL_PERIPH_I3C0,
    AM_HAL_PWRCTRL_PERIPH_I3C1,
    AM_HAL_PWRCTRL_PERIPH_AUDREC,
    AM_HAL_PWRCTRL_PERIPH_AUDPB,
    AM_HAL_PWRCTRL_PERIPH_PDM0,
    AM_HAL_PWRCTRL_PERIPH_PDM1,
    AM_HAL_PWRCTRL_PERIPH_PDM2,
    AM_HAL_PWRCTRL_PERIPH_PDM3,
    AM_HAL_PWRCTRL_PERIPH_I2S0,
    AM_HAL_PWRCTRL_PERIPH_I2S1,
    AM_HAL_PWRCTRL_PERIPH_I2S2,
    AM_HAL_PWRCTRL_PERIPH_I2S3,
    AM_HAL_PWRCTRL_PERIPH_AUDADC,
    AM_HAL_PWRCTRL_PERIPH_DSP,
    AM_HAL_PWRCTRL_PERIPH_MAX
} am_hal_pwrctrl_periph_e;

//*****************************************************************************
//
// MCU memory control settings.
//
//*****************************************************************************

//
// Cache enable settings.
//
typedef enum
{
    AM_HAL_PWRCTRL_CACHE_NONE,
    AM_HAL_PWRCTRL_CACHEB0_ONLY,
    AM_HAL_PWRCTRL_CACHE_ALL,
} am_hal_pwrctrl_cache_select_e;

//
// DTCM enable settings.
//
typedef enum
{
    AM_HAL_PWRCTRL_DTCM_NONE    = PWRCTRL_MEMPWREN_PWRENDTCM_NONE,
    AM_HAL_PWRCTRL_DTCM_8K      = PWRCTRL_MEMPWREN_PWRENDTCM_TCM8K,
    AM_HAL_PWRCTRL_DTCM_128K     = PWRCTRL_MEMPWREN_PWRENDTCM_TCM128K,
    AM_HAL_PWRCTRL_DTCM_384K    = PWRCTRL_MEMPWREN_PWRENDTCM_TCM384K,
} am_hal_pwrctrl_dtcm_select_e;

//
// MCU memory configuration structure.
//
typedef struct
{
    //
    // Cache configuration.
    //
    am_hal_pwrctrl_cache_select_e       eCacheCfg;
    bool                                bRetainCache;

    //
    // DTCM configuration
    //
    am_hal_pwrctrl_dtcm_select_e        eDTCMCfg;
    am_hal_pwrctrl_dtcm_select_e        eRetainDTCM;

    //
    // NVM configuration.
    //
    bool                                bEnableNVM0;
    bool                                bRetainNVM0;
} am_hal_pwrctrl_mcu_memory_config_t;


  //*****************************************************************************
  //
  // DSP memory control settings.
  //
  //*****************************************************************************
#if 0
  // Check that all the reg defs are consistent with enums below.
#if ((0 == PWRCTRL_DSP0MEMPWREN_PWRENDSP0IRAM_NONE)     &&  \
     (0 == PWRCTRL_DSP0MEMRETCFG_IRAMPWDDSP0OFF_NONE)   &&  \
     (1 == PWRCTRL_DSP0MEMPWREN_PWRENDSP0IRAM_GROUP0)   &&  \
     (1 == PWRCTRL_DSP0MEMRETCFG_IRAMPWDDSP0OFF_GROUP0) &&  \
     (3 == PWRCTRL_DSP0MEMPWREN_PWRENDSP0IRAM_ALL)      &&  \
     (3 == PWRCTRL_DSP0MEMRETCFG_IRAMPWDDSP0OFF_ALL)    &&  \
     (0 == PWRCTRL_DSP1MEMPWREN_PWRENDSP1IRAM_NONE)     &&  \
     (0 == PWRCTRL_DSP1MEMRETCFG_IRAMPWDDSP1OFF_NONE)   &&  \
     (1 == PWRCTRL_DSP1MEMPWREN_PWRENDSP1IRAM_GROUP0)   &&  \
     (1 == PWRCTRL_DSP1MEMRETCFG_IRAMPWDDSP1OFF_GROUP0))
#error DSP IRAM Register Definitions do not support enum defintion!
#endif

typedef enum
{
    AM_HAL_PWRCTRL_DSP_IRAM_NONE        = PWRCTRL_DSP0MEMPWREN_PWRENDSP0IRAM_NONE,
    AM_HAL_PWRCTRL_DSP_IRAM_BANK0_ONLY  = PWRCTRL_DSP0MEMPWREN_PWRENDSP0IRAM_GROUP0,
    AM_HAL_PWRCTRL_DSP_IRAM_ALL_BANKS   = PWRCTRL_DSP0MEMPWREN_PWRENDSP0IRAM_ALL
}
am_hal_pwrctrl_dsp_iram_select_e;

// Check that all the reg defs are consistent with enums below.
#if ((0 == PWRCTRL_DSP0MEMPWREN_PWRENDSP0DRAM_NONE)     &&  \
     (0 == PWRCTRL_DSP0MEMRETCFG_DRAMPWDDSP0OFF_NONE)   &&  \
     (1 == PWRCTRL_DSP0MEMPWREN_PWRENDSP0DRAM_GROUP0)   &&  \
     (1 == PWRCTRL_DSP0MEMRETCFG_DRAMPWDDSP0OFF_GROUP0) &&  \
     (3 == PWRCTRL_DSP0MEMPWREN_PWRENDSP0DRAM_ALL)      &&  \
     (3 == PWRCTRL_DSP0MEMRETCFG_DRAMPWDDSP0OFF_ALL)    &&  \
     (0 == PWRCTRL_DSP1MEMPWREN_PWRENDSP1DRAM_NONE)     &&  \
     (0 == PWRCTRL_DSP1MEMRETCFG_DRAMPWDDSP1OFF_NONE)   &&  \
     (1 == PWRCTRL_DSP1MEMPWREN_PWRENDSP1DRAM_GROUP0)   &&  \
     (1 == PWRCTRL_DSP1MEMRETCFG_DRAMPWDDSP1OFF_GROUP0))
)
#error DSP IRAM Register Definitions do not support enum defintion!
#endif

typedef enum
{
    AM_HAL_PWRCTRL_DSP_DRAM_NONE        = PWRCTRL_DSP0MEMPWREN_PWRENDSP0DRAM_NONE,
    AM_HAL_PWRCTRL_DSP_DRAM_BANK0_ONLY  = PWRCTRL_DSP0MEMPWREN_PWRENDSP0DRAM_GROUP0,
    AM_HAL_PWRCTRL_DSP_DRAM_ALL_BANKS   = PWRCTRL_DSP0MEMPWREN_PWRENDSP0DRAM_ALL
} am_hal_pwrctrl_dsp_dram_select_e;
#endif

typedef struct
{
    bool        bEnableICache;
    bool        bRetainCache;
    bool        bEnableRAM;
    bool        bActiveRAM;
    bool        bRetainRAM;
} am_hal_pwrctrl_dsp_memory_config_t;

//*****************************************************************************
//
// Shared memory control settings.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PWRCTRL_SRAM_NONE      = 0,
    AM_HAL_PWRCTRL_SRAM_512K      = 1,
    AM_HAL_PWRCTRL_SRAM_1M        = 3
} am_hal_pwrctrl_sram_select_e;

typedef struct
{
    //
    // SRAM banks to enable.
    //
    am_hal_pwrctrl_sram_select_e        eSRAMCfg;

    //
    // Always activate SRAM when the MCU is active.
    //
    am_hal_pwrctrl_sram_select_e        eActiveWithMCU;

    //
    // Always activate SRAM when either of the DSPs are active.
    //
    am_hal_pwrctrl_sram_select_e        eActiveWithDSP;

    //
    // Retain SRAM in deep sleep.
    //
    am_hal_pwrctrl_sram_select_e        eSRAMRetain;
} am_hal_pwrctrl_sram_memcfg_t;

//*****************************************************************************
//
// System power status structure
//
//*****************************************************************************
typedef struct
{
    //
    // DEVPWRSTATUS - Device Power ON Status
    //
    uint32_t ui32Device;

    //
    // AUDSSPWRSTATUS - Audio Subsystem ON Status
    //
    uint32_t ui32AudioSS;

    //
    // MEMPWRSTATUS - MCU Memory Power ON Status
    //
    uint32_t ui32Memory;

    //
    // SYSPWRSTATUS - Power ON Status for MCU and DSP0/1 Cores
    //
    uint32_t ui32System;

    //
    // SSRAMPWRST - Shared SRAM Power ON Status
    //
    uint32_t ui32SSRAM;

    //
    // AUDSSPWRSTATUS - Audio Subsystem Power ON Status
    //
    uint32_t ui32Audio;

    //
    // DSP0MEMPWRST - DSP0 Memories Power ON Status
    //
    uint32_t ui32DSP0MemStatus;

    //
    // DSP1MEMPWRST - DSP1 Memories Power ON Status
    //
    uint32_t ui32DSP1MemStatus;

    //
    // VRSTATUS - Voltage Regulators status
    //
    uint32_t ui32VRStatus;

    //
    // ADCSTATUS - Power Status Register for ADC Block
    //
    uint32_t ui32ADC;

    //
    // AUDADCSTATUS - Power Status Register for audio ADC Block
    //
    uint32_t ui32AudioADC;
} am_hal_pwrctrl_status_t;

//*****************************************************************************
//
// Default configurations.
//
//*****************************************************************************
extern const am_hal_pwrctrl_mcu_memory_config_t      g_DefaultMcuMemCfg;
extern const am_hal_pwrctrl_sram_memcfg_t            g_DefaultSRAMCfg;
extern const am_hal_pwrctrl_dsp_memory_config_t      g_DefaultDSPMemCfg;

//*****************************************************************************
//
//! @brief Change the MCU performance mode.
//!
//! @param ePowerMode is the performance mode for the MCU
//!
//! Select the power mode for the MCU.
//!
//! @return Status code.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_mcu_mode_select(am_hal_pwrctrl_mcu_mode_e ePowerMode);

//*****************************************************************************
//
//! @brief Configure the power settings for the MCU memory.
//!
//! @param psConfig is a structure describing the desired memory configuration.
//!
//! Use this function to enable, disable, or change the sleep configuration of
//! MCU memory.
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_mcu_memory_config(am_hal_pwrctrl_mcu_memory_config_t *psConfig);

//*****************************************************************************
//
//! @brief Read the power settings for the MCU memory.
//!
//! @param psConfig is a structure describing the desired memory configuration.
//!
//! Use this function to check the current settings for the MCU memories.
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_mcu_memory_config_get(am_hal_pwrctrl_mcu_memory_config_t *psConfig);

//*****************************************************************************
//
//! @brief Configure the power settings for the Shared RAM.
//!
//! @param psConfig is a structure describing the desired memory configuration.
//!
//! Use this function to enable, disable, or change the sleep configuration of
//! MCU memory.
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_sram_config(am_hal_pwrctrl_sram_memcfg_t *psConfig);

//*****************************************************************************
//
//! @brief Get the power settings for the Shared RAM.
//!
//! @param psConfig is a structure describing the desired memory configuration.
//!
//! Use this function to check the current settings for the MCU memories.
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_sram_config_get(am_hal_pwrctrl_sram_memcfg_t *psConfig);

//*****************************************************************************
//
//! @brief Change the DSP performance mode.
//!
//! @param eDSP is the instance of the DSP 0 or 1.
//! @param ePowerMode is the performance mode for the DSP
//!
//! Select the power mode for the DSP.
//!
//! @return Status code.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_dsp_mode_select(am_hal_dsp_select_e eDSP,
                                               am_hal_pwrctrl_dsp_mode_e ePowerMode);

//*****************************************************************************
//
//! @brief Configure the power and memory settings for the DSP.
//!
//! @param eDSP is the instance of the DSP 0 or 1.
//! @param psConfig is a structure containing settings for the DSP
//!
//! Use this function to configure the DSP. See the documentation for the
//! configuration structure for more information on the available settings.
//!
//! @return Status code.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_dsp_memory_config(am_hal_dsp_select_e eDSP,
                                                 am_hal_pwrctrl_dsp_memory_config_t *psConfig);

//*****************************************************************************
//
//! @brief Get the current power and memory settings for the DSP.
//!
//! @param eDSP is the instance of the DSP 0 or 1.
//! @param psConfig is a returns the current settings for the DSP
//!
//! Use this function to check the current configuration of the DSP. This will
//! populate a configuration structure with exactly the same format required by
//! the configuration function.
//!
//! @return Status code.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_dsp_memory_config_get(am_hal_dsp_select_e eDSP,
                                                     am_hal_pwrctrl_dsp_memory_config_t *psConfig);

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
//! @param ePeripheral - The peripheral to enable.
//! @param pui32Enabled - Pointer to a ui32 that will return as 1 or 0.
//!
//! This function determines to the caller whether a given peripheral is
//! currently enabled or disabled.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_periph_enabled(am_hal_pwrctrl_periph_e ePeripheral,
                                              bool *bEnabled);

//*****************************************************************************
//
//! @brief Get the current powercontrol status registers.
//!
//! @param psStatus returns a structure containing power status information.
//!
//! This function can be used to determine the current status of a wide variety
//! of system components.
//!
//! @return status code.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_status_get(am_hal_pwrctrl_status_t *psStatus);

//*****************************************************************************
//
//! @brief Initialize system for low power configuration.
//!
//! @param none.
//!
//! This function handles low power initialization.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_pwrctrl_low_power_init(void);

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
