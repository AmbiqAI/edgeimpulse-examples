//*****************************************************************************
//
//! @file pwm_config.c
//!
//! @brief PWM config example code
//!
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "pwm_config.h"
#include "am_util.h"
#include "am_bsp.h"

static const am_hal_gpio_pincfg_t g_PWMOutPinCfg =
{
    .GP.cfg_b.uFuncSel             = 6, //CT
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
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
//! @brief set up the PWM timer
//!
//!
//! @return standard hal status
//
//*****************************************************************************

uint32_t
pwm_setupPwm(pwm_setup_t *pPwmSetup)
{
    //
    // The default config parameters include:
    //  eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16
    //  eFunction = AM_HAL_TIMER_FN_EDGE
    //  Compare0 and Compare1 maxed at 0xFFFFFFFF
    //

    //
    // compute timer counter-reset value in PWM mode
    //
    if (pPwmSetup->ePwmClk > AM_HAL_TIMER_CLOCK_HFRC_DIV4K)
    {
        am_util_stdio_printf("Timer PWM Frequency too fast\r\n");
        return AM_HAL_STATUS_INVALID_OPERATION;
    }


    am_hal_timer_config_t tPwmTimerConfig;

    am_hal_timer_default_config_set(&tPwmTimerConfig);

    tPwmTimerConfig.eFunction = AM_HAL_TIMER_FN_PWM;
    tPwmTimerConfig.eInputClock = pPwmSetup->ePwmClk;

    //
    // Configure the PWM timer.
    //
    am_hal_timer_config(pPwmSetup->ui32TimerNumber, &tPwmTimerConfig);

    // Config output pins
    // the second output pin is the compliment of the first
    if (pPwmSetup->ui32PadNumber1 < 0xFFFF)
    {
        am_hal_timer_output_config(pPwmSetup->ui32PadNumber1, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + AM_BSP_PWM_LED_TIMER * 2);
        am_hal_gpio_pinconfig(pPwmSetup->ui32PadNumber1, g_PWMOutPinCfg);
    }
    if (pPwmSetup->ui32PadNumber2 < 0xFFFF)
    {
        am_hal_timer_output_config(pPwmSetup->ui32PadNumber2, AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + AM_BSP_PWM_LED_TIMER * 2);
        am_hal_gpio_pinconfig(pPwmSetup->ui32PadNumber2, g_PWMOutPinCfg);
    }

    // #### INTERNAL BEGIN ####
#if APOLLO4_FPGA
    ui64Div = 16 << (2 * PWM_CLK);
#else
    // #### INTERNAL END ####
    uint64_t ui64Div = 4 << (2 * pPwmSetup->ePwmClk);
    // #### INTERNAL BEGIN ####
#endif
    // #### INTERNAL END ####
    //
    // note: often more precision if multiply happens before divide
    //
    uint64_t ui64BaseFreq_x128 = (96000000ull / ui64Div) * pPwmSetup->ui32FreqFractionalScaling;

    //
    // in the divide below the extra scaling factor will divide out
    // round and compute the compare 0 value
    //
    uint64_t ui64EndCounts = ((ui64BaseFreq_x128 + (pPwmSetup->ui32PwmFreq_x128 / 2)) / pPwmSetup->ui32PwmFreq_x128);
    uint32_t ui32Compare0 = (uint32_t) ui64EndCounts;

    am_hal_timer_compare0_set(pPwmSetup->ui32TimerNumber, ui32Compare0);

    //
    // rounding divide by 100 to compute comp1 value for duty cycle
    //
    uint32_t ui32Compare1 = (ui64EndCounts * pPwmSetup->ui32DC_x100 + 50) / 100;
    ui32Compare1 = ui32Compare1 <= ui32Compare0 ? ui32Compare1 : ui32Compare0;

    am_hal_timer_compare1_set(pPwmSetup->ui32TimerNumber, ui32Compare1);

    //
    // Enable the TIMER.
    //
    am_hal_timer_enable(pPwmSetup->ui32TimerNumber);

    return AM_HAL_STATUS_SUCCESS;

} // pwm_examp_setupPwm
