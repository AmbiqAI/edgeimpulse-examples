//*****************************************************************************
//
//! @file am_widget_uart.c
//!
//! @brief This widget allows test cases to exercise the Apollo2 UART HAL
//! using loopback between UART #0 and #1.
//!
//!
//!
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdlib.h>
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_widget_mspi.h"
#include "am_bsp.h"
#include "am_util.h"
#include "mspi_test_common.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define MSPI_DEBUG_PRINTING     1

//*****************************************************************************
//
// Global data
//
//*****************************************************************************
am_widget_mspi_config_t g_MSPITestConfig[AM_REG_MSPI_NUM_MODULES];               // Test configuration.

#if defined (AM_PART_APOLLO510L)
AM_SHARED_RW uint8_t                 g_SectorTXBuffer[512*1024] __attribute__((aligned(32)));      // 512K SRAM TX buffer
AM_SHARED_RW uint8_t                 g_SectorRXBuffer[512*1024] __attribute__((aligned(32)));      // 512K SRAM RX buffer.
AM_SHARED_RW uint8_t                 g_SectorRXBuffer2[1024] __attribute__((aligned(32)));          // 1K SRAM RX buffer.
#else
AM_SHARED_RW uint8_t                 g_SectorTXBuffer[1024*1024] __attribute__((aligned(32)));      // 1M SRAM TX buffer
AM_SHARED_RW uint8_t                 g_SectorRXBuffer[1024*1024] __attribute__((aligned(32)));      // 1M SRAM RX buffer.
AM_SHARED_RW uint8_t                 g_SectorRXBuffer2[1024] __attribute__((aligned(32)));          // 1K SRAM RX buffer.
#endif

uint8_t       ui8Byte;
uint32_t      ui32Word, ui32CmpWord, ui32MiscompareCount;

mspi_device_func_t mspi_device_func =
{
#if defined(ADESTO_ATXP032)
    .devName = "MSPI FLASH ATXP032",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_atxp032_init,
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_apply_sdr_timing,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_read,
    .mspi_read_adv              = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_read_adv,
    .mspi_read_callback         = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_read_callback,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_write,
    .mspi_mass_erase            = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase          = (uint32_t (*)(void *, ...))am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable            = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_atxp032_disable_scrambling,
#elif defined(ISSI_IS25WX064)
    .devName = "MSPI FLASH IS25WX064",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_is25wx064_init,
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_is25wx064_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_apply_ddr_timing,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_read,
    .mspi_read_adv              = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_read_adv,
    .mspi_read_callback         = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_read_callback,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_write,
    .mspi_mass_erase            = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase          = (uint32_t (*)(void *, ...))am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable            = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_is25wx064_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_is25wx064_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_is25wx064_disable_scrambling,
#elif defined(MACRONIX_MX25UM51245G)
    .devName = "MSPI FLASH MX25UM51245G",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_mx25um51245g_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_mx25um51245g_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_apply_timing,
#endif
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_read,
    .mspi_read_adv              = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_read_adv,
    .mspi_read_callback         = NULL,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_write,
    .mspi_mass_erase            = am_devices_mspi_mx25um51245g_chip_erase,
    .mspi_sector_erase          = (uint32_t (*)(void *, ...))am_devices_mspi_mx25um51245g_sector_erase,
    .mspi_xip_enable            = am_devices_mspi_mx25um51245g_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_mx25um51245g_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_mx25um51245g_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_mx25um51245g_disable_scrambling,
#elif defined(AM_WIDGET_MSPI_PSRAM_DEVICE)
    .devName = "MSPI PSRAM",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_init,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_psram_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_psram_write,
    .mspi_xip_enable            = am_devices_mspi_psram_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_psram_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_disable_scrambling,
    .mspi_init_timing_check     = NULL,
    .mspi_init_timing_apply     = NULL,
#elif defined(AM_WIDGET_MSPI_DDR_PSRAM_DEVICE)
    .devName = "MSPI DDR PSRAM APS12808L",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_aps12808l_ddr_init,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps12808l_ddr_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps12808l_ddr_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps12808l_ddr_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps12808l_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_aps12808l_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_aps12808l_ddr_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_psram_aps12808l_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_aps12808l_ddr_disable_scrambling,
#elif defined(AM_WIDGET_MSPI_HEX_DDR_PSRAM_DEVICE)
    .devName = "MSPI HEX DDR PSRAM APS25616N",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_ddr_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_xip_config            = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_xip_config,
    .mspi_scrambling_enable     = am_devices_mspi_psram_aps25616n_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_aps25616n_ddr_disable_scrambling,
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_apply_ddr_timing,
    .mspi_write_adv             = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_ddr_write_adv,
    .mspi_read_adv              = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616n_ddr_read_adv,
#elif defined(AM_WIDGET_MSPI_HEX_DDR_APM_PSRAM_1P2V_DEVICE)
    .devName = "MSPI HEX DDR PSRAM APS25616BA",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_aps25616ba_ddr_init,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_ddr_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_ddr_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_ddr_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_aps25616ba_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_aps25616ba_ddr_disable_xip,
    .mspi_xip_config            = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_xip_config,
    .mspi_scrambling_enable     = am_devices_mspi_psram_aps25616ba_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_aps25616ba_ddr_disable_scrambling,
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_aps25616ba_ddr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_apply_ddr_timing,
    .mspi_write_adv             = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_ddr_write_adv,
    .mspi_read_adv              = (uint32_t (*)(void *, ...))am_devices_mspi_psram_aps25616ba_ddr_read_adv,
#elif defined(AM_WIDGET_MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE)
    .devName = "MSPI HEX DDR WINBOND PSRAM",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_w958d6nw_ddr_init,
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_w958d6nw_ddr_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_psram_w958d6nw_ddr_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_psram_w958d6nw_ddr_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_psram_w958d6nw_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_w958d6nw_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_w958d6nw_ddr_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_psram_w958d6nw_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_w958d6nw_ddr_disable_scrambling,
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_psram_hex_ddr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_psram_apply_hex_ddr_timing,
    .mspi_write_adv             = (uint32_t (*)(void *, ...))am_devices_mspi_psram_w958d6nw_ddr_write_adv,
    .mspi_read_adv              = (uint32_t (*)(void *, ...))am_devices_mspi_psram_w958d6nw_ddr_read_adv,
#elif defined(DOSILICON_DS35X1GA)
    .devName = "DS35X1GA",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_ds35x1ga_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_ds35x1ga_sdr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_ds35x1ga_apply_sdr_timing,
#endif
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_ds35x1ga_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_ds35x1ga_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_ds35x1ga_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_ds35x1ga_write,
    .mspi_block_erase           = (uint32_t (*)(void *, ...))am_devices_mspi_ds35x1ga_block_erase,
#elif defined(TOSHIBA_TC58CYG0)
    .devName = "TC58CYG0",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_tc58cyg0_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_tc58cyg0_sdr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_tc58cyg0_apply_sdr_timing,
#endif
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_tc58cyg0_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_tc58cyg0_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_tc58cyg0_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_tc58cyg0_write,
    .mspi_block_erase           = (uint32_t (*)(void *, ...))am_devices_mspi_tc58cyg0_block_erase,
#elif defined(WINBOND_W25N02KW)
    .devName = "W25N02KW",
    .mspi_init                  = (uint32_t (*)(uint32_t, ...))am_devices_mspi_w25n02kw_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check     = (uint32_t (*)(uint32_t, ...))am_devices_mspi_w25n02kw_sdr_init_timing_check,
    .mspi_init_timing_apply     = (uint32_t (*)(void *, ...))am_devices_mspi_w25n02kw_apply_sdr_timing,
#endif
    .mspi_term                  = (uint32_t (*)(void *, ...))am_devices_mspi_w25n02kw_deinit,
    .mspi_read_id               = (uint32_t (*)(void *, ...))am_devices_mspi_w25n02kw_id,
    .mspi_read                  = (uint32_t (*)(void *, ...))am_devices_mspi_w25n02kw_read,
    .mspi_write                 = (uint32_t (*)(void *, ...))am_devices_mspi_w25n02kw_write,
    .mspi_block_erase           = (uint32_t (*)(void *, ...))am_devices_mspi_w25n02kw_block_erase,
#else
#error "Unknown MSPI Device"
#endif
};
#if defined (AM_PART_APOLLO5A)
const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_XIP_BASEADDR, MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_END_ADDR},
  {MSPI1_XIP_BASEADDR, MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_END_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_END_ADDR},
  {MSPI3_XIP_BASEADDR, MSPI3_APERTURE_START_ADDR, MSPI3_APERTURE_END_ADDR},
};
#elif defined (AM_PART_BRONCO) || defined (AM_PART_APOLLO5B)
const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_END_ADDR},
  {MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_END_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_END_ADDR},
  {MSPI3_APERTURE_START_ADDR, MSPI3_APERTURE_START_ADDR, MSPI3_APERTURE_END_ADDR},
};
#elif defined (AM_PART_APOLLO510L)
const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_END_ADDR},
  {MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_END_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_END_ADDR},
};
#endif
//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************

//*****************************************************************************
//
// Interrupt handler for MSPI
//
//*****************************************************************************
//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
  #if !defined(AM_PART_APOLLO510L)
    MSPI3_IRQn,
  #endif
};

volatile uint32_t               g_MSPIInterruptStatus[AM_REG_MSPI_NUM_MODULES];

#if defined(am_widget_mspi_devices_timing_config_t)
am_widget_mspi_devices_timing_config_t g_MSPITimingConfig;
#endif

#define AM_MSPIn_ISR(n)                                                                   \
void am_mspi##n##_isr(void)                                                               \
{                                                                                         \
    uint32_t      ui32Status;                                                             \
    am_hal_mspi_interrupt_status_get(g_MSPITestConfig[n].pHandle, &ui32Status, false);    \
    am_hal_mspi_interrupt_clear(g_MSPITestConfig[n].pHandle, ui32Status);                 \
    am_hal_mspi_interrupt_service(g_MSPITestConfig[n].pHandle, ui32Status);               \
    g_MSPIInterruptStatus[n] &= ~ui32Status;                                              \
}                                                                                         \

AM_MSPIn_ISR(0);
AM_MSPIn_ISR(1);
AM_MSPIn_ISR(2);
#if !defined(AM_PART_APOLLO510L)
AM_MSPIn_ISR(3);
#endif

static int am_widget_mspi_prepare_test_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len);

uint32_t
am_widget_mspi_power_save_and_restore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode)
{
#ifdef MSPI_TEST_POWER_SAVE_RESTORE
    am_widget_mspi_config_t *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;
    // Invalidate D-Cache before disabling MSPI
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    am_util_delay_ms(1);
#ifdef MSPI_TEST_POWER_MSPI_ENABLE_DISABLE
    // This test will do disable IOM before power control
    if(AM_HAL_STATUS_SUCCESS != am_hal_mspi_disable(pWidgetConfig->pHandle))
    {
        return AM_WIDGET_ERROR;
    }
#endif // MSPI_TEST_POWER_MSPI_ENABLE_DISABLE
    if(AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pWidgetConfig->pHandle, sleepMode, true))
    {
        return AM_WIDGET_ERROR;
    }
