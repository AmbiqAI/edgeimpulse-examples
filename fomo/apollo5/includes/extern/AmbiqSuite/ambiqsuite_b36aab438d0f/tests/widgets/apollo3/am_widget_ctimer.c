//*****************************************************************************
//
//! @file am_widget_ctimer.c
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
#include "am_widget_ctimer.h"
#include "am_util.h"
#include "am_bsp.h"

//*****************************************************************************
//
// Timer configuration register locations.
//
//*****************************************************************************
#if AM_CMSIS_REGS
uint32_t* g_pui32TimerConfigRegs[] =
{
    (uint32_t*)&CTIMER->CTRL0,
    (uint32_t*)&CTIMER->CTRL1,
    (uint32_t*)&CTIMER->CTRL2,
    (uint32_t*)&CTIMER->CTRL3
};
#else // AM_CMSIS_REGS
uint32_t g_pui32TimerConfigRegs[] =
{
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL1_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL2_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL3_O
};
#endif // AM_CMSIS_REGS

#if AM_CMSIS_REGS
uint32_t* g_pui32TimerCmprARegs[] =
{
    (uint32_t*)&CTIMER->CMPRA0,
    (uint32_t*)&CTIMER->CMPRA1,
    (uint32_t*)&CTIMER->CMPRA2,
    (uint32_t*)&CTIMER->CMPRA3
};
#else // AM_CMSIS_REGS
uint32_t  g_pui32TimerCmprARegs[] =
{
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRA0_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRA1_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRA2_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRA3_O
};
#endif // AM_CMSIS_REGS

#if AM_CMSIS_REGS
uint32_t* g_pui32TimerCmprBRegs[] =
{
    (uint32_t*)&CTIMER->CMPRB0,
    (uint32_t*)&CTIMER->CMPRB1,
    (uint32_t*)&CTIMER->CMPRB2,
    (uint32_t*)&CTIMER->CMPRB3
};
#else // AM_CMSIS_REGS
uint32_t  g_pui32TimerCmprBRegs[] =
{
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRB0_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRB1_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRB2_O,
    AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRB3_O
};
#endif // AM_CMSIS_REGS

//*****************************************************************************
//
// GPIO Definitions.
//
//*****************************************************************************
#if AM_APOLLO3_CTIMER
const am_hal_gpio_pincfg_t g_AM_HAL_GPIO_0_OUTPUT =
{
  .uFuncSel             = AM_HAL_PIN_0_GPIO
};
#endif

#define TEST_TIMERNUM       0

//
// Timer configuration.
//
static am_hal_ctimer_config_t g_sContTimer =
{
    // Create 32-bit timer
    1,

    // Set up TimerA.
    (AM_HAL_CTIMER_FN_CONTINUOUS    |
     AM_HAL_CTIMER_LFRC_32HZ),

    // Set up Timer0B.
    0
};

am_widget_ctimer_freq_tick_t g_sTimerTick[] =
{
    {"LFRC 32Hz",     AM_HAL_CTIMER_LFRC_32HZ,     32},
    {"LFRC 512Hz",    AM_HAL_CTIMER_LFRC_512HZ,    512},
    {"XT 256Hz",      AM_HAL_CTIMER_XT_256HZ,      256},
    {"XT 2.048KHz",   AM_HAL_CTIMER_XT_2_048KHZ,   2097},
    {"XT 16.384KHz",  AM_HAL_CTIMER_XT_16_384KHZ,  16777},
    {"XT 32.768KHz",  AM_HAL_CTIMER_XT_32_768KHZ,  33554},
    {"HFRC 12KHz",    AM_HAL_CTIMER_HFRC_12KHZ,    12288},
    {"HFRC 47KHz",    AM_HAL_CTIMER_HFRC_47KHZ,    48128},
    {"HRRC 187.5KHz", AM_HAL_CTIMER_HFRC_187_5KHZ, 192000},
    {"HFRC 3MHz",     AM_HAL_CTIMER_HFRC_3MHZ,     3145728},
    {"HFRC 12MHz",    AM_HAL_CTIMER_HFRC_12MHZ,    12582912},
};

#define CLK_SRC_START   0
#define CLK_SRC_END     10
#define ITERATION_NUM   5
#define ERR_THRES       10.0

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
int32_t
get_speed_from_div(const am_widget_ctimer_single_config_t *psCTimerConfig)
{
    uint32_t ui32Div;

    //
    // Grab the desired clock divider value.
    //
#if AM_CMSIS_REGS
    if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERA ||
        psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_BOTH)
    {
        ui32Div = (psCTimerConfig->ui32TimerAConfig & CTIMER_CTRL0_TMRA0CLK_Msk);
    }
    else
    {
        ui32Div = (psCTimerConfig->ui32TimerBConfig & CTIMER_CTRL0_TMRA0CLK_Msk);
    }
#else // AM_CMSIS_REGS
    if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERA ||
        psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_BOTH)
    {
        ui32Div = (psCTimerConfig->ui32TimerAConfig &
                   AM_REG_CTIMER_CTRL0_TMRA0CLK_M);
    }
    else
    {
        ui32Div = (psCTimerConfig->ui32TimerBConfig &
                   AM_REG_CTIMER_CTRL0_TMRA0CLK_M);
    }
#endif // AM_CMSIS_REGS

    switch (ui32Div)
    {
        //
        // HFRC dividers.
        //
#if AM_CMSIS_REGS
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

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4):
            return 255100;
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV16):
            return 255100;
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV256):
            return 255100;
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV1024):
            return 255100;
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4K):
            return 255100;

        //
        // XT dividers.
        //
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT):
            return (12500 / 1);
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV2):
            return (12500 / 2);
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV16):
            return (12500 / 16);
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV128):
            return (12500 / 128);

        //
        // LFRC dividers.
        //
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC):
            return (391 / 1);
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC_DIV2):
            return (391 / 2);
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC_DIV32):
            return (391 / 32);
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC_DIV1K):
            return (391 / 1024);

        //
        // MISC
        //
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_RTC_100HZ):
            return 100;

        default:
            return 0x1;
