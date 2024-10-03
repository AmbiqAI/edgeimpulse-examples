//*****************************************************************************
//
//! @file mspi_flash_task.c
//!
//! @brief mspi flash task program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "rtos.h"
#include "mspi_flash_task.h"
#include "mspi_multiple_instances_test_cases.h"

//*****************************************************************************
//
// Global data
//
//*****************************************************************************
static AM_SHARED_RW uint8_t                 g_FlashTXBuffer[MSPI_FLASH_TASK_TEST_BLOCK_SIZE] __attribute__((aligned(32)));      // 5K SRAM TX buffer
static AM_SHARED_RW uint8_t                 g_FlashRXBuffer[MSPI_FLASH_TASK_TEST_BLOCK_SIZE] __attribute__((aligned(32)));      // 5K SRAM RX buffer

static uint8_t       ui8Byte;
static uint32_t      ui32Word, ui32CmpWord;
static void          *pWidget[AM_REG_MSPI_NUM_MODULES];
static char          errStr[128];

static mspi_flash_config_t      g_MSPITestConfig[AM_REG_MSPI_NUM_MODULES];               // Test configuration.
static volatile uint32_t        g_MSPIInterruptStatus[AM_REG_MSPI_NUM_MODULES];
static mspi_flash_config_t      g_sMspiFlashConfig;
AM_SHARED_RW uint32_t FlashDMATCBBuffer[4096];

extern MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES];

SemaphoreHandle_t g_semMspiFlashTimingScan = NULL;

am_devices_mspi_flash_config_t MSPI_Flash_Task_Config =
{
#if defined(NOR_FLASH_ADESTO_ATXP032)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
#elif defined(NOR_FLASH_ISSI_IS25WX064)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,
#endif
    .eClockFreq = AM_HAL_MSPI_CLK_96MHZ,
    .pNBTxnBuf = FlashDMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(FlashDMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

//
// Typedef - to encapsulate device driver functions
//
mspi_flash_device_func_t mspi_flash_device_func =
{
#if defined(NOR_FLASH_ADESTO_ATXP032)
    .devName = "ADESTO ATXP032",
    .mspi_init = am_devices_mspi_atxp032_init,
    .mspi_term = am_devices_mspi_atxp032_deinit,
    .mspi_read = am_devices_mspi_atxp032_read,
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_atxp032_apply_sdr_timing,
#elif defined(NOR_FLASH_ISSI_IS25WX064)
    .devName = "ISSI IS25WX064",
    .mspi_init = am_devices_mspi_is25wx064_init,
    .mspi_term = am_devices_mspi_is25wx064_deinit,
    .mspi_read = am_devices_mspi_is25wx064_read,
    .mspi_write = am_devices_mspi_is25wx064_write,
    .mspi_mass_erase = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase = am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable = am_devices_mspi_is25wx064_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_is25wx064_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_is25wx064_apply_ddr_timing,
#else
#pragma message("Unknown FLASH Device")
#endif
};
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
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define flash_mspi_isr                                                          \
    am_mspi_isr1(MSPI_FLASH_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void flash_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPITestConfig[MSPI_FLASH_MODULE].pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPITestConfig[MSPI_FLASH_MODULE].pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPITestConfig[MSPI_FLASH_MODULE].pHandle, ui32Status);

    g_MSPIInterruptStatus[MSPI_FLASH_MODULE] &= ~ui32Status;
}

//*****************************************************************************
//
// Set up MSPI clock for the device.
//
//*****************************************************************************
char* clkfreq_string(am_hal_mspi_clock_e clk)
{
    const char* clk_str = "error";
    switch(clk)
    {
        case AM_HAL_MSPI_CLK_250MHZ:
            clk_str = "250";
            break;
        case AM_HAL_MSPI_CLK_192MHZ:
            clk_str = "192";
            break;
        case AM_HAL_MSPI_CLK_125MHZ:
            clk_str = "125";
            break;
        case AM_HAL_MSPI_CLK_96MHZ:
            clk_str = "96";
            break;
        case AM_HAL_MSPI_CLK_62P5MHZ:
            clk_str = "62.5";
            break;
        case AM_HAL_MSPI_CLK_48MHZ:
            clk_str = "48";
            break;
        default :
            clk_str = "error";
    }

    return clk_str;
}

char* device_mode_string(am_hal_mspi_device_e mode)
{
    const char* mode_str = "error";
    switch(mode)
    {
        case  AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case  AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case  AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8:
        case  AM_HAL_MSPI_FLASH_OCTAL_CE1_1_8_8:
            mode_str = "Octal";
            break;
        case  AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
        case  AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            mode_str = "DDR Octal"  ;
            break;
        case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
        case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
            mode_str = "DDR Hex";
            break;
        default :
            mode_str = "error";
            break ;
    }

    return mode_str;
}

#if defined(am_devices_mspi_flash_timing_config_t)
am_devices_mspi_flash_timing_config_t MSPITimingConfig;
#endif

//*****************************************************************************
//
// Mspi Flash Task Init Function.
//
//*****************************************************************************
bool mspi_flash_task_init(mspi_flash_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr)
{
  uint32_t      ui32Status;

  am_util_stdio_printf("Flash device name is %s, MSPI#%d interface mode is %s @ %s MHz !\n\n",\
                      mspi_flash_device_func.devName,         \
                      MSPI_FLASH_MODULE ,                     \
                      device_mode_string(g_sMspiFlashConfig.MSPIConfig.eDeviceConfig),
                      clkfreq_string(g_sMspiFlashConfig.MSPIConfig.eClockFreq) );

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_flash_timing_config_t)

  am_util_stdio_printf("Starting MSPI Flash SDR Timing Scan: \n");
  if (mspi_flash_device_func.mspi_init_timing_check != NULL)
  {
      ui32Status = mspi_flash_device_func.mspi_init_timing_check(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &MSPITimingConfig);
      if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
      {
          am_util_stdio_printf("Scan Result: Failed, no valid setting.  \n");
          return MSPI_FLASH_STATUS_ERROR;
      }
  }

#endif // APOLLO5_FPGA

  //
  // mspi flash timing scan end
  //
  if ( g_semMspiFlashTimingScan )
  {
      xSemaphoreGive(g_semMspiFlashTimingScan);
  }

  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_MSPITestConfig[pMSPITestConfig->ui32Module];

  g_MSPIInterruptStatus[pMSPITestConfig->ui32Module] = 0;

  //
  // Configure the MSPI and Flash Device.
  //
  ui32Status = mspi_flash_device_func.mspi_init(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &pMSPITestConfig->pDevHandle, &pMSPITestConfig->pHandle);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
      am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
      return MSPI_FLASH_STATUS_ERROR;
  }

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_flash_timing_config_t)
  //
  //  Set the SDR timing from previous scan.
  //
  if ( mspi_flash_device_func.mspi_init_timing_apply != NULL )
  {
    ui32Status = mspi_flash_device_func.mspi_init_timing_apply(pMSPITestConfig->pDevHandle, &MSPITimingConfig);
    if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply mspi timing .  \n");
        return MSPI_FLASH_STATUS_ERROR;
    }
  }
