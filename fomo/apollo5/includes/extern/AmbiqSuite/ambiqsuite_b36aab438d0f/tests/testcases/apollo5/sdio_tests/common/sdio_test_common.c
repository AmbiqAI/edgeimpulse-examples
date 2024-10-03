//*****************************************************************************
//
//! @file sdio_test_common.c
//!
//! @brief Example test case program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "sdio_test_common.h"

#define DEBUG_PRINT        am_util_stdio_printf

#ifdef EMMC_SCATTER_TEST
uint8_t         *pIoVectorWrBuf[IO_VECTOR_NUM];
uint8_t         *pIoVectorRdBuf[IO_VECTOR_NUM];

am_hal_card_iovec_t WriteVec[IO_VECTOR_NUM];
am_hal_card_iovec_t ReadVec[IO_VECTOR_NUM];

uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

AM_SHARED_RW uint8_t ui8WrBufSSRAM[BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t ui8RdBufSSRAM[BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
#else
AM_SHARED_RW uint8_t ui8RdBuf[BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t ui8WrBuf[BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
#endif

AM_SHARED_RW  uint32_t  DMATCBBuffer[2560] __attribute__((aligned(32))); //algined 32 byte to match a cache line

am_hal_mpu_region_config_t sMPUConfig =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};

am_hal_mpu_attr_t sMPUAttribute =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
} ;

sdio_speed_t sdio_test_speeds[] =
{
  { 96000000,    "96MHz" },
  { 48000000,    "48MHz" },
  { 24000000,    "24MHz" },
  { 12000000,    "12MHz" },
  {  3000000,     "3MHz" },
  {   750000,    "750KHz"},
  {   375000,    "375KHz"},
};

sdio_width_t sdio_test_widths[] =
{
  { AM_HAL_HOST_BUS_WIDTH_1,    "1bit" },
  { AM_HAL_HOST_BUS_WIDTH_4,    "4bit" },
  { AM_HAL_HOST_BUS_WIDTH_8,    "8bit" },
};

sdio_mode_t sdio_test_modes[] =
{
  { AM_HAL_HOST_UHS_SDR50,    "SDR50" },
  { AM_HAL_HOST_UHS_DDR50,    "DDR50" },
  { AM_HAL_HOST_UHS_SDR104,   "SDR104" },
};

sdio_xfer_mode_t sdio_test_xfer_modes[] = 
{
  { AM_WIDGET_HOST_XFER_PIO_SYNC,     "PIO Synchronous Transfer" },
  { AM_WIDGET_HOST_XFER_ADMA_SYNC,    "ADMA Synchronous Transfer" },
  { AM_WIDGET_HOST_XFER_ADMA_ASYNC,   "ADMA Asynchronous Transfer" },
  { AM_WIDGET_HOST_XFER_SDMA_SYNC,    "SDMA Synchronous Transfer" },
  { AM_WIDGET_HOST_XFER_SDMA_ASYNC,   "SDMA Asynchronous Transfer" },
};

sdio_pwr_ctrl_policy_t sdio_test_pwrctrl_policies[] =
{
  { AM_HAL_CARD_PWR_CTRL_NONE,                      "Power Policy None" },
  { AM_HAL_CARD_PWR_CTRL_SDHC_OFF,                  "Power Policy SDHC Off" },
  { AM_HAL_CARD_PWR_CTRL_SDHC_OFF_AND_CARD_SLEEP,   "Power Policy SDHC Off & Card Sleep" },
  { AM_HAL_CARD_PWR_CTRL_SDHC_AND_CARD_OFF,         "Power Policy SDHC and Card Off" },
};

sdio_partition_mode_t sdio_test_partition_modes[] = 
{
  { AM_DEVICES_EMMC_NO_BOOT_ACCESS,  "no boot access" },
  { AM_DEVICES_EMMC_RPMB_ACCESS,     "rpmb access" },
};

sdio_erase_type_t sdio_test_erase_types[] = 
{
  { AM_HAL_ERASE,         "Erase" },
  { AM_HAL_SECURE_ERASE,  "Secure Erase" },
  { AM_HAL_TRIM,          "Trim" },
  { AM_HAL_SECURE_TRIM1,  "Secure Trim1" },
  { AM_HAL_SECURE_TRIM2,  "Secure Trim2" },
  { AM_HAL_DISCARD,       "Dicard" },
};

sdio_voltage_t sdio_test_voltages[] =
{
  { AM_HAL_HOST_BUS_VOLTAGE_3_3,    "3.3V" },
  { AM_HAL_HOST_BUS_VOLTAGE_1_8,    "1.8V" },
};

#ifdef COLLECT_BENCHMARKS
uint32_t g_ui32Count = 0; 

sdio_xfer_mode_t sdio_benchmark_xfer_modes[] = 
{
  { AM_WIDGET_HOST_XFER_PIO_SYNC,     "PIO" },
  { AM_WIDGET_HOST_XFER_ADMA_SYNC,    "Sync" },
  { AM_WIDGET_HOST_XFER_ADMA_ASYNC,   "Async" },
};
//*****************************************************************************
//
// Init function for Timer A0.  per DaveC
//
//*****************************************************************************
void
stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}

//*****************************************************************************
//
// Init ctimer for emmc benchmark
//
//*****************************************************************************
void
ctimer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up the default configuration.
    //
    am_hal_timer_default_config_set(&TimerConfig);

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock             = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction               = AM_HAL_TIMER_FN_UPCOUNT;

    //
    // Configure the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_config(TIMER_NUM, &TimerConfig))
    {
        am_util_stdio_printf("Fail to config timer%d\n", TIMER_NUM);
    }

    //
    // Clear the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_clear_stop(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to clear&stop timer%d\n", TIMER_NUM);
    }

    //
    // Enable the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_enable(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to enable timer%d\n", TIMER_NUM);
    }

    //
    // Start the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS !=  am_hal_timer_start(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to start timer%d\n", TIMER_NUM);
    }
}

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)  also per DaveC
//
//*****************************************************************************
void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    g_ui32Count++;
}

