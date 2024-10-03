//*****************************************************************************
//
//! @file clkgen_test_cases.c
//!
//! @brief CLKGEN test cases.
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
#include "am_widget_clkgen.h"
#include "am_util.h"

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

#if 0
  //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
#endif
    
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 CLKGEN Test Cases\n\n");
}

void
globalTearDown(void)
{
}


//*****************************************************************************
//  Helper functions
//*****************************************************************************
static void clkgen_test_05s_delay(uint32_t nHalfSecs)
{
    // nHalfSecs = The number of half seconds to wait.

    while ( nHalfSecs )
    {
        am_util_delay_ms(500);
        am_util_stdio_printf(".");      // Progress indicator
        nHalfSecs--;
    }
    am_util_stdio_printf("\n");
}

static uint32_t rtc_seconds(uint32_t ui32DelayMS)
{
    uint32_t ui32Seconds;
    am_hal_rtc_time_t sTimeBegin, sTimeEnd;

    //
    // Get the current RTC time, wait, get it again.
    //
    am_hal_rtc_time_get(&sTimeBegin);

    //
    // Divide the total delay into a series of half-second delays with a
    // progress indicator.
    // The time taken for the printing will also provide a built in
    // timing tolerance.
    //
    clkgen_test_05s_delay(ui32DelayMS / 500);

    //
    // If needed, provide a little additional timing buffer.
    //
    //  am_util_delay_ms(10);

    //
    // Get the current time
    //
    am_hal_rtc_time_get(&sTimeEnd);

    //
    // Return the elapsed number of seconds.
    //
    ui32Seconds = 0;
    if (sTimeEnd.ui32Second >= sTimeBegin.ui32Second)
    {
        ui32Seconds += sTimeEnd.ui32Second - sTimeBegin.ui32Second;
    }
    else
    {
        ui32Seconds += 60 + sTimeEnd.ui32Second - sTimeBegin.ui32Second;
        --sTimeEnd.ui32Minute;
    }

    if (sTimeEnd.ui32Minute >= sTimeBegin.ui32Minute)
    {
        ui32Seconds += 60 * (sTimeEnd.ui32Minute - sTimeBegin.ui32Minute);
    }
    else
    {
        ui32Seconds += 60 + (60 * (sTimeEnd.ui32Minute - sTimeBegin.ui32Minute));
        --sTimeEnd.ui32Hour;
    }

    if (sTimeEnd.ui32Hour >= sTimeBegin.ui32Hour)
    {
        ui32Seconds += 3600 * (sTimeEnd.ui32Hour - sTimeBegin.ui32Hour);
    }
    else
    {
        // Right now we just won't worry about flipping over to a new day.
        ui32Seconds = 0;
    }

    return ui32Seconds;
} // rtc_seconds()

