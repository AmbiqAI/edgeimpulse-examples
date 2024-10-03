//*****************************************************************************
//
//! am_hal_sysctrl.h
//! @file
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl2 System Control (SYSCTRL)
//! @ingroup apollo2hal
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_HAL_SYSCTRL_H
#define AM_HAL_SYSCTRL_H


//*****************************************************************************
//
// Definitions for sleep mode parameter
//
//*****************************************************************************
#define AM_HAL_SYSCTRL_SLEEP_DEEP       true
#define AM_HAL_SYSCTRL_SLEEP_NORMAL     false

//*****************************************************************************
//
// Parameters for am_hal_sysctrl_buck_ctimer_isr_init()
//
//*****************************************************************************
//
// Define the maximum valid timer number
//
#define BUCK_TIMER_MAX                  (AM_HAL_CTIMER_TIMERS_NUM - 1)

//
// Define the valid timer numbers
//
#define AM_HAL_SYSCTRL_BUCK_CTIMER_TIMER0   0
#define AM_HAL_SYSCTRL_BUCK_CTIMER_TIMER1   1
#define AM_HAL_SYSCTRL_BUCK_CTIMER_TIMER2   2
#define AM_HAL_SYSCTRL_BUCK_CTIMER_TIMER3   3

//
// The following is an invalid timer number. If used, it is the caller telling
// the HAL to use the "Hard Option", which applies a constant value to the zero
// cross. The applied value is more noise immune, if less energy efficent.
//
#define AM_HAL_SYSCTRL_BUCK_CTIMER_ZX_CONSTANT      0x01000000  // No timer, apply a constant value
// #### INTERNAL BEGIN ####
// FUTURE FEATURE
#define AM_HAL_SYSCTRL_BUCK_CTIMER_ZX_TIMED         0x02000000  // Apply after a period of time

//
// Use this macro to provide a ZX override value to the init function.
//
#define AM_HAL_SYSCTRL_BUCK_CTIMER_ZX_VALUE(n)  ((n + 1) | AM_HAL_SYSCTRL_BUCK_CTIMER_ZX_NO_TIMER)
// #### INTERNAL END ####

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void     am_hal_sysctrl_sleep(bool bSleepDeep);
extern void     am_hal_sysctrl_fpu_enable(void);
extern void     am_hal_sysctrl_fpu_disable(void);
extern void     am_hal_sysctrl_fpu_stacking_enable(bool bLazy);
extern void     am_hal_sysctrl_fpu_stacking_disable(void);
extern void     am_hal_sysctrl_aircr_reset(void);

//
// Apollo2 zero-cross buck/ctimer related functions
//
extern uint32_t am_hal_sysctrl_buck_ctimer_isr_init(uint32_t ui32BuckTimerNumber);
extern bool     am_hal_sysctrl_buck_update_complete(void);
// #### INTERNAL BEGIN ####
// FUTURE FEATURE
extern void     am_hal_sysctrl_buck_restore(void);
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_SYSCTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