#ifdef MSPI_TEST_POWER_SAVE_RESTORE_SLEEP
    stimer_init();
    if(sleepMode == AM_HAL_SYSCTRL_DEEPSLEEP)
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
    else
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
    }
    stimer_deinit();
#else // MSPI_TEST_POWER_SAVE_RESTORE_SLEEP
    am_util_delay_ms(1);
#endif // MSPI_TEST_POWER_SAVE_RESTORE_SLEEP
    if(AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pWidgetConfig->pHandle, AM_HAL_SYSCTRL_WAKE, true))
    {
        return AM_WIDGET_ERROR;
    }
#ifdef MSPI_TEST_POWER_MSPI_ENABLE_DISABLE
    // This test will do disable IOM before power control
    if(AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pWidgetConfig->pHandle))
    {
        return AM_WIDGET_ERROR;
    }
#endif // MSPI_TEST_POWER_MSPI_ENABLE_DISABLE
#else
    (void) pWidget;
#endif // MSPI_TEST_POWER_SAVE_RESTORE
    return AM_WIDGET_SUCCESS;
}
//*****************************************************************************
//
// Widget Setup Function.
//
//*****************************************************************************
bool am_widget_mspi_setup(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr)
{
  uint32_t      ui32Status = AM_WIDGET_MSPI_SUCCESS;
#if defined(APOLLO5_FPGA)
  am_hal_pwrctrl_sram_memcfg_t  SRAMCfg =
  {
    .eSRAMCfg       = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL,
    .eActiveWithMCU = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL,
    .eSRAMRetain    = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL
  };
#endif

#if !defined(APOLLO5_FPGA) && defined(am_widget_mspi_devices_timing_config_t)
  if ( mspi_device_func.mspi_init_timing_check != NULL )
  {
    ui32Status = mspi_device_func.mspi_init_timing_check(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &g_MSPITimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("MSPI timing scan failed.  Status = %d\n", ui32Status);
        return false;
    }
  }
#endif
  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_MSPITestConfig[pMSPITestConfig->ui32Module];

  g_MSPIInterruptStatus[pMSPITestConfig->ui32Module] = 0;

  // Set up the MSPI configuraton for the N25Q256A part.
  ui32Status = mspi_device_func.mspi_init(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &pMSPITestConfig->pDevHandle, &pMSPITestConfig->pHandle);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("MSPI Initialization failed.  Status = %d\n", ui32Status);
    return false;
  }

#if !defined(APOLLO5_FPGA) && defined(am_widget_mspi_devices_timing_config_t)
  if ( mspi_device_func.mspi_init_timing_apply != NULL )
  {
    ui32Status = mspi_device_func.mspi_init_timing_apply(pMSPITestConfig->pDevHandle, &g_MSPITimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("MSPI Timing apply failed.  Status = %d\n", ui32Status);
        return false;
    }
  }
#endif

  g_MSPITestConfig[pMSPITestConfig->ui32Module] = *pMSPITestConfig;

#if defined(APOLLO5_FPGA)
  ui32Status = am_hal_pwrctrl_sram_config(&SRAMCfg);
#endif

  NVIC_SetPriority(mspi_interrupts[pMSPITestConfig->ui32Module], AM_IRQ_PRIORITY_DEFAULT);
  NVIC_EnableIRQ(mspi_interrupts[pMSPITestConfig->ui32Module]);

  am_hal_interrupt_master_enable();

  // Return the result.
  return (AM_WIDGET_MSPI_SUCCESS == ui32Status);

}

//*****************************************************************************
//
// Widget Init Function without timing scan
//
//*****************************************************************************
bool am_widget_mspi_init(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr)
{
  uint32_t      ui32Status;
  am_hal_pwrctrl_sram_memcfg_t  SRAMCfg =
  {
    .eSRAMCfg       = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL,
    .eActiveWithMCU = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL,
    .eSRAMRetain    = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL
  };

  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_MSPITestConfig[pMSPITestConfig->ui32Module];

  g_MSPIInterruptStatus[pMSPITestConfig->ui32Module] = 0;

  // Call init function in driver
  ui32Status = mspi_device_func.mspi_init(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &pMSPITestConfig->pDevHandle, &pMSPITestConfig->pHandle);
  g_MSPITestConfig[pMSPITestConfig->ui32Module] = *pMSPITestConfig;

  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return false;
  }

  ui32Status = am_hal_pwrctrl_sram_config(&SRAMCfg);

  NVIC_SetPriority(mspi_interrupts[pMSPITestConfig->ui32Module], AM_IRQ_PRIORITY_DEFAULT);
  NVIC_EnableIRQ(mspi_interrupts[pMSPITestConfig->ui32Module]);

  am_hal_interrupt_master_enable();

  // Return the result.
  return (AM_WIDGET_MSPI_SUCCESS == ui32Status);

}

//*****************************************************************************
//
// Widget Cleanup Function.
//
//*****************************************************************************
bool am_widget_mspi_cleanup(void *pWidget, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  am_hal_interrupt_master_disable();

  NVIC_DisableIRQ(mspi_interrupts[pWidgetConfig->ui32Module]);

  // Need to make sure all pending XIPMM transactions are flushed
  am_hal_cachectrl_dcache_invalidate(NULL, true);

  ui32Status = mspi_device_func.mspi_term(pWidgetConfig->pDevHandle);

  // Return the result.
  return (AM_WIDGET_MSPI_SUCCESS == ui32Status);
}

//*****************************************************************************
//
// Widget Check Buffer Helper Function.
//
//*****************************************************************************
static bool am_widget_mspi_check_buffer(uint32_t ui32NumBytes, uint8_t *pRxBuffer, uint8_t *pTxBuffer)
{
  for (uint32_t i = 0; i < ui32NumBytes; i++)
  {
    if (pRxBuffer[i] != pTxBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, pRxBuffer[i], pTxBuffer[i]);
      return false;
    }
  }
  return true;
}

//*****************************************************************************
//
// Widget Test Execution Functions
//
//*****************************************************************************

#if defined(DOSILICON_DS35X1GA) || defined(TOSHIBA_TC58CYG0) || defined(WINBOND_W25N02KW)
// Just check that we can read the Device ID correctly.
bool am_widget_mspi_test_get_eflash_id(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  uint32_t      pui32DeviceID;
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  ui32Status = mspi_device_func.mspi_read_id(pWidgetConfig->pDevHandle, &pui32DeviceID);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read the MSPI Device ID\n");
    return false;
  }
  else
  {
    am_util_stdio_printf("Device ID: %08x\n", pui32DeviceID);
    return true;
  }
}

uint32_t am_widget_mspi_nand_flash_bad_block_check(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  uint8_t ui8Read_oob[64];
  uint8_t ui8EccStatus;

#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Page %d\n", pTestConfig->NumBytes,
                       pTestConfig->PageNum + pTestConfig->PageOffset);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, pTestConfig->PageNum + pTestConfig->PageOffset,
                                          &g_SectorRXBuffer[pTestConfig->ByteOffset],
                                          pTestConfig->NumBytes,
                                          ui8Read_oob,
                                          64,
                                          &ui8EccStatus);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return 0xFFFFFFFF;
  }
  //assuming internal ECC is off
  if (0 != ui8EccStatus)
  {
    am_util_stdio_printf("Bad block %d detected, ECC Status = %d\n", (pTestConfig->PageNum + pTestConfig->PageOffset) >> 6, ui8EccStatus);
    return true;
  }
  return false;
}

bool am_widget_mspi_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  uint8_t ui8Write_oob[64];
  uint8_t ui8Read_oob[64];
  uint8_t ui8EccStatus;

// Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[pTestConfig->ByteOffset + i] = (i & 0xFF);
  }

  //
  // fill the oob area with zero
  //
  memset(ui8Write_oob, 0x0, sizeof(ui8Write_oob));

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the target block.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing block %d\n", (pTestConfig->PageNum + pTestConfig->PageOffset) >> 6);
#endif
  ui32Status = mspi_device_func.mspi_block_erase(pWidgetConfig->pDevHandle,
                                                (pTestConfig->PageNum + pTestConfig->PageOffset) >> 6);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target block.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target page.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Page %d\n", pTestConfig->NumBytes,
                       pTestConfig->PageNum + pTestConfig->PageOffset);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle, pTestConfig->PageNum + pTestConfig->PageOffset,
                                           &g_SectorTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->NumBytes,
                                           ui8Write_oob,
                                           64);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target page.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Page %d\n", pTestConfig->NumBytes,
                       pTestConfig->PageNum + pTestConfig->PageOffset);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, pTestConfig->PageNum + pTestConfig->PageOffset,
                                          &g_SectorRXBuffer[pTestConfig->ByteOffset],
                                          pTestConfig->NumBytes,
                                          ui8Read_oob,
                                          64,
                                          &ui8EccStatus);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  return am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->ByteOffset, g_SectorTXBuffer  + pTestConfig->ByteOffset);
}
#else

// Just check that we can read the Device ID correctly.
bool am_widget_mspi_test_get_eflash_id(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  ui32Status = mspi_device_func.mspi_read_id(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read the MSPI Device ID\n");
    return false;
  }
  else
  {
    return true;
  }

}

// Write/Read test.
#if defined(AMBT52)
bool am_widget_mspi_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr, am_hal_mspi_callback_t pfnCallback, void *pCallbackCtxt)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;
  static uint32_t DataOffset = 0;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[pTestConfig->ByteOffset + i] = ((i + DataOffset) & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();
  DataOffset++;

  // Write the TX buffer into the target sector.
  ui32Status = mspi_device_func.mspi_write_adv(pWidgetConfig->pDevHandle,
                                           &g_SectorTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                           pTestConfig->NumBytes, 0, 0, pfnCallback, pCallbackCtxt);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }
  while ( *(volatile bool*)pCallbackCtxt == false );
  *(volatile bool*)pCallbackCtxt = false;

  // Read the data back into the RX buffer.
  ui32Status = mspi_device_func.mspi_read_adv(pWidgetConfig->pDevHandle, &g_SectorRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, 0, 0, pfnCallback, pCallbackCtxt);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }
  while ( *(volatile bool*)pCallbackCtxt == false );
  *(volatile bool*)pCallbackCtxt = false;

  // Compare the buffers
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      return false;
    }
  }
  return true;
}
#else
bool am_widget_mspi_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[pTestConfig->ByteOffset + i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           &g_SectorTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_SectorRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  bool pass = am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->ByteOffset, g_SectorTXBuffer  + pTestConfig->ByteOffset);
#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif

  return pass;
}

#if defined(COLLECT_BANDWIDTH)

