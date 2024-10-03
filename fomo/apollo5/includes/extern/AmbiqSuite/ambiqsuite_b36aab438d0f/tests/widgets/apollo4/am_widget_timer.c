
//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_widget_timer.h"
#include "unity.h"
#include "am_util.h"

volatile uint32_t g_TimerValue;
volatile uint32_t g_TimerNum;
volatile bool     g_TimerDone;

void am_ctimer_isr(void)
{
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(g_TimerNum,AM_HAL_TIMER_COMPARE1));

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
    }
    else
    {
        ui32Mask = (2 << (ui32timernum * 2));
    }

    if ( am_hal_timer_interrupt_clear(ui32Mask) )
    {
        ui32Ret |= 0x08;
    }

    return ui32Ret;
}


void am_timer00_isr(void)
{
    g_ui32timernum = 0;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer01_isr(void)
{
    g_ui32timernum = 1;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer02_isr(void)
{
    g_ui32timernum = 2;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer03_isr(void)
{
    g_ui32timernum = 3;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer04_isr(void)
{
    g_ui32timernum = 4;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer05_isr(void)
{
    g_ui32timernum = 5;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer06_isr(void)
{
    g_ui32timernum = 6;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer07_isr(void)
{
    g_ui32timernum = 7;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer08_isr(void)
{
    g_ui32timernum = 8;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer09_isr(void)
{
    g_ui32timernum = 9;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer10_isr(void)
{
    g_ui32timernum = 10;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer11_isr(void)
{
    g_ui32timernum = 11;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer12_isr(void)
{
    g_ui32timernum = 12;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer13_isr(void)
{
    g_ui32timernum = 13;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer14_isr(void)
{
    g_ui32timernum = 14;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}
void am_timer15_isr(void)
{
    g_ui32timernum = 15;
    g_ui32timererr = timer_int_check(g_ui32timernum);
    am_util_stdio_printf("  ..TIMER%02d INT\n", g_ui32timernum);
    am_hal_delay_us(200000);
}


#if defined(APOLLO4_FPGA)
#define HFRC_BASE (APOLLO4_FPGA*1000000)
#define XT_BASE   16384
#define LFRC_BASE 50 // FIXME - For now it is 500Hz.
#elif defined(AM_PART_APOLLO4)
#define HFRC_BASE 48000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#elif defined(AM_PART_APOLLO4B)
#define HFRC_BASE 96000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#endif

struct
{
    uint32_t    lower;
    uint32_t    upper;
} am_widget_timer_1sec_count[AM_HAL_TIMER_CLOCK_RTC_100HZ+1] =
{
    // Lower Bound Pass         Upper Bound Pass
    { HFRC_BASE/4-50,           HFRC_BASE/4 + 50 },
    { HFRC_BASE/16-30,          HFRC_BASE/16+30 },
    { HFRC_BASE/64-10,          HFRC_BASE/64+10 },
    { HFRC_BASE/256-2,          HFRC_BASE/256+2 },
    { HFRC_BASE/1024-1,         HFRC_BASE/1024+1 },
    { HFRC_BASE/4096-1,         HFRC_BASE/4096+1 },
    { LFRC_BASE-10,             LFRC_BASE+10 },
    { LFRC_BASE/2-5,            LFRC_BASE/2+5 },
    { LFRC_BASE/32-1,           LFRC_BASE/32+1 },
    { 0,                        LFRC_BASE/1024+1 },
    { XT_BASE-1,                XT_BASE+1 },
    { XT_BASE/2-1,              XT_BASE/2+1 },
    { XT_BASE/4-1,              XT_BASE/4+1 },
    { XT_BASE/8-1,              XT_BASE/8+1 },
    { XT_BASE/16-1,             XT_BASE/16+1 },
    { XT_BASE/32-1,             XT_BASE/32+1 },
    { XT_BASE/128-1,            XT_BASE/128+1 },
    { 98,                       102 }
};

uint32_t        ui8PeriodicTimerSteps[] = {1000, 2000, 3000, 4000, 5000};

#define AM_WIDGET_TIMER_EXPECTED_PERIODIC_COUNT 15000

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
                                           am_widget_timer_1sec_count[eTimerClock].upper)/2;

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
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(ui32TimerNum,AM_HAL_TIMER_COMPARE1)));
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
    // Stop the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_stop(ui32TimerNum));

    //
    // Disable the Interrupt.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_interrupt_disable(AM_HAL_TIMER_MASK(ui32TimerNum,AM_HAL_TIMER_COMPARE1)));
    NVIC_DisableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_disable();

    //
    // Read the count from the TIMER.
    //
    ui32TimerCount = am_hal_timer_read(ui32TimerNum);

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
    TimerConfig.eFunction = AM_HAL_TIMER_FN_CONTINUOUS;
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
