//*****************************************************************************
//
//! @file etm_test_cases.c
//!
//! @brief Example test case program.
//
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

// TODO - Consider enhancing the test by installing HardFault_Handler and adding
//        writes/reads from peripheral registers and memories as the devices
//        are enabled/disabled.

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

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
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Helper macro
//
//*****************************************************************************
#define PRINT_ERROR(error)                                                    \
    print_error((error), __FILE__, __LINE__)

void
print_error(uint32_t ui32Error, const char *pcFilename, uint32_t ui32Line)
{
    if (ui32Error)
    {
        am_util_stdio_printf("\n***ERROR***\n"
                             "File: %s\n"
                             "Line: %d\n"
                             "Error: 0x%08X\n",
                             pcFilename, ui32Line, ui32Error);
    }
}

//*****************************************************************************
//
// Test Cases.
//
//*****************************************************************************

am_hal_gpio_pincfg_t TracePinCfg =
{
  .GP.cfg_b.uFuncSel         = 2,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uSlewRate        = 0,
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    .GP.cfg_b.eForceInputEn    = 0,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0,
};

extern int etm_trace (void);

void
etm1_test(void)
{
  // Enable the DEBUG TPIU.
  *(uint32_t*)(0x40020250) = 0x4233;

  // Set up the Trace Pins on FPGA.
  am_hal_gpio_pinconfig(50, TracePinCfg);
  am_hal_gpio_pinconfig(51, TracePinCfg);
  am_hal_gpio_pinconfig(52, TracePinCfg);
  am_hal_gpio_pinconfig(53, TracePinCfg);
  am_hal_gpio_pinconfig(54, TracePinCfg);

  etm_trace();
}
