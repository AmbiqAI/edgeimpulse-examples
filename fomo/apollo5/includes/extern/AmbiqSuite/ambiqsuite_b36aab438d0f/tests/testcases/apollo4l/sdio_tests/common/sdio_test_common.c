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

uint8_t ui8RdBuf[BUF_LEN] AM_BIT_ALIGNED(128);
uint8_t ui8WrBuf[BUF_LEN] AM_BIT_ALIGNED(128);

#ifdef EMMC_SCATTER_TEST
uint8_t         *pIoVectorWrBuf[IO_VECTOR_NUM];
uint8_t         *pIoVectorRdBuf[IO_VECTOR_NUM];

am_hal_card_iovec_t WriteVec[IO_VECTOR_NUM];
am_hal_card_iovec_t ReadVec[IO_VECTOR_NUM];

AM_SHARED_RW uint8_t ui8WrBufSSRAM[BUF_LEN] AM_BIT_ALIGNED(128);   //algined 128 bit
AM_SHARED_RW uint8_t ui8RdBufSSRAM[BUF_LEN] AM_BIT_ALIGNED(128);   //algined 128 bit
#endif

sdio_speed_t sdio_test_speeds[] =
{
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
};

sdio_voltage_t sdio_test_voltages[] =
{
  { AM_HAL_HOST_BUS_VOLTAGE_3_3,    "3.3V" },
  { AM_HAL_HOST_BUS_VOLTAGE_1_8,    "1.8V" },
};

#ifdef COLLECT_BENCHMARKS
float32_t f32WriteBandwidthTotal = 0.0;
float32_t f32ReadBandwidthTotal = 0.0;
uint32_t g_ui32Count = 0; 

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
    
#if (!defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P) && !defined(AM_PART_APOLLO4L))
  //
  // Set the clock frequency.
  //
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
#endif
  
#if !defined(APOLLO4_FPGA)
  //
  // Set the default cache configuration
  //
  am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
  am_hal_cachectrl_enable();
#endif

  //
  // Configure the board for low power operation.
  //
  am_bsp_low_power_init();

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
  am_util_stdio_printf("Apollo4 SDIO Test Cases\n\n");

  //
  // Configure SDIO PINs.
  //
  am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);

  //
  // level shift control in emmc board
  //
#if defined(FMC_BOARD_EMMC_TEST) || defined(SD_CARD_BOARD_SUPPORT_1_8_V)
  am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
  am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#else
  am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
  am_hal_gpio_output_set(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif

  //
  // initialize the test read and write buffers
  //
  for (int i = 0; i < BUF_LEN; i++)
  {
      ui8WrBuf[i] = rand() % 256;
      ui8RdBuf[i] = 0x0;
#ifdef EMMC_SCATTER_TEST
      ui8WrBufSSRAM[i] = ~(i % 256);
#endif
  }
}

void
globalTearDown(void)
{
}

