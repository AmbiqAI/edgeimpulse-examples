//*****************************************************************************
//
//! @file usb_test_common.c
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

#include "usb_test_common.h"

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
#if !defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B)
  //
  // Set the clock frequency.
  //
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
#endif

  //
  // Set the default cache configuration
  //
  //am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

  //
  // Configure the board for low power operation.
  //
  am_bsp_low_power_init();

  //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo4 USB Test Cases\n\n");
}

void
globalTearDown(void)
{

}

