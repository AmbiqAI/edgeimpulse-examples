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

#if 0  // per YD
#define BLK_NUM 64  //128
#define BUF_LEN 512*BLK_NUM
#endif

// at the moment results under 1ms are not reliable.  
#define WAKE_INTERVAL_IN_MS     1  
                       
#define XT_PERIOD               32768

#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000)

#ifdef APOLLO4_FPGA
// The FPGA XT appears to run at about 1.5MHz (1.5M / 32K = 48).
#undef  WAKE_INTERVAL
#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000 * 48)
#endif


extern  void stimer_init();  // per YD

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
  
  stimer_init();
    
  am_hal_interrupt_master_enable();
  
    //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo4b  emmc read_write_tests with Filesystem \n\n");
    
}

void
globalTearDown(void)
{
}

