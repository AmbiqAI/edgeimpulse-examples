//*****************************************************************************
//
//! @file am_widget_timer.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_widget_timer.h"
#include "unity.h"
#include "am_util.h"

// Delay defined in microsecond
#define DELAY_MICROSEC       500000

// Conversion from second to microsecond
#define SEC_TO_MICROSEC      1000000

volatile uint32_t g_TimerValue;
volatile uint32_t g_TimerNum;
volatile bool     g_TimerDone;

void am_ctimer_isr(void)
{
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(g_TimerNum, AM_HAL_TIMER_COMPARE1));

    g_TimerValue = am_hal_timer_read(g_TimerNum);

    g_TimerDone = true;
}

//*****************************************************************************
//
// Individual TIMER ISRs.
//
//*****************************************************************************
// Variables needed for timer_rudimentary_interrupt().
static uint32_t g_ui32timernum = 0;
static uint32_t g_ui32timererr = 0;
static uint32_t g_ui32intnum = 0;
static uint8_t g_ui8cmpnum = 0;

static uint32_t timer_int_check(uint32_t ui32timernum)
{
    uint32_t ui32Stat, ui32Mask, ui32Ret;

    ui32Ret = 0;

    //
    // bit1=CMP1, bit0=CMP0
    //
    ui32Mask = 0x3 << (ui32timernum * 2);

    if ( am_hal_timer_interrupt_status_get(true, &ui32Stat) )
    {
        ui32Ret  |= 0x01;
    }

    if ( ui32Stat & ~ui32Mask )
    {
        ui32Ret |= 0x02;
    }

    // Only 1 INT bit should be set.
    if ( (ui32Stat & ui32Mask) == ui32Mask )
    {
        ui32Ret |= 0x04;
    }

    if ( ui32Stat & (1 << (ui32timernum * 2)) )
    {
        ui32Mask = (1 << (ui32timernum * 2));
        g_ui8cmpnum = 0;
    }
    else
    {
        ui32Mask = (2 << (ui32timernum * 2));
        g_ui8cmpnum = 1;
    }

    if ( am_hal_timer_interrupt_clear(ui32Mask) )
    {
        ui32Ret |= 0x08;
    }
    g_ui32intnum++;

    return ui32Ret;
}