#ifndef BANDWIDTH_TEST_LOOP
#define BANDWIDTH_TEST_LOOP 10
#endif

uint32_t am_widget_mspi_cal_time(uint32_t ui32prev, uint32_t ui32curr)
{
  if(ui32prev > ui32curr)
  {
      return 0xFFFFFFFF - ui32prev + ui32curr;
  }
  else
  {
      return ui32curr - ui32prev;
  }
}

bool am_widget_mspi_test_write_read_bandwidth(void *pWidget, void *pTestCfg, char *pErrStr)
{

  uint32_t ui32TimerTickBefore = 0;
  uint32_t ui32TimerTickAfter = 0;
  float num_of_ms_write,num_of_ms_read;

  uint32_t loop;

  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    pTestConfig->pTxBuffer[pTestConfig->ByteOffset + i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
    uint32_t ui32EraseCnt = pTestConfig->NumBytes * BANDWIDTH_TEST_LOOP / AM_DEVICES_MSPI_FLASH_SECTOR_SIZE;
    if ( (pTestConfig->SectorAddress + pTestConfig->NumBytes * BANDWIDTH_TEST_LOOP) % AM_DEVICES_MSPI_FLASH_SECTOR_SIZE )
    {
        ui32EraseCnt += 1;
    }
    for ( uint8_t i = 0; i < ui32EraseCnt; i++ )
    {
        // Erase the target sector.
        ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                        (pTestConfig->SectorAddress / AM_DEVICES_MSPI_FLASH_SECTOR_SIZE + i)*AM_DEVICES_MSPI_FLASH_SECTOR_SIZE);
        if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
            return false;
        }
    }
#endif

  num_of_ms_write = 0;
  for(loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
  {
    ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);
    ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           pTestConfig->pTxBuffer + pTestConfig->ByteOffset,
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset * loop,
                                           pTestConfig->NumBytes, true);
    ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);
    if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
      break;
    }
    num_of_ms_write += am_widget_mspi_cal_time(ui32TimerTickBefore, ui32TimerTickAfter);
  }
  if(loop == 0)
  {
    return false;
  }
  else
  {
    if(loop != BANDWIDTH_TEST_LOOP)
    {
      am_util_stdio_printf("############# Warning: write loop = %d #############\n",loop);
    }
    num_of_ms_write /= loop;
  }

  num_of_ms_read = 0;
  for(loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
  {
    ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);
    ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle,
                                            pTestConfig->pRxBuffer + pTestConfig->ByteOffset + pTestConfig->SectorOffset * loop,
                                            pTestConfig->SectorAddress + pTestConfig->SectorOffset * loop,
                                            pTestConfig->NumBytes, true);
    ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);
    if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
      break;
    }
    num_of_ms_read += am_widget_mspi_cal_time(ui32TimerTickBefore, ui32TimerTickAfter);
  }
  if(loop == 0)
  {
    return false;
  }
  else
  {
    if(loop != BANDWIDTH_TEST_LOOP)
    {
      am_util_stdio_printf("############# Warning: read loop = %d #############\n",loop);
    }
    num_of_ms_read /= loop;
  }

  bool pass = true;
  for(loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
  {
    if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer + pTestConfig->ByteOffset + pTestConfig->SectorOffset * loop, pTestConfig->pTxBuffer + pTestConfig->ByteOffset))
    {
      am_util_stdio_printf("Loop#%d, Read & Write buffer doesn not match!\n", loop);
      pass = false;
    }
  }

  pTestConfig->bandwidth.f32WriteBandwidth = (float)pTestConfig->NumBytes / num_of_ms_write / TIME_DIVIDER;
  pTestConfig->bandwidth.f32ReadBandwidth = (float)pTestConfig->NumBytes / num_of_ms_read / TIME_DIVIDER;

  am_util_stdio_printf("Write: %d bytes per loop, Bandwidth is %f MB/s\n", pTestConfig->NumBytes, pTestConfig->bandwidth.f32WriteBandwidth);
  am_util_stdio_printf("Read:  %d bytes per loop, Bandwidth is %f MB/s\n", pTestConfig->NumBytes, pTestConfig->bandwidth.f32ReadBandwidth);

  return pass;
}
#endif

#endif

// Write/Read test.
bool am_widget_mspi_test_b2b(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  uint32_t                       readBlockSize;
  uint32_t                       numBytesToRead;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[pTestConfig->ByteOffset + i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           &g_SectorTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes as a single block from Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_SectorRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif

  if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->ByteOffset, g_SectorTXBuffer + pTestConfig->ByteOffset))
  {
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  numBytesToRead = pTestConfig->NumBytes;
  // Divide into very small sizes - to create a flurry of back to back transactions
  readBlockSize = pTestConfig->ReadBlockSize;

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes using smaller blocks of %d bytes each and queued transactions from Sector %8X\n",
                       pTestConfig->NumBytes,
                       readBlockSize,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  while (numBytesToRead)
  {
      uint32_t num = (numBytesToRead > readBlockSize) ? readBlockSize : numBytesToRead;
      ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_SectorRXBuffer[pTestConfig->ByteOffset + pTestConfig->NumBytes - numBytesToRead],
                                              pTestConfig->SectorAddress + pTestConfig->SectorOffset + pTestConfig->NumBytes - numBytesToRead,
                                              num,
                                              (numBytesToRead > readBlockSize) ? false : true);
      if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
      {
        am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
        return false;
      }
      numBytesToRead -= num;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  bool pass = am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->ByteOffset, g_SectorTXBuffer + pTestConfig->ByteOffset);
#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif

  return pass;
}


// How many reads to split the Readback into
#define AM_WIDGET_MSPI_MAX_QUEUE_SIZE    16
// Write/Read test.
bool am_widget_mspi_test_write_queue_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t      *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  uint32_t                       readBlockSize;
  uint32_t                       numBytesToRead;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[pTestConfig->ByteOffset + i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           &g_SectorTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes as a single block from Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_SectorRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->ByteOffset, g_SectorTXBuffer + pTestConfig->ByteOffset))
  {
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  numBytesToRead = pTestConfig->NumBytes;
  for (uint32_t i = AM_WIDGET_MSPI_MAX_QUEUE_SIZE - 1; i > 0; i--)
  {
    readBlockSize = numBytesToRead / i;
    if (readBlockSize > 0)
    {
        break;
    }
  }
  if (readBlockSize == 0)
  {
    readBlockSize = numBytesToRead;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes using smaller blocks of %d bytes each and queued transactions from Sector %8X\n",
                       pTestConfig->NumBytes,
                       readBlockSize,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
#endif
  while (numBytesToRead)
  {
      uint32_t num = (numBytesToRead > readBlockSize) ? readBlockSize : numBytesToRead;
      ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_SectorRXBuffer[pTestConfig->ByteOffset + pTestConfig->NumBytes - numBytesToRead],
                                              pTestConfig->SectorAddress + pTestConfig->SectorOffset + pTestConfig->NumBytes - numBytesToRead,
                                              num,
                                              true);
      if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
      {
        am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
        return false;
      }
      numBytesToRead -= num;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  bool pass = am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->ByteOffset, g_SectorTXBuffer + pTestConfig->ByteOffset);
#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif

  return pass;
}

// XIP Data bus read test.
bool am_widget_mspi_test_xip_databus(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_SectorTXBuffer, pTestConfig->SectorAddress,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Set up for XIP operation.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Putting the MSPI and External Flash into XIP mode\n");
#endif
  ui32Status = mspi_device_func.mspi_xip_enable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

  // Read and check the data by comparing it to the base structure (word-wise)
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i += 4)
  {
    ui32Word = *(uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + i);
    ui32CmpWord = *(uint32_t *)&g_SectorTXBuffer[i];
    if (ui32CmpWord != ui32Word)
    {
      am_util_stdio_printf("Word Aligned Data Bus compare with External Flash failed at Address %8X\n",
                           (g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + i));
      return false;
    }
  }

  // Read and check the data by comparing it to the base structure (byte-wise)
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    ui8Byte = *(uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + i);
    if (g_SectorTXBuffer[i] != ui8Byte)
    {
      am_util_stdio_printf("Byte Aligned Data Bus compare with External Flash failed at Address %8X\n",
                           (g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + i));
      return false;
    }
  }

  // Shutdown XIP operation.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
#endif
  ui32Status = mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif
  return true;
}

//*****************************************************************************
//
// Static function to be executed from external flash device
//
//*****************************************************************************
#if defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void xip_test_function(void)
{
    __asm
    (
        "   nop\n"              // Just execute NOPs and return.
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   bx      lr\n"
    );
}

#elif defined(__ARMCC_VERSION)
__asm static void xip_test_function(void)
{
    nop                         // Just execute NOPs and return.
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    bx      lr
}

#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static void xip_test_function(void)
{
    __asm("    nop");           // Just execute NOPs and return.
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    bx      lr");
}
#endif

#define MSPI_XIP_FUNCTION_SIZE  72

// Patchable section of binary
extern uint32_t * __pPatchable;

// Configurable option to load a position independent library instead
void get_test_function(mspi_xip_test_funcinfo_t *pFuncInfo)
{
    // some of the parameters are controllable through binary patching
    if (__pPatchable[0])
    {
        pFuncInfo->binAddr = __pPatchable[0];
        pFuncInfo->funcOffset = __pPatchable[1];
        pFuncInfo->binSize = __pPatchable[2];
        pFuncInfo->param0 = __pPatchable[3];
        pFuncInfo->param1 = __pPatchable[4];
        pFuncInfo->param2 = __pPatchable[5];
        pFuncInfo->result = __pPatchable[6];
    }
#if defined RUN_XIP_CODE
    else
    {
     /* defined on linker script file */
    extern uint32_t _sxipcode;
    extern uint32_t _exipcode;
    extern uint32_t __xip_text_load_addr;
        pFuncInfo->binAddr = (uint32_t)&__xip_text_load_addr;
        pFuncInfo->funcOffset = 0;
        pFuncInfo->binSize = (uint32_t)&_exipcode - (uint32_t)&_sxipcode;
        pFuncInfo->param0 = 1;
        pFuncInfo->param1 = 2;
        pFuncInfo->param2 = 3;
        pFuncInfo->result = 0;
    }
#else
    else
    {
        pFuncInfo->binAddr = ((uint32_t)&xip_test_function) & 0xFFFFFFFE;
        pFuncInfo->funcOffset = 0;
        pFuncInfo->binSize = MSPI_XIP_FUNCTION_SIZE;
        pFuncInfo->param0 = 0xDEADBEEF;
        pFuncInfo->result = 0xDEADBEEF;
    }
#endif
}

