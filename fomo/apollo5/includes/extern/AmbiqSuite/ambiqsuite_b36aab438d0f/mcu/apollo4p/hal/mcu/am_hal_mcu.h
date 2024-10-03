//*****************************************************************************
//
//! @file am_hal_mcu.h
//!
//! @brief Functions for accessing and configuring MCU specific HAL modules
//!
//! @addtogroup mcu_4p MCU Hal Modules
//! @ingroup apollo4p_hal
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

