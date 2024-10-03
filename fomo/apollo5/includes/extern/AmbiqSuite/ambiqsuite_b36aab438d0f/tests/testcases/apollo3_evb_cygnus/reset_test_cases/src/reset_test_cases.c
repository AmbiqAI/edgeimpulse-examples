//*****************************************************************************
//
//! @file reset_test_cases.c
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

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_reset.h"

void          *pWidget;
char          errStr[128];

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
    //
    // Set the default cache configuration
    //
    //am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Configure the board for low power operation.
    //
    // am_bsp_low_power_init();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo3 Reset Test Cases\n\n");
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Test cases for the main program to run.
//
//*****************************************************************************
void
reset_test_enable_int(void)
{
    uint32_t ui32Ret;

    ui32Ret = am_widget_reset_enable_int_test(pWidget, (char*)&errStr);

    if (ui32Ret == AM_WIDGET_SUCCESS)
    {
        TEST_ASSERT_TRUE(true);
    }
    else
    {
        TEST_ASSERT_TRUE(false);
    }
}