#else // APOLLO3_FPGA
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4):
            return (48000000 / 4);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV16):
            return (48000000 / 16);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV256):
            return (48000000 / 256);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV1024):
            return (48000000 / 1024);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4K):
            return (48000000 / 4096);

        //
        // XT dividers.
        //
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT):
            return (32768);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV2):
            return (32768 / 2);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV16):
            return (32768 / 16);
#if AM_APOLLO3_CTIMER
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV128):
            return (32768 / 128);
#else
        // Note: This enum is wrong. It should say "DIV128"
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_XT_DIV256):
            return (32768 / 128);
#endif

        //
        // LFRC dividers.
        //
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC):
            return (1024);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC_DIV2):
            return (1024 / 2);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC_DIV32):
            return (1024 / 32);

        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_LFRC_DIV1K):
            return (1); // (1024 / 1024);

        //
        // MISC
        //
        case _VAL2FLD(CTIMER_CTRL0_TMRA0CLK, CTIMER_CTRL0_TMRA0CLK_RTC_100HZ):
            return 100;

        default:
            return 0x1;
#endif // FPGA
#else // AM_CMSIS_REGS
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

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4:
            return 255100;
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV16:
            return 255100;
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV256:
            return 255100;
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV1024:
            return 255100;
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4K:
            return 255100;

        //
        // XT dividers.
        //
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT:
            return (12500 / 1);
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV2:
            return (12500 / 2);
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV16:
            return (12500 / 16);
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV128:
            return (12500 / 128);

        //
        // LFRC dividers.
        //
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC:
            return (391 / 1);
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC_DIV2:
            return (391 / 2);
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC_DIV32:
            return (391 / 32);
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC_DIV1K:
            return (391 / 1024);

        //
        // MISC
        //
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_RTC_100HZ:
            return 100;

        default:
            return 0x1;
#else
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4:
            return (48000000 / 4);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV16:
            return (48000000 / 16);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV256:
            return (48000000 / 256);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV1024:
            return (48000000 / 1024);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_HFRC_DIV4K:
            return (48000000 / 4096);

        //
        // XT dividers.
        //
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT:
            return (32768);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV2:
            return (32768 / 2);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV16:
            return (32768 / 16);
#if AM_APOLLO3_CTIMER
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV128:
            return (32768 / 128);
#else
        // Note: This enum is wrong. It should say "DIV128"
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_XT_DIV256:
            return (32768 / 128);
#endif

        //
        // LFRC dividers.
        //
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC:
            return (1024);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC_DIV2:
            return (1024 / 2);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC_DIV32:
            return (1024 / 32);

        case AM_REG_CTIMER_CTRL0_TMRA0CLK_LFRC_DIV1K:
            return (1); // (1024 / 1024);

        //
        // MISC
        //
        case AM_REG_CTIMER_CTRL0_TMRA0CLK_RTC_100HZ:
            return 100;

        default:
            return 0x1;
#endif // FPGA
#endif // AM_CMSIS_REGS
    }
}

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile uint64_t g_ui64SysTickWrappedTime = 0;

//*****************************************************************************
//
// Systick ISR.
//
//*****************************************************************************
void
#if AM_CMSIS_REGS
SysTick_Handler(void)
#else
am_systick_isr(void)
#endif
{
    //
    // Add enough cycles to account for one full cycle of systick.
    //
    g_ui64SysTickWrappedTime += 0x01000000;
}

