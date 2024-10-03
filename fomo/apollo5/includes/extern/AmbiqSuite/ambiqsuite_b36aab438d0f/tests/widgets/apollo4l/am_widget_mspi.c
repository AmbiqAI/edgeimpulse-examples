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

AM_SHARED_RW uint8_t                 g_SectorTXBuffer[65*1024] AM_BIT_ALIGNED(128);      // 64K SRAM TX buffer
AM_SHARED_RW uint8_t                 g_SectorRXBuffer[65*1024] AM_BIT_ALIGNED(128);      // 64K SRAM RX buffer.
//uint8_t                 g_SectorTXBuffer[4*1024];      // 4K SRAM TX buffer
//uint8_t                 g_SectorRXBuffer[4*1024];      // 4K SRAM RX buffer.
AM_SHARED_RW uint8_t                 g_SectorRXBuffer2[1024] AM_BIT_ALIGNED(128);      // 64K SRAM RX buffer.

uint8_t       ui8Byte;
uint32_t      ui32Word, ui32CmpWord, ui32MiscompareCount;

mspi_device_func_t mspi_device_func =
{
#if defined(MICRON_N25Q256A)
    .devName = "MSPI FLASH N25Q256A",
    .mspi_init = am_devices_n25q256a_init,
    .mspi_init_timing_check = NULL,
    .mspi_init_timing_apply = NULL,
    .mspi_term = am_devices_n25q256a_deinit,
    .mspi_read_id = am_devices_n25q256a_id,
    .mspi_read = am_devices_n25q256a_read,
    .mspi_read_adv = am_devices_n25q256a_read_adv,
    .mspi_read_callback = am_devices_n25q256a_read_callback,
    .mspi_write = am_devices_n25q256a_write,
    .mspi_mass_erase = am_devices_n25q256a_mass_erase,
    .mspi_sector_erase = am_devices_n25q256a_sector_erase,
    .mspi_xip_enable = am_devices_n25q256a_enable_xip,
    .mspi_xip_disable = am_devices_n25q256a_disable_xip,
    .mspi_scrambling_enable = am_devices_n25q256a_enable_scrambling,
    .mspi_scrambling_disable = am_devices_n25q256a_disable_scrambling,
#elif defined(MACRONIX_MX25U12835F)
    .devName = "MSPI FLASH MX25U12835F",
    .mspi_init = am_devices_mspi_mx25u12835f_init,
    .mspi_init_timing_check = NULL,
    .mspi_init_timing_apply = NULL,
    .mspi_term = am_devices_mspi_mx25u12835f_deinit,
    .mspi_read_id = am_devices_mspi_mx25u12835f_id,
    .mspi_read = am_devices_mspi_mx25u12835f_read,
    .mspi_read_adv = am_devices_mspi_mx25u12835f_read_adv,
    .mspi_read_callback = am_devices_mspi_mx25u12835f_read_callback,
    .mspi_write = am_devices_mspi_mx25u12835f_write,
    .mspi_mass_erase = am_devices_mspi_mx25u12835f_mass_erase,
    .mspi_sector_erase = am_devices_mspi_mx25u12835f_sector_erase,
    .mspi_xip_enable = am_devices_mspi_mx25u12835f_enable_xip,
    .mspi_xip_disable = am_devices_mspi_mx25u12835f_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_mx25u12835f_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_mx25u12835f_disable_scrambling,
#elif defined(ADESTO_ATXP032)
    .devName = "MSPI FLASH ATXP032",
    .mspi_init = am_devices_mspi_atxp032_init,
    .mspi_init_timing_check = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_atxp032_apply_sdr_timing,
//    .mspi_init_timing_check = NULL,
//    .mspi_init_timing_apply = NULL,
    .mspi_term = am_devices_mspi_atxp032_deinit,
    .mspi_read_id = am_devices_mspi_atxp032_id,
    .mspi_read = am_devices_mspi_atxp032_read,
    .mspi_read_adv = am_devices_mspi_atxp032_read_adv,
    .mspi_read_callback = am_devices_mspi_atxp032_read_callback,
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_atxp032_disable_scrambling,
#elif defined(CYPRESS_S25FS064S)
    .devName = "MSPI FLASH S25FS064S",
    .mspi_init = am_devices_mspi_s25fs064s_init,
    .mspi_init_timing_check = NULL,
    .mspi_init_timing_apply = NULL,
    .mspi_term = am_devices_mspi_s25fs064s_deinit,
    .mspi_read_id = am_devices_mspi_s25fs064s_id,
    .mspi_read = am_devices_mspi_s25fs064s_read,
    .mspi_read_adv = am_devices_mspi_s25fs064s_read_adv,
    .mspi_read_callback = am_devices_mspi_s25fs064s_read_callback,
    .mspi_write = am_devices_mspi_s25fs064s_write,
    .mspi_mass_erase = am_devices_mspi_s25fs064s_mass_erase,
    .mspi_sector_erase = am_devices_mspi_s25fs064s_sector_erase,
    .mspi_xip_enable = am_devices_mspi_s25fs064s_enable_xip,
    .mspi_xip_disable = am_devices_mspi_s25fs064s_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_s25fs064s_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_s25fs064s_disable_scrambling,
#elif defined(AM_WIDGET_MSPI_PSRAM_DEVICE)
    .devName = "MSPI PSRAM",
    .mspi_init = am_devices_mspi_psram_init,
    .mspi_term = am_devices_mspi_psram_deinit,
    .mspi_read_id = am_devices_mspi_psram_id,
    .mspi_read = am_devices_mspi_psram_read,
    .mspi_write = am_devices_mspi_psram_write,
    .mspi_xip_enable = am_devices_mspi_psram_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_disable_scrambling,
//    .mspi_init_timing_check = am_devices_mspi_psram_sdr_init_timing_check,
//    .mspi_init_timing_apply = am_devices_mspi_psram_apply_sdr_timing,
    .mspi_init_timing_check = NULL,
    .mspi_init_timing_apply = NULL,
#elif defined(AM_WIDGET_MSPI_DDR_PSRAM_DEVICE)
    .devName = "MSPI DDR PSRAM",
    .mspi_init = am_devices_mspi_psram_aps12808l_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps12808l_ddr_deinit,
    .mspi_read_id = am_devices_mspi_psram_aps12808l_ddr_id,
    .mspi_read = am_devices_mspi_psram_aps12808l_ddr_read,
    .mspi_write = am_devices_mspi_psram_aps12808l_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps12808l_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps12808l_ddr_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_aps12808l_ddr_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_aps12808l_ddr_disable_scrambling,
#elif defined(AM_WIDGET_MSPI_HEX_DDR_PSRAM_DEVICE)
    .devName = "MSPI HEX DDR PSRAM",
    .mspi_init = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read_id = am_devices_mspi_psram_aps25616n_ddr_id,
    .mspi_read = am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_write = am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_aps25616n_ddr_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_aps25616n_ddr_disable_scrambling,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616n_apply_ddr_timing,

#elif defined(AM_WIDGET_MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE)
    .devName = "MSPI HEX DDR WINBOND PSRAM",
    .mspi_init = am_devices_mspi_psram_w958d6nw_ddr_init,
    .mspi_term = am_devices_mspi_psram_w958d6nw_ddr_deinit,
    .mspi_read_id = am_devices_mspi_psram_w958d6nw_ddr_id,
    .mspi_read = am_devices_mspi_psram_w958d6nw_ddr_read,
    .mspi_write = am_devices_mspi_psram_w958d6nw_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_w958d6nw_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_w958d6nw_ddr_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_w958d6nw_ddr_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_w958d6nw_ddr_disable_scrambling,
    .mspi_init_timing_check = am_devices_mspi_psram_hex_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_apply_hex_ddr_timing,
//    .mspi_init_timing_check = NULL,
//    .mspi_init_timing_apply = NULL,
#elif defined(AM_WIDGET_MSPI_FRAM_DEVICE)
    .devName = "MSPI FRAM",
    .mspi_init = am_devices_mb85rs64v_mspi_init,
    .mspi_init_timing_check = NULL,
    .mspi_init_timing_apply = NULL,
    .mspi_term = am_devices_mb85rs64v_mspi_deinit,
    .mspi_read_id = am_devices_mb85rs64v_mspi_read_id,
    .mspi_read = am_devices_mb85rs64v_mspi_read,
    .mspi_write = am_devices_mb85rs64v_mspi_write,
//    .mspi_xip_enable = ,
//    .mspi_xip_disable = ,
//    .mspi_scrambling_enable = ,
//    .mspi_scrambling_disable = ,
#elif defined(AMBT53)
    .devName = "AMBT53",
    .mspi_init = am_devices_mspi_ambt53_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check = NULL,
    .mspi_init_timing_apply = NULL,
#endif
    .mspi_term = am_devices_mspi_ambt53_deinit,
    .mspi_read_id = NULL,
    .mspi_read = am_devices_mspi_ambt53_dma_read,
    .mspi_read_adv = am_devices_mspi_ambt53_dma_read_adv,
    .mspi_write = am_devices_mspi_ambt53_dma_write,
    .mspi_write_adv = am_devices_mspi_ambt53_dma_write_adv,
    .mspi_xip_enable = am_devices_mspi_ambt53_enable_xip,
    .mspi_xip_disable = am_devices_mspi_ambt53_disable_xip,
#elif defined(DOSILICON_DS35X1GA)
    .devName = "DS35X1GA",
    .mspi_init = am_devices_mspi_ds35x1ga_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check = am_devices_mspi_ds35x1ga_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_ds35x1ga_apply_sdr_timing,
#endif
    .mspi_term = am_devices_mspi_ds35x1ga_deinit,
    .mspi_read_id = am_devices_mspi_ds35x1ga_id,
    .mspi_read = am_devices_mspi_ds35x1ga_read,
    .mspi_write = am_devices_mspi_ds35x1ga_write,
    .mspi_block_erase = am_devices_mspi_ds35x1ga_block_erase,
#elif defined(TOSHIBA_TC58CYG0)
    .devName = "TC58CYG0",
    .mspi_init = am_devices_mspi_tc58cyg0_init,
#if defined(am_widget_mspi_devices_timing_config_t)
    .mspi_init_timing_check = am_devices_mspi_tc58cyg0_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_tc58cyg0_apply_sdr_timing,
#endif
    .mspi_term = am_devices_mspi_tc58cyg0_deinit,
    .mspi_read_id = am_devices_mspi_tc58cyg0_id,
    .mspi_read = am_devices_mspi_tc58cyg0_read,
    .mspi_write = am_devices_mspi_tc58cyg0_write,
    .mspi_block_erase = am_devices_mspi_tc58cyg0_block_erase,
#else
#error "Unknown MSPI Device"
#endif
};

