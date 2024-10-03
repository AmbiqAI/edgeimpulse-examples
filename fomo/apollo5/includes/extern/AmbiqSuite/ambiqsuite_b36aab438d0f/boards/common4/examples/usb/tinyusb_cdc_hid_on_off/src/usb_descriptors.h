//*****************************************************************************
//
//! @file tinyusb_config.h
//!
//! @brief contains prototypes for functions added to usb_descriptors_cdc_hid.h
//!
//!
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "tusb.h"

typedef enum
{
    e_PROFILE_CDC,
    e_PROFILE_COMPOSITE_CDC,
    e_PROFILE_COMPOSITE_CDC_HID,
    e_PROFILE_MAX,
    e_PROFILE_x32 = 0x40000000,  // force size / sizeof to be 4 bytes
}
usb_profile_e;

//*****************************************************************************
//
//! @brief print the profile name that is in use
//!
//! @return profile enum
//
//*****************************************************************************
usb_profile_e usb_desc_print_profile_name(void);
//*****************************************************************************
//
//! @brief set profile to supplied enum
//!
//! @param eProfile
//
//*****************************************************************************
void usb_desc_set_profile(usb_profile_e eProfile);
//*****************************************************************************
//
//! @brief set profile to the next in line (in a circular fashion)
//!
//! @return enum for new profile
//
//*****************************************************************************
usb_profile_e usb_desc_set_next_profile(void);


#endif //USB_DESCRIPTORS_H