#if defined(CYPRESS_S25FS064S)
bool am_widget_mspi_test_xip_mixedmode(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  am_widget_mspi_config_t        *pDevConfig  = (am_widget_mspi_config_t *)pWidget;

  //
  // Read the MSPI Device ID.
  //
  ui32Status = mspi_device_func.mspi_read_id(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read Flash Device ID!\n");
    return false;
  }

  //
  // Generate data into the Sector Buffer
  //
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

  //
  // Erase the target sector.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %d\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase Flash Device sector!\n");
    return false;
  }

  //
  // Make sure we aren't in XIP mode.
  //
  ui32Status = mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    return false;
  }

  //
  // Write the TX buffer into the target sector.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %d\n", pTestConfig->NumBytes, pTestConfig->SectorAddress);
#endif
  // PP command doesn't support 1:2:2 or 1:1:2, but only 1:1:1 or 4:4:4 or 1:1:4
  // so we switch back to serial mode to program the flash
  am_widget_mspi_devices_config_t mode = am_devices_mspi_s25fs064s_mode_switch(pWidgetConfig->pDevHandle, &pDevConfig->MSPIConfig);
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_SectorTXBuffer,
                                           pTestConfig->SectorAddress,
                                           pTestConfig->NumBytes, true);
  am_devices_mspi_s25fs064s_mode_switch(pWidgetConfig->pDevHandle, &mode);

  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
    return false;
  }

  //
  // Read the data back into the RX buffer.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %d\n", pTestConfig->NumBytes, pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer, pTestConfig->SectorAddress, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
    return false;
  }

  //
  // Compare the buffers
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("TX and RX buffers failed to compare!\n");
      return false;
    }
  }

  //
  // Erase the target sector.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %d\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase Flash Device sector!\n");
    return false;
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif
  return true;
}
#endif

// XIP Instruction bus execute test.
bool am_widget_mspi_test_xip_instrbus(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  mspi_xip_test_funcinfo_t  funcInfo;
  uint32_t                  result;
  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  get_test_function(&funcInfo);
  //
  // Cast a pointer to the begining of the sector as the test function to call.
  //
  mspi_xip_test_function_t test_function = (mspi_xip_test_function_t)((g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + funcInfo.funcOffset) | 0x00000001);

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  //
  // Erase the target sector.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  //
  // Set up for XIP operation.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Putting the MSPI and External Flash into XIP mode\n");
#endif
  ui32Status = mspi_device_func.mspi_xip_enable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    return false;
  }

  //
  // Write the TX buffer into the target sector.
  //
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", funcInfo.binSize, pTestConfig->SectorAddress);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           (uint8_t *)funcInfo.binAddr,
                                           pTestConfig->SectorAddress,
                                           funcInfo.binSize, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

    //
    // Execute a call to the test function in the sector.
    //
    result = test_function(funcInfo.param0, funcInfo.param1, funcInfo.param2);
    if (funcInfo.binSize != MSPI_XIP_FUNCTION_SIZE)
    {
        if (result != funcInfo.result)
        {
            am_util_stdio_printf("XIP function returned wrong result - Expected 0x%x, Actual 0x%x\n", funcInfo.result, result);
            mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
            return false;
        }
    }

  // Shutdown XIP operation.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
#endif
  ui32Status = mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    return false;
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif
  return true;
}

// Scrambling test.
bool am_widget_mspi_test_scrambling(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32MiscompareCount;
  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[i] = (i & 0xFF);
  }
  am_hal_sysctrl_bus_write_flush();

  // Turn on scrambling.
  ui32Status = mspi_device_func.mspi_scrambling_enable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable scrambling. Status = %d\n", ui32Status);
    return false;
  }

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the 1st scrambling region sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_SectorTXBuffer,
                                           pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer, g_SectorTXBuffer))
  {
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the unscrambled test sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pTestConfig->UnscrambledSector);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->UnscrambledSector);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pTestConfig->UnscrambledSector);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_SectorTXBuffer,
                                           pTestConfig->UnscrambledSector,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pTestConfig->UnscrambledSector);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer + pTestConfig->UnscrambledSector, pTestConfig->UnscrambledSector, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  if (!am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->UnscrambledSector, g_SectorTXBuffer))
  {
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  // Turn off Scrambling.
  ui32Status = mspi_device_func.mspi_scrambling_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable Scrambling. Status = %d\n", ui32Status);
    return false;
  }


  // Read the scrambled data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  ui32MiscompareCount = 0;
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      ui32MiscompareCount++;
    }
  }
  if (ui32MiscompareCount <= (0.9 * pTestConfig->NumBytes))
  {
    am_util_stdio_printf("Scrambled data too closely compared to test data.  %d of %d bytes were the same!\n", ui32MiscompareCount, pTestConfig->NumBytes);
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  // Read the unscrambled data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pTestConfig->UnscrambledSector);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer + pTestConfig->UnscrambledSector, pTestConfig->UnscrambledSector, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  if (!am_widget_mspi_check_buffer(pTestConfig->NumBytes, g_SectorRXBuffer + pTestConfig->UnscrambledSector, g_SectorTXBuffer))
  {
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  // Compare the received data in scrambled and unscrambled sector, they are not supposed to match
  ui32Status = memcmp(g_SectorRXBuffer, g_SectorRXBuffer + pTestConfig->UnscrambledSector, pTestConfig->NumBytes);
  if(ui32Status == 0)
  {
    return false;
  }

  // Turn on scrambling.
  ui32Status = mspi_device_func.mspi_scrambling_enable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    return false;
  }

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the 1st scrambling region sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_SectorTXBuffer,
                                           pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  // Erase the last scrambling region sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Erasing Sector %8X\n", pWidgetConfig->MSPIConfig.ui32ScramblingEndAddr);
#endif
  ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pWidgetConfig->MSPIConfig.ui32ScramblingEndAddr);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }
#endif

  // Write the TX buffer into the target sector.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingEndAddr);
#endif
  ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_SectorTXBuffer,
                                           pWidgetConfig->MSPIConfig.ui32ScramblingEndAddr,
                                           pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Turn off scrambling.
  ui32Status = mspi_device_func.mspi_scrambling_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    return false;
  }


  AM_UPDATE_TEST_PROGRESS();

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer, pWidgetConfig->MSPIConfig.ui32ScramblingStartAddr, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pWidgetConfig->MSPIConfig.ui32ScramblingEndAddr);
#endif
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer2, pWidgetConfig->MSPIConfig.ui32ScramblingEndAddr, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  ui32MiscompareCount = 0;
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      ui32MiscompareCount++;
    }
  }
  if (ui32MiscompareCount <= (0.9 * pTestConfig->NumBytes))
  {
    am_util_stdio_printf("Scrambled data from two different sectors too closely compared.  %d of %d bytes were the same!\n", ui32MiscompareCount, pTestConfig->NumBytes);
    return false;
  }


#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif
  return true;
}

//*****************************************************************************
//
// XIPMM test widget section
//
//*****************************************************************************

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
__asm static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
  push    {r3-r10}              // Save r3-r10 - used by this function
__octal_copy_loop
  ldmia r0!, {r3-r10}           // Load 8 words to registers
  stmia r1!, {r3-r10}           // Store 8 words from registers
  subs    r2, r2, #1
  bne     __octal_copy_loop
  pop     {r3-r10}              // Restore registers
  bx      lr
}
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
__attribute__((naked))
static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
__asm
    (
     "    push    {r3-r10}\n\t"              // Save r3-r10 - used by this function
     "__octal_copy_loop:\n\t"
     "    ldmia r0!, {r3-r10}\n\t"           // Load 8 words to registers
     "    stmia r1!, {r3-r10}\n\t"           // Store 8 words from registers
     "    subs    r2, r2, #1\n\t"
     "    bne     __octal_copy_loop\n\t"
     "    pop     {r3-r10}\n\t"              // Restore registers
     "    bx      lr\n\t"
    );
}
#elif defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
__asm
    (
     "    push    {r3-r10}\n\t"              // Save r3-r10 - used by this function
     "__octal_copy_loop:\n\t"
     "    ldmia r0!, {r3-r10}\n\t"           // Load 8 words to registers
     "    stmia r1!, {r3-r10}\n\t"           // Store 8 words from registers
     "    subs    r2, r2, #1\n\t"
     "    bne     __octal_copy_loop\n\t"
     "    pop     {r3-r10}\n\t"              // Restore registers
     "    bx      lr\n\t"
    );
}
#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
__asm volatile (
                "    push    {r3-r10}\n"              // Save r3-r10 - used by this function
                "__octal_copy_loop:\n"
                "    ldmia r0!, {r3-r10}\n"           // Load 8 words to registers
                "    stmia r1!, {r3-r10}\n"           // Store 8 words from registers
                "    subs    r2, r2, #1\n"
                "    bne     __octal_copy_loop\n"
                "    pop     {r3-r10}\n"              // Restore registers
                "    bx      lr\n"
               );
}
#else
#error "Compiler Not supported"
#endif

#if defined (COLLECT_BANDWIDTH)

bool
am_widget_mspi_xipmm_bandwidth(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t ix;
  uint32_t *pu32Ptr;
  uint16_t *pu16Ptr;
  uint8_t *pu8Ptr;
  uint32_t u32Val;
  uint16_t u16Val;
  uint8_t u8Val;

  uint32_t ui32TimerTickBeforeWrite = 0;
  uint32_t ui32TimerTickAfterWrite = 0;
  uint32_t ui32TimerTickBeforeRead = 0;
  uint32_t ui32TimerTickAfterRead = 0;
  uint32_t num_of_ms_write,num_of_ms_read;

  bool pass = true;

  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  uint32_t ui32Status = mspi_device_func.mspi_xip_enable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    return false;
  }

  pu16Ptr = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase);
  pu8Ptr = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase);

  switch(pTestConfig->eXIPMMAccess)
  {
    case MSPI_XIPMM_MEMCPY_ACCESS:
    {
      if(pTestConfig->pTxBuffer == NULL || pTestConfig->pRxBuffer == NULL)
      {
        return false;
      }

#if !defined(AM_WIDGET_MSPI_FLASH_DEVICE)
      uint8_t *pu8TxPtr = (uint8_t *)(pTestConfig->pTxBuffer);
      for(ix = 0; ix < pTestConfig->NumBytes; ix++)
      {
        *pu8TxPtr++ = ix;
      }

      num_of_ms_write = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        am_hal_sysctrl_bus_write_flush();
        pu32Ptr = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
        memcpy(pu32Ptr, pTestConfig->pTxBuffer, pTestConfig->NumBytes);
        ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        num_of_ms_write += am_widget_mspi_cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
      }
#endif
      am_hal_sysctrl_bus_write_flush();

      num_of_ms_read = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        pu32Ptr = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
        memcpy(pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop, pu32Ptr, pTestConfig->NumBytes);
        ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        num_of_ms_read += am_widget_mspi_cal_time(ui32TimerTickBeforeRead,ui32TimerTickAfterRead);
      }

      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop, pTestConfig->pTxBuffer))
        {
          am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
          pass = false;
        }
      }
      break;
    }

    case MSPI_XIPMM_WORD_ACCESS:
    {
#if !defined(AM_WIDGET_MSPI_FLASH_DEVICE)
      num_of_ms_write = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        am_hal_sysctrl_bus_write_flush();
        pu32Ptr = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        if(pTestConfig->pTxBuffer == NULL)
        {
          ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
          {
            *pu32Ptr++ = ix;
          }
          ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        }
        else
        {
          uint32_t *pu32TxPtr = (uint32_t *)(pTestConfig->pTxBuffer);
          uint32_t NumBytes = pTestConfig->NumBytes/4;
          for(ix = 0; ix < NumBytes; ix++)
          {
            *pu32TxPtr++ = ix;
          }
          pu32TxPtr = (uint32_t *)(pTestConfig->pTxBuffer);

          ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
          {
            *pu32Ptr++ = *pu32TxPtr++;
          }
          ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        }
        num_of_ms_write += am_widget_mspi_cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
      }