//*****************************************************************************
//
// Check to make sure the registers are what we expect.
//
//*****************************************************************************
bool
check_config(const am_widget_ctimer_single_config_t *psTimerConfig)
{
    uint32_t ui32ConfigVal, ui32CmprA, ui32CmprB;

    uint32_t ui32Function;
    uint32_t ui32Compare0;
    uint32_t ui32Compare1;
    uint32_t ui32ActualPeriod;
    uint32_t ui32ActualOnTime;

    uint32_t ui32TestPeriod;
    uint32_t ui32TestOnTime;

    //
    // Read the relevant configuration registers.
    //
    ui32ConfigVal = AM_REGVAL(g_pui32TimerConfigRegs[psTimerConfig->ui32TestTimer]);
    ui32CmprA = AM_REGVAL(g_pui32TimerCmprARegs[psTimerConfig->ui32TestTimer]);
    ui32CmprB = AM_REGVAL(g_pui32TimerCmprBRegs[psTimerConfig->ui32TestTimer]);

    //
    // Figure out what our effective configuration and compare values are.
    //
    if (psTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERA)
    {
        if ((ui32ConfigVal & (AM_HAL_CTIMER_TIMERA | AM_HAL_CTIMER_LINK)) !=
            psTimerConfig->ui32TimerAConfig)
        {
            return false;
        }

#if AM_CMSIS_REGS
        ui32Function = _FLD2VAL(CTIMER_CTRL0_TMRA0FN, ui32ConfigVal);
        ui32Compare0 = _FLD2VAL(CTIMER_CMPRA0_CMPR0A0, ui32CmprA);
        ui32Compare1 = _FLD2VAL(CTIMER_CMPRA0_CMPR1A0, ui32CmprA);
#else // AM_CMSIS_REGS
        ui32Function = AM_BFX(CTIMER, CTRL0, TMRA0FN, ui32ConfigVal);
        ui32Compare0 = AM_BFX(CTIMER, CMPRA0, CMPR0A0, ui32CmprA);
        ui32Compare1 = AM_BFX(CTIMER, CMPRA0, CMPR1A0, ui32CmprA);
#endif // AM_CMSIS_REGS

        ui32TestPeriod = psTimerConfig->ui32TimerAPeriod;
        ui32TestOnTime = psTimerConfig->ui32TimerAOnTime;
    }
    else if (psTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERB)
    {
        if ((ui32ConfigVal & (AM_HAL_CTIMER_TIMERB | AM_HAL_CTIMER_LINK)) !=
            psTimerConfig->ui32TimerBConfig << 16)
        {
            return false;
        }

#if AM_CMSIS_REGS
        ui32Function = _FLD2VAL(CTIMER_CTRL0_TMRB0FN,  ui32ConfigVal);
        ui32Compare0 = _FLD2VAL(CTIMER_CMPRB0_CMPR0B0, ui32CmprB);
        ui32Compare1 = _FLD2VAL(CTIMER_CMPRB0_CMPR1B0, ui32CmprB);
#else // AM_CMSIS_REGS
        ui32Function = AM_BFX(CTIMER, CTRL0, TMRB0FN, ui32ConfigVal);
        ui32Compare0 = AM_BFX(CTIMER, CMPRB0, CMPR0B0, ui32CmprB);
        ui32Compare1 = AM_BFX(CTIMER, CMPRB0, CMPR1B0, ui32CmprB);
#endif // AM_CMSIS_REGS

        ui32TestPeriod = psTimerConfig->ui32TimerBPeriod;
        ui32TestOnTime = psTimerConfig->ui32TimerBOnTime;
    }
    else
    {
        if ((ui32ConfigVal & (AM_HAL_CTIMER_TIMERA | AM_HAL_CTIMER_LINK)) !=
            (psTimerConfig->ui32TimerAConfig | AM_HAL_CTIMER_LINK))
        {
            return false;
        }

#if AM_CMSIS_REGS
        ui32Function =  _FLD2VAL(CTIMER_CTRL0_TMRA0FN,  ui32ConfigVal);
        ui32Compare0 =  _FLD2VAL(CTIMER_CMPRA0_CMPR0A0, ui32CmprA);
        ui32Compare0 += _FLD2VAL(CTIMER_CMPRB0_CMPR0B0, ui32CmprB) << 16;

        ui32Compare1 =  _FLD2VAL(CTIMER_CMPRA0_CMPR1A0, ui32CmprA);
        ui32Compare1 += _FLD2VAL(CTIMER_CMPRB0_CMPR1B0, ui32CmprB) << 16;
#else // AM_CMSIS_REGS
        ui32Function = AM_BFX(CTIMER, CTRL0, TMRA0FN, ui32ConfigVal);
        ui32Compare0 = AM_BFX(CTIMER, CMPRA0, CMPR0A0, ui32CmprA);
        ui32Compare0 += AM_BFX(CTIMER, CMPRB0, CMPR0B0, ui32CmprB) << 16;

        ui32Compare1 = AM_BFX(CTIMER, CMPRA0, CMPR1A0, ui32CmprA);
        ui32Compare1 += AM_BFX(CTIMER, CMPRB0, CMPR1B0, ui32CmprB) << 16;
#endif // AM_CMSIS_REGS

        ui32TestPeriod = psTimerConfig->ui32TimerAPeriod;
        ui32TestOnTime = psTimerConfig->ui32TimerAOnTime;
    }

    //
    // Calculate what our period and on-time should have been based on our
    // compare values and compare against the actual.
    //
    if (ui32Function == 2 || ui32Function == 3)
    {
        ui32ActualPeriod = ui32Compare1;
        ui32ActualOnTime = ui32Compare1 - ui32Compare0;
    }
    else
    {
        ui32ActualPeriod = ui32Compare0;
        ui32ActualOnTime = 0;
    }

    //
    // Make sure the period and duty cycle we came up with from reading the
    // registers is the same as what we expected.
    //
    if (ui32TestPeriod != ui32ActualPeriod)
    {
        return false;
    }

    if ((ui32Function == 2 || ui32Function == 3) &&
        (ui32TestOnTime != ui32ActualOnTime))
    {
        return false;
    }

    //
    // If nothing made us fail up to this point, return a pass.
    //
    return true;
}

//*****************************************************************************
//
// Config version 1
//
//*****************************************************************************
void
single_config(const am_widget_ctimer_single_config_t *psCTimerConfig)
{
    uint32_t ui32Period = 0;
    uint32_t ui32OnTime = 0;
    uint32_t ui32ConfigVal = 0;

    if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERA)
    {
        ui32ConfigVal = psCTimerConfig->ui32TimerAConfig;
        ui32Period = psCTimerConfig->ui32TimerAPeriod;
        ui32OnTime = psCTimerConfig->ui32TimerAOnTime;
    }
    else if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERB)
    {
        ui32ConfigVal = psCTimerConfig->ui32TimerBConfig;
        ui32Period = psCTimerConfig->ui32TimerBPeriod;
        ui32OnTime = psCTimerConfig->ui32TimerBOnTime;
    }
    else if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_BOTH)
    {
        ui32ConfigVal = psCTimerConfig->ui32TimerAConfig;
        ui32Period = psCTimerConfig->ui32TimerAPeriod;
        ui32OnTime = psCTimerConfig->ui32TimerAOnTime;
    }

    //
    // Configure the timer based on the input settings.
    //
    am_hal_ctimer_config_single(psCTimerConfig->ui32TestTimer,
                                psCTimerConfig->ui32TestSegment,
                                ui32ConfigVal);
    //
    // Set the period and duty cycle.
    //
    am_hal_ctimer_period_set(psCTimerConfig->ui32TestTimer,
                             psCTimerConfig->ui32TestSegment,
                             ui32Period, ui32OnTime);
}

