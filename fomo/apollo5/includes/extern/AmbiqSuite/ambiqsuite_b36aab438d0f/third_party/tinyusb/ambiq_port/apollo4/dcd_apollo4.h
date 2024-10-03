//*****************************************************************************
//
//! @file dcd_apollo4.h
//!
//! @brief Addional Functions for USB module not defined in dcd.h
//!
//! @addtogroup tinyUSBAmbiq Functionality
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

#ifndef _DCD_APOLLO4_H
#define _DCD_APOLLO4_H

#include "tusb_option.h"

#if CFG_TUD_ENABLED

#include "device/dcd.h"
#include "am_mcu_apollo.h"
#include "am_util_delay.h"
#include "am_bsp.h"         // Declare BSP functions am_bsp_external_vddusb33_switch() and am_bsp_external_vddusb0p9_switch()

//
//! This is to enable extra functions not defined in dcd.h
//! These functions are prototyped below
//
#define TUSB_ADDED_FUNCTIONS 1

//*****************************************************************************
//
//! @brief This is called for a graceful USB power-up.
//!
//! @note this will typically be called as an even callback from the tud_task
//!
//! @param rhport unused (usb number)
//! @param force  when true, forces the code to execute the power up sequence
//
//*****************************************************************************
extern void dcd_powerup(uint8_t rhport, bool force );

//*****************************************************************************
//! @brief this is called for a graceful USB powerdown
//!
//! @note this will typically be called as an even callback from the tud_task
//!
//! @param rhport unused (usb number)
//! @param force  when true, force the code to execute the power down sequence
//*****************************************************************************
extern void dcd_powerdown(uint8_t rhport, bool force );

//*****************************************************************************
//
//! @brief This is called when VUSB changes state
//!
//! @note the primary use to set VUSB is so the init function will run
//! on system startup
//!
//! @param rhport         USB port number
//! @param vddIsPresent   true if VUSB is present
//! @param takeAction     will run the init functions (recommended to usually be false)
//!
//
//*****************************************************************************
extern void dvd_set_vddPresent(
    uint8_t rhport,
    bool vddIsPresent,
    bool takeAction);

//*****************************************************************************
//
//! @brief shutdown and powerdown the usb
//!
//! @param rhport
//
//*****************************************************************************
extern void dcd_deInit( uint8_t rhport) ;

#endif // CFG_TUD_ENABLED

#endif //_DCD_APOLLO4_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
