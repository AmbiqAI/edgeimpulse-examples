//*****************************************************************************
//
//! @file dspram_read_test_common.c
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

#include "dspram_read_test_common.h"

#define DEBUG_PRINT        am_util_stdio_printf

uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

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
#if (!defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B))
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
    // Print the banner.
    //
    am_bsp_itm_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 DSPRAM Read Test Cases\n\n");
    am_util_stdio_printf("Stack and heap are placed into DSPRAM to use CM4 and SDIO DMA to read DSPRAM simultaneously \n\n");

    if ( !APOLLO4_GE_B2 )
    {
        TEST_FAIL();
        am_util_stdio_printf("\nTEST FAIL: This test must be run on Apollo4 B2 or greater.\n");
    }


    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);

#ifdef FMC_BOARD_EMMC_TEST
    //
    // FPGA level shift control
    //
    am_hal_gpio_pinconfig(76, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(76);
#endif

    //
    // initialize the test read and write buffers
    //
    for (int i = 0; i < BUF_LEN; i++)
    {
        ui8WrBuf[i] = rand() % 256;
        ui8RdBuf[i] = 0x0;
    }
}

void
globalTearDown(void)
{
}

