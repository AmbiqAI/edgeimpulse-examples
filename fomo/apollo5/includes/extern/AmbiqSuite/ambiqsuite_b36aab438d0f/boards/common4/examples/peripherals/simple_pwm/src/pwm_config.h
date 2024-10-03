//*****************************************************************************
//
//! @file pwm_config.h
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

#ifndef PWM_CONFIG
#define PWM_CONFIG

#include "am_mcu_apollo.h"

typedef struct
{
    //
    //! pad number
    //! pad is disabled if this value > 0xFFFF
    //
    uint32_t ui32PadNumber1;
    //
    //! pad number
    //! pad is disabled if this value > 0xFFFF
    //
    uint32_t ui32PadNumber2;

    uint32_t ui32TimerNumber;
    //
    //! scaled freq number, this value is the frequency multiplies by the scale factor below
    //
    uint32_t ui32PwmFreq_x128;
    //
    //! frequency scale factor
    //
    uintptr_t ui32FreqFractionalScaling;

    //
    //! percent duty cycle (0-100)
    //
    uint32_t ui32DC_x100;

    //
    // clock mode used for PWM
    //
    am_hal_timer_clock_e ePwmClk;

} pwm_setup_t;


//***************************************************************************************
//
//! @brief will setup pwm
//!
//! @param[in] pwmSetup  pointer to struct containing params
//!
//! @return standard hal status
//
//***************************************************************************************
extern uint32_t pwm_setupPwm( pwm_setup_t *pwmSetup ) ;





#endif //PWM_CONFIG
