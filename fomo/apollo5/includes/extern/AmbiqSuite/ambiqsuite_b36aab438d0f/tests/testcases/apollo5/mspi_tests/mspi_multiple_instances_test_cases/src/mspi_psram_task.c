//*****************************************************************************
//
//! @file mspi_psram_task.c
//!
//! @brief MSPI psram task for use with APMemory PSRAMs.
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
#include "mspi_psram_task.h"
#include "am_util.h"
#include "mspi_multiple_instances_test_cases.h"
#include "am_devices_mspi_psram_aps25616n.h"

//*****************************************************************************
//
// Global data
//
//*****************************************************************************

//
// Typedef - to encapsulate device driver functions
//
mspi_psram_device_func_t mspi_psram_device_func =
{
#if defined(PSRAM_APS12808L)
    .devName = "APS12808",
    .mspi_init = am_devices_mspi_psram_aps12808l_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps12808l_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps12808l_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps12808l_ddr_nonblocking_read
    .mspi_write = am_devices_mspi_psram_aps12808l_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps12808l_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps12808l_ddr_disable_xip,
#elif defined(PSRAM_APS25616N)
    .devName = "APS25616N",
    .mspi_init = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read,
    .mspi_write = am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616n_apply_ddr_timing,
#elif defined(PSRAM_W958D6NW)
    .devName = "WINBOND W958D6NW",
    .mspi_init = am_devices_mspi_psram_w958d6nw_ddr_init,
    .mspi_term = am_devices_mspi_psram_w958d6nw_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_w958d6nw_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_w958d6nw_ddr_nonblocking_read,
    .mspi_write = am_devices_mspi_psram_w958d6nw_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_w958d6nw_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_w958d6nw_ddr_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_psram_hex_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_apply_hex_ddr_timing,
#else
#pragma message("Unknown PSRAM Device")
#endif
};

static AM_SHARED_RW uint8_t                 g_PsramTXBuffer[MSPI_PSRAM_TASK_TEST_BLOCK_SIZE] __attribute__((aligned(32)));      // 5K SRAM TX buffer
static AM_SHARED_RW uint8_t                 g_PsramRXBuffer[MSPI_PSRAM_TASK_TEST_BLOCK_SIZE] __attribute__((aligned(32)));      // 5K SRAM RX buffer

AM_SHARED_RW uint32_t PSRAMDMATCBBuffer[4096];
static void     *pWidget[AM_REG_MSPI_NUM_MODULES];
static char     errStr[128];

static mspi_psram_config_t        g_MSPITestConfig[AM_REG_MSPI_NUM_MODULES];               // Test configuration.
static volatile uint32_t          g_MSPIInterruptStatus[AM_REG_MSPI_NUM_MODULES];
mspi_psram_config_t               g_sMspiPsramConfig;

extern SemaphoreHandle_t g_semMspiFlashTimingScan;

#if defined (apollo5_fpga_stmb_rev1)
uint32_t    mspi_instances[1] = {0};
#elif defined (apollo5_fpga_ps)
uint32_t    mspi_instances[2] = {0, 2};
#else
uint32_t    mspi_instances[3] = {0, 1, 2};
#endif

extern const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_START_ADDR},
  {MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_START_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR},
  {MSPI3_APERTURE_START_ADDR, MSPI3_APERTURE_START_ADDR},
};

mspi_speed_t mspi_test_speeds[] =
{
  { AM_HAL_MSPI_CLK_250MHZ,     " 250MHz" },
  { AM_HAL_MSPI_CLK_192MHZ,     " 192MHz" },
  { AM_HAL_MSPI_CLK_125MHZ,     " 125MHz" },
  { AM_HAL_MSPI_CLK_96MHZ,      " 96MHz" },
  { AM_HAL_MSPI_CLK_62P5MHZ,    " 62p5MHz" },
  { AM_HAL_MSPI_CLK_48MHZ,      " 48MHz" },
};

am_devices_mspi_psram_config_t MSPI_Psram_Task_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_96MHZ,
    .pNBTxnBuf = PSRAMDMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(PSRAMDMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};