#endif
      am_hal_sysctrl_bus_write_flush();

      num_of_ms_read = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        pu32Ptr = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        if(pTestConfig->pRxBuffer == NULL)
        {
          ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
          {
            u32Val = *pu32Ptr++;
          }
          ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
          if(u32Val == ix - 1)
          {
            am_util_stdio_printf("loop#%d OK\n",loop);
          }
        }
        else
        {
          uint32_t *pu32RXPtr = (uint32_t *)(pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop);
          uint32_t NumBytes = pTestConfig->NumBytes/4;
          ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < NumBytes; ix++)
          {
            *pu32RXPtr++ = *pu32Ptr++;
          }
          ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        }
        num_of_ms_read += am_widget_mspi_cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
      }

      //verify
      if(pTestConfig->pRxBuffer != NULL && pTestConfig->pTxBuffer != NULL)
      {
        for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
        {
          if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop, pTestConfig->pTxBuffer))
          {
            am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
            pass = false;
          }
        }
      }
      else if(pTestConfig->pRxBuffer == NULL && pTestConfig->pTxBuffer == NULL)
      {
        for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
        {
          pu32Ptr = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
          for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
          {
            if((*pu32Ptr++ != ix))
            {
              break;
            }
          }
          if(ix != pTestConfig->NumBytes/4)
          {
            am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
            pass = false;
          }
        }
      }
      else
      {
        am_util_stdio_printf("Warning: Read & Write data not checked\n");
      }
      break;
    }

    case MSPI_XIPMM_SHORT_ACCESS:
    {
#if !defined(AM_WIDGET_MSPI_FLASH_DEVICE)
      num_of_ms_write = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        am_hal_sysctrl_bus_write_flush();
        pu16Ptr = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        if(pTestConfig->pTxBuffer == NULL)
        {
          ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
          {
            *pu16Ptr++ = ix;
          }
          ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        }
        else
        {
          uint16_t *pu16TxPtr = (uint16_t *)(pTestConfig->pTxBuffer);
          for(ix = 0; ix < pTestConfig->NumBytes/2; ix++)
          {
            *pu16TxPtr++ = ix;
          }
          pu16TxPtr = (uint16_t *)(pTestConfig->pTxBuffer);

          ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
          {
            *pu16Ptr++ = *pu16TxPtr++;
          }
          ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        }
        num_of_ms_write += am_widget_mspi_cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
      }
#endif
      am_hal_sysctrl_bus_write_flush();

      num_of_ms_read = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        pu16Ptr = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        if(pTestConfig->pRxBuffer == NULL)
        {
          ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
          {
            u16Val = *pu16Ptr++;
          }
          ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
          if(u16Val == (uint16_t)((ix -1) & 0x0000FFFF))
          {
            am_util_stdio_printf("loop#%d OK\n",loop);
          }
        }
        else
        {
          uint16_t *pu16RXPtr = (uint16_t *)(pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop);
          ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
          {
            *pu16RXPtr++ = *pu16Ptr++;
          }
          ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        }
        num_of_ms_read += am_widget_mspi_cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
      }

      //verify
      if(pTestConfig->pRxBuffer != NULL && pTestConfig->pTxBuffer != NULL)
      {
        for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
        {
          if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop, pTestConfig->pTxBuffer))
          {
            am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
            pass = false;
          }
        }
      }
      else if(pTestConfig->pRxBuffer == NULL && pTestConfig->pTxBuffer == NULL)
      {
        for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
        {
          pu16Ptr = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
          for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
          {
            u16Val = *pu16Ptr++;
            if(u16Val != (uint16_t)(ix & 0x0000FFFF))
            {
              break;
            }
          }
          if(ix != pTestConfig->NumBytes/2)
          {
            am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
            pass = false;
          }
        }
      }
      else
      {
        am_util_stdio_printf("Warning: Read & Write data not checked\n");
      }
      break;
    }

    case MSPI_XIPMM_BYTE_ACCESS:
    {
#if !defined(AM_WIDGET_MSPI_FLASH_DEVICE)
      num_of_ms_write = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        am_hal_sysctrl_bus_write_flush();
        pu8Ptr = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        if(pTestConfig->pTxBuffer == NULL)
        {
          ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes; ix++)
          {
            *pu8Ptr++ = ix;
          }
          ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        }
        else
        {
          uint8_t *pu8TxPtr = (uint8_t *)(pTestConfig->pTxBuffer);
          for(ix = 0; ix < pTestConfig->NumBytes; ix++)
          {
            *pu8TxPtr++ = ix;
          }
          pu8TxPtr = (uint8_t *)(pTestConfig->pTxBuffer);

          ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes; ix++)
          {
            *pu8Ptr++ = *pu8TxPtr++;
          }
          ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        }
        num_of_ms_write += am_widget_mspi_cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
      }
#endif
      am_hal_sysctrl_bus_write_flush();

      num_of_ms_read = 0;
      for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
      {
        pu8Ptr = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
        if(pTestConfig->pRxBuffer == NULL)
        {
          ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes; ix++)
          {
            u8Val = *pu8Ptr++;
          }
          ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
          if(u8Val == (uint8_t)((ix - 1) & 0x000000FF))
          {
            am_util_stdio_printf("loop#%d OK\n",loop);
          }
        }
        else
        {
          uint8_t *pu8RXPtr = (uint8_t *)(pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop);
          ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
          for (ix = 0; ix < pTestConfig->NumBytes; ix++)
          {
            *pu8RXPtr++ = *pu8Ptr++;
          }
          ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        }
        num_of_ms_read += am_widget_mspi_cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
      }

      //verify
      if(pTestConfig->pRxBuffer != NULL && pTestConfig->pTxBuffer != NULL)
      {
        for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
        {
          if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer + pTestConfig->ByteOffset * loop, pTestConfig->pTxBuffer))
          {
            am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
            pass = false;
          }
        }
      }
      else if(pTestConfig->pRxBuffer == NULL && pTestConfig->pTxBuffer == NULL)
      {
        for(uint32_t loop = 0; loop < BANDWIDTH_TEST_LOOP; loop++)
        {
          pu8Ptr = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset * loop);
          for (ix = 0; ix < pTestConfig->NumBytes; ix++)
          {
            u8Val = *pu8Ptr++;
            if(u8Val != (uint8_t)(ix & 0x000000FF))
            {
              break;
            }
          }
          if(ix != pTestConfig->NumBytes)
          {
            am_util_stdio_printf("Loop#%d, Read & Write buffer doesn't match\n", loop);
            pass = false;
          }
        }
      }
      else
      {
        am_util_stdio_printf("Warning: Read & Write data not checked\n");
      }
      break;
    }

    default:
      am_util_stdio_printf("Collect write bandwidth function undefined!\n");
      pass = false;
      break;
  }

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
  pTestConfig->bandwidth.f32WriteBandwidth = 0.0;
#else
  pTestConfig->bandwidth.f32WriteBandwidth = (float)pTestConfig->NumBytes * BANDWIDTH_TEST_LOOP / num_of_ms_write / TIME_DIVIDER;
#endif
  pTestConfig->bandwidth.f32ReadBandwidth = (float)pTestConfig->NumBytes * BANDWIDTH_TEST_LOOP / num_of_ms_read / TIME_DIVIDER;

  am_util_stdio_printf("Write: %d bytes, Bandwidth is %f MB/s\n", pTestConfig->NumBytes, pTestConfig->bandwidth.f32WriteBandwidth);
  am_util_stdio_printf("Read:  %d bytes, Bandwidth is %f MB/s\n", pTestConfig->NumBytes, pTestConfig->bandwidth.f32ReadBandwidth);

  // Need to make sure all pending XIPMM transactions are flushed
  am_hal_cachectrl_dcache_invalidate(NULL, true);

  ui32Status = mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

  return pass;
}
#endif