const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_START_ADDR},
  {MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_START_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR},
};

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
#if (defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) )
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

volatile uint32_t               g_MSPIInterruptStatus[AM_REG_MSPI_NUM_MODULES];

#if defined(am_widget_mspi_devices_timing_config_t)
am_widget_mspi_devices_timing_config_t g_MSPITimingConfig;
#endif
void
am_mspi0_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPITestConfig[0].pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPITestConfig[0].pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPITestConfig[0].pHandle, ui32Status);

    g_MSPIInterruptStatus[0] &= ~ui32Status;
}

void
am_mspi1_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPITestConfig[1].pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPITestConfig[1].pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPITestConfig[1].pHandle, ui32Status);

    g_MSPIInterruptStatus[1] &= ~ui32Status;
}

void
am_mspi2_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPITestConfig[2].pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPITestConfig[2].pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPITestConfig[2].pHandle, ui32Status);

    g_MSPIInterruptStatus[2] &= ~ui32Status;
}
uint32_t
am_widget_mspi_power_save_and_restore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode)
{
#ifdef MSPI_TEST_POWER_SAVE_RESTORE
    am_widget_mspi_config_t *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;
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
  uint32_t      ui32Status;
  am_hal_pwrctrl_sram_memcfg_t  SRAMCfg =
  {
    .eSRAMCfg       = AM_HAL_PWRCTRL_SRAM_1M,
    .eActiveWithMCU = AM_HAL_PWRCTRL_SRAM_1M,
    .eActiveWithDSP = AM_HAL_PWRCTRL_SRAM_NONE,
    .eSRAMRetain    = AM_HAL_PWRCTRL_SRAM_1M
  };

#if !defined(APOLLO4_FPGA) && defined(am_widget_mspi_devices_timing_config_t)
  if ( mspi_device_func.mspi_init_timing_check != NULL )
  {
    ui32Status = mspi_device_func.mspi_init_timing_check(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &g_MSPITimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
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
    return false;
  }

#if !defined(APOLLO4_FPGA) && defined(am_widget_mspi_devices_timing_config_t)
  if ( mspi_device_func.mspi_init_timing_apply != NULL )
  {
    ui32Status = mspi_device_func.mspi_init_timing_apply(pMSPITestConfig->pDevHandle, &g_MSPITimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return false;
    }
  }
#endif

  g_MSPITestConfig[pMSPITestConfig->ui32Module] = *pMSPITestConfig;

  ui32Status = am_hal_pwrctrl_sram_config(&SRAMCfg);

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

  // Clean up the MSPI configuraton for the N25Q256A part.
  ui32Status = mspi_device_func.mspi_term(pWidgetConfig->pDevHandle);

  // Return the result.
  return (AM_WIDGET_MSPI_SUCCESS == ui32Status);
}

//*****************************************************************************
//
// Widget Test Execution Functions
//
//*****************************************************************************

#if defined(DOSILICON_DS35X1GA) || defined(TOSHIBA_TC58CYG0)
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

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  for (uint32_t i = pTestConfig->ByteOffset; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif

  return true;
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
#if defined(AMBT53)
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
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif

  return true;
}
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
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
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
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif
  return true;
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
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
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
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
  }

