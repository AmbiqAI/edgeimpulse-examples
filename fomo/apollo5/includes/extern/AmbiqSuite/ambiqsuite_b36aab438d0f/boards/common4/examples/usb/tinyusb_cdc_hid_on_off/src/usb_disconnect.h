//*****************************************************************************
//
//! @file usb_disconnect.h
//!
//! @brief tinyusb cdc- button scan with USB disconnect/connect.
//!
//! This function programs the buttons to  disconnect and reconnect
//! (USB V bus voltage lost and restore) to shutdown / restart the usb module
//! This specifically contains code for helper functions that
//! will poll the buttons on an evb. These are used to simulate programmable actions
//! where the firmware wants to disable on USB profile and start another.
//! the example here is switching between three profiles
//!   CDC
//!   composite CDC
//!   composite CDC - HID
//!
//! when the back button (closet to edge) is pressed
//! usb will be disconnected and powered down
//! when the forward button is pressed
//! the next profile is selected and the usb is re-started causing a USB enumeration
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
//! @brief This is used to configure the button press.
//
//*****************************************************************************
extern void usb_disconnect_init( void );

//*****************************************************************************
//
//! @brief periodic button scan mgr, this is used to simulate a usb disconnect
//!
//! @details this is called periodically to scan the buttons and
//! initiate action when pressed
//!
//! @note, Used to simulate internally USB requested power down, and internally
//! requested USB power up. On simulated USB power up the USB configuration
//! is advanced (e.g. from CDC to CDC-HID)
//!
//! @param calledFromIsrOrThread set to true when in multithreaded environment
//!                              or when calling from ISR
//
//*****************************************************************************
extern void usb_disconnect_periodic(bool calledFromIsrOrThread);

#endif //USB_DISCONNECT_EXAMP_H