bool
am_widget_mspi_test_xipmm(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t ix;
  uint32_t *pu32Ptr1, *pu32Ptr2;
  uint32_t u32Val1, u32Val2;
  uint16_t *pu16Ptr1, *pu16Ptr2;
  uint16_t u16Val1, u16Val2;
  uint8_t *pu8Ptr1, *pu8Ptr2;
  uint8_t u8Val1, u8Val2;

  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  ui32Status = mspi_device_func.mspi_xip_enable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    return false;
  }

  switch (pTestConfig->eXIPMMAccess)
  {
  case MSPI_XIPMM_MEMCPY_ACCESS:
    {
      if(pTestConfig->pTxBuffer == NULL || pTestConfig->pRxBuffer == NULL)
      {
        return false;
      }

      uint8_t *pu8TxPtr = (uint8_t *)(pTestConfig->pTxBuffer);
      for(ix = 0; ix < pTestConfig->NumBytes; ix++)
      {
        *pu8TxPtr++ = ix;
      }

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);

			memset(pu32Ptr1, 0, pTestConfig->NumBytes);
      memcpy(pu32Ptr1, pTestConfig->pTxBuffer, pTestConfig->NumBytes);
      memcpy(pTestConfig->pRxBuffer, pu32Ptr1, pTestConfig->NumBytes);

      if(!am_widget_mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer, pTestConfig->pTxBuffer))
      {
        return false;
      }
    }
    break;

  case MSPI_XIPMM_WORD_ACCESS:
    {
      // Word Read, Word Write
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
    }
    break;

  case MSPI_XIPMM_SHORT_ACCESS:
    {
      // Half word Write, Half word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        *pu16Ptr1++ = (uint16_t)(ix & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((ix & 0xFFFF) ^ 0xFFFF);
      }

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        if ( (*pu16Ptr1++ != (uint16_t)(ix & 0xFFFF)) || (*pu16Ptr2++ != (uint16_t)((ix & 0xFFFF) ^ 0xFFFF)) )
        {
          return false;
        }
      }
      // Half word Write, word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = ix;
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
      // word Write, Half word Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = *pu16Ptr1++;
        u32Val1 |= (*pu16Ptr1++ << 16);
        u32Val2 = *pu16Ptr2++;
        u32Val2 |= (*pu16Ptr2++ << 16);
        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }

    }
    break;

  case MSPI_XIPMM_BYTE_ACCESS:
    {
      // Byte Write, Byte Read
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        *pu8Ptr1++ = (uint8_t)(ix & 0xFF);
        *pu8Ptr2++ = (uint8_t)((ix & 0xFF) ^ 0xFF);
      }

      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        if ( (*pu8Ptr1++ != (uint8_t)(ix & 0xFF)) || (*pu8Ptr2++ != (uint8_t)((ix & 0xFF) ^ 0xFF)) )
        {
          return false;
        }
      }
      // Byte Write, Half word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        u16Val1 = ((((2*ix + 1) & 0xFF) << 8) | ((2*ix) & 0xFF));
        u16Val2 = (((((2*ix + 1) & 0xFF) ^ 0xFF) << 8) | (((2*ix) & 0xFF) ^ 0xFF));
        if ( (*pu16Ptr1++ != u16Val1) || (*pu16Ptr2++ != u16Val2))
        {
          return false;
        }
      }
      // Byte Write, word Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = ((((4*ix + 3) & 0xFF) << 24) |(((4*ix + 2) & 0xFF) << 16) |(((4*ix + 1) & 0xFF) << 8) | ((4*ix) & 0xFF));
        u32Val2 = (((((4*ix + 3) & 0xFF) ^ 0xFF) << 24) |((((4*ix + 2) & 0xFF) ^ 0xFF) << 16) |((((4*ix + 1) & 0xFF) ^ 0xFF) << 8) | (((4*ix) & 0xFF) ^ 0xFF));
        if ( (*pu32Ptr1++ != u32Val1) || (*pu32Ptr2++ != u32Val2))
        {
          return false;
        }
      }
      // word Write, Byte Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = *pu8Ptr1++;
        u32Val1 |= (*pu8Ptr1++ << 8);
        u32Val1 |= (*pu8Ptr1++ << 16);
        u32Val1 |= (*pu8Ptr1++ << 24);
        u32Val2 = *pu8Ptr2++;
        u32Val2 |= (*pu8Ptr2++ << 8);
        u32Val2 |= (*pu8Ptr2++ << 16);
        u32Val2 |= (*pu8Ptr2++ << 24);
        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Half word Write, Byte Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        *pu16Ptr1++ = (uint16_t)(ix & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((ix & 0xFFFF) ^ 0xFFFF);
      }
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        u16Val1 = *pu8Ptr1++;
        u16Val1 |= (*pu8Ptr1++ << 8);
        u16Val2 = *pu8Ptr2++;
        u16Val2 |= (*pu8Ptr2++ << 8);
        if ( (u16Val1 != (uint16_t)(ix & 0xFFFF)) || (u16Val2 != (uint16_t)((ix & 0xFFFF) ^ 0xFFFF)) )
        {
          return false;
        }
      }
    }
    break;
  case MSPI_XIPMM_SHORT_ACCESS_UNALIGNED:
    {
      // Write aligned, read unaligned
      // Write 4 pages
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = ix;
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      u8Val1 = 0;
      u8Val2 = 0xFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = (*pu16Ptr1++) << 8 | u8Val1;
        u16Val1 = *pu16Ptr1++;
        u32Val1 |= (u16Val1 & 0xFF) << 24;
        u8Val1 = u16Val1 >> 8;
        u32Val2 = (*pu16Ptr2++) << 8 | u8Val2;
        u16Val2 = *pu16Ptr2++;
        u32Val2 |= (u16Val2 & 0xFF) << 24;
        u8Val2 = u16Val2 >> 8;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write unaligned, read aligned
      // Write 4 pages
      *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4)) = 0;
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1);
      u16Val1 = 0xAABB;
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4);
      am_util_debug_printf("Writing Half Word 0x%04x to address 0x%08x\n", u16Val1, pu16Ptr1);
      *pu16Ptr1 = u16Val1;
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4);

      *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4)) = 0;
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1);
      u32Val1 = 0xAABBCCDD;
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4);
      am_util_debug_printf("Writing Word 0x%08x to address 0x%08x\n", u32Val1, pu32Ptr1);
      *pu32Ptr1 = u32Val1;
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 4);

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 8) | ((ix + 1) << 24);
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 8 | (((ix + 1) ^ 0xFFFFFFFF) << 24);
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }
      *((uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFF;

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu16Ptr1++;
        u32Val1 |= (*pu16Ptr1++ << 16);
        u32Val2 = *pu16Ptr2++;
        u32Val2 |= (*pu16Ptr2++ << 16);

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
    }
    break;
  case MSPI_XIPMM_WORD_ACCESS_UNALIGNED:
    {
      // Write aligned, read unaligned
      // Write 4 pages
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = ix;
        *pu32Ptr1++ = u32Val1;
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu32Ptr2++ = u32Val2;
      }

      // Read at offset 1
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      u32Val1 = 0;
      u32Val2 = 0xFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        uint32_t tempVal1, tempVal2;
        tempVal1 = *pu32Ptr1++;
        u32Val1 |= tempVal1 << 8;
        tempVal2 = *pu32Ptr2++;
        u32Val2 |= tempVal2 << 8;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
        u32Val1 = tempVal1 >> 24;
        u32Val2 = tempVal2 >> 24;
      }
      // Read at offset 2
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2 + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      u32Val1 = 0;
      u32Val2 = 0xFFFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        uint32_t tempVal1, tempVal2;
        tempVal1 = *pu32Ptr1++;
        u32Val1 |= tempVal1 << 16;
        tempVal2 = *pu32Ptr2++;
        u32Val2 |= tempVal2 << 16;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
        u32Val1 = tempVal1 >> 16;
        u32Val2 = tempVal2 >> 16;
      }
      // Read at offset 3
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 3);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 3 + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      u32Val1 = 0;
      u32Val2 = 0xFFFFFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        uint32_t tempVal1, tempVal2;
        tempVal1 = *pu32Ptr1++;
        u32Val1 |= tempVal1 << 24;
        tempVal2 = *pu32Ptr2++;
        u32Val2 |= tempVal2 << 24;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
        u32Val1 = tempVal1 >> 8;
        u32Val2 = tempVal2 >> 8;
      }
      // Write unaligned, read aligned
      // Write 4 pages
      // Write at offset 1
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 8) | ((ix + 1) << 24);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 8 | (((ix + 1) ^ 0xFFFFFFFF) << 24);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write at offset 2
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 16) | ((ix + 1) << 16);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 16 | (((ix + 1) ^ 0xFFFFFFFF) << 16);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFFFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write at offset 3
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 3);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 3 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 24) | ((ix + 1) << 8);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 24 | (((ix + 1) ^ 0xFFFFFFFF) << 8);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint16_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFFFF;
      *((uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2)) = 0;
      *((uint8_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2 + 2*pTestConfig->NumBytes)) = 0xFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
    }
    break;
  case MSPI_XIPMM_OWORD_ACCESS:
    {

      uint32_t pattern[16];
      uint32_t pattern2[16];
      uint32_t addr = g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase;

      // Initialize a pattern
      for (ix = 0; ix < 8; ix++)
      {
        pattern[ix] = ix;
        pattern[ix+8] = ix ^ 0xFFFFFFFF;
      }

      // Octal Word Write
      pu32Ptr1 = (uint32_t *)(addr + pTestConfig->ByteOffset);
      octal_copy((uint32_t)pattern, addr + pTestConfig->ByteOffset, 2);

      octal_copy(addr + pTestConfig->ByteOffset, (uint32_t)pattern2, 2);
      pu32Ptr1 = (uint32_t *)(pattern2);
      pu32Ptr2 = (uint32_t *)(pattern2 + 8);

      am_widget_mspi_power_save_and_restore(pWidget, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < 8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
    }
    break;
    default:break;
  }

  // Need to make sure all pending XIPMM transactions are flushed
  am_hal_cachectrl_dcache_invalidate(NULL, true);

  ui32Status = mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

  return true;
}

static int am_widget_mspi_prepare_test_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len)
{
    uint32_t *pui32TxPtr = (uint32_t*)buff;
    uint8_t  *pui8TxPtr  = (uint8_t*)buff;
    //
    // length has to be multiple of 4 bytes
    //
    if ( len % 4 )
    {
        return -1;
    }

    switch ( pattern_index )
    {
        case 0:
            //
            // 0x5555AAAA
            //
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0x5555AAAA);
            }
            break;
        case 1:
            //
            // 0xFFFF0000
            //
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0xFFFF0000);
            }
            break;
        case 2:
            //
            // walking
            //
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = 0x01 << (i % 8);
            }
            break;
        case 3:
            //
            // incremental from 1
            //
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;
        case 4:
            //
            // decremental from 0xff
            //
            for ( uint32_t i = 0; i < len; i++ )
            {
                //
                // decrement starting from 0xff
                //
                pui8TxPtr[i] = (0xff - i) & 0xFF;
            }
            break;
        default:
            //
            // incremental from 1
            //
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;

    }

    return 0;
}

//*****************************************************************************
//
//! @brief Count the longest consecutive 1s in a 32bit word
//! @details Static helper function:
//!
//! @param pVal
//!
//! @return
//
//*****************************************************************************
static uint32_t
count_consecutive_ones(uint32_t* pVal)
{
    uint32_t count = 0;
    uint32_t data = *pVal;

    while ( data )
    {
        data = (data & (data << 1));
        count++;
    }
    return count;
}

//*****************************************************************************
//
//! @brief Find and return the mid point of the longest continuous 1s in a 32bit word
//! @details Static helper function:
//!
//! @param pVal
//!
//! @return
//
//*****************************************************************************
static uint32_t
find_mid_point(uint32_t* pVal)
{
    uint32_t pattern_len = 0;
    uint32_t max_len = 0;
    uint32_t pick_point = 0;
    bool pattern_start = false;
    uint32_t val = *pVal;
    uint8_t remainder = 0;
    bool pick_point_flag = false;

    for ( uint32_t i = 0; i < 32; i++ )
    {
        if ( val & (0x01 << i) )
        {
            pattern_start = true;
            pattern_len++;
        }
        else
        {
            if ( pattern_start == true )
            {
                pattern_start = false;
                pick_point_flag = true;
            }
        }
        if ( (i == 31) && ( pattern_start == true ) )
        {
            pick_point_flag = true;
        }

        if (pick_point_flag == true)
        {
            if ( pattern_len > max_len )
            {
                max_len = pattern_len;
                pick_point = i - 1 - pattern_len / 2;
                remainder = pattern_len % 2;
            }
            pattern_len = 0;
            pick_point_flag = false;
        }
    }

    //
    // check the passing window side
    //
// #### INTERNAL BEGIN ####
//
// HSP20-339: Try to locate and move the middle value found one bit further away from the failure window.
// In our current tests, most of the time, we see the failure window is in the middle of 1-30 range.
// Passing windows are likely to be on either end of the 1-30 range.
// Therefore, adding comparison here, if we see the middle point is below 16, it is likely that we found
// a passing window starting from 1 at the lower side, if it is true (setting 1 passed),
// we further move the middle value to the lower side by 1.
// And the same for the higher values.
//
// #### INTERNAL END ####

    if ( (pick_point < 16) && (val & 0x00000002) )
    {
        // window is likely on low side
        pick_point = pick_point - remainder;    // minus only when pattern length is odd
    }
    else if ( (pick_point > 15) && (val & 0x40000000) )
    {
        // window is likely on high side
        pick_point = pick_point + 1;
    }
    else
    {
        // window is in the middle, no action
    }

    return pick_point;
}

