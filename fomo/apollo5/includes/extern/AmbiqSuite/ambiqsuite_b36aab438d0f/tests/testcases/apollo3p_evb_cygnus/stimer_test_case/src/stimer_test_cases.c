//*****************************************************************************
//
//! @file sample_test_cases.c
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

#include "am_mcu_apollo.h"
#include "am_widget_stimer.h"
#include "unity.h"
//#include <setjmp.h>

#define XTAL_CFG_DELAY(n)                                                       \
    if (1)                                                                      \
    {                                                                           \
    /*                                                                          \
    // For yet unexplained reasons, when switching to XTAL, it seems that we    \
    //  have to configure then wait for a short period of time (1/2 sec seems   \
    //  about adequate) before going on to the actual configuration.            \
    // We'll do the workaround but print a message about it.  Since printing of \
    //  the message takes some time, we can shorten the delay.                  \
    // It's not clear if this may be a Corvette A1 issue, a 3-read issue,       \
    // or something else.                                                       \
    */                                                                          \
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_RUN);     \
    am_util_stdio_printf("\n*** Workaround " #n ": XTAL ***");                  \
    am_hal_flash_delay(FLASH_CYCLES_US_NOCACHE(300000));                        \
    }

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
    am_util_stdio_printf("\n");
    am_util_stdio_printf("*********************************************************************\n");
    am_util_stdio_printf("*                                                                   *\n");
    am_util_stdio_printf("* NOTICE: stimer_test may not run properly if Fireball is attached! *\n");
    am_util_stdio_printf("*                                                                   *\n");
    am_util_stdio_printf("*********************************************************************\n");
    am_util_stdio_printf("\n");

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Make sure interrupt registers work.
//
//*****************************************************************************
void
int_test(void)
{
    TEST_ASSERT_EQUAL_INT(0, am_widget_stimer_int_test());
}

//*****************************************************************************
//
// Make sure STIMERs run at the correct speed.
//
//*****************************************************************************
void
speed_test(void)
{
#if APOLLO3_FPGA
    //
    // For the FPGA:
    //  HFRC setting of 12MHz or less  is actually 255.1 KHz (roughly 1024 KHz / 4).
    //  HFRC setting of 24MHz or 48MHz is actually 12.5 MHz.
    //  XTAL setting of 32KHz is actually 12.5KHz.
    //  XTAL setting of 32KHz/2 is actually 6.25KHz.
    //  LFRC setting of LFRC  is actually 390.6 Hz.
    //  LFRC setting of LFRC/2  is actually 195.3 Hz.
    //
    TEST_ASSERT_INT_WITHIN(2551, 255100, am_widget_stimer_speed_test(AM_HAL_STIMER_HFRC_3MHZ));
    TEST_ASSERT_INT_WITHIN(2551, 255100, am_widget_stimer_speed_test(AM_HAL_STIMER_HFRC_187_5KHZ));
    TEST_ASSERT_INT_WITHIN(1250,  12500, am_widget_stimer_speed_test(AM_HAL_STIMER_XTAL_32KHZ));
    TEST_ASSERT_INT_WITHIN( 625,   6250, am_widget_stimer_speed_test(AM_HAL_STIMER_XTAL_16KHZ));
    TEST_ASSERT_INT_WITHIN(  39,    391, am_widget_stimer_speed_test(AM_HAL_STIMER_XTAL_1KHZ));
    TEST_ASSERT_INT_WITHIN(  39,    391, am_widget_stimer_speed_test(AM_HAL_STIMER_LFRC_1KHZ));
#else
    //
    // Tolerance of uncalibrated HFRC at room is roughly 1% (per Jamie H 5/15/18).
    //
    TEST_ASSERT_INT_WITHIN(3000, 3000000, am_widget_stimer_speed_test(AM_HAL_STIMER_HFRC_3MHZ));
    TEST_ASSERT_INT_WITHIN(1875,  187500, am_widget_stimer_speed_test(AM_HAL_STIMER_HFRC_187_5KHZ));

    XTAL_CFG_DELAY(1);
    TEST_ASSERT_INT_WITHIN( 328,   32768, am_widget_stimer_speed_test(AM_HAL_STIMER_XTAL_32KHZ));
    TEST_ASSERT_INT_WITHIN( 164,   16384, am_widget_stimer_speed_test(AM_HAL_STIMER_XTAL_16KHZ));
    TEST_ASSERT_INT_WITHIN(  10,    1024, am_widget_stimer_speed_test(AM_HAL_STIMER_XTAL_1KHZ));

    //
    // Tolerance of uncalibrated LFRC at room is roughly 10% (per Jamie H 5/15/18).
    // For testing purposes, we'll allow 3% tolerance.
    //
    TEST_ASSERT_INT_WITHIN(  100,    1024, am_widget_stimer_speed_test(AM_HAL_STIMER_LFRC_1KHZ));
#endif // APOLLO3_FPGA
}

