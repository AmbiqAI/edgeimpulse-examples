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
#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
const am_hal_gpio_pincfg_t g_AM_HAL_GPIO_0_OUTPUT =
{
  .uFuncSel             = AM_HAL_PIN_0_GPIO
};
#endif

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
#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
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
#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
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

#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
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
    am_util_stdio_printf(" ron: ProdH=%u, ProdL=%u, TotTimeH=%u, TotTimeL=%u\n",
        (uint32_t)(ui64Product >> 32), (uint32_t)ui64Product & 0xFFFFFFFF,
        (uint32_t)(ui64TotalTime >> 32), (uint32_t)ui64TotalTime & 0xFFFFFFFF);
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

int32_t
am_widget_ctimer_api_test(const void *psSettings, char *pErrorStr)
{
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


//
// TODO: Need to update these test cases for the am_hal_ctimer_output_config() function.
//
#if 0
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
        #if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
        uint32_t ui32Mask = (TestCase.ui32Segment == AM_HAL_CTIMER_TIMERA) ?
            (CTIMER_CTRL0_TMRA0POL_Msk) :
            (CTIMER_CTRL0_TMRB0POL_Msk);
        #else
        uint32_t ui32Mask = (TestCase.ui32Segment == AM_HAL_CTIMER_TIMERA) ?
            (CTIMER_CTRL0_TMRA0PE_Msk | CTIMER_CTRL0_TMRA0POL_Msk) :
            (CTIMER_CTRL0_TMRB0PE_Msk | CTIMER_CTRL0_TMRB0POL_Msk);
        #endif
#else // AM_CMSIS_REGS
        #if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
        uint32_t ui32Mask = (TestCase.ui32Segment == AM_HAL_CTIMER_TIMERA) ?
            (AM_REG_CTIMER_CTRL0_TMRA0POL_M) :
            (AM_REG_CTIMER_CTRL0_TMRB0POL_M);
        #else
        uint32_t ui32Mask = (TestCase.ui32Segment == AM_HAL_CTIMER_TIMERA) ?
            (AM_REG_CTIMER_CTRL0_TMRA0PE_M | AM_REG_CTIMER_CTRL0_TMRA0POL_M) :
            (AM_REG_CTIMER_CTRL0_TMRB0PE_M | AM_REG_CTIMER_CTRL0_TMRB0POL_M);
        #endif
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