void am_systick_isr(void)
{
}

mspi_psram_config_t g_xipmmWidgetConfig;

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
    MSPI3_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPITestConfig[MSPI_PSRAM_MODULE].pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPITestConfig[MSPI_PSRAM_MODULE].pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPITestConfig[MSPI_PSRAM_MODULE].pHandle, ui32Status);

    g_MSPIInterruptStatus[MSPI_PSRAM_MODULE] &= ~ui32Status;
}

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_psram_timing_config_t)
  am_devices_mspi_psram_timing_config_t MSPIPsramTimingConfig;
#endif

//*****************************************************************************
//
// Mspi Psram Init Function.
//
//*****************************************************************************
static bool
mspi_psram_task_init(mspi_psram_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr)
{
  uint32_t      ui32Status;

  am_util_stdio_printf("Psram device name is %s, MSPI#%d interface mode is %s @ %s MHz !\n\n",\
                      mspi_psram_device_func.devName,         \
                      MSPI_PSRAM_MODULE ,                     \
                      device_mode_string(g_sMspiPsramConfig.MSPIConfig.eDeviceConfig),
                      clkfreq_string(g_sMspiPsramConfig.MSPIConfig.eClockFreq) );

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_psram_timing_config_t)

  am_util_stdio_printf("Starting MSPI Psram DDR Timing Scan: \n");

  if ( mspi_psram_device_func.mspi_init_timing_check != NULL )
  {
    ui32Status = mspi_psram_device_func.mspi_init_timing_check(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &MSPIPsramTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return MSPI_PSRAM_STATUS_ERROR;
    }
  }

#endif

  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_MSPITestConfig[pMSPITestConfig->ui32Module];

  g_MSPIInterruptStatus[pMSPITestConfig->ui32Module] = 0;

  //
  // Configure the MSPI and PSRAM Device.
  //
  ui32Status = mspi_psram_device_func.mspi_init(pMSPITestConfig->ui32Module, &pMSPITestConfig->MSPIConfig, &pMSPITestConfig->pDevHandle, &pMSPITestConfig->pHandle);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return MSPI_PSRAM_STATUS_ERROR;
  }

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_psram_timing_config_t)
    //
    // Apply DDR timing setting
    //
    ui32Status = mspi_psram_device_func.mspi_init_timing_apply(pMSPITestConfig->pDevHandle, &MSPIPsramTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return MSPI_PSRAM_STATUS_ERROR;
    }
#endif

  //
  // Waiting for mspi flash timing scan
  //
  if ( g_semMspiFlashTimingScan )
  {
    xSemaphoreTake( g_semMspiFlashTimingScan, MSPI_SEMAPHORE_TIMEOUT );
  }

  g_MSPITestConfig[pMSPITestConfig->ui32Module] = *pMSPITestConfig;

  NVIC_EnableIRQ(mspi_interrupts[pMSPITestConfig->ui32Module]);

  am_hal_interrupt_master_enable();

  // Return the result.
  return (MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
}

//*****************************************************************************
//
// Mspi Psram Deinit Function.
//
//*****************************************************************************
static bool
mspi_psram_task_deinit(void *pWidget, char *pErrStr)
{
  uint32_t      ui32Status;
  mspi_psram_config_t        *pWidgetConfig = (mspi_psram_config_t *)pWidget;

  //
  // Need to make sure all pending XIPMM transactions are flushed
  //
  am_hal_cachectrl_dcache_invalidate(NULL, true);
  //
  // Clean up the MSPI before exit.
  //
  ui32Status = mspi_psram_device_func.mspi_term(pWidgetConfig->pDevHandle);

  return (MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
}

static bool
mspi_psram_task_write_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  uint32_t      randomDelay;
  mspi_psram_config_t      *pWidgetConfig = (mspi_psram_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  mspi_task_test_t        *pTestConfig = (mspi_task_test_t *)pTestCfg;

  // Write the TX buffer into the target sector.
  am_util_debug_printf("Writing %d Bytes to Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);

  ui32Status = mspi_psram_device_func.mspi_write(pWidgetConfig->pDevHandle,
                                           &g_PsramTXBuffer[pTestConfig->ByteOffset],
                                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                                           pTestConfig->NumBytes, true);
  if (MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return MSPI_PSRAM_STATUS_ERROR;
  }

  // Read the data back into the RX buffer.
  am_util_debug_printf("Read %d Bytes from Sector %8X\n", pTestConfig->NumBytes,
                       pTestConfig->SectorAddress + pTestConfig->SectorOffset);

  ui32Status = mspi_psram_device_func.mspi_read(pWidgetConfig->pDevHandle, &g_PsramRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress + pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return MSPI_PSRAM_STATUS_ERROR;
  }

  srand(xTaskGetTickCount());
  randomDelay = rand() % MAX_MSPI_PSRAM_TASK_DELAY;
  vTaskDelay(randomDelay);

  return true;
}

uint32_t
mspi_psram_task_power_save_and_restore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode)
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
    return MSPI_PSRAM_STATUS_SUCCESS;
}