#if !defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf(".");
#endif
  return true;
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

#if !defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P) && !defined(AM_PART_APOLLO4L)
  if (pTestConfig->TurnOnCaching)
  {
    // Turn on flash caching so we don't see a fetch for every read.
    CACHECTRL->CACHECFG =
      _VAL2FLD(CACHECTRL_CACHECFG_CONFIG, CACHECTRL_CACHECFG_CONFIG_W2_128B_512E) |
      CACHECTRL_CACHECFG_ENABLE_Msk                                               |
      CACHECTRL_CACHECFG_ICACHE_ENABLE_Msk                                        |
      CACHECTRL_CACHECFG_DCACHE_ENABLE_Msk;
  }
  else
  {
    CACHECTRL->CACHECFG = 0;
  }
#endif

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
typedef uint32_t (*mspi_xip_test_function_t)(uint32_t, uint32_t, uint32_t);

typedef struct
{
    uint32_t binAddr;
    uint32_t funcOffset;
    uint32_t binSize;
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t result;
} mspi_xip_test_funcinfo_t;

// Patchable section of binary
extern uint32_t __Patchable[];

// Configurable option to load a position independent library instead
void get_test_function(mspi_xip_test_funcinfo_t *pFuncInfo)
{
    // some of the parameters are controllable through binary patching
    if (__Patchable[0])
    {
        pFuncInfo->binAddr = __Patchable[0];
        pFuncInfo->funcOffset = __Patchable[1];
        pFuncInfo->binSize = __Patchable[2];
        pFuncInfo->param0 = __Patchable[3];
        pFuncInfo->param1 = __Patchable[4];
        pFuncInfo->param2 = __Patchable[5];
        pFuncInfo->result = __Patchable[6];
    }
    else
    {
        pFuncInfo->binAddr = ((uint32_t)&xip_test_function) & 0xFFFFFFFE;
        pFuncInfo->funcOffset = 0;
        pFuncInfo->binSize = MSPI_XIP_FUNCTION_SIZE;
        pFuncInfo->param0 = 0xDEADBEEF;
        pFuncInfo->result = 0xDEADBEEF;
    }
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
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
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
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer, pTestConfig->UnscrambledSector, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
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
  ui32Status = mspi_device_func.mspi_read(pWidgetConfig->pDevHandle, g_SectorRXBuffer, pTestConfig->UnscrambledSector, pTestConfig->NumBytes, true);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Compare the buffers
#if defined(MSPI_DEBUG_PRINTING)
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
#endif
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_SectorRXBuffer[i], g_SectorTXBuffer[i]);
      return false;
    }
  }

  AM_UPDATE_TEST_PROGRESS();

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
#endif