#if defined(AM_WIDGET_MSPI_FAST_SCAN)
bool bFirst = false;
#endif

bool am_widget_mspi_write_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t ui32Status;

  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t   *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  switch (pTestConfig->eMSPIAccess)
  {
    case AM_WIDGET_MSPI_PIO_ACCESS:
    {
      //TX
      am_util_debug_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pTestConfig->SectorAddress + pTestConfig->SectorOffset);
      am_hal_mspi_pio_transfer_t  Transaction;
      Transaction.ui32NumBytes       = pTestConfig->NumBytes;
      Transaction.bScrambling        = false;
      Transaction.eDirection         = AM_HAL_MSPI_TX;
      Transaction.bSendAddr          = true;
      Transaction.ui32DeviceAddr     = pTestConfig->SectorAddress + pTestConfig->SectorOffset;
      Transaction.bSendInstr         = true;
      Transaction.ui16DeviceInstr    = MSPIn(pWidgetConfig->ui32Module)->DEV0INSTR_b.WRITEINSTR0;
      Transaction.bTurnaround        = false;
      Transaction.bDCX               = false;
      Transaction.bEnWRLatency       = true;
      Transaction.bContinue          = false;
      Transaction.pui32Buffer        = (uint32_t *)&pTestConfig->pTxBuffer[pTestConfig->ByteOffset];
      ui32Status = am_hal_mspi_blocking_transfer(pWidgetConfig->pHandle, &Transaction, AM_WIDGET_MSPI_TIMEOUT);
      if(AM_HAL_STATUS_SUCCESS != ui32Status)
      {
          am_util_debug_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
          return false;
      }
      //RX
      am_util_debug_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes, pTestConfig->SectorAddress + pTestConfig->SectorOffset);
      Transaction.ui32NumBytes       = pTestConfig->NumBytes;
      Transaction.bScrambling        = false;
      Transaction.eDirection         = AM_HAL_MSPI_RX;
      Transaction.bSendAddr          = true;
      Transaction.ui32DeviceAddr     = pTestConfig->SectorAddress + pTestConfig->SectorOffset;
      Transaction.bSendInstr         = true;
      Transaction.ui16DeviceInstr    = MSPIn(pWidgetConfig->ui32Module)->DEV0INSTR_b.READINSTR0;
      Transaction.bTurnaround        = true;
      Transaction.bDCX               = false;
      Transaction.bEnWRLatency       = false;
      Transaction.bContinue          = false;
      Transaction.pui32Buffer        = (uint32_t *)&pTestConfig->pRxBuffer[pTestConfig->ByteOffset];
      ui32Status = am_hal_mspi_blocking_transfer(pWidgetConfig->pHandle, &Transaction, AM_WIDGET_MSPI_TIMEOUT);
      if(AM_HAL_STATUS_SUCCESS != ui32Status)
      {
          am_util_debug_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
          return false;
      }
      // Compare the buffers
      am_util_debug_printf("Comparing the TX and RX Buffers\n");
      ui32Status = memcmp(pTestConfig->pRxBuffer + pTestConfig->ByteOffset, pTestConfig->pTxBuffer + pTestConfig->ByteOffset, pTestConfig->NumBytes);
      if(ui32Status == 0)
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    case AM_WIDGET_MSPI_DMA_ACCESS:
    {

#if defined(AM_WIDGET_MSPI_FAST_SCAN)
      if(!bFirst)
      {
        bFirst = true;
#endif

#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
        // Erase the target sector.
        for ( uint8_t i = 0; i < (pTestConfig->NumBytes / AM_DEVICES_MSPI_FLASH_SECTOR_SIZE); i++ )
        {
          am_util_debug_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress + i * AM_DEVICES_MSPI_FLASH_SECTOR_SIZE);

          ui32Status = mspi_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                          pTestConfig->SectorAddress + i * AM_DEVICES_MSPI_FLASH_SECTOR_SIZE);
          if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
          {
            am_util_debug_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
            return false;
          }
        }
#endif

        // Write the TX buffer into the target sector.
        am_util_debug_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes,
                                                                pTestConfig->SectorAddress + pTestConfig->SectorOffset);

        ui32Status = mspi_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                                &pTestConfig->pTxBuffer[pTestConfig->ByteOffset],
                                                pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                                pTestConfig->NumBytes, true);
        if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
        {
          am_util_debug_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
          return false;
        }
#if defined(AM_WIDGET_MSPI_FAST_SCAN)
      }
#endif


      // Read the data back into the RX buffer.
      am_util_debug_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes,
                          pTestConfig->SectorAddress + pTestConfig->SectorOffset);

      ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, &pTestConfig->pRxBuffer[pTestConfig->ByteOffset],
                              pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                              pTestConfig->NumBytes, true);
      if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
      {
        am_util_debug_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
        return false;
      }

      // Compare the buffers
      am_util_debug_printf("Comparing the TX and RX Buffers\n");
      ui32Status = memcmp(pTestConfig->pRxBuffer + pTestConfig->ByteOffset, pTestConfig->pTxBuffer + pTestConfig->ByteOffset, pTestConfig->NumBytes);
      if(ui32Status == 0)
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    case AM_WIDGET_MSPI_MEMCPY_XIPMM_ACCESS:
    {

      if(pTestConfig->pTxBuffer == NULL || pTestConfig->pRxBuffer == NULL)
      {
        return false;
      }

      ui32Status = mspi_device_func.mspi_xip_enable(pWidgetConfig->pDevHandle);
      if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
      {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
      }

      uint32_t *pu32Ptr1 = (uint32_t *)(pTestConfig->SectorAddress + pTestConfig->SectorOffset);

			memset(pu32Ptr1, 0, pTestConfig->NumBytes);
      am_hal_sysctrl_bus_write_flush();
      memcpy(pu32Ptr1, pTestConfig->pTxBuffer, pTestConfig->NumBytes);
      am_hal_sysctrl_bus_write_flush();
      memcpy(pTestConfig->pRxBuffer, pu32Ptr1, pTestConfig->NumBytes);

      // Need to make sure all pending XIPMM transactions are flushed
      am_hal_cachectrl_dcache_invalidate(NULL, true);

      ui32Status = mspi_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
      if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
      {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
        return false;
      }

      ui32Status = memcmp(pTestConfig->pRxBuffer, pTestConfig->pTxBuffer, pTestConfig->NumBytes);
      if(ui32Status == 0)
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    default:
      return false;
  }
}

bool
am_widget_mspi_xip_range_test(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t ui32Status;
  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_xip_range_test_t   *pTestConfig = (am_widget_mspi_xip_range_test_t *)pTestCfg;

  am_widget_mspi_test_t testConfig;

  am_hal_mspi_xip_config_t xipConfig;
  xipConfig.scramblingStartAddr = pTestConfig->ui32ScramblingStartAddr;
  xipConfig.scramblingEndAddr = pTestConfig->ui32ScramblingEndAddr;

  xipConfig.eAPMode = pTestConfig->eAPMode;
  xipConfig.eAPSize = pTestConfig->eAPSize;
#if defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5A)
  uint32_t ui32TotalSize = 64*1024*1024;
#else
  uint32_t ui32TotalSize = g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMEnd - g_MSPIAddresses[pWidgetConfig->ui32Module].XIPMMBase;
#endif

  if(pTestConfig->bScramble)
  {
    // Turn on scrambling.
    ui32Status = mspi_device_func.mspi_scrambling_enable(pWidgetConfig->pDevHandle);
    if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to enable scrambling. Status = %d\n", ui32Status);
      return false;
    }
  }

  for(uint32_t i = 0; i < ui32TotalSize / pTestConfig->ui32NumBytes; ++i)
  {
    srand(am_hal_timer_read(TIMER_NUM));
    xipConfig.ui32APBaseAddr = pTestConfig->ui32APStartAddr + i * pTestConfig->ui32NumBytes;
    if (AM_WIDGET_MSPI_SUCCESS != mspi_device_func.mspi_xip_config(pWidgetConfig->pDevHandle, &xipConfig))
    {
      am_util_stdio_printf("Failed to configure XIP in the MSPI!\n");
      return false;
    }
    am_util_stdio_printf("Set region starting from: %08X\n", xipConfig.ui32APBaseAddr);

    testConfig.SectorOffset = 0;
    testConfig.pTxBuffer = g_SectorTXBuffer;
    testConfig.pRxBuffer = g_SectorRXBuffer;
    testConfig.eMSPIAccess = AM_WIDGET_MSPI_MEMCPY_XIPMM_ACCESS;
    for(uint32_t j = 0; j < pTestConfig->ui32NumBytes / sizeof(g_SectorTXBuffer); ++j)
    {
      testConfig.NumBytes = sizeof(g_SectorTXBuffer);
      testConfig.SectorAddress = xipConfig.ui32APBaseAddr + j * testConfig.NumBytes;

      am_widget_mspi_prepare_test_pattern(rand() % 5, testConfig.pTxBuffer, testConfig.NumBytes);
      am_hal_sysctrl_bus_write_flush();
      am_util_stdio_printf("  Target Address: %08X\n", testConfig.SectorAddress);
      if(!am_widget_mspi_write_read(pWidget, &testConfig, pErrStr))
      {
        return false;
      }
    }

  }

  if(pTestConfig->bScramble)
  {
    // Turn off Scrambling.
    ui32Status = mspi_device_func.mspi_scrambling_disable(pWidgetConfig->pDevHandle);
    if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to disable Scrambling. Status = %d\n", ui32Status);
      return false;
    }
  }

  return true;
}

#if defined (AM_DEVICES_MSPI_FLASH_PAGE_SIZE)
#define AM_WIDGET_DEVICE_PAGE_SIZE          AM_DEVICES_MSPI_FLASH_PAGE_SIZE
#elif defined (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)
#define AM_WIDGET_DEVICE_PAGE_SIZE          AM_DEVICES_MSPI_PSRAM_PAGE_SIZE
#endif
#define TIMING_SCAN_SIZE_BYTES              (128*AM_WIDGET_DEVICE_PAGE_SIZE)