//*****************************************************************************
//
// XIPMM check
//
//*****************************************************************************
#define MSPI_PSRAM_SIZE         0x00800000

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

static bool
run_mspi_xipmm(uint32_t ui32Module, uint32_t ui32BlockOffset, mspi_xipmm_access_e eAccess)
{
  uint32_t ix;
  uint32_t *pu32Ptr1, *pu32Ptr2;
  uint32_t u32Val1, u32Val2;
  uint16_t *pu16Ptr1, *pu16Ptr2;
  uint16_t u16Val1, u16Val2;
  uint8_t *pu8Ptr1, *pu8Ptr2;
  uint8_t u8Val1, u8Val2;

  switch (eAccess)
  {
  case MSPI_XIPMM_WORD_ACCESS:
    {
      // Word Read, Word Write
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
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
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/4; ix++)
      {
        *pu16Ptr1++ = (uint16_t)(ix & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((ix & 0xFFFF) ^ 0xFFFF);
      }

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/4; ix++)
      {
        if ( (*pu16Ptr1++ != (uint16_t)(ix & 0xFFFF)) || (*pu16Ptr2++ != (uint16_t)((ix & 0xFFFF) ^ 0xFFFF)) )
        {
          return false;
        }
      }
      // Half word Write, word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
      {
        u32Val1 = ix;
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
      // word Write, Half word Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
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
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        *pu8Ptr1++ = (uint8_t)(ix & 0xFF);
        *pu8Ptr2++ = (uint8_t)((ix & 0xFF) ^ 0xFF);
      }

      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        if ( (*pu8Ptr1++ != (uint8_t)(ix & 0xFF)) || (*pu8Ptr2++ != (uint8_t)((ix & 0xFF) ^ 0xFF)) )
        {
          return false;
        }
      }
      // Byte Write, Half word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/4; ix++)
      {
        u16Val1 = ((((2*ix + 1) & 0xFF) << 8) | ((2*ix) & 0xFF));
        u16Val2 = (((((2*ix + 1) & 0xFF) ^ 0xFF) << 8) | (((2*ix) & 0xFF) ^ 0xFF));
        if ( (*pu16Ptr1++ != u16Val1) || (*pu16Ptr2++ != u16Val2))
        {
          return false;
        }
      }
      // Byte Write, word Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
      {
        u32Val1 = ((((4*ix + 3) & 0xFF) << 24) |(((4*ix + 2) & 0xFF) << 16) |(((4*ix + 1) & 0xFF) << 8) | ((4*ix) & 0xFF));
        u32Val2 = (((((4*ix + 3) & 0xFF) ^ 0xFF) << 24) |((((4*ix + 2) & 0xFF) ^ 0xFF) << 16) |((((4*ix + 1) & 0xFF) ^ 0xFF) << 8) | (((4*ix) & 0xFF) ^ 0xFF));
        if ( (*pu32Ptr1++ != u32Val1) || (*pu32Ptr2++ != u32Val2))
        {
          return false;
        }
      }
      // word Write, Byte Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/8; ix++)
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
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/4; ix++)
      {
        *pu16Ptr1++ = (uint16_t)(ix & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((ix & 0xFFFF) ^ 0xFFFF);
      }
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/4; ix++)
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
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);
      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        u32Val1 = ix;
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      u8Val1 = 0;
      u8Val2 = 0xFF;
      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
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
      *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)) = 0;
      *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4)) = 0;
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1);
      u16Val1 = 0xAABB;
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4);
      am_util_debug_printf("Writing Half Word 0x%04x to address 0x%08x\n", u16Val1, pu16Ptr1);
      *pu16Ptr1 = u16Val1;
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4);

      *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)) = 0;
      *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4)) = 0;
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1);
      u32Val1 = 0xAABBCCDD;
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4);
      am_util_debug_printf("Writing Word 0x%08x to address 0x%08x\n", u32Val1, pu32Ptr1);
      *pu32Ptr1 = u32Val1;
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4)), g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 4);

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);
      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        u32Val1 = (ix >> 8) | ((ix + 1) << 24);
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 8 | (((ix + 1) ^ 0xFFFFFFFF) << 24);
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }
      *((uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)) = 0;
      *((uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)) = 0xFF;

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
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
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);
      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        u32Val1 = ix;
        *pu32Ptr1++ = u32Val1;
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu32Ptr2++ = u32Val2;
      }

      // Read at offset 1
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      u32Val1 = 0;
      u32Val2 = 0xFF;
      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
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
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      u32Val1 = 0;
      u32Val2 = 0xFFFF;
      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
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
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 3);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 3 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      u32Val1 = 0;
      u32Val2 = 0xFFFFFF;
      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
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
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 1 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);
      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        u32Val1 = (ix >> 8) | ((ix + 1) << 24);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 8 | (((ix + 1) ^ 0xFFFFFFFF) << 24);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)) = 0;
      *((uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)) = 0xFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write at offset 2
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);
      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        u32Val1 = (ix >> 16) | ((ix + 1) << 16);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 16 | (((ix + 1) ^ 0xFFFFFFFF) << 16);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)) = 0;
      *((uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)) = 0xFFFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write at offset 3
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 3);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 3 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);
      // Initialize a pattern
      for (ix = 0; ix < AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2; ix++)
      {
        u32Val1 = (ix >> 24) | ((ix + 1) << 8);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 24 | (((ix + 1) ^ 0xFFFFFFFF) << 8);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset)) = 0;
      *((uint16_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)) = 0xFFFF;
      *((uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2)) = 0;
      *((uint8_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2 + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)) = 0xFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[ui32Module].XIPMMBase + ui32BlockOffset + 2*AM_DEVICES_MSPI_PSRAM_PAGE_SIZE);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

      // Verify the pattern
      for (ix = 0; ix < (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE/2); ix++)
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
      uint32_t addr = g_MSPIAddresses[ui32Module].XIPMMBase;

      // Initialize a pattern
      for (ix = 0; ix < 8; ix++)
      {
        pattern[ix] = ix;
        pattern[ix+8] = ix ^ 0xFFFFFFFF;
      }

      // Octal Word Write
      pu32Ptr1 = (uint32_t *)(addr + ui32BlockOffset);
      octal_copy((uint32_t)pattern, addr + ui32BlockOffset, 2);

      octal_copy(addr + ui32BlockOffset, (uint32_t)pattern2, 2);
      pu32Ptr1 = (uint32_t *)(pattern2);
      pu32Ptr2 = (uint32_t *)(pattern2 + 8);

      mspi_psram_task_power_save_and_restore(&g_xipmmWidgetConfig, MSPI_TEST_POWERSAVE_MODE);

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
  }

  return true;
}