//*****************************************************************************
//
// Config version 2
//
//*****************************************************************************
void
structure_config(const am_widget_ctimer_single_config_t *psCTimerConfig)
{
    am_hal_ctimer_config_t sConfigStruct;
    uint32_t ui32Period = 0;
    uint32_t ui32OnTime = 0;

    //
    // Fill the config structure with our parameters.
    //
    sConfigStruct.ui32Link = psCTimerConfig->ui32Link;
    sConfigStruct.ui32TimerAConfig = psCTimerConfig->ui32TimerAConfig;
    sConfigStruct.ui32TimerBConfig = psCTimerConfig->ui32TimerBConfig;

    //
    // Configure the timer.
    //
    am_hal_ctimer_config(psCTimerConfig->ui32TestTimer, &sConfigStruct);

    //
    // Figure out which period and duty cycle we're paying attention to.
    //
    if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERA)
    {
        ui32Period = psCTimerConfig->ui32TimerAPeriod;
        ui32OnTime = psCTimerConfig->ui32TimerAOnTime;
    }
    else if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERB)
    {
        ui32Period = psCTimerConfig->ui32TimerBPeriod;
        ui32OnTime = psCTimerConfig->ui32TimerBOnTime;
    }
    else if (psCTimerConfig->ui32TestSegment == AM_HAL_CTIMER_BOTH)
    {
        ui32Period = psCTimerConfig->ui32TimerAPeriod;
        ui32OnTime = psCTimerConfig->ui32TimerAOnTime;
    }

    //
    // Set the period and duty cycle.
    //
    am_hal_ctimer_period_set(psCTimerConfig->ui32TestTimer,
                             psCTimerConfig->ui32TestSegment,
                             ui32Period, ui32OnTime);
}

//*****************************************************************************
//
// Check timer configuration.
//
//*****************************************************************************
uint32_t
am_widget_ctimer_config_test(const void *psSettings, char *pErrorStr)
{
    uint32_t ui32Status = 0;

    single_config(psSettings);

    ui32Status = check_config(psSettings) ? 0x0 : 0x1;

    return ui32Status;
}

//*****************************************************************************
//
// Check timer configuration using structs.
//
//*****************************************************************************
uint32_t
am_widget_ctimer_config_test2(const void *psSettings, char *pErrorStr)
{
    uint32_t ui32Status = 0;

    structure_config(psSettings);

    ui32Status = check_config(psSettings) ? 0x0 : 0x1;

    return ui32Status;
}

//*****************************************************************************
//
// Check timer speed.
//
//*****************************************************************************
int32_t
am_widget_ctimer_speed_test(const void *psSettings, char *pErrorStr)
{
    const am_widget_ctimer_single_config_t *psConfig = psSettings;

    uint32_t ui32ExpectedPeriod;
    uint32_t ui32IntMask;
    uint64_t ui64TotalTime, ui64Product, ui64Speed;

    //
    // Must clear the timer before setting singlecount. From datasheet concerning
    // the configuration of single count mode, "CLR has been asserted previously".
    //
    am_hal_ctimer_clear(psConfig->ui32TestTimer, psConfig->ui32TestSegment);

    //
    // Configure the timer.
    //
    structure_config(psConfig);

    //
    // Figure out how long we're supposed to wait.
    //
    ui32ExpectedPeriod = (psConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERB ?
                          psConfig->ui32TimerBPeriod :
                          psConfig->ui32TimerAPeriod);

#if AM_APOLLO3_CTIMER
    am_hal_gpio_pinconfig(0, g_AM_HAL_GPIO_0_OUTPUT);
    am_hal_gpio_state_write(0,AM_HAL_GPIO_OUTPUT_CLEAR);
#else
    am_hal_gpio_pin_config(0, AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_out_bit_clear(0);
#endif

    //
    // Prepare to measure time with systick.
    //
    g_ui64SysTickWrappedTime = 0;
#if AM_CMSIS_REGS
    SysTick->CTRL = 0;
    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
#else // AM_CMSIS_REGS
    AM_REG(SYSTICK, SYSTCSR) = 0;
    AM_REG(SYSTICK, SYSTRVR) = 0xFFFFFF;
    AM_REG(SYSTICK, SYSTCVR) = 0;
#endif // AM_CMSIS_REGS

    //
    // Start the timer.
    //
    am_hal_ctimer_int_disable(0xFFFFFFFF);
    am_hal_ctimer_int_clear(0xFFFFFFFF);
    am_hal_ctimer_start(psConfig->ui32TestTimer, psConfig->ui32TestSegment);

    //
    // Enable systick
    //
#if AM_CMSIS_REGS
    SysTick->CTRL = _VAL2FLD(SysTick_CTRL_TICKINT, 1) |
                    _VAL2FLD(SysTick_CTRL_ENABLE, 1);
#else // AM_CMSIS_REGS
    AM_REG(SYSTICK, SYSTCSR) = (AM_BFV(SYSTICK, SYSTCSR, TICKINT, 1) |
                                AM_BFV(SYSTICK, SYSTCSR, ENABLE, 1));
#endif // AM_CMSIS_REGS

    //
    // Wait for the timer to expire.
    //
#if AM_CMSIS_REGS
    ui32IntMask = CTIMER_INTSTAT_CTMRA0C0INT_Msk << (psConfig->ui32TestTimer * 2);
#else // AM_CMSIS_REGS
    ui32IntMask = AM_REG_CTIMER_INTSTAT_CTMRA0C0INT_M << (psConfig->ui32TestTimer * 2);
#endif // AM_CMSIS_REGS
    if ( psConfig->ui32TestSegment == AM_HAL_CTIMER_TIMERB )
    {
        ui32IntMask <<= 1;
    }

    while (1)
    {
        if (am_hal_ctimer_int_status_get(false) & ui32IntMask)
        {
            break;
        }
    }

    //
    // Stop SysTick
    //
#if AM_CMSIS_REGS
    SysTick->CTRL = 0;
#else // AM_CMSIS_REGS
    AM_REG(SYSTICK, SYSTCSR) = 0;
#endif // AM_CMSIS_REGS

    am_hal_ctimer_stop(psConfig->ui32TestTimer, psConfig->ui32TestSegment);
    am_hal_ctimer_clear(psConfig->ui32TestTimer, psConfig->ui32TestSegment);

    //
    // Check our time based on SysTick.
    // freq_of_timer = TimerTicks * HFRC / TotalSysTicks
    //
#if AM_CMSIS_REGS
    ui64TotalTime = g_ui64SysTickWrappedTime + 0x00FFFFFF - SysTick->VAL;
#else // AM_CMSIS_REGS
    ui64TotalTime = g_ui64SysTickWrappedTime + 0x00FFFFFF - AM_REG(SYSTICK, SYSTCVR);
#endif // AM_CMSIS_REGS
#if APOLLO3_FPGA
    ui64Product = (ui32ExpectedPeriod + 2) * 12500000ULL;
#else
    ui64Product = (ui32ExpectedPeriod + 2) * 48000000ULL;
#endif // APOLLO3_FPGA

#if 0 // REH
    am_util_stdio_printf(" ron: ProdH=%u, ProdL=%u, TotTimeH=%u, TotTimeL=%u\n",
        (uint32_t)(ui64Product >> 32), (uint32_t)ui64Product & 0xFFFFFFFF,
        (uint32_t)(ui64TotalTime >> 32), (uint32_t)ui64TotalTime & 0xFFFFFFFF);
#endif
    ui64Speed = (ui64Product / ui64TotalTime);

    //
    // Return the error.
    //
    return ui64Speed;
}

//*****************************************************************************
//
// Short interrupt handler for testing.
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_ctimer_int_status_get(false);
    am_hal_ctimer_int_clear(ui32Status);

    if (am_hal_ctimer_int_status_get(false))
    {
        while (1);
    }

    am_hal_ctimer_int_service(ui32Status);
}

