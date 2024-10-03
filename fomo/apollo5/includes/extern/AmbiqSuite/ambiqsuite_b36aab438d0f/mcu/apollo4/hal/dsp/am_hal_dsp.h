//*****************************************************************************
//
//! @file am_hal_dsp.h
//!
//! @brief Functions for accessing and configuring DSP specific HAL modules
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @ingroup hal
//! @{

//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_HAL_DSP_H
#define AM_HAL_DSP_H

#if __XTENSA__

#include <xtensa/xtruntime.h>

#define am_hal_get_core_id()                          (1 + XT_RSR_PRID())
#define am_count_num_leading_zeros(num)               XT_NSAU(num)
#define COMPILER_VERSION                              ("XTENSA " XTENSA_RELEASE_NAME "SW" XTENSA_SWVERSION_NAME)

#include "hal/dsp/am_hal_dsp_sysctrl.h"
#include "hal/dsp/am_hal_dsp_interrupt.h"

#else
#error "Xtensa not defined!!"
#endif
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
#endif // AM_HAL_DSP_H
