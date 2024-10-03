//*****************************************************************************
//
//! @file pwr_timer_utils.h
//!
//! @brief This will manage the timer for the example
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_mspi MSPI Power Example
//! @ingroup power_examples
//! @{
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


#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

#define TMR_TICKS_PER_SECOND 1024

typedef void (*tmr_callback_fcn_t)(void);

//*****************************************************************************
//
//! @brief  Init and start timer interrupt
//!
//! @note The timer is set to use XT/32 timer
//!
//! @param ui32TimerCounts    -- Number of timer counts for interrupt
//! @param pfnCallbackFcnParam  -- Function called from the timer interrupt
//!
//! @return standard hal status
//
//*****************************************************************************
extern uint32_t timer_init(uint32_t ui32TimerCounts, tmr_callback_fcn_t pfnCallbackFcnParam);

//*****************************************************************************
//
//! @brief  Disable timer and timer interrupt
//
//*****************************************************************************
extern void timer_disable(void);


#endif //TIMER_UTILS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