void am_timer00_isr(void)
{
    g_ui32timernum = 0;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer01_isr(void)
{
    g_ui32timernum = 1;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer02_isr(void)
{
    g_ui32timernum = 2;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer03_isr(void)
{
    g_ui32timernum = 3;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer04_isr(void)
{
    g_ui32timernum = 4;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer05_isr(void)
{
    g_ui32timernum = 5;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer06_isr(void)
{
    g_ui32timernum = 6;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer07_isr(void)
{
    g_ui32timernum = 7;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer08_isr(void)
{
    g_ui32timernum = 8;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer09_isr(void)
{
    g_ui32timernum = 9;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer10_isr(void)
{
    g_ui32timernum = 10;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer11_isr(void)
{
    g_ui32timernum = 11;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer12_isr(void)
{
    g_ui32timernum = 12;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer13_isr(void)
{
    g_ui32timernum = 13;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer14_isr(void)
{
    g_ui32timernum = 14;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}
void am_timer15_isr(void)
{
    g_ui32timernum = 15;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf(".");
    //am_hal_delay_us(200000);
}


#if defined(APOLLO4_FPGA)
#define HFRC_BASE (APOLLO4_FPGA*1000000)
#define XT_BASE   16384
#define LFRC_BASE 50 // FIXME - For now it is 500Hz.
#elif defined(AM_PART_APOLLO4)
#define HFRC_BASE 48000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#elif defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
#define HFRC_BASE 96000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#endif

//
// Note:
//
// I've adjusted the thresholds for these tests by 2 clock periods because of a
// silicon issue tracked in JIRA under SYSTEST-14. Per this issue, TIMERS take
// an extra 1 to 2 clock cycles to start counting, so the final TIMER count
// after 1 second will be 1 to two clocks lower than expected.
//
// The adjustments below allow the low frequency test cases to pass despite
// this 1-2 clock discrepancy.
//
// 
// The LFRC clock and its pre-scaled clocks have been adjusted to a tolerance 
// of +-20% as described in the JIRA under SYSTEST-48. 
struct
{
    uint32_t    lower;
    uint32_t    upper;
} am_widget_timer_1sec_count[AM_HAL_TIMER_CLOCK_RTC_100HZ + 1] =
{
    // Lower Bound Pass         Upper Bound Pass
    { HFRC_BASE / 4    * 95 / 100 - 5, HFRC_BASE / 4    * 105 / 100},
    { HFRC_BASE / 16   * 95 / 100 - 5, HFRC_BASE / 16   * 105 / 100},
    { HFRC_BASE / 64   * 95 / 100 - 5, HFRC_BASE / 64   * 105 / 100},
    { HFRC_BASE / 256  * 95 / 100 - 5, HFRC_BASE / 256  * 105 / 100},
    { HFRC_BASE / 1024 * 95 / 100 - 5, HFRC_BASE / 1024 * 105 / 100},
    { HFRC_BASE / 4096 * 95 / 100 - 5, HFRC_BASE / 4096 * 105 / 100},
    { LFRC_BASE        * 75 / 100 - 5, LFRC_BASE        * 105 / 100},
    { LFRC_BASE / 2    * 75 / 100 - 5, LFRC_BASE / 2    * 105 / 100},
    { LFRC_BASE / 32   * 75 / 100 - 5, LFRC_BASE / 32   * 105 / 100},
    { 0,                               LFRC_BASE / 1024 * 105 / 100},
    { XT_BASE          * 95 / 100 - 5, XT_BASE          * 105 / 100},
    { XT_BASE / 2      * 95 / 100 - 5, XT_BASE / 2      * 105 / 100},
    { XT_BASE / 4      * 95 / 100 - 5, XT_BASE / 4      * 105 / 100},
    { XT_BASE / 8      * 95 / 100 - 5, XT_BASE / 8      * 105 / 100},
    { XT_BASE / 16     * 95 / 100 - 5, XT_BASE / 16     * 105 / 100},
    { XT_BASE / 32     * 95 / 100 - 5, XT_BASE / 32     * 105 / 100},
    { XT_BASE / 128    * 95 / 100 - 5, XT_BASE / 128    * 105 / 100},
    { 93,                              102 }
};

uint32_t        ui8PeriodicTimerSteps[] = {1000, 2000, 3000, 4000, 5000};

#define AM_WIDGET_TIMER_EXPECTED_PERIODIC_COUNT 15000

#define PATTERN_OUT0  12
#define PATTERN_OUT1  13
#define TRIGGER_OUT   11
#define TRIGGER_IN    10

am_hal_gpio_pincfg_t g_OutPinCfg =
{
    .GP.cfg_b.uFuncSel             = 6, //CT
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

//*****************************************************************************
//
// Run the TIMER to see if it runs according to the settings.
//
//*****************************************************************************
bool
am_widget_timer_speed_check(uint32_t ui32TimerNum,
                            am_hal_timer_clock_e eTimerClock)
{
    am_hal_timer_config_t       TimerConfig;
    uint32_t                    ui32TimerCount;
    bool                        bRetVal;

    //
    // Set up the default configuration.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&TimerConfig));

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock = eTimerClock;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    //
    // Configure the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32TimerNum, &TimerConfig));

    //
    // Clear the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32TimerNum));

    //
    // Enable the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_enable(ui32TimerNum));

    //
    // Start the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_start(ui32TimerNum));

    //
    // Delay for a pre-determined amount of time.
    //
    am_hal_delay_us(1000000);     // 1sec

    //
    // Stop the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_stop(ui32TimerNum));

    //
    // Read the count from the TIMER.
    //
    ui32TimerCount = am_hal_timer_read(ui32TimerNum);

    if ( (ui32TimerCount >= am_widget_timer_1sec_count[eTimerClock].lower) &&
             (ui32TimerCount <= am_widget_timer_1sec_count[eTimerClock].upper) )
    {
      am_util_stdio_printf(".");
      bRetVal = true;
    }
    else
    {
      am_util_stdio_printf("Timer Number %d Out of Range\n", ui32TimerNum);
      am_util_stdio_printf("Timer Delta: %d\n", ui32TimerCount);
      am_util_stdio_printf("Lower Bound: %d\n", am_widget_timer_1sec_count[eTimerClock].lower);
      am_util_stdio_printf("Upper Bound: %d\n", am_widget_timer_1sec_count[eTimerClock].upper);
      bRetVal = false;
    }

    //
    // Reset the timer configuration to power up values.
    //
    am_hal_timer_reset_config(ui32TimerNum);

    return bRetVal;
}

//*****************************************************************************
//
// Run the TIMER to see if it runs according to the settings and interrupts MCU.
//
//*****************************************************************************
bool
am_widget_timer_int_check(uint32_t ui32TimerNum,
                          am_hal_timer_clock_e eTimerClock)
{
    am_hal_timer_config_t       TimerConfig;
    uint32_t                    ui32TimerCount;

    //
    // Set up the default configuration.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&TimerConfig));

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock             = eTimerClock;
    TimerConfig.eFunction               = AM_HAL_TIMER_FN_EDGE;
    TimerConfig.ui32PatternLimit        = 0;
    TimerConfig.ui32Compare1            = (am_widget_timer_1sec_count[eTimerClock].lower +
                                           am_widget_timer_1sec_count[eTimerClock].upper) / 2;

    //
    // Configure the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32TimerNum, &TimerConfig));

    //
    // Clear the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear_stop(ui32TimerNum));

    //
    // Enabled the Interrupt.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(ui32TimerNum, AM_HAL_TIMER_COMPARE1)));
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Enable the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_enable(ui32TimerNum));

    //
    // Start the TIMER.
    //
    g_TimerValue = 0;
    g_TimerNum = ui32TimerNum;
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_start(ui32TimerNum));

    //
    // Wait for TIMER ISR to complete.
    //
    while (0 == g_TimerValue);

    //
    // Read the count from the TIMER.
    //
    ui32TimerCount = am_hal_timer_read(ui32TimerNum);

    //
    // Stop the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_stop(ui32TimerNum));

    //
    // Disable the Interrupt.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_interrupt_disable(AM_HAL_TIMER_MASK(ui32TimerNum, AM_HAL_TIMER_COMPARE1)));
    NVIC_DisableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_disable();

    //
    // Reset the timer configuration to power up values.
    //
    am_hal_timer_reset_config(ui32TimerNum);

    if ( (ui32TimerCount >= am_widget_timer_1sec_count[eTimerClock].lower) &&
             (ui32TimerCount <= am_widget_timer_1sec_count[eTimerClock].upper) )
    {
      am_util_stdio_printf(".");
      return true;
    }
    else
    {
      am_util_stdio_printf("Timer Number %d Out of Range\n", ui32TimerNum);
      am_util_stdio_printf("Timer Delta: %d\n", ui32TimerCount);
      am_util_stdio_printf("Lower Bound: %d\n", am_widget_timer_1sec_count[eTimerClock].lower);
      am_util_stdio_printf("Upper Bound: %d\n", am_widget_timer_1sec_count[eTimerClock].upper);
      return false;
    }

}
//*****************************************************************************
//
// Use Output TIMER to trigger Input TIMER.  Check that Input TIMER increments
//
//*****************************************************************************
bool
am_widget_timer_triggering(uint32_t ui32SourceTimerNum,
                           uint32_t ui32SinkTimerNum)
{
    am_hal_timer_config_t       TimerConfig;
    uint32_t                    ui32TimerCount;
    uint32_t                    TimerMask = (1 << ui32SourceTimerNum) | (1 << ui32SinkTimerNum);

    //
    // Synchronous disable of Source/Sink timers
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_disable_sync(TimerMask));

    //
    // Set up the default configuration.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&TimerConfig));

    //
    // Set the trigger period with CMP0/CMP1 and PWM mode.
    //
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32Compare0 = 100;
    TimerConfig.ui32Compare1 = 0;

    //
    // Configure the Source TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32SourceTimerNum, &TimerConfig));

    //
    // Clear the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32SourceTimerNum));

    //
    // Set up the default configuration.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&TimerConfig));

    //
    // Update the trigger in the config.
    //
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    TimerConfig.eInputClock     = (am_hal_timer_clock_e)(AM_HAL_TIMER_CLOCK_TMR0_OUT0 + 2*ui32SourceTimerNum);

    //
    // Configure the Sink TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32SinkTimerNum, &TimerConfig));

    //
    // Clear the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32SinkTimerNum));

    //
    // Synchronous enable of Source/Sink timers
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_enable_sync(TimerMask));

    //
    // Delay for a pre-determined amount of time.
    //
    am_hal_delay_us(100000);     // 100ms

    //
    // Synchronous disable of Source/Sink timers
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_disable_sync(TimerMask));

    //
    // Reset the GLOBEN register to default.
    //
    TIMER->GLOBEN = 0xFFFF;

    //
    // Read the Sink TIMER value.
    //
    ui32TimerCount = am_hal_timer_read(ui32SinkTimerNum);
    am_util_stdio_printf("Timer Delta %d\n", ui32TimerCount);

    //
    // Clear the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32SourceTimerNum));
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32SinkTimerNum));

    //
    // Reset the timer configurations to power up values.
    //
    am_hal_timer_reset_config(ui32SourceTimerNum);
    am_hal_timer_reset_config(ui32SinkTimerNum);

    //
    // Return TRUE if the Sink Timer triggered.
    //
    return ( ui32TimerCount > 0);

}

//*****************************************************************************
//
// TIMER rudimentary interrupt test.
// Basically tests some of the interrupt API and the individual IRQs.
//
// IMPORTANT - This rudimentary interrupt test does not test any TIMER
// functionality whatsoever, TIMER functionality can be tested using the
// combined TIMER IRQ using am_ctimer_isr().
// This test is intended to simply check that the individual TIMER IRQs are
// functioning such that they vector to the proper ISRs.
//
//*****************************************************************************
uint32_t
am_widget_timer_rudimentary_individual_interrupts(uint32_t ux)
{

    am_hal_timer_config_t       TimerConfig;

    am_hal_interrupt_master_enable();
    uint32_t ui32Errcnt = 0;
    am_util_stdio_printf("Testing TIMER IRQs (rudimentary interrupt test)\n");
//    for ( uint32_t ux = 0; ux <= 15; ux++ )
//    {
        uint32_t ui32mask;

#if 0
        ui32mask = 1 << (ux * 2);

        //
        // Test the CMP0 interrupt.
        //
        g_ui32timererr = 0;
        am_util_stdio_printf("Testing TIMER%02d CMP0:\n", ux);
        am_hal_delay_us(200000);

        am_hal_timer_interrupt_clear(0xFFFFFFFF);
        NVIC_EnableIRQ((IRQn_Type)(TIMER0_IRQn + ux));
        am_hal_timer_interrupt_enable(ui32mask);

        TIMER->INTSET = ui32mask;

        am_util_stdio_printf("  INTEN   = 0x%08x.\n", TIMER->INTEN);
        am_util_stdio_printf("  INTSTAT = 0x%08x.\n", TIMER->INTSTAT);
        if ( g_ui32timernum != ux )
        {
            ui32Errcnt++;
            am_util_stdio_printf("  ERROR: Timer number %d does not match ISR number %d.\n", ux, g_ui32timernum);
        }
        if ( TIMER->INTSTAT != 0 )
        {
            ui32Errcnt++;
            am_util_stdio_printf("  ERROR: INTSTAT for timer %d was not cleared by ISR %d.\n", ux, g_ui32timernum);
        }
        if ( g_ui32timererr != 0 )
        {
            ui32Errcnt++;
            am_util_stdio_printf("  ERROR: g_ui32timererr returned 0x%08x.\n", g_ui32timererr);
        }

        am_util_stdio_printf("  Done with TIMER%02d.\n", ux);
        am_hal_delay_us(200000);

        am_hal_timer_interrupt_disable(ui32mask);
        NVIC_DisableIRQ((IRQn_Type)(TIMER0_IRQn + ux));
#endif

        //
        // Test the CMP1 interrupt.
        //
        ui32mask = 2 << (ux * 2);

        am_util_stdio_printf("Testing TIMER%02d CMP1:\n", ux);
        am_hal_delay_us(200000);

        am_hal_timer_default_config_set(&TimerConfig);

        am_hal_timer_config(ux, &TimerConfig);

        am_hal_timer_interrupt_clear(0xFFFFFFFF);
        NVIC_EnableIRQ((IRQn_Type)(TIMER0_IRQn + ux));
        am_hal_timer_interrupt_enable(ui32mask);

        TIMER->INTSET = ui32mask;

        am_util_stdio_printf("  INTEN   = 0x%08x.\n", TIMER->INTEN);
        am_util_stdio_printf("  INTSTAT = 0x%08x.\n", TIMER->INTSTAT);
        if ( g_ui32timernum != ux )
        {
            ui32Errcnt++;
            am_util_stdio_printf("  ERROR: Timer number %d does not match ISR number %d.\n", ux, g_ui32timernum);
        }
        if ( TIMER->INTSTAT != 0 )
        {
            ui32Errcnt++;
            am_util_stdio_printf("  ERROR: INTSTAT for timer %d was not cleared by ISR %d.\n", ux, g_ui32timernum);
        }
        if ( g_ui32timererr != 0 )
        {
            ui32Errcnt++;
            am_util_stdio_printf("  ERROR: g_ui32timererr returned 0x%08x.\n", g_ui32timererr);
        }

        am_util_stdio_printf("  Done with TIMER%02d.\n", ux);
        am_hal_delay_us(200000);

        am_hal_timer_interrupt_disable(ui32mask);
        NVIC_DisableIRQ((IRQn_Type)(TIMER0_IRQn + ux));
//    }

    am_util_stdio_printf("\n");
    if ( ui32Errcnt == 0 )
    {
        am_util_stdio_printf("TIMER interrupt testing completed ... PASS.\n");
    }
    else
    {
        am_util_stdio_printf("TIMER interrupt testing completed ... FAIL.\n");
    }

    return ui32Errcnt;

}

uint8_t g_ui8RepeatNum[] = 
{
    1, 100, 255, 0
};

uint8_t g_ui8DutyCycle[] = 
{
    25, 50, 75
};

uint32_t g_ui32Freq[] = 
{
    60, 120, 500, 1000, 5000, 14000, 20000
};

uint8_t g_ui8PatternLen[] = 
{
    0, 31, 63
};

#if defined(FPGA_HOST)
extern volatile bool g_bFPGAReady;
uint8_t g_bFPGAError0[AM_HAL_TIMER_CLOCK_HFRC_DIV16 + 1][AM_REG_NUM_TIMERS] = {0};
uint8_t g_bFPGAError1[7][3] = {0};
uint8_t g_timerIdx, g_clockIdx, g_dutyIdx, g_freqIdx = 0;
uint32_t g_timer_func = AM_HAL_TIMER_FN_PWM;

#define TIMER_SYNC_PIN      16
#define TIMER_SWITCH_PIN    15
#define TIMER_ERROR_PIN     14

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
// ISR callback for the slave ready int
static void fpga_rdy_handler(void *pArg)
{
    g_bFPGAReady = true;
}

static void fpga_err_handler(void *pArg)
{
    if(g_timer_func == AM_HAL_TIMER_FN_PWM)
    {
        uint64_t ui64Div, ui64BaseFreq = 0;
#if APOLLO4_FPGA
        ui64Div = 16 << (2 * g_clockIdx);
#else
        ui64Div = 4 << (2 * g_clockIdx);
#endif
        ui64BaseFreq = 96000000ull / ui64Div;
        // Base frequency should be high enough to generate corresponding frequency PWM
        if(ui64BaseFreq >= g_ui32Freq[g_freqIdx] * 10)
        {
            g_bFPGAError0[g_clockIdx][g_timerIdx]++;
            g_bFPGAError1[g_freqIdx][g_dutyIdx]++;
            am_util_stdio_printf("\nTest failed at clock source %d, timer%d, ", g_clockIdx, g_timerIdx);
            am_util_stdio_printf("%dHz, %dduty\n", g_ui32Freq[g_freqIdx], g_ui8DutyCycle[g_dutyIdx]);
        }
    }
    else
    {
        g_bFPGAError0[g_clockIdx][g_timerIdx]++;
        am_util_stdio_printf("%d length pattern\n", g_ui8PatternLen[g_dutyIdx]);
    }
}

void am_widget_timer_gpio_int_init()
{
    uint32_t IntNum = TIMER_SWITCH_PIN;
    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_input);

    uint32_t ui32IntStatus = (1 << IntNum);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, fpga_rdy_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);

    IntNum = TIMER_ERROR_PIN;
    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_input);

    ui32IntStatus = (1 << IntNum);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, fpga_err_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);

    IntNum = TRIGGER_IN;
    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_input);

    ui32IntStatus = (1 << IntNum);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, fpga_rdy_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);

    IntNum = TIMER_SYNC_PIN;
    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(TIMER_SYNC_PIN);

    NVIC_EnableIRQ(GPIO0_001F_IRQn);
    am_hal_interrupt_master_enable();
}
#endif

void
am_widget_timer_repeat_count_int(uint32_t ui32TimerNum)
{
    am_hal_timer_config_t  TimerConfig ;
    am_hal_timer_default_config_set( &TimerConfig ) ;
    TimerConfig.eFunction    = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.ui32Compare0 = HFRC_BASE / 16 * 105 / 10000;
    TimerConfig.ui32Compare1 = HFRC_BASE / 16 * 105 / 10000;

    am_hal_gpio_pinconfig(PATTERN_OUT0, g_OutPinCfg);
    am_hal_gpio_pinconfig(PATTERN_OUT1, g_OutPinCfg);
    am_hal_timer_interrupt_clear(0xFFFFFFFF);
    NVIC_EnableIRQ((IRQn_Type)(TIMER0_IRQn + ui32TimerNum));
    am_hal_interrupt_master_enable();

    for(uint8_t j = 0; j < sizeof(g_ui8RepeatNum); j++)
    {
        TimerConfig.ui32PatternLimit = g_ui8RepeatNum[j];
        am_util_stdio_printf("\nRepeat %d times\n", g_ui8RepeatNum[j]);
        //
        // Configure the TIMER.
        //
        TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32TimerNum, &TimerConfig));

        for(uint8_t i = 0; i < 2; i++)
        {
            //
            // Clear the TIMER.
            //
            am_hal_timer_clear(ui32TimerNum);

            //
            // Config output pins
            //
            am_hal_timer_output_config(PATTERN_OUT0 + i, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + i + ui32TimerNum * 2);
            am_util_stdio_printf("\nCOMPARE%d\n", i);

            //
            // Enabled the Interrupt.
            //
            am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(ui32TimerNum, AM_HAL_TIMER_COMPARE0 + i));
            g_ui32intnum = 0;

            //
            // Enable the TIMER.
            //
            am_hal_timer_enable(ui32TimerNum);
            //
            // Start the TIMER.
            //
            am_hal_timer_start(ui32TimerNum);

            if(TimerConfig.ui32PatternLimit == 0)
            {
                am_hal_delay_us(2000000);     // 2s
                while((g_ui32intnum != 0) || (g_ui32timernum != ui32TimerNum) || (g_ui8cmpnum != i));
            }
            else
            {
                while((g_ui32intnum != TimerConfig.ui32PatternLimit) || (g_ui32timernum != ui32TimerNum) || (g_ui8cmpnum != i));
            }

            am_hal_timer_interrupt_disable(AM_HAL_TIMER_MASK(ui32TimerNum, AM_HAL_TIMER_COMPARE0 + i));

            //
            // Stop the TIMER.
            //
            am_hal_timer_clear_stop(ui32TimerNum);
            //
            // Disable the TIMER.
            //
            am_hal_timer_disable(ui32TimerNum);
        }
    }
    am_hal_interrupt_master_disable();
    NVIC_DisableIRQ(TIMER_IRQn);
    //
    // Reset the timer configuration to power up values.
    //
    am_hal_timer_reset_config(ui32TimerNum);
}