#endif

  g_MSPITestConfig[pMSPITestConfig->ui32Module] = *pMSPITestConfig;

  NVIC_EnableIRQ(mspi_interrupts[pMSPITestConfig->ui32Module]);

  am_hal_interrupt_master_enable();

  // Return the result.
  return (MSPI_FLASH_STATUS_SUCCESS == ui32Status);
}

//*****************************************************************************
//
// Mspi Task Deinit Function.
//
//*****************************************************************************
bool mspi_flash_task_deinit(void *pWidget, char *pErrStr)
{
  uint32_t      ui32Status;
  mspi_flash_config_t        *pWidgetConfig = (mspi_flash_config_t *)pWidget;

  NVIC_DisableIRQ(mspi_interrupts[pWidgetConfig->ui32Module]);

  //
  // Clean up the MSPI before exit.
  //
  ui32Status = mspi_flash_device_func.mspi_term(pWidgetConfig->pDevHandle);

  return (MSPI_FLASH_STATUS_SUCCESS == ui32Status);
}

static bool mspi_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  uint32_t      randomDelay;

  mspi_flash_config_t      *pWidgetConfig = (mspi_flash_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  mspi_task_test_t        *pTestConfig = (mspi_task_test_t *)pTestCfg;

  // Erase the target sector.
  am_util_debug_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);
  ui32Status = mspi_flash_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_debug_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
  ui32Status = mspi_flash_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           &g_FlashTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                           pTestConfig->NumBytes, true);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
  am_util_debug_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);
  ui32Status = mspi_flash_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_FlashRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  srand(xTaskGetTickCount());
  randomDelay = rand() % MAX_MSPI_FLASH_TASK_DELAY;
  vTaskDelay(randomDelay);

  return true;
}


// XIP Data bus read test.
static bool
mspi_test_xip_databus(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI Widget configuration.
  //
  mspi_flash_config_t        *pWidgetConfig = (mspi_flash_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  mspi_task_test_t          *pTestConfig = (mspi_task_test_t *)pTestCfg;

  // Erase the target sector.
  am_util_debug_printf("Erasing Sector %8X\n", pTestConfig->SectorAddress);

  ui32Status = mspi_flash_device_func.mspi_sector_erase(pWidgetConfig->pDevHandle,
                                                  pTestConfig->SectorAddress);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_debug_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes, pTestConfig->SectorAddress);

  ui32Status = mspi_flash_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           g_FlashTXBuffer, pTestConfig->SectorAddress,
                                           pTestConfig->NumBytes, true);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Set up for XIP operation.
  am_util_debug_printf("Putting the MSPI and External Flash into XIP mode\n");

  ui32Status = mspi_flash_device_func.mspi_xip_enable(pWidgetConfig->pDevHandle);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

  // Read and check the data by comparing it to the base structure (word-wise)
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i += 4)
  {
    ui32Word = *(uint32_t *)(g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + i);
    ui32CmpWord = *(uint32_t *)&g_FlashTXBuffer[i];
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
    if (g_FlashTXBuffer[i] != ui8Byte)
    {
      am_util_stdio_printf("Byte Aligned Data Bus compare with External Flash failed at Address %8X\n",
                           (g_MSPIAddresses[pWidgetConfig->ui32Module].XIPBase + pTestConfig->SectorAddress + i));
      return false;
    }
  }

  // Shutdown XIP operation.
  am_util_debug_printf("Disabling the MSPI and External Flash from XIP mode\n");

  ui32Status = mspi_flash_device_func.mspi_xip_disable(pWidgetConfig->pDevHandle);
  if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

  return true;
}


