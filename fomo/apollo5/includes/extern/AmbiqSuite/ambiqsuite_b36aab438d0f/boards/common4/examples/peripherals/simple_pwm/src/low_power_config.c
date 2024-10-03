//*****************************************************************************
//
//! @file low_power_config.c
//!
//! @brief Low power config example code
//!
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "low_power_config.h"
#include "am_util.h"

//
//! setup memory for lowest power
//
static const am_hal_pwrctrl_sram_memcfg_t SRAMMemCfg =
{
    .eSRAMCfg           = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithMCU     = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithGFX     = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithDISP    = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithDSP     = AM_HAL_PWRCTRL_SRAM_NONE,
    .eSRAMRetain        = AM_HAL_PWRCTRL_SRAM_NONE
};


//
//! setup memory for lowest power
//
static const am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
{
    .eCacheCfg    = AM_HAL_PWRCTRL_CACHE_ALL,
    .bRetainCache = false,
#if defined(AM_PART_APOLLO4L)
    .eDTCMCfg     = AM_HAL_PWRCTRL_DTCM_32K,
.eRetainDTCM  = AM_HAL_PWRCTRL_DTCM_32K,
#else
#if (EXAMPLE_DEBUG == 1)
    .eDTCMCfg     = AM_HAL_PWRCTRL_DTCM_128K,
    .eRetainDTCM  = AM_HAL_PWRCTRL_DTCM_128K,
#else
    .eDTCMCfg     = AM_HAL_PWRCTRL_DTCM_8K,
    .eRetainDTCM  = AM_HAL_PWRCTRL_DTCM_8K,
#endif
#endif
    .bEnableNVM0  = true,
    .bRetainNVM0  = false,
};


#if defined(AM_PART_APOLLO4B)
//
//! Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
//
AM_SHARED_RW uint8_t g_axiScratchBuf[96];
#endif

//*****************************************************************************
// standard cache setup
//*****************************************************************************
uint32_t
std_cache_setup(void)
{
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#if defined(AM_PART_APOLLO4B)
    //
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
    uint32_t ui32BuffAddr = ((uint32_t) g_axiScratchBuf + 15) & ~0x0000000Ful;
    uint32_t ui32Status = am_hal_daxi_control(
        AM_HAL_DAXI_CONTROL_AXIMEM,
        (uint8_t *) ui32BuffAddr);
#else
    uint32_t ui32Status = am_hal_daxi_control(
        AM_HAL_DAXI_CONTROL_ENABLE,
        NULL);
#endif

    return ui32Status;
}

//*****************************************************************************
// configure the processor for very low power
//*****************************************************************************
uint32_t
deepsleep_config(void)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
#if defined (AM_PART_APOLLO4B)
    ui32Status = am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_CRYPTO_POWERDOWN, 0);
#else
    ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
#endif

    if (ui32Status == AM_HAL_STATUS_SUCCESS)
    {
        //
        // Disable all peripherals
        //
        ui32Status = am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL, 0);
    }

#if RTC_CLK_SRC != XT
    //
    // Disable XTAL in deepsleep
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP, 0);
#endif

#ifdef AM_DEVICES_BLECTRLR_RESET_PIN
    //
    // For SiP packages, put the BLE Controller in reset.
    //
    am_hal_gpio_state_write(AM_DEVICES_BLECTRLR_RESET_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(AM_DEVICES_BLECTRLR_RESET_PIN,   am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(AM_DEVICES_BLECTRLR_RESET_PIN, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(AM_DEVICES_BLECTRLR_RESET_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
#endif // AM_DEVICES_BLECTRLR_RESET_PIN

    return ui32Status;
}

//*****************************************************************************
// standard cache setup
//*****************************************************************************
uint32_t
lowpower_memory_config(void)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    do
    {
        ui32Status = am_hal_pwrctrl_mcu_memory_config((am_hal_pwrctrl_mcu_memory_config_t *) &McuMemCfg);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Error - am_hal_pwrctrl_mcu_memory_config. ui32Status = %d\n\nExample Stopping\n",
                                 ui32Status);
            break;
        }

        ui32Status = am_hal_pwrctrl_sram_config((am_hal_pwrctrl_sram_memcfg_t *) &SRAMMemCfg);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Error - am_hal_pwrctrl_sram_config. ui32Status = %d\n\nExample Stopping\n",
                                 ui32Status);
        }
    } while (false);

    return ui32Status;
}