volatile uint32_t ui32TimerIntStatus = 0;

//*****************************************************************************
//
// Dummy interrupt handler to prove we got an interrupt.
//
//*****************************************************************************
void
ctimer_handler(void)
{
    ui32TimerIntStatus = 1;
}


//*****************************************************************************
//
// Helper structure for checking timer pin enables.
//
//*****************************************************************************
#if AM_CMSIS_REGS
typedef struct
{
    uint32_t ui32Timer;
    uint32_t ui32Segment;
    uint32_t* ui32Register;
    uint32_t ui32Mask;
}
pin_enable_check_t;

pin_enable_check_t PinEnableChecklist[] =
{
    {0, AM_HAL_CTIMER_TIMERA, ((uint32_t*)&CTIMER->CTRL0)},
    {1, AM_HAL_CTIMER_TIMERA, ((uint32_t*)&CTIMER->CTRL1)},
    {2, AM_HAL_CTIMER_TIMERA, ((uint32_t*)&CTIMER->CTRL2)},
    {3, AM_HAL_CTIMER_TIMERA, ((uint32_t*)&CTIMER->CTRL3)},
    {0, AM_HAL_CTIMER_TIMERB, ((uint32_t*)&CTIMER->CTRL0)},
    {1, AM_HAL_CTIMER_TIMERB, ((uint32_t*)&CTIMER->CTRL1)},
    {2, AM_HAL_CTIMER_TIMERB, ((uint32_t*)&CTIMER->CTRL2)},
    {3, AM_HAL_CTIMER_TIMERB, ((uint32_t*)&CTIMER->CTRL3)}
};
#else // AM_CMSIS_REGS
typedef struct
{
    uint32_t ui32Timer;
    uint32_t ui32Segment;
    uint32_t ui32Register;
    uint32_t ui32Mask;
}
pin_enable_check_t;

pin_enable_check_t PinEnableChecklist[] =
{
    {0, AM_HAL_CTIMER_TIMERA, (CTIMER_BASE + AM_REG_CTIMER_CTRL0_O)},
    {1, AM_HAL_CTIMER_TIMERA, (CTIMER_BASE + AM_REG_CTIMER_CTRL1_O)},
    {2, AM_HAL_CTIMER_TIMERA, (CTIMER_BASE + AM_REG_CTIMER_CTRL2_O)},
    {3, AM_HAL_CTIMER_TIMERA, (CTIMER_BASE + AM_REG_CTIMER_CTRL3_O)},
    {0, AM_HAL_CTIMER_TIMERB, (CTIMER_BASE + AM_REG_CTIMER_CTRL0_O)},
    {1, AM_HAL_CTIMER_TIMERB, (CTIMER_BASE + AM_REG_CTIMER_CTRL1_O)},
    {2, AM_HAL_CTIMER_TIMERB, (CTIMER_BASE + AM_REG_CTIMER_CTRL2_O)},
    {3, AM_HAL_CTIMER_TIMERB, (CTIMER_BASE + AM_REG_CTIMER_CTRL3_O)}
};
#endif // AM_CMSIS_REGS

#define PIN_ENABLE_LENGTH                                                     \
    (sizeof(PinEnableChecklist) / sizeof(pin_enable_check_t))

#define TMRCMB(seg,tmr,out)     ((tmr << 0) | (out << 16) | ((seg == AM_HAL_CTIMER_TIMERA ? 0 : 1) << 8))

