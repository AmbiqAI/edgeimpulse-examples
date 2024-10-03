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
#define DSP_RAM0_WORKAROUND

#define BLK_NUM 64  //128
#define BUF_LEN 512*BLK_NUM

#ifdef DSP_RAM0_WORKAROUND
static void dsp_ram0_init(void)
{
    am_hal_pwrctrl_dsp_memory_config_t sExtSRAMMemCfg =
    {
        .bEnableICache      = false,
        .bRetainCache       = false,
        .bEnableRAM         = true,
        .bActiveRAM         = false,
        .bRetainRAM         = true
    };

    if (am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &sExtSRAMMemCfg) != 0)
    // ||am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &sExtSRAMMemCfg) != 0)
    {
        am_util_stdio_printf("DSP memory init error.\n");
    }
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
  
#ifdef DSP_RAM0_WORKAROUND
    dsp_ram0_init();
#endif

  //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo4 emmc_raw_read_write_tests \n\n");

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
    
}

void
globalTearDown(void)
{
}

