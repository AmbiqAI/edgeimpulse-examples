//*****************************************************************************
//
//! @file am_util_math.c
//!
//! @brief A few useful math functions.
//!
//! Functions for performing some number manipulation.
//!
//! @addtogroup math Math Functions
//! @ingroup utils
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
#include <stdint.h>
#include <stdbool.h>
#include "am_util_math.h"

//*****************************************************************************
//
// Converts a Binary Coded Decimal (BCD) byte to its Decimal form.
//
//*****************************************************************************
uint8_t
am_util_math_bcd_to_dec(uint8_t ui8BCDByte)
{
  return (((ui8BCDByte & 0xF0) >> 4) * 10) + (ui8BCDByte & 0x0F);
}

//*****************************************************************************
//
//Converts a Decimal byte to its Binary Coded Decimal (BCD) form.
//
//*****************************************************************************
uint8_t
am_util_math_dec_to_bcd(uint8_t ui8DecimalByte)
{
  return (((ui8DecimalByte / 10) << 4) | (ui8DecimalByte % 10));
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