//*****************************************************************************
//
// Run the TIMER to see if the PWM waveform it generates is correct.
//
//*****************************************************************************
void
am_widget_timer_pwm(uint32_t ui32TimerNum,
                         am_hal_timer_clock_e eTimerClock)
{
    uint64_t ui64Div, ui64BaseFreq, ui64EndCounts, ui64DcTemp = 0;
    uint32_t ui32Freq_x100 = 0;
    uint8_t ui8FreqIndex = sizeof(g_ui32Freq) / sizeof(uint32_t);
    am_hal_timer_config_t  TimerConfig ;
    am_hal_timer_default_config_set( &TimerConfig ) ;
    TimerConfig.eFunction    = AM_HAL_TIMER_FN_PWM;
    TimerConfig.eInputClock = eTimerClock;
#if defined(FPGA_HOST)
    g_timer_func = TimerConfig.eFunction;
#endif

    am_util_stdio_printf("\nTimer clock %d\n", eTimerClock);
    //
    // Configure the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32TimerNum, &TimerConfig));

    // Config output pins
    am_hal_timer_output_config(PATTERN_OUT0, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + ui32TimerNum * 2);
    am_hal_timer_output_config(PATTERN_OUT1, AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + ui32TimerNum * 2);
    am_hal_gpio_pinconfig(PATTERN_OUT0, g_OutPinCfg);
    am_hal_gpio_pinconfig(PATTERN_OUT1, g_OutPinCfg);

    //
    // compute timer counter reset value in PWM mode
    //
    if(eTimerClock <= AM_HAL_TIMER_CLOCK_HFRC_DIV4K)
    {
#if APOLLO4_FPGA
        ui64Div = 16 << (2 * eTimerClock);
#else
        ui64Div = 4 << (2 * eTimerClock);
#endif
        ui64BaseFreq = 96000000ull / ui64Div;
        ui64BaseFreq *= 100;     // this will still fit in 32 bits
    }

    for (uint8_t j = 0; j < ui8FreqIndex; j++)
    {
        am_util_stdio_printf("\nFreq %d\n", g_ui32Freq[j]);
        ui32Freq_x100 = g_ui32Freq[j] * 100;
        ui64EndCounts      = ((ui64BaseFreq + ui32Freq_x100 / 2) / ui32Freq_x100) ;
        TimerConfig.ui32Compare0    = (uint32_t) ui64EndCounts;
        am_hal_timer_compare0_set(ui32TimerNum, TimerConfig.ui32Compare0);

        for (uint8_t i = 0; i < sizeof(g_ui8DutyCycle); i++)
        {
            am_util_stdio_printf("\nDuty cycle %d\n", g_ui8DutyCycle[i]);
            //
            // this is the duty cycle computation for PWM mode
            //
            ui64DcTemp         = ((uint64_t) ui64EndCounts * (100 - g_ui8DutyCycle[i]) + 100 / 2) / 100 ;
            if ( ui64DcTemp > ui64EndCounts) ui64DcTemp = ui64EndCounts ;
            TimerConfig.ui32Compare1    = (uint32_t) ui64DcTemp;
            am_hal_timer_compare1_set(ui32TimerNum, TimerConfig.ui32Compare1);
#if defined(FPGA_HOST)
            while(!g_bFPGAReady);
            AM_CRITICAL_BEGIN
            g_bFPGAReady = false;
            AM_CRITICAL_END
            g_timerIdx = ui32TimerNum;
            g_clockIdx = eTimerClock;
            g_dutyIdx = i;
            g_freqIdx = j;
#endif
            //
            // Enable the TIMER.
            //
            am_hal_timer_enable(ui32TimerNum);
            //
            // Start the TIMER.
            //
            am_hal_timer_start(ui32TimerNum);

#if defined(FPGA_HOST)
            am_hal_delay_us(100000);
            am_hal_gpio_output_set(TIMER_SYNC_PIN);
            am_hal_delay_us(100000);
            am_hal_gpio_output_clear(TIMER_SYNC_PIN);
#else
            am_hal_delay_us(200000);     // 200ms
#endif

            //
            // Stop the TIMER.
            //
            am_hal_timer_clear_stop(ui32TimerNum);
            //
            // Disable the TIMER.
            //
            am_hal_timer_disable(ui32TimerNum);
            am_hal_delay_us(50000);     // 50ms
        }
    }

    //
    // Reset the timer configuration to power up values.
    //
    am_hal_timer_reset_config(ui32TimerNum);
}