bool output_chk_return(uint32_t ui32Retval,
                       uint32_t ui32Timer,  uint32_t ui32TimerSeg,
                       uint32_t ui32Padnum, uint32_t ui32Outtype)
{
    bool bRetval, bValidCTXpin;
    uint32_t TimerCmb = TMRCMB(ui32TimerSeg, ui32Timer, ui32Outtype);

#if 1
    uint32_t ui32Seg = (ui32TimerSeg == AM_HAL_CTIMER_TIMERA ? 0 : 1);
    if ( (TimerCmb & 0xFFFF0000) > 0x00040000 )
    {
        am_util_stdio_printf("Error TimerCmb out: 0x%08x, tmr=%d, seg=%d, out=%d\n", TimerCmb, ui32Timer, ui32Seg, ui32Outtype);
        while(1);
    }
    if ( (TimerCmb & 0x0000FF00) > 0x00000100 )
    {
        am_util_stdio_printf("Error TimerCmb seg: 0x%08x, tmr=%d, seg=%d, out=%d\n", TimerCmb, ui32Timer, ui32Seg, ui32Outtype);
        while(1);
    }
    if ( (TimerCmb & 0x000000FF) > 0x00000007 )
    {
        am_util_stdio_printf("Error TimerCmb tmr: 0x%08x, tmr=%d, seg=%d, out=%d\n", TimerCmb, ui32Timer, ui32Seg, ui32Outtype);
        while(1);
    }
#endif

    //
    //  Check for the common configurations, A6OUT2, A7OUT2, FORCE0, FORCE1,
    //  which are always valid.
    //
    if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
         (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
         (ui32Outtype == AM_HAL_CTIMER_OUTPUT_FORCE0 )                                  ||
         (ui32Outtype == AM_HAL_CTIMER_OUTPUT_FORCE1 ) )
    {
        //
        // These settings are supported on ALL pins that support CTX.
        // CTX pins: 4-7,11-13,18-19,22-33,35,37,39,42-49
        //
        if ( (ui32Padnum >=  4  &&  ui32Padnum <=  7)   ||
             (ui32Padnum >= 11  &&  ui32Padnum <= 13)   ||
             (ui32Padnum >= 18  &&  ui32Padnum <= 19)   ||
             (ui32Padnum >= 22  &&  ui32Padnum <= 33)   ||
             (ui32Padnum == 35)                         ||
             (ui32Padnum == 37)                         ||
             (ui32Padnum == 39)                         ||
             (ui32Padnum >= 42  &&  ui32Padnum <= 49) )
        {
            bValidCTXpin = true;
        }
        else
        {
            bValidCTXpin = false;
        }

        if ( ui32Retval == AM_HAL_STATUS_SUCCESS )
        {
            if ( bValidCTXpin )
                return true;
            else
                return false;
        }
        else
        {
            if ( !bValidCTXpin )
                return true;
            else
                return false;
        }
    }

    //
    // Check for valid combinations.
    //
    switch ( ui32Padnum )
    {
        case 4:    // CTX=17, PAD=4
            // 17    4  0   1   A4OUT2  B7OUT   A4OUT   A1OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 5:    // CTX= 8, PAD=5
            //  8    5  0   1   A2OUT   A3OUT2  A4OUT2  B6OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 6:    // CTX=10, PAD=6
            // 10    6  0   1   B2OUT   B3OUT2  B4OUT2  A6OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 7:    // CTX=19, PAD=7
            // 19    7  0   1   B4OUT2  A2OUT   B4OUT   B1OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 11:    // CTX=31, PAD=11
            // 31   11  0   1   B7OUT2  A6OUT   B7OUT   B3OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 12:    // CTX= 0, PAD=12
            // 0    12  0   1   A0OUT   B2OUT2  A5OUT2  A6OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 13:    // CTX= 2, PAD=13
            // 2    13  0   1   B0OUT   B1OUT2  B6OUT2  A7OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 18:    // CTX= 4, PAD=18
            // 4    18  0   1   A1OUT   A2OUT2  A5OUT2  B5OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 19:    // CTX= 6, PAD=19
            // 6    19  0   1   B1OUT   A1OUT   B5OUT2  B7OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 22:    // CTX=12, PAD=22
            // 12   22   0   1   A3OUT   B1OUT   B0OUT2  B6OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 23:    // CTX=14, PAD=23
            // 14   23  0   1   B3OUT   B1OUT   B7OUT2  A7OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 24:    // CTX=21, PAD=24
            // 21   24  0   1   A5OUT2  A1OUT   B5OUT   A0OUT2  A6OUT2  A7OUT2

            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 25:    // CTX=1, PAD=25
            //  1   25  0   1   A0OUT2  A0OUT   A5OUT   B7OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 26:    // CTX=3, PAD=26
            //  3   26  0   1   B0OUT2  B0OUT   A1OUT   A6OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 27:    // CTX=5, PAD=27
            //  5   27  0   1   A1OUT2  A1OUT   B6OUT   A7OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 28:    // CTX=7, PAD=28
            //  7   28  0   1   B1OUT2  B1OUT   B5OUT   A7OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 29:    // CTX=9, PAD=29
            //  9   29  0   1   A2OUT2  A2OUT   A4OUT   B0OUT   A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_NORMAL )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 30:    // CTX=11, PAD=30
            // 11   30  0   1   B2OUT2  B2OUT   B4OUT   B5OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 31:    // CTX=13, PAD=31
            // 13   31  0   1   A3OUT2  A3OUT   A6OUT   B4OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 32:    // CTX=15, PAD=32
            // 15   32  0   1   B3OUT2  B3OUT   A7OUT   A4OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 33:    // CTX=23, PAD=33
            // 23   33  0   1   B5OUT2  A7OUT   A5OUT   B0OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 35:    // CTX=27, PAD=35
            // 27   35  0   1   B6OUT2  A1OUT   B6OUT   B2OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 37:    // CTX=29, PAD=37
            // 29   37  0   1   B5OUT2  A1OUT   A7OUT   A3OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 39:    // CTX=25, PAD=39
            // 25   39  0   1   B4OUT2  B2OUT   A6OUT   A2OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 42:    // CTX=16, PAD=42
            // 16   42  0   1   A4OUT   A0OUT   A0OUT2  B3OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 43:    // CTX=18, PAD=43
            // 18   43  0   1   B4OUT   B0OUT   A0OUT   A3OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 4, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 44:    // CTX=20, PAD=44
            // 20   44  0   1   A5OUT   A1OUT   A1OUT2  B2OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY))  ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 45:    // CTX=22, PAD=45
            // 22   45  0   1   B5OUT   A6OUT   A1OUT  A2OUT2 A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 5, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL))     ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 46:    // CTX=24, PAD=46
            // 24   46  0   1   A6OUT   A2OUT   A1OUT   B1OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_NORMAL))     ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 47:    // CTX=26, PAD=47
            // 26   47  0   1   B6OUT   B2OUT   A5OUT   A1OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 6, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 2, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_NORMAL))     ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 1, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 48:    // CTX=28, PAD=48
            // 28   48  0   1   A7OUTB  A3OUT   A5OUT2  B0OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 3, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 5, AM_HAL_CTIMER_OUTPUT_SECONDARY))  ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        case 49:    // CTX=30, PAD=49
            // 30   49  0   1   B7OUT   B3OUT   A4OUT2  A0OUT2  A6OUT2  A7OUT2
            if ( (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 7, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERB, 3, AM_HAL_CTIMER_OUTPUT_NORMAL ))    ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 4, AM_HAL_CTIMER_OUTPUT_SECONDARY))  ||
                 (TimerCmb == TMRCMB(AM_HAL_CTIMER_TIMERA, 0, AM_HAL_CTIMER_OUTPUT_SECONDARY )) )
            {
                bRetval = ( ui32Retval == AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            else
            {
                bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;
            }
            break;

        default:
            bRetval = ( ui32Retval != AM_HAL_STATUS_SUCCESS ) ? true : false;

    } // switch (ui32Timer)

    return bRetval;


} // output_chk_return()


