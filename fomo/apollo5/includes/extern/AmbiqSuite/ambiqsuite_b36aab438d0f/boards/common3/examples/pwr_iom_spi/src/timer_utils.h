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

#define LOCAL_IRQ_PRIORITY_DEFAULT 0x04

#define SPI_IRQ_PRIORITY 0x03
#define SPI_TIMER_PRIORITY LOCAL_IRQ_PRIORITY_DEFAULT

#define TMR_TICKS_PER_SECOND 512

//
//! prototye for timer callback
//

typedef void (*tmr_callback_fcn_t)(void *x);

//*****************************************************************************
//
//! @brief enable the periodic timer interrupt
//!
//! @param ui32LfrcCounts   // number of timer counts used, use TMR_TICKS_PER_SECOND as reference
//! @param callbackFcnParam // function called from timer isr when timer expires
//!
//! @return standard hal status
//
//*****************************************************************************
extern uint32_t timer_init(uint32_t ui32LfrcCounts, tmr_callback_fcn_t callbackFcnParam);

//*****************************************************************************
//
//! @brief disable the timer
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