//*****************************************************************************
//
//  Test Case
//  CLKGEN status
//
//*****************************************************************************
bool
clkgen_test_status(void)
{
    bool bTestPass = true;
    uint32_t ui32RetVal;
    am_hal_clkgen_status_t sStatus;


    am_util_stdio_printf("\nBegin clkgen_test_status():\n");

    //
    // Force RTC oscillator to known state.
    // We need to do this because the OSCF bit powers up 1, but resets to 0.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL, 0);
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);

    //
    // Wait 2 seconds before checking status in case the RTC oscillator has
    // been recently updated.
    //
    clkgen_test_05s_delay(4);       // 2.0 second delay with progress indicator
    am_util_stdio_printf(".\n");    // End progress indication

    ui32RetVal = am_hal_clkgen_status_get(&sStatus);

    if ( ui32RetVal )
    {
        am_util_stdio_printf("Fail: call to am_hal_clkgen_status() failed, return = %d.\n",
                             ui32RetVal);
        bTestPass = false;
    }

    if ( sStatus.ui32SysclkFreq != AM_HAL_CLKGEN_FREQ_MAX_HZ )
    {
        am_util_stdio_printf("Fail: ui32SysclkFreq returned %d (expected %d).\n",
                              sStatus.ui32SysclkFreq, AM_HAL_CLKGEN_FREQ_MAX_HZ);
        bTestPass = false;
    }

    if ( sStatus.eRTCOSC != AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC )
    {
        am_util_stdio_printf("Fail: XTAL is not the current RTC oscillator.\n");

        // Wait additional time to see if it clears up
        am_util_stdio_printf("Attempting additional delay after RTC oscillator change.\n");
        clkgen_test_05s_delay(3);   // Total of 3.5 seconds
        ui32RetVal = am_hal_clkgen_status_get(&sStatus);
        if ( sStatus.eRTCOSC == AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL )
        {
            am_util_stdio_printf("      The additional time was enough for the osc status change to show up.\n");
            am_util_stdio_printf("      Nonetheless this is still a FAIL condition.\n");
        }
        else
        {
            am_util_stdio_printf("Fail: Even the additional wait time was not enough.\n");
        }
    }


    //
    // Check OSCF.
    //
    if ( sStatus.bXtalFailure )
    {
        am_util_stdio_printf("Fail: XtalFailure detected, expect no fail after a reset..\n");
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // clkgen_test_status()

//*****************************************************************************
//
//  Test Case
//  CLKGEN control
//
//*****************************************************************************
void
clkgen_test_control(void)
{
    uint32_t ui32RetVal, ui32RetVal2, ui32Seconds;
    bool bTestPass = true;
    am_hal_clkgen_status_t sStatus;

    am_util_stdio_printf("\nBegin clkgen_test_control():\n");

    // *************************************************************************
    //
    // XTAL testing.
    //
    // *************************************************************************
    //
    // Disable the RTC oscillator
    //
    am_hal_rtc_osc_disable();

    //
    // Set up the RTC to run on the XTAL.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);
    am_hal_rtc_osc_enable();

    // OSEL: 0=XT, 1=LFRC
    if ( CLKGEN->OCTRL_b.OSEL )
    {
        am_util_stdio_printf("Fail: XTAL not selected.\n");
        bTestPass = false;
    }

    //
    // Wait 2 seconds after changing the RTC oscillator before checking status.
    //
    clkgen_test_05s_delay(4);       // 2.0 second delay with progress indicator

    am_hal_clkgen_status_get(&sStatus);

    if ( sStatus.eRTCOSC != AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC )
    {
        am_util_stdio_printf("Fail: XTAL is not identified as the RTC oscillator (RTCOSC=%d).\n", sStatus.eRTCOSC);
        bTestPass = false;
    }

    if ( sStatus.bXtalFailure )
    {
        am_util_stdio_printf("Fail: XTAL is shown as in fail mode.\n");
        bTestPass = false;
    }

    ui32Seconds = rtc_seconds(1000);
    if ( ui32Seconds != 1 )
    {
        am_util_stdio_printf("Fail: RTC with XTAL expected to count 1 second, actual count=%d.\n", ui32Seconds);
        bTestPass = false;
    }

#if defined(AM_PART_APOLLO4)
    if ( CLKGEN->OCTRL_b.STOPXT == 0 )
    {
        am_util_stdio_printf("Fail: XTAL is still enabled.\n");
        bTestPass = false;
    }
#endif

    //
    //  Give the LFRC a little time to come to a stop.
    //
    clkgen_test_05s_delay(1);       // 0.5 second delay with progress indicator

    //
    // Get current RTC values (extra half-second delay to give plenty of chance
    // for an increment if it should happen to still be running).
    //
    ui32Seconds = rtc_seconds(2000);
    if ( ui32Seconds != 2 )
    {
        am_util_stdio_printf("Fail: RTC still counting with XTAL, expected 2 seconds, actual count=%d.\n", ui32Seconds);
        bTestPass = false;
    }

    // *************************************************************************
    //
    // LFRC testing.
    //
    // *************************************************************************
    //
    // Disable the RTC oscillator
    //
    am_hal_rtc_osc_disable();

    //
    // Do initializations via HAL calls.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);

#if defined(AM_PART_APOLLO4)
    if ( CLKGEN->OCTRL_b.STOPRC )
    {
        am_util_stdio_printf("Fail: LFRC is still stopped.\n");
        bTestPass = false;
    }
#endif

    //
    // Set up the RTC to run on the LFRC.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);
    am_hal_rtc_osc_enable();

    //
    // Wait 2 seconds after changing the RTC oscillator before checking status.
    //
    //am_util_stdio_printf("\nCheck that LFRC is the RTC oscillator, then check the count:\n");
    clkgen_test_05s_delay(4);       // 2.0 second delay with progress indicator

    am_hal_clkgen_status_get(&sStatus);

    if ( sStatus.eRTCOSC != AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC )
    {
        am_util_stdio_printf("Fail: LFRC is not identified as the RTC oscillator (RTCOSC=%d).\n", sStatus.eRTCOSC);
        bTestPass = false;
    }

    ui32Seconds = rtc_seconds(1000);
    if ( ui32Seconds != 1 )
    {
        am_util_stdio_printf("Fail: RTC with LFRC expected to count 1 second, actual count=%d.\n", ui32Seconds);
        bTestPass = false;
    }

#if defined(AM_PART_APOLLO4)
    if ( CLKGEN->OCTRL_b.STOPRC == 0 )
    {
        am_util_stdio_printf("Fail: LFRC is still enabled.\n");
        bTestPass = false;
    }
#endif

    ui32Seconds = rtc_seconds(2000);
    if ( ui32Seconds != 2 )
    {
        am_util_stdio_printf("Fail: RTC still counting with LFRC, expected 2 seconds, actual count=%d.\n", ui32Seconds);
        bTestPass = false;
    }

    am_hal_rtc_osc_disable();

    am_util_stdio_printf(".\n");    // End progress indication

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // clkgen_test_control()
