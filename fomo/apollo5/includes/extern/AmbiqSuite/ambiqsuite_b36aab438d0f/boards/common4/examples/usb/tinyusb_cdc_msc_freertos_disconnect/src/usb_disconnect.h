//*****************************************************************************
//
//! @file usb_disconnect.h
//!
//! @brief tinyusb cdc- disconnect example.
//!
//! This example demonstrates how to use disconnect and reconnect
//! (USB V bus voltage lost and restore) to shutdown / restart the usb module
//! This file specifically is a header for the
//! helper function that simulates power loss/restore
//! on boards that don't have detection capability, it is intended to be used
//! as an example and template..
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef USB_DISCONNECT_EXAMP_H
#define USB_DISCONNECT_EXAMP_H

#include "am_mcu_apollo.h"

//*****************************************************************************
//
//! @brief This is used to configure the button press, that has been done in
//!  the bsp so is empty for this application.
//
//*****************************************************************************
void usb_disconnect_init( void );

//*****************************************************************************
//
//! @brief periodic button scan mgr, this is used to simulate a usb disconnect
//! @details this is called periodically to scan the buttons and
//! initiate action when pressed
//! @note, one way to use, disconnect the usb connector, then push the button assigned to
//! the disconnect simulation
//!
//! @param calledFromIsrOrThread set to true when in multithreaded environment
//!                              or when calling from ISR
//
//*****************************************************************************
void usb_disconnect_periodic(bool calledFromIsrOrThread);

#endif //USB_DISCONNECT_EXAMP_H