//*****************************************************************************
//
// Make sure STIMER capture registers work.
//
//*****************************************************************************
void
capture_test(void)
{
#if APOLLO3_FPGA
    //
    // For the FPGA:
    //  HFRC setting of 12MHz or less  is actually 255.1 KHz (roughly 1024 KHz / 4).
    //  HFRC setting of 24MHz or 48MHz is actually 12.5 MHz.
    //  XTAL setting of 32KHz is actually 12.5KHz.
    //  XTAL setting of 32KHz/2 is actually 6.25KHz.
    //  LFRC setting of LFRC  is actually 390.6 Hz.
    //  LFRC setting of LFRC/2  is actually 195.3 Hz.
    //
    TEST_ASSERT_INT_WITHIN( 2551, 255100, am_widget_stimer_capture_test(AM_HAL_STIMER_HFRC_3MHZ));
    TEST_ASSERT_INT_WITHIN( 2551, 255100, am_widget_stimer_capture_test(AM_HAL_STIMER_HFRC_187_5KHZ));
    TEST_ASSERT_INT_WITHIN( 1250,  12500, am_widget_stimer_capture_test(AM_HAL_STIMER_XTAL_32KHZ));
    TEST_ASSERT_INT_WITHIN(  625,   6250, am_widget_stimer_capture_test(AM_HAL_STIMER_XTAL_16KHZ));
    TEST_ASSERT_INT_WITHIN(   39,    391, am_widget_stimer_capture_test(AM_HAL_STIMER_XTAL_1KHZ));
    TEST_ASSERT_INT_WITHIN(   39,    391, am_widget_stimer_capture_test(AM_HAL_STIMER_LFRC_1KHZ));
#else
    TEST_ASSERT_INT_WITHIN(30000, 3000000, am_widget_stimer_capture_test(AM_HAL_STIMER_HFRC_3MHZ));
    TEST_ASSERT_INT_WITHIN( 1875,  187500, am_widget_stimer_capture_test(AM_HAL_STIMER_HFRC_187_5KHZ));
    XTAL_CFG_DELAY(2);
    TEST_ASSERT_INT_WITHIN(  328,   32768, am_widget_stimer_capture_test(AM_HAL_STIMER_XTAL_32KHZ));
    TEST_ASSERT_INT_WITHIN(  164,   16384, am_widget_stimer_capture_test(AM_HAL_STIMER_XTAL_16KHZ));
    TEST_ASSERT_INT_WITHIN(   10,    1024, am_widget_stimer_capture_test(AM_HAL_STIMER_XTAL_1KHZ));
    TEST_ASSERT_INT_WITHIN(  102,    1024, am_widget_stimer_capture_test(AM_HAL_STIMER_LFRC_1KHZ));
#endif
}

//*****************************************************************************
//
// Make sure STIMER compare registers work.
//
//*****************************************************************************
void
compare_test(void)
{
#if APOLLO3_FPGA
#define CMPR_DELTA  3
    //
    // NOTE - delta values may need to be larger for Apollo3 due to longer
    //       APB register read access times than previous devices.
    // For the FPGA, 3 seems to be adequate.  May be longer on silicon.
    //

    //
    // For the FPGA:
    //  HFRC setting of 12MHz or less  is actually 255.1 KHz (roughly 1024 KHz / 4).
    //  HFRC setting of 24MHz or 48MHz is actually 12.5 MHz.
    //  XTAL setting of 32KHz is actually 12.5KHz.
    //  XTAL setting of 32KHz/2 is actually 6.25KHz.
    //  LFRC setting of LFRC  is actually 390.6 Hz.
    //  LFRC setting of LFRC/2  is actually 195.3 Hz.
    //
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_HFRC_3MHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_HFRC_187_5KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_XTAL_32KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_XTAL_16KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_XTAL_1KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_LFRC_1KHZ));
#else
#define CMPR_DELTA  1
    //
    // NOTE - delta values may need to be larger for Apollo3 due to longer
    //       APB register read access times than previous devices.
    //

#if 1
    //
    // For this test, we seem to need a little extra delta for 3MHz (latency is up to 7 cycles).
    //
#undef CMPR_DELTA
#define CMPR_DELTA  (1+15)
    am_util_stdio_printf("\n*** Workaround 3: 3MHz CMPR test ***");
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_HFRC_3MHZ));
#undef CMPR_DELTA
#define CMPR_DELTA  1
#endif
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_HFRC_187_5KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_XTAL_32KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_XTAL_16KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_XTAL_1KHZ));
    TEST_ASSERT_INT_WITHIN(CMPR_DELTA, 0, am_widget_stimer_compare_test(AM_HAL_STIMER_LFRC_1KHZ));
#endif // APOLLO3_FPGA
}