#endif

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{

#if defined(AM_PART_BRONCO)
  //
  // Set the default cache configuration
  //
  am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
  am_hal_cachectrl_enable();
#elif defined(AM_PART_APOLLO5A)
  //
  //  Enable the I-Cache and D-Cache.
  //
  am_hal_cachectrl_icache_enable();
  am_hal_cachectrl_dcache_enable(true);
#endif

  //
  // Configure the board for low power operation.
  //
  am_bsp_low_power_init();

  //
  // Set up the attributes.
  //
  am_hal_mpu_attr_configure(&sMPUAttribute, 1);
  //
  // Clear the MPU regions.
  //
  am_hal_mpu_region_clear();
  //
  // Set up the regions.
  //
  am_hal_mpu_region_configure(&sMPUConfig, 1);
  //
  // Invalidate and clear DCACHE, this is required by CM55 TRF.
  //
  am_hal_cachectrl_dcache_invalidate(NULL, true);
  //
  // MPU enable
  //
  am_hal_mpu_enable(true, true);

#if defined(APOLLO5_FPGA) && defined(AM_PART_APOLLO5A)
  //SCB_DisableICache();
  //SCB_DisableDCache();
#endif

#if COLLECT_BENCHMARKS
  ctimer_init();
#endif
  
  //
  // Global interrupt enable
  //
  am_hal_interrupt_master_enable();

#ifdef MBEDTLS_SHA256
    am_devices_emmc_mebedtls_init();
#endif

  //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo5 SDIO Test Cases\n\n");

  //
  // level shift control in emmc board
  //
#ifdef SD_CARD_TEST
  am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
  am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif

  //
  // initialize the test read and write buffers
  //
  for (int i = 0; i < BUF_LEN; i++)
  {
      ui8WrBuf[i] = rand() % 256;
      ui8RdBuf[i] = 0x0;
#ifdef EMMC_SCATTER_TEST
      ui8WrBufSSRAM[i] = 0xFF & ( 0xFF - i );
#endif
  }
}

void
globalTearDown(void)
{
}