//*****************************************************************************
//
// Test cases for the main program to run.
//
//*****************************************************************************
static void
mspi_flash_test(void)
{
    mspi_task_test_t   testConfig;
    bool  testPassed = true;
    uint32_t ui32TestCnt = 0;

    uint32_t speed = MSPI_FLASH_TASK_START_SPEED + (g_ui32TestCnt % 3);
    if( speed > MSPI_FLASH_TASK_END_SPEED )
    {
      speed = MSPI_FLASH_TASK_START_SPEED;
    }

    uint32_t index = MSPI_FLASH_MODULE;

    g_sMspiFlashConfig.ui32Module = MSPI_FLASH_MODULE;
    g_sMspiFlashConfig.MSPIConfig = MSPI_Flash_Task_Config;
    g_sMspiFlashConfig.MSPIConfig.eClockFreq = mspi_test_speeds[speed].speed;
    g_sMspiFlashConfig.ui32DMACtrlBufferSize = (sizeof(FlashDMATCBBuffer)/sizeof(uint32_t));
    g_sMspiFlashConfig.pDMACtrlBuffer = FlashDMATCBBuffer;

    am_util_debug_printf("\n=============== Starting Octal Erase, Write, Read test at speed = %s, MSPI #%d ===============\n", mspi_test_speeds[speed].string, g_sMspiFlashConfig.ui32Module);

    TEST_ASSERT_TRUE(mspi_flash_task_init(&g_sMspiFlashConfig, &pWidget[index], errStr));

    for (ui32TestCnt = 0; ui32TestCnt < MSPI_FLASH_TEST_NUM; ui32TestCnt ++)
    {
      uint32_t size = MSPI_FLASH_TASK_TEST_BLOCK_SIZE;
      testConfig.NumBytes = size;
      testConfig.ByteOffset = 0;
      testConfig.SectorAddress = 0x00000000;
      testConfig.SectorOffset = 0;
      testConfig.UnscrambledSector = 0;

      TEST_ASSERT_TRUE(mspi_prepare_data_pattern((ui32TestCnt % MSPI_TEST_PATTERN_NUMBER), (uint8_t*) g_FlashTXBuffer, testConfig.NumBytes));
      am_hal_sysctrl_bus_write_flush();

      TEST_ASSERT_TRUE(mspi_test_write_read(pWidget[index], &testConfig, errStr));

      testPassed = mspi_check_data_match((uint8_t *)g_FlashRXBuffer, (uint8_t *)g_FlashTXBuffer, testConfig.NumBytes);

      if (!testPassed)
      {
        am_util_stdio_printf("Octal Flash Write Read test failed at NumBytes = %d, speed = %s, MSPI #%d\n", testConfig.NumBytes, mspi_test_speeds[speed].string, index);
        break;
      }
      else
      {
        am_util_stdio_printf("Octal Flash Write Read test at speed = %s, test_cnt= %d, passed on MSPI #%d\n",mspi_test_speeds[speed].string, ui32TestCnt, index);
      }

      testConfig.NumBytes = MSPI_FLASH_TASK_TEST_BLOCK_SIZE;
      testConfig.SectorAddress = 0x00000000;
      testConfig.ByteOffset = 0;
      testConfig.SectorOffset = 0;
      testConfig.UnscrambledSector = 0;
      testConfig.TurnOnCaching = true;

      memset((void*)g_FlashRXBuffer, 0, testConfig.NumBytes);

      testPassed = mspi_test_xip_databus(pWidget[index], &testConfig, errStr);

      if (!testPassed)
      {
        am_util_stdio_printf("Octal Flash XIP Data test failed at speed = %s, test_cnt= %d, MSPI #%d\n", mspi_test_speeds[speed].string, ui32TestCnt, index);
      }
      else
      {
        am_util_stdio_printf("Octal Flash XIP Data test at speed = %s, test_cnt= %d, passed on MSPI #%d\n",mspi_test_speeds[speed].string, ui32TestCnt, index);
      }
    }

    TEST_ASSERT_TRUE(mspi_flash_task_deinit(pWidget[index], errStr));

    TEST_ASSERT_TRUE(testPassed);
}


//*****************************************************************************
//
// mspi flash task.
//
//*****************************************************************************
void MspiFlashTask(void *pvParameters)
{
    am_util_stdio_printf("\nmspi flash task start\n");

    mspi_flash_test();

    am_util_stdio_printf("\nmspi flash: task end!\n");

    if( g_semMspiFlashTimingScan )
    {
      vSemaphoreDelete(g_semMspiFlashTimingScan);
    }

    vTaskDelete(NULL);

    while (1);
}
