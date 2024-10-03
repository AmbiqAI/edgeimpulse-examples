//*****************************************************************************
//
//! @file usb_test_common.h
//!
//! @brief USB test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef USB_TEST_COMMON_H
#define USB_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

extern void          *pWidget[AM_REG_USB_NUM_MODULES];
extern char          errStr[128];

#endif // USB_TEST_COMMON_H