int32_t
am_widget_ctimer_read_test(const void *psSettings, char *pErrorStr)
{
    uint32_t timer_elapsed_ticks;
    float fError;

    am_util_stdio_printf("am_widget_ctimer_read_test()\n");

    for(uint8_t i = CLK_SRC_START; i <= CLK_SRC_END; i++)
    {
        g_sContTimer.ui32TimerAConfig = AM_HAL_CTIMER_FN_CONTINUOUS | (g_sTimerTick[i].ui32Freq);
        am_util_stdio_printf("**********%s test begins**********\n", g_sTimerTick[i].ui8Name);
        for(uint8_t j = 1; j <= ITERATION_NUM; j++)
        {
            //
            // Set up and start the timer.
            //
            am_hal_ctimer_stop(TEST_TIMERNUM, AM_HAL_CTIMER_BOTH);
            am_hal_ctimer_clear(TEST_TIMERNUM, AM_HAL_CTIMER_BOTH);
            am_hal_ctimer_config(TEST_TIMERNUM, &g_sContTimer);
            am_hal_ctimer_start(TEST_TIMERNUM, AM_HAL_CTIMER_TIMERA);

            am_hal_flash_delay(FLASH_CYCLES_US_NOCACHE(1000000 * j)); //1s

            //
            // Stop the timer and save off the run times.
            //
            timer_elapsed_ticks = am_hal_ctimer_read(TEST_TIMERNUM, AM_HAL_CTIMER_BOTH);
            am_hal_ctimer_stop(TEST_TIMERNUM, AM_HAL_CTIMER_BOTH);
            if(timer_elapsed_ticks > g_sTimerTick[i].ui32Tick * j)
            {
                fError = ((float)timer_elapsed_ticks - (float)(g_sTimerTick[i].ui32Tick * j)) / (float)(g_sTimerTick[i].ui32Tick * j) * 100.0;
            }
            else
            {
                fError = ((float)(g_sTimerTick[i].ui32Tick * j) - (float)timer_elapsed_ticks) / (float)(g_sTimerTick[i].ui32Tick * j) * 100.0;
            }
            //
            // Print out the run time
            //
            am_util_stdio_printf("delay %d ticks, error %.3f%%\n", timer_elapsed_ticks, fError);
            if(fError >= ERR_THRES)
            {
                am_util_stdio_printf("large error, test fails!\n");
                return 1;
            }
        }
        am_util_stdio_printf("**********%s test ends**********\n", g_sTimerTick[i].ui8Name);
    }
    return 0;
}

int32_t
am_widget_ctimer_api_test(const void *psSettings, char *pErrorStr)
{
    am_util_stdio_printf("am_widget_ctimer_api_test()\n");

    //
    // Interrupt test. Check to see if we can enable and disable an interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERB2C1);

    if (!(am_hal_ctimer_int_enable_get() & AM_HAL_CTIMER_INT_TIMERB2C1))
    {
        return 1;
    }

    am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERB2C1);

    if (am_hal_ctimer_int_enable_get() & AM_HAL_CTIMER_INT_TIMERB2C1)
    {
        return 2;
    }

    //
    // Check to see if we can actually get an interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERB2C1);
    am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERB2C1, ctimer_handler);
    am_hal_ctimer_int_set(AM_HAL_CTIMER_INT_TIMERB2C1);
