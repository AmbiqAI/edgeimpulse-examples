//*****************************************************************************
//
//! @file timer_utils.c
//!
//! @brief
//!
//! @addtogroup ap3x_power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup ap3x_power_examples
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
#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"


typedef void (*tmr_callback_fcn_t)(void *x);

extern uint32_t timer_init(uint32_t ui32TimerCounts, tmr_callback_fcn_t callbackFcnParam);

extern void timer_disable(void);

#endif //TIMER_UTILS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

