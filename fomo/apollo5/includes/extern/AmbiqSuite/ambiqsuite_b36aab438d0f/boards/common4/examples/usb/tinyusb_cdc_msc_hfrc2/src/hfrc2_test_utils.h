//*****************************************************************************
//
//! @file hfrc2_test.h
//!
//! @brief tinyusb hfrc2 utilities
//!
//! @brief contains the common board init function
//!
//! @addtogroup usb_examples USB Examples
//!
//! @defgroup tinyusb_cdc_msc_hfrc2 TinyUSB CDC MSC HFRC2 Example
//! @ingroup usb_examples
//! @{
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef HFRC2_TEST_H
#define HFRC2_TEST_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

// ****************************************************************************
//
//! @brief enable HFRC2 24Mhz on output pin
//!
//! @param eClockOut - take value from enum: am_hal_clkgen_clkout_e
//! @param pinNumber - GPIO pin number
//!
//! @return standard hal status, success if pin was configured.
//
// ****************************************************************************
uint32_t hfrc2_enable_hfrc2_clock_out(am_hal_clkgen_clkout_e eClockOut, uint32_t pinNumber) ;

#endif //HFRC2_TEST_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