//*****************************************************************************
//
// Run the TIMER to see if the pattern it generates is correct.
//
//*****************************************************************************
void
am_widget_timer_single_pattern(uint32_t ui32TimerNum,
                          am_hal_timer_clock_e eTimerClock)
{
    am_hal_timer_config_t       TimerConfig;

    am_util_stdio_printf("\nTimer clock %d\n", eTimerClock);

    //
    // Set up the default configuration.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&TimerConfig));

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock = eTimerClock;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_SINGLEPATTERN;
    TimerConfig.ui32PatternLimit = 63;
    TimerConfig.ui32Compare0 = 0x11111111;
    TimerConfig.ui32Compare1 = 0x22222222;
    TimerConfig.eTriggerSource = AM_HAL_TIMER_TRIGGER_GPIO10;
    TimerConfig.eTriggerType = AM_HAL_TIMER_TRIGGER_RISE;
#if defined(FPGA_HOST)
    g_timer_func = TimerConfig.eFunction;
#else
    am_hal_gpio_pinconfig(TRIGGER_IN, am_hal_gpio_pincfg_input);
    am_hal_gpio_pinconfig(TRIGGER_OUT, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(TRIGGER_OUT);
#endif
    //
    // Configure the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32TimerNum, &TimerConfig));

    //
    // Clear the TIMER.
    //
    am_hal_timer_clear(ui32TimerNum);

    // Config output pins
    am_hal_timer_output_config(PATTERN_OUT0, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + ui32TimerNum * 2);
    am_hal_timer_output_config(PATTERN_OUT1, AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + ui32TimerNum * 2);
    am_hal_gpio_pinconfig(PATTERN_OUT0, g_OutPinCfg);
    am_hal_gpio_pinconfig(PATTERN_OUT1, g_OutPinCfg);

    for (uint8_t i = 0; i < sizeof(g_ui8PatternLen); i++)
    {
        TIMER->CTRL0_b.TMR0LMT = g_ui8PatternLen[i];
        am_util_stdio_printf("\nPattern length %d\n", g_ui8PatternLen[i]);

        //
        // Enable the TIMER.
        //
        am_hal_timer_enable(ui32TimerNum);
        //
        // Start the TIMER.
        //
        am_hal_timer_start(ui32TimerNum);

#if defined(FPGA_HOST)
        while(!g_bFPGAReady);
        AM_CRITICAL_BEGIN
        g_bFPGAReady = false;
        AM_CRITICAL_END
        g_timerIdx = ui32TimerNum;
        g_clockIdx = eTimerClock;
        g_dutyIdx = i;
        am_hal_delay_us(50000);     // 50ms
#else
        //
        // Repeatly trigger the pattern
        //
        for(uint8_t i = 0; i < 3; i++)
        {
            am_hal_gpio_output_set(TRIGGER_OUT);
            am_hal_gpio_output_clear(TRIGGER_OUT);
            am_hal_delay_us(10000);     // 10ms
        }
#endif
        //
        // Stop the TIMER.
        //
        am_hal_timer_clear_stop(ui32TimerNum);
        //
        // Disable the TIMER.
        //
        am_hal_timer_disable(ui32TimerNum);
    }
    //
    // Reset the timer configuration to power up values.
    //
    am_hal_timer_reset_config(ui32TimerNum);
}

//*****************************************************************************
//
// Run the TIMER to see if the pattern it generates is correct.
//
//*****************************************************************************
void
am_widget_timer_repeat_pattern(uint32_t ui32TimerNum,
                          am_hal_timer_clock_e eTimerClock)
{
    am_hal_timer_config_t       TimerConfig;

    am_util_stdio_printf("\nTimer clock %d\n", eTimerClock);

    //
    // Set up the default configuration.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&TimerConfig));

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock = eTimerClock;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_REPEATPATTERN;
    TimerConfig.ui32PatternLimit = 63;
    TimerConfig.ui32Compare0 = 0x11111111;
    TimerConfig.ui32Compare1 = 0x22222222;
    TimerConfig.eTriggerSource = AM_HAL_TIMER_TRIGGER_GPIO10;
    TimerConfig.eTriggerType = AM_HAL_TIMER_TRIGGER_RISE;
#if defined(FPGA_HOST)
    g_timer_func = TimerConfig.eFunction;
#else
    am_hal_gpio_pinconfig(TRIGGER_IN, am_hal_gpio_pincfg_input);
    am_hal_gpio_pinconfig(TRIGGER_OUT, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(TRIGGER_OUT);
#endif
    //
    // Configure the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32TimerNum, &TimerConfig));

    //
    // Clear the TIMER.
    //
    am_hal_timer_clear(ui32TimerNum);

    // Config output pins
    am_hal_timer_output_config(PATTERN_OUT0, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + ui32TimerNum * 2);
    am_hal_timer_output_config(PATTERN_OUT1, AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + ui32TimerNum * 2);
    am_hal_gpio_pinconfig(PATTERN_OUT0, g_OutPinCfg);
    am_hal_gpio_pinconfig(PATTERN_OUT1, g_OutPinCfg);

    for (uint8_t i = 0; i < sizeof(g_ui8PatternLen); i++)
    {
        TIMER->CTRL0_b.TMR0LMT = g_ui8PatternLen[i];
        am_util_stdio_printf("\nPattern length %d\n", g_ui8PatternLen[i]);

        //
        // Enable the TIMER.
        //
        am_hal_timer_enable(ui32TimerNum);
        //
        // Start the TIMER.
        //
        am_hal_timer_start(ui32TimerNum);

#if defined(FPGA_HOST)
        while(!g_bFPGAReady);
        AM_CRITICAL_BEGIN
        g_bFPGAReady = false;
        AM_CRITICAL_END
        g_timerIdx = ui32TimerNum;
        g_clockIdx = eTimerClock;
        g_dutyIdx = i;
#else
        //
        // Delay for a pre-determined amount of time.
        //
        am_hal_gpio_output_set(TRIGGER_OUT);
        am_hal_gpio_output_clear(TRIGGER_OUT);
#endif
        am_hal_delay_us(50000);     // 50ms
        //
        // Stop the TIMER.
        //
        am_hal_timer_clear_stop(ui32TimerNum);
        //
        // Disable the TIMER.
        //
        am_hal_timer_disable(ui32TimerNum);
    }
    //
    // Reset the timer configuration to power up values.
    //
    am_hal_timer_reset_config(ui32TimerNum);
}

//*****************************************************************************
//
// Use one Timer Output as other Timer Output
//
//***************************************************************************** 
bool
am_widget_timer_chain(uint32_t ui32SourceTimer, uint32_t ui32SinkTimer, 
                                        am_hal_timer_clock_e eTimerClock)
{
    am_hal_timer_config_t    sSourceTimerConfig;
    am_hal_timer_config_t    sSinkTimerConfig;
    uint32_t                 TimerMask = (1 << ui32SourceTimer) | (1 << ui32SinkTimer);

    //
    // The LFRC clock and its pre-scaled clocks have been adjusted to a tolerance 
    // of +-20% as described in the JIRA under SYSTEST-48. 
    //
    struct
    {
        uint32_t    inputClock;
    } am_widget_timer_clock[AM_HAL_TIMER_CLOCK_RTC_100HZ + 1] =
    {

        { HFRC_BASE / 4             },
        { HFRC_BASE / 16            },
        { HFRC_BASE / 64            },
        { HFRC_BASE / 256           },
        { HFRC_BASE / 1024          },
        { HFRC_BASE / 4096          },
        { LFRC_BASE * 80 / 100      },
        { LFRC_BASE / 2 * 80 / 100  },
        { LFRC_BASE / 32 * 80 / 100 },
        { LFRC_BASE / 1024          },
        { XT_BASE                   },
        { XT_BASE / 2               },
        { XT_BASE / 4               },
        { XT_BASE / 8               },
        { XT_BASE / 16              },
        { XT_BASE / 32              },
        { XT_BASE / 128             },
        { 100                       }
    };

    bool                     bRetVal;

    uint32_t                 ui32Timer1Count;
    uint32_t                 ui32Timer2Count;

    //
    // These variable are used to set the calculated reference values for the sink timer after a delay of 1 sec
    //

    uint32_t                 ui32Timer1SecLowerBound   =    0;
    uint32_t                 ui32Timer1SecUpperBound   =    0;

    //
    // Synchronous disable of Source/Sink timers
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_disable_sync(TimerMask));

    //
    // Set default configurations for Timer1 and Timer2 
    //

    TEST_ASSERT_TRUE( AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&sSourceTimerConfig));

    TEST_ASSERT_TRUE( AM_HAL_STATUS_SUCCESS == am_hal_timer_default_config_set(&sSinkTimerConfig));

    //
    // Configure the Clock and Mode for Timer1
    //

    sSourceTimerConfig.eInputClock = eTimerClock;
    sSourceTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    sSourceTimerConfig.ui32Compare0 = 10;
    sSourceTimerConfig.ui32Compare1 = 0;

    //
    // Configure the Output for Timer 1
    //

    TEST_ASSERT_TRUE( AM_HAL_STATUS_SUCCESS == am_hal_timer_output_config(ui32SourceTimer, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + ( ui32SourceTimer * 2 )));

    //
    // Configure the Output of Timer1 as the clock for Timer2 and the mode of Timer2
    //

    sSinkTimerConfig.eInputClock = ( am_hal_timer_clock_e )( AM_HAL_TIMER_CLOCK_TMR0_OUT0 + ( ui32SourceTimer * 2 ) );
    sSinkTimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    //
    // Configure Timer1 and Timer2
    //

    TEST_ASSERT_TRUE( AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32SourceTimer, &sSourceTimerConfig));
    TEST_ASSERT_TRUE( AM_HAL_STATUS_SUCCESS == am_hal_timer_config(ui32SinkTimer, &sSinkTimerConfig));

    //
    // Clear Timer1 and Timer2
    //

    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32SourceTimer));
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_clear(ui32SinkTimer));

    //
    // Enable Timer1 and Timer2
    //

    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_enable_sync(TimerMask));

    //
    // Introduce a delay
    //

    am_hal_delay_us(DELAY_MICROSEC);

    //
    // Stop Timer1 and Timer2
    //

    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_disable_sync(TimerMask));

    //
    // Reset the GLOBEN register to default.
    //
    TIMER->GLOBEN = 0xFFFF;

    //
    // Read Timer1 and Timer2
    //

    ui32Timer1Count = am_hal_timer_read(ui32SourceTimer);
    ui32Timer2Count = am_hal_timer_read(ui32SinkTimer);

    //
    // Adjust the range of the calculated reference value to the expected value of the sink timer
    //
    // The variance is kept at 3% implying 97%-103%. 
    //
    // Note:
    //
    // I've adjusted the thresholds for these tests by 2 clock periods because of a
    // silicon issue tracked in JIRA under SYSTEST-14. Per this issue, TIMERS take
    // an extra 1 to 2 clock cycles to start counting, so the final TIMER count
    // after 1 second will be 1 to two clocks lower than expected.
    //
    // The adjustments below allow the low frequency test cases to pass despite
    // this 1-2 clock discrepancy.
    //

    if ((int32_t)((((am_widget_timer_clock[eTimerClock].inputClock / (SEC_TO_MICROSEC/DELAY_MICROSEC)) / sSourceTimerConfig.ui32Compare0) * 97/100) - 4) < 1)
    {
        ui32Timer1SecLowerBound = 0;
    }
    else
    {
        ui32Timer1SecLowerBound = (uint32_t)((((am_widget_timer_clock[eTimerClock].inputClock / (SEC_TO_MICROSEC/DELAY_MICROSEC)) / sSourceTimerConfig.ui32Compare0) * 97/100) - 4);
    }

    ui32Timer1SecUpperBound = ((((am_widget_timer_clock[eTimerClock].inputClock / (SEC_TO_MICROSEC/DELAY_MICROSEC)) / sSourceTimerConfig.ui32Compare0) * 103/100));

    //
    // Compare the timer value to the calculated reference value range
    //

    if ( (ui32Timer2Count >= ui32Timer1SecLowerBound) && (ui32Timer2Count <= ui32Timer1SecUpperBound) )
    {
        am_util_stdio_printf(".");
        bRetVal = true;
    }
    else
    {
        am_util_stdio_printf("\n");
        am_util_stdio_printf("Timer Number %d Out of Range\n", ui32SinkTimer);
        am_util_stdio_printf("Source Timer value: %d\n", ui32Timer1Count);
        am_util_stdio_printf("Sink Timer Value: %d\n", ui32Timer2Count);
        am_util_stdio_printf("Lower Bound: %u\n", ui32Timer1SecLowerBound);
        am_util_stdio_printf("Upper Bound: %u\n", ui32Timer1SecUpperBound);
        bRetVal = false;
    }

    //
    // Reset the timer
    //

    am_hal_timer_reset_config(ui32SourceTimer);
    am_hal_timer_reset_config(ui32SinkTimer);

    return bRetVal;
}

