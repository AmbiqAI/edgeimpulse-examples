//*****************************************************************************
//
//! @file usb_timer_utils.h
//!
//! @brief This will manage the timer for the example
//!
//! @brief contains the common board init function
//!
//! @addtogroup usb_examples USB Examples
//!
//! @defgroup tinyusb_cdc_msc_hfrc2 TinyUSB CDC MSC HFRC2 Example
//! @ingroup usb_examples
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

#ifndef USB_TIMER_UTILS_H
#define USB_TIMER_UTILS_H

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


#endif // USB_TIMER_UTILS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

