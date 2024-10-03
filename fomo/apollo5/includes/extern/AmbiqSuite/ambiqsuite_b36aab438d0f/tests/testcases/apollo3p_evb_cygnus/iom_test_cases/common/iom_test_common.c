//*****************************************************************************
//
//! @file iom_test_common.c
//!
//! @brief IOM test cases common definitions and functions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "iom_test_common.h"

AMTEST_IOM_FRAM_STATUS_E g_bFramExpected[AM_REG_IOM_NUM_MODULES];

bool    g_bMaxSpeed48MHz = true;

const am_widget_iom_devices_t      DefaultI2CConfig =
{
    .ui32ClockFreq        = AM_HAL_IOM_100KHZ,
    .pNBTxnBuf            = NULL,
    .ui32NBTxnBufLength   = 0,
};

const am_widget_iom_devices_t      DefaultSPIConfig =
{
//    .ui32ClockFreq        = AM_HAL_IOM_96MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_48MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_24MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_16MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_12MHZ,
    .ui32ClockFreq        = AM_HAL_IOM_8MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_6MHZ,
    .pNBTxnBuf            = NULL,
    .ui32NBTxnBufLength   = 0,
};

const struct
{
    const uint32_t MHz;
    const char *MHzString;
} speedString[] =
{
    {AM_HAL_IOM_48MHZ,  "48MHZ"},
    {AM_HAL_IOM_24MHZ,  "24MHZ"},
    {AM_HAL_IOM_16MHZ,  "16MHZ"},
    {AM_HAL_IOM_12MHZ,  "12MHZ"},
    {AM_HAL_IOM_8MHZ,   "8MHZ"},
    {AM_HAL_IOM_6MHZ,   "6MHZ"},
    {AM_HAL_IOM_4MHZ,   "4MHZ"},
    {AM_HAL_IOM_3MHZ,   "3MHZ"},
    {AM_HAL_IOM_2MHZ,   "2MHZ"},
    {AM_HAL_IOM_1_5MHZ, "1.5MHZ"},
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
    {AM_HAL_IOM_750KHZ, "750KHZ"},
    {AM_HAL_IOM_500KHZ, "500KHZ"},
    {AM_HAL_IOM_400KHZ, "400KHZ"},
    {AM_HAL_IOM_375KHZ, "375KHZ"},
    {AM_HAL_IOM_250KHZ, "250KHZ"},
    {AM_HAL_IOM_125KHZ, "125KHZ"},
    {AM_HAL_IOM_100KHZ, "100KHZ"},
    {AM_HAL_IOM_50KHZ,  "50KHZ"},
    {AM_HAL_IOM_10KHZ,  "10KHZ"},
};

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
#if defined(APOLLO4_FPGA)
uint32_t    iom_instances[IOM_TEST_NUM_INSTANCES] = {0};
#else
uint32_t    iom_instances[IOM_TEST_NUM_INSTANCES] = {0,1,2,3,5,6,7};
#endif
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
iom_test_globalSetUp(void)
{

    //
    // Set the default cache configuration
    //
    //am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);
    //am_bsp_low_power_init();

    //
    // Print the banner.
    //
    am_bsp_itm_printf_enable();
    am_util_stdio_terminal_clear();
    // Dummy access to g_IomRegs to avoid linker taking it off
    // am_util_stdio_printf("Apollo3 IOM Test Cases %p\n\n", g_IomRegs[0].pRegFIFO);

#if (ENABLE_BURST_MODE == 1)
    {
      am_hal_burst_avail_e          eBurstModeAvailable;
      am_hal_burst_mode_e           eBurstMode;

      // Check that the Burst Feature is available.
      if (AM_HAL_STATUS_SUCCESS == am_hal_burst_mode_initialize(&eBurstModeAvailable))
      {
        if (AM_HAL_BURST_AVAIL == eBurstModeAvailable)
        {
          am_util_stdio_printf("Apollo3 Burst Mode is Available\n");
        }
        else
        {
          am_util_stdio_printf("Apollo3 Burst Mode is Not Available\n");
        }
      }
      else
      {
        am_util_stdio_printf("Failed to Initialize for Burst Mode operation\n");
      }

      // Put the MCU into "Burst" mode.
      if (AM_HAL_STATUS_SUCCESS == am_hal_burst_mode_enable(&eBurstMode))
      {
        if (AM_HAL_BURST_MODE == eBurstMode)
        {
          am_util_stdio_printf("Apollo3 operating in Burst Mode (96MHz)\n");
        }
      }
      else
      {
        am_util_stdio_printf("Failed to Enable Burst Mode operation\n");
      }

    }
#endif // ENABLE_BURST_MODE
#if 0
    am_hal_gpio_out_bit_clear(48);
    am_hal_gpio_pin_config(48, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(49);
    am_hal_gpio_pin_config(49, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(37);
    am_hal_gpio_pin_config(37, AM_HAL_PIN_OUTPUT);
#endif
#ifdef MCU_VALIDATION_DEBUG_REG2
    // MCU_VALIDATION_DEBUG_REG2 contains a bitmask indicating which IOMs to expect FRAM on
    // Wait for the input to be non-zero
    while(AM_REGVAL(MCU_VALIDATION_DEBUG_REG2) == 0)
    {
        am_util_delay_ms(10);
    }
    uint32_t input = AM_REGVAL(MCU_VALIDATION_DEBUG_REG2);
    for (uint32_t i = 0; i < AM_REG_IOM_NUM_MODULES; i++)
    {
        g_bFramExpected[i] = ((input >> i) & 0x1) ? AMTEST_IOM_FRAM_STATUS_PRESENT : AMTEST_IOM_FRAM_STATUS_NOT_PRESENT;
    }
        if (input & 0x40)
        {
            g_bMaxSpeed48MHz = true;
        }
        else
        {
            g_bMaxSpeed48MHz = false;
        }
#else
    for (uint32_t i = 0; i < AM_REG_IOM_NUM_MODULES; i++)
    {
        g_bFramExpected[i] = AMTEST_IOM_FRAM_STATUS_NOT_KNOWN;
    }
#endif
}

void
iom_test_globalTearDown(void)
{
}

//
//! Convert IOM speed setting to string.
//! @note Caller doesn't need to process an error (Unknown).
//
const char *getSpeedString(int speedIndex)
{
    for ( int i = 0; i < SPEED_STRING_SIZE; i++ )
    {
        if (speedIndex == speedString[i].MHz)
        {
            return speedString[i].MHzString;
        }
    }

    //return NULL;
    return "Unknown";
}

AMTEST_IOM_FRAM_STATUS_E isFramExpected(uint32_t ui32Module)
{
    return g_bFramExpected[ui32Module];
}

