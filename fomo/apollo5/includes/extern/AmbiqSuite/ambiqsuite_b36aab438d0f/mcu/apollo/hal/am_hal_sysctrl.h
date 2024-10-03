//*****************************************************************************
//
//  am_hal_sysctrl.h
//! @file
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl1 System Control (SYSCTRL)
//! @ingroup apollo1hal
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
// Macros to make intrinsics a little easier.
//
//*****************************************************************************
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
#define am_wfi() __wfi();
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
#define am_wfi() __asm("    wfi")
#elif defined(__GNUC_STDC_INLINE__)
#define am_wfi() __asm("    wfi")
#elif defined(__IAR_SYSTEMS_ICC__)
#define am_wfi() asm("    wfi")
#else
#error Compiler is unknown, please contact Ambiq support team
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_sysctrl_sleep(bool bSleepDeep);
extern void am_hal_sysctrl_sleep_auto(void);
extern void am_hal_sysctrl_fpu_enable(void);
extern void am_hal_sysctrl_fpu_disable(void);
extern void am_hal_sysctrl_fpu_stacking_enable(bool bLazy);
extern void am_hal_sysctrl_fpu_stacking_disable(void);
extern void am_hal_sysctrl_aircr_reset(void);
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