#if AM_CMSIS_REGS
    NVIC_EnableIRQ(CTIMER_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
#endif // AM_CMSIS_REGS
    am_hal_interrupt_master_enable();

    //
    // Wait for the interrupt to happen.
    //
    for (uint32_t i = 0; i < 100 ; i++)
    {
        if(ui32TimerIntStatus == 1)
        {
            ui32TimerIntStatus = 0;
            break;
        }
        else if (i == 99)
        {
            return 3;
        }
    }

#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
#define TIMER_NUM_MSK   ((AM_HAL_CTIMER_TIMERS_NUM - 1))

    uint32_t ui32Ret, ui32PadNum, ui32Timer, ui32Outtype, ui32TimerSeg;

    //
    // Test the am_hal_ctimer_output_config() function.
    // Run through all combinations of pin number, timer, timer segments,
    // and output types.
    //
    for ( ui32PadNum = 0; ui32PadNum < AM_HAL_GPIO_MAX_PADS; ui32PadNum++ )
    {
        // Check A and B timers
        for ( ui32Timer = 0; ui32Timer < AM_HAL_CTIMER_TIMERS_NUM * 2; ui32Timer++ )
        {
            ui32TimerSeg = (ui32Timer >= AM_HAL_CTIMER_TIMERS_NUM) ? AM_HAL_CTIMER_TIMERA : AM_HAL_CTIMER_TIMERB;

            for ( ui32Outtype = AM_HAL_CTIMER_OUTPUT_NORMAL; ui32Outtype <= AM_HAL_CTIMER_OUTPUT_FORCE1; ui32Outtype++ )
            {
                ui32Ret = am_hal_ctimer_output_config(ui32Timer & TIMER_NUM_MSK,
                                                      ui32TimerSeg,
                                                      ui32PadNum,
                                                      ui32Outtype,
                                                      AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA);
#if 0   // Debug
                char *pTimer;
                if (ui32TimerSeg == AM_HAL_CTIMER_TIMERA)
                    pTimer = "TIMERA";
                else if (ui32TimerSeg == AM_HAL_CTIMER_TIMERB)
                    pTimer = "TIMERB";
                else if (ui32TimerSeg == AM_HAL_CTIMER_BOTH)
                    pTimer = "TIMER BOTH";
                else
                    pTimer = "Invalid timer segment";

                am_util_stdio_printf("  ui32Ret=%d, pad=%d, seg=%s, tmr=%2d, out=%d\n", ui32Ret, ui32PadNum, pTimer, ui32Timer & TIMER_NUM_MSK, ui32Outtype);
#endif

                if ( !output_chk_return(ui32Ret, ui32Timer & TIMER_NUM_MSK, ui32TimerSeg, ui32PadNum, ui32Outtype) )
                {
                    return 1;
                }
            } // for (ui32Outtype )
        } // for ( ui32Timer )
    } // for ( ui32PadNum )
#endif // AM_PART_APOLLO3


#if !AM_APOLLO3_CTIMER
    //
    // Register checks.
    //
    for (uint32_t i = 0; i < PIN_ENABLE_LENGTH; i++)
    {
        pin_enable_check_t TestCase = PinEnableChecklist[i];

        //
        // Figure out where the correct pin-enable and polarity bits are, and
        // make a mask where those bits are set.
        //
#if AM_CMSIS_REGS
        uint32_t ui32Mask = (TestCase.ui32Segment == AM_HAL_CTIMER_TIMERA) ?
            (CTIMER_CTRL0_TMRA0PE_Msk | CTIMER_CTRL0_TMRA0POL_Msk) :
            (CTIMER_CTRL0_TMRB0PE_Msk | CTIMER_CTRL0_TMRB0POL_Msk);
#else // AM_CMSIS_REGS
        uint32_t ui32Mask = (TestCase.ui32Segment == AM_HAL_CTIMER_TIMERA) ?
            (AM_REG_CTIMER_CTRL0_TMRA0PE_M | AM_REG_CTIMER_CTRL0_TMRA0POL_M) :
            (AM_REG_CTIMER_CTRL0_TMRB0PE_M | AM_REG_CTIMER_CTRL0_TMRB0POL_M);
#endif // AM_CMSIS_REGS

        //
        // Enable the pin, invert the polarity, and make sure those bits are
        // actually set.
        //
        am_hal_ctimer_pin_enable(TestCase.ui32Timer, TestCase.ui32Segment);
        am_hal_ctimer_pin_invert(TestCase.ui32Timer, TestCase.ui32Segment, true);

        if ((AM_REGVAL(TestCase.ui32Register) & ui32Mask) != ui32Mask)
        {
            return 4;
        }

        //
        // Check the disable functions as well.
        //
        am_hal_ctimer_pin_disable(TestCase.ui32Timer, TestCase.ui32Segment);
        am_hal_ctimer_pin_invert(TestCase.ui32Timer, TestCase.ui32Segment, false);

        if ((AM_REGVAL(TestCase.ui32Register) & ui32Mask) != 0)
        {
            return 5;
        }
    }
#endif

    //
    // Check the ADC trigger functions.
    //
    am_hal_ctimer_adc_trigger_enable();

#if AM_CMSIS_REGS
    if (!CTIMER->CTRL3_b.ADCEN)
#else // AM_CMSIS_REGS
    if (!AM_BFM(CTIMER, CTRL3, ADCEN))
#endif // AM_CMSIS_REGS
    {
        return 6;
    }

    am_hal_ctimer_adc_trigger_disable();

#if AM_CMSIS_REGS
    if ( CTIMER->CTRL3_b.ADCEN)
#else // AM_CMSIS_REGS
    if (AM_BFM(CTIMER, CTRL3, ADCEN))
#endif // AM_CMSIS_REGS
    {
        return 7;
    }

    return 0;
}
