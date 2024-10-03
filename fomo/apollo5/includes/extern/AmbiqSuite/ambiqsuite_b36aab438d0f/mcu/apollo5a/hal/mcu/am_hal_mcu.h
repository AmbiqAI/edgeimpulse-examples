//*****************************************************************************
//
//! @file am_hal_mcu.h
//!
//! @brief Functions for accessing and configuring MCU specific HAL modules
//!
//! @addtogroup mcu MCU Hal Modules
//! @ingroup apollo5a_hal
//! @{

//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_HAL_MCU_H
#define AM_HAL_MCU_H

#include "hal/mcu/am_hal_mcu_sysctrl.h"
#include "hal/mcu/am_hal_mcu_interrupt.h"

#define am_hal_get_core_id()    (DSP_MUTEX0_MUTEX0_CPU)

#ifdef __IAR_SYSTEMS_ICC__
#define am_count_num_leading_zeros(n)                     __CLZ(n)
#else
#define am_count_num_leading_zeros(n)                     __builtin_clz(n)
#endif

//
// The Arm6 compiler defines both GNUC and ARMCC_VERSION. So check ARMCC first.
//
#if defined(__ARMCC_VERSION)
#define COMPILER_VERSION                    ("ARMCC " STRINGIZE_VAL(__ARMCC_VERSION))
#elif __GNUC__
#define COMPILER_VERSION                    ("GCC " __VERSION__)
#elif defined(__KEIL__)
#define COMPILER_VERSION                    "KEIL_CARM " STRINGIZE_VAL(__CA__)
#elif defined(__IAR_SYSTEMS_ICC__)
#define COMPILER_VERSION                    __VERSION__
#else
#error "Unknown Compiler"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
#endif // AM_HAL_MCU_H