static bool
mspi_ddr_xipmm_data_test(void)
{
    bool  testPassed = true;
    uint32_t ui32Status;
    uint32_t mspi = MSPI_PSRAM_MODULE;

    g_xipmmWidgetConfig = g_sMspiPsramConfig;

    ui32Status = mspi_psram_device_func.mspi_xip_enable(g_sMspiPsramConfig.pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
#if 1
    // First Page
    testPassed = run_mspi_xipmm(mspi, 0,MSPI_XIPMM_WORD_ACCESS);
    TEST_ASSERT(testPassed);
    // Middle Page
    testPassed = run_mspi_xipmm(mspi, MSPI_PSRAM_SIZE/2,MSPI_XIPMM_WORD_ACCESS);
    TEST_ASSERT(testPassed);
    // Last Page
    testPassed = run_mspi_xipmm(mspi, MSPI_PSRAM_SIZE - AM_DEVICES_MSPI_PSRAM_PAGE_SIZE,MSPI_XIPMM_WORD_ACCESS);
    TEST_ASSERT(testPassed);
    if (!testPassed)
    {
      am_util_stdio_printf("DDR XIPMM Data test failed WORD_ACCESS, MSPI #%d\n", mspi_instances[mspi]);
    }
#endif
#if 1
    // First Page
    testPassed = run_mspi_xipmm(mspi, 0,MSPI_XIPMM_SHORT_ACCESS);
    TEST_ASSERT(testPassed);
    // Middle Page
    testPassed = run_mspi_xipmm(mspi, MSPI_PSRAM_SIZE/2,MSPI_XIPMM_SHORT_ACCESS);
    TEST_ASSERT(testPassed);
    // Last Page
    testPassed = run_mspi_xipmm(mspi, MSPI_PSRAM_SIZE - AM_DEVICES_MSPI_PSRAM_PAGE_SIZE,MSPI_XIPMM_SHORT_ACCESS);
    TEST_ASSERT(testPassed);
    if (!testPassed)
    {
      am_util_stdio_printf("DDR XIPMM Data test failed SHORT_ACCESS, MSPI #%d\n", mspi_instances[mspi]);
    }
#endif
#if 1
    // First Page
    testPassed = run_mspi_xipmm(mspi, 0,MSPI_XIPMM_BYTE_ACCESS);
    TEST_ASSERT(testPassed);
    // Middle Page
    testPassed = run_mspi_xipmm(mspi, MSPI_PSRAM_SIZE/2,MSPI_XIPMM_BYTE_ACCESS);
    TEST_ASSERT(testPassed);
    // Last Page
    testPassed = run_mspi_xipmm(mspi, MSPI_PSRAM_SIZE - AM_DEVICES_MSPI_PSRAM_PAGE_SIZE,MSPI_XIPMM_BYTE_ACCESS);
    TEST_ASSERT(testPassed);
    if (!testPassed)
    {
      am_util_stdio_printf("DDR XIPMM Data test failed BYTE_ACCESS, MSPI #%d\n", mspi_instances[mspi]);
    }
#endif
#if 1
    // First Page
    testPassed = run_mspi_xipmm(mspi, 0,MSPI_XIPMM_SHORT_ACCESS_UNALIGNED);
    TEST_ASSERT(testPassed);
    // First Page
    testPassed = run_mspi_xipmm(mspi, 0,MSPI_XIPMM_WORD_ACCESS_UNALIGNED);
    TEST_ASSERT(testPassed);
    if (!testPassed)
    {
      am_util_stdio_printf("DDR XIPMM Data test failed ACCESS_UNALIGNED, MSPI #%d\n", mspi_instances[mspi]);
    }
#endif
#if 1
    testPassed = run_mspi_xipmm(mspi, AM_DEVICES_MSPI_PSRAM_PAGE_SIZE-4,MSPI_XIPMM_OWORD_ACCESS);
    TEST_ASSERT(testPassed);
    if (!testPassed)
    {
      am_util_stdio_printf("DDR XIPMM Data test failed OWORD_ACCESS, MSPI #%d\n", mspi_instances[mspi]);
    }
#endif

    return testPassed;
}

//*****************************************************************************
//
// Test cases for the main task to run.
//
//*****************************************************************************
static void
mspi_psram_test(void)
{
    mspi_task_test_t testConfig;
    bool  testPassed = true;
    uint32_t ui32TestCnt, ui32TestCntMax= 0;

    uint32_t mspi = MSPI_PSRAM_MODULE;

    uint32_t speed = MSPI_PSRAM_TASK_START_SPEED + (g_ui32TestCnt % 3);
    if( speed > MSPI_PSRAM_TASK_END_SPEED )
    {
      speed = MSPI_PSRAM_TASK_START_SPEED;
    }

    ui32TestCntMax = MSPI_PSRAM_TEST_NUM;

    g_sMspiPsramConfig.ui32Module = MSPI_PSRAM_MODULE;
    g_sMspiPsramConfig.MSPIConfig = MSPI_Psram_Task_Config;
    g_sMspiPsramConfig.MSPIConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0;
    g_sMspiPsramConfig.ePSRAM = MSPI_TASK_MSPI_DDR_PSRAM_APS25616N;
    g_sMspiPsramConfig.MSPIConfig.eClockFreq = mspi_test_speeds[speed].speed;
    g_sMspiPsramConfig.ui32DMACtrlBufferSize = (sizeof(PSRAMDMATCBBuffer)/sizeof(uint32_t));
    g_sMspiPsramConfig.pDMACtrlBuffer = PSRAMDMATCBBuffer;

    TEST_ASSERT_TRUE(mspi_psram_task_init(&g_sMspiPsramConfig, &pWidget[mspi], errStr));

    for (ui32TestCnt = 0; ui32TestCnt < ui32TestCntMax; ui32TestCnt ++)
    {
      testConfig.NumBytes = MSPI_PSRAM_TASK_TEST_BLOCK_SIZE;
      testConfig.ByteOffset = 0;
      testConfig.SectorAddress = 0x00000000;
      testConfig.SectorOffset = 0;
      testConfig.UnscrambledSector = 0;

      TEST_ASSERT_TRUE(mspi_prepare_data_pattern( (ui32TestCnt % MSPI_TEST_PATTERN_NUMBER), (uint8_t*) g_PsramTXBuffer, testConfig.NumBytes));
      am_hal_sysctrl_bus_write_flush();

      TEST_ASSERT_TRUE(mspi_psram_task_write_read(pWidget[mspi], &testConfig, errStr));

      testPassed = mspi_check_data_match((uint8_t *)g_PsramRXBuffer, (uint8_t *)g_PsramTXBuffer, testConfig.NumBytes);

      if (!testPassed)
      {
        am_util_stdio_printf("DDR PSRAM Write Read test fail at NumBytes = %d, speed = %s, test_cnt= %d, MSPI #%d\n", testConfig.NumBytes, mspi_test_speeds[speed].string, ui32TestCnt, mspi);
      }
      else
      {
        am_util_stdio_printf("DDR PSRAM Write Read test at speed = %s, test_cnt= %d, passed on MSPI #%d\n",mspi_test_speeds[speed].string, ui32TestCnt, mspi);
      }

      // write/read in xip mode
      testPassed = mspi_ddr_xipmm_data_test();
      if (!testPassed)
      {
        am_util_stdio_printf("DDR PSRAM XIPMM Data test failed at speed = %s, test_cnt= %d, MSPI #%d\n", mspi_test_speeds[speed].string, ui32TestCnt, mspi);
      }
      else
      {
        am_util_stdio_printf("DDR PSRAM XIPMM Data test at speed = %s, test_cnt= %d, passed on MSPI #%d\n",mspi_test_speeds[speed].string, ui32TestCnt, mspi);
      }
    }

    TEST_ASSERT_TRUE(mspi_psram_task_deinit(pWidget[MSPI_PSRAM_MODULE],errStr));

    TEST_ASSERT_TRUE(testPassed);
}


//*****************************************************************************
//
// mspi psram task.
//
//*****************************************************************************
void MspiPsramTask(void *pvParameters)
{
    am_util_stdio_printf("\nmspi psram task start\n");

    g_semMspiFlashTimingScan = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semMspiFlashTimingScan != NULL);

    while(1)
    {
      mspi_psram_test();

      if (eTaskGetState(mspi_flash_task_handle) == eDeleted)
      {
          break;
      }
    }

    am_util_stdio_printf("\nmspi psram: task end!\n");

    vTaskDelete(NULL);

    while (1);
}