bool
am_widget_mspi_timing_scan_data_check(void *pWidget, void *pTestCfg, char *pErrStr)
{
  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_timing_scan_t   *pTimingScanTestConfig = (am_widget_mspi_timing_scan_t *)pTestCfg;

  am_widget_mspi_test_t testConfig;

  uint32_t ui32NumberOfBytesLeft = TIMING_SCAN_SIZE_BYTES;
  uint32_t ui32TestBytes = 0;
  uint32_t ui32DataPattern = 0;
  uint8_t  *pui32TxBuffer = g_SectorTXBuffer;

  while ( ui32NumberOfBytesLeft )
  {
    if ( ui32NumberOfBytesLeft > AM_WIDGET_DEVICE_PAGE_SIZE )
    {
        ui32TestBytes = AM_WIDGET_DEVICE_PAGE_SIZE;
        ui32NumberOfBytesLeft -= AM_WIDGET_DEVICE_PAGE_SIZE;
    }
    else
    {
        ui32TestBytes = ui32NumberOfBytesLeft;
        ui32NumberOfBytesLeft = 0;
    }

    // Generate data into the Sector Buffer
    am_widget_mspi_prepare_test_pattern(ui32DataPattern % 5, pui32TxBuffer, ui32TestBytes);
    am_hal_sysctrl_bus_write_flush();
    pui32TxBuffer += ui32TestBytes;
    ui32DataPattern++;
  }

  testConfig.pTxBuffer = g_SectorTXBuffer;
  testConfig.pRxBuffer = g_SectorRXBuffer;
  testConfig.NumBytes = TIMING_SCAN_SIZE_BYTES;
  testConfig.SectorOffset = 0;
  testConfig.ByteOffset = 0;
  testConfig.SectorAddress = pTimingScanTestConfig->ui32SectorAddress;
  testConfig.eMSPIAccess = pTimingScanTestConfig->eMSPIAccess;

  if(!am_widget_mspi_write_read(pWidget, &testConfig, pErrStr))
  {
    return false;
  }

  return true;
}

bool
am_widget_mspi_timing_scan(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t ui32Status;

  uint32_t Txdqsdelay = 0;
  uint32_t Rxdqsdelay = 0;

  uint32_t ui32CCOResult = 0;
  uint32_t ui32TxResult = 0;
  uint32_t ui32RxResultArray[32];
  uint32_t ui32WindowMax = 0;
  //
  // Create a pointer to the MSPI Widget configuration.
  //
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_timing_scan_t   *pTestConfig = (am_widget_mspi_timing_scan_t *)pTestCfg;

  am_hal_mspi_timing_scan_t scanCfg = {0};

  //
  //get value configured by init & hal
  //
  ui32Status = am_hal_mspi_control(pWidgetConfig->pHandle, AM_HAL_MSPI_REQ_TIMING_SCAN_GET, &scanCfg);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
      return false;
  }

  pWidgetConfig->sTimingResult = scanCfg;

  uint32_t ui32TxNeg = scanCfg.bTxNeg;
  uint32_t ui32RxNeg = scanCfg.bRxNeg;
  uint32_t ui32RxCap = scanCfg.bRxCap;
  uint32_t ui32Turnaround = scanCfg.ui8Turnaround;
  uint32_t ui32SectorAddress = pTestConfig->ui32SectorAddress;

  if(!pTestConfig->bScanTxNeg)
  {
    goto INIT_TXNEG;
  }

  //LOOP_TXNEG:
  for(ui32TxNeg = pTestConfig->ui32ScanTxNegStart; ui32TxNeg <= pTestConfig->ui32ScanTxNegEnd; ui32TxNeg++)
  {
    INIT_TXNEG:
    scanCfg.bTxNeg = (bool)ui32TxNeg;

    if(!pTestConfig->bScanRxNeg)
    {
      goto INIT_RXNEG;
    }

    //LOOP_RXNEG:
    for(ui32RxNeg = pTestConfig->ui32ScanRxNegStart; ui32RxNeg <= pTestConfig->ui32ScanRxNegEnd; ui32RxNeg++)
    {
      INIT_RXNEG:
      scanCfg.bRxNeg = (bool)ui32RxNeg;

      if(!pTestConfig->bScanRxCap)
      {
        goto INIT_RXCAP;
      }

      //LOOP_RXCAP:
      for(ui32RxCap = pTestConfig->ui32ScanRxCapStart; ui32RxCap <= pTestConfig->ui32ScanRxCapEnd; ui32RxCap++)
      {
        INIT_RXCAP:
        scanCfg.bRxCap = (bool)ui32RxCap;

        if(!pTestConfig->bScanTurnAround)
        {
          goto INIT_SCAN;
        }

        //LOOP_TURNAROUND:
        for(scanCfg.ui8Turnaround = ui32Turnaround + pTestConfig->i32ScanTurnAroundStart; scanCfg.ui8Turnaround <= ui32Turnaround + pTestConfig->i32ScanTurnAroundEnd; scanCfg.ui8Turnaround++)
        {
          INIT_SCAN:
          am_util_stdio_printf("    TxNeg=%d, RxNeg=%d, RxCap=%d, Turnaround=%d\n", scanCfg.bTxNeg, scanCfg.bRxNeg, scanCfg.bRxCap, scanCfg.ui8Turnaround);
          ui32TxResult = 0;
          memset(ui32RxResultArray, 0, sizeof(ui32RxResultArray));

          if(!pTestConfig->bScanTxDQSDelay)
          {
            goto INIT_TXDQSDELAY;
          }

          //LOOP_TXDQSDELAY:
          for (scanCfg.ui8TxDQSDelay = pTestConfig->ui32ScanTxDQSDelayStart; scanCfg.ui8TxDQSDelay <= pTestConfig->ui32ScanTxDQSDelayEnd; scanCfg.ui8TxDQSDelay++)
          {
            INIT_TXDQSDELAY:
            if(!pTestConfig->bScanRxDQSDelay)
            {
              goto INIT_RXDQSDELAY;
            }

            //LOOP_RXDQSDELAY:
            for (scanCfg.ui8RxDQSDelay = pTestConfig->ui32ScanRxDQSDelayStart; scanCfg.ui8RxDQSDelay <= pTestConfig->ui32ScanRxDQSDelayEnd; scanCfg.ui8RxDQSDelay++)
            {
              INIT_RXDQSDELAY:
              //
              // apply settings
              //
              ui32Status = am_hal_mspi_control(pWidgetConfig->pHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, &scanCfg);
              if (AM_HAL_STATUS_SUCCESS != ui32Status)
              {
                  return false;
              }
              if(pTestConfig->eMSPIAccess == AM_WIDGET_MSPI_MEMCPY_XIPMM_ACCESS)
              {
                uint32_t ui32Offset = (scanCfg.bTxNeg + scanCfg.bRxNeg + scanCfg.bRxCap + scanCfg.ui8Turnaround) * TIMING_SCAN_SIZE_BYTES + (scanCfg.ui8TxDQSDelay + scanCfg.ui8RxDQSDelay) * 2;
                pTestConfig->ui32SectorAddress = ui32SectorAddress + ui32Offset;
              }
              //
              // run data check
              //
              if ( am_widget_mspi_timing_scan_data_check(pWidget, pTestCfg, pErrStr) )
              {
                  //
                  // data check pass
                  //
                  ui32RxResultArray[scanCfg.ui8TxDQSDelay] |= 0x01 << scanCfg.ui8RxDQSDelay;
              }
              else
              {
                  //
                  // data check failed
                  //
              }

              if(!pTestConfig->bScanRxDQSDelay)
              {
                break;
              }
            }

            if(pTestConfig->bScanRxDQSDelay)
            {
              ui32CCOResult = count_consecutive_ones(&ui32RxResultArray[scanCfg.ui8TxDQSDelay]);
              if ( ui32CCOResult >= pTestConfig->ui32TimingScanMinAcceptLen)
              {
                  ui32TxResult |= 0x01 << scanCfg.ui8TxDQSDelay;
              }
              am_util_stdio_printf("    TxDQSDelay: %d, RxDQSDelay Scan = 0x%08X, Window size = %d\n", scanCfg.ui8TxDQSDelay, ui32RxResultArray[scanCfg.ui8TxDQSDelay], ui32CCOResult);
            }
            else
            {
              if( ui32RxResultArray[scanCfg.ui8TxDQSDelay] != 0 )
              {
                  ui32TxResult |= 0x01 << scanCfg.ui8TxDQSDelay;
              }
              am_util_stdio_printf("    TxDQSDelay: %d, RxDQSDelay Scan = 0x%08X\n", scanCfg.ui8TxDQSDelay, ui32RxResultArray[scanCfg.ui8TxDQSDelay]);
            }

            if(!pTestConfig->bScanTxDQSDelay)
            {
              break;
            }
          }

          //
          // Find TXDQSDELAY Value
          //
          if(pTestConfig->bScanTxDQSDelay)
          {
            Txdqsdelay = find_mid_point(&ui32TxResult);
          }
          else
          {
            Txdqsdelay = scanCfg.ui8TxDQSDelay;
          }

          //
          // Find RXDQSDELAY Value
          //
          if(pTestConfig->bScanRxDQSDelay)
          {
            Rxdqsdelay = find_mid_point(&ui32RxResultArray[Txdqsdelay]);
          }
          else
          {
            Rxdqsdelay = scanCfg.ui8RxDQSDelay;
          }

          uint32_t ui32Window = count_consecutive_ones(&ui32TxResult);
          if(ui32WindowMax < ui32Window)
          {
            ui32WindowMax = ui32Window;

            pWidgetConfig->sTimingResult = scanCfg;
            pWidgetConfig->sTimingResult.ui8TxDQSDelay = Txdqsdelay;
            pWidgetConfig->sTimingResult.ui8RxDQSDelay = Rxdqsdelay;
            am_util_stdio_printf("Selected timing scan setting: TxNeg=%d, RxNeg=%d, RxCap=%d, Turnaround=%d, TxDQSDelay=%d, RxDQSDelay=%d\n", scanCfg.bTxNeg, scanCfg.bRxNeg, scanCfg.bRxCap, scanCfg.ui8Turnaround, Txdqsdelay, Rxdqsdelay);
          }
          else
          {
            am_util_stdio_printf("Candidate timing scan setting: TxNeg=%d, RxNeg=%d, RxCap=%d, Turnaround=%d, TxDQSDelay=%d, RxDQSDelay=%d\n", scanCfg.bTxNeg, scanCfg.bRxNeg, scanCfg.bRxCap, scanCfg.ui8Turnaround, Txdqsdelay, Rxdqsdelay);
          }

          if(!pTestConfig->bScanTurnAround)
          {
            break;
          }
        }

        if(!pTestConfig->bScanRxCap)
        {
          break;
        }
      }

      if(!pTestConfig->bScanRxNeg)
      {
        break;
      }
    }

    if(!pTestConfig->bScanTxNeg)
    {
      break;
    }

  }

  if(ui32WindowMax == 0)
  {
    return false;
  }
  else
  {
    return true;
  }

}

#endif
