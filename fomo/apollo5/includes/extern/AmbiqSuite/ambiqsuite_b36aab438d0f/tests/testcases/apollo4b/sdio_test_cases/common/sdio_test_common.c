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
#if (!defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P))
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

