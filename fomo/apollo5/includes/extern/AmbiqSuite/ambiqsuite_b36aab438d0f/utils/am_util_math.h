//*****************************************************************************
//
//! @file am_util_math.h
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
#ifndef AM_UTIL_MATH_H
#define AM_UTIL_MATH_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Converts a Decimal byte to its Binary Coded Decimal (BCD) form.
//!
//! @param ui8DecimalByte is a Decimal byte.
//!
//! This function is useful when working with RTC's as they tend to be in BCD
//! format.
//!
//! @returns the Decimal byte converted to BCD.
//
//*****************************************************************************
extern uint8_t am_util_math_dec_to_bcd(uint8_t ui8DecimalByte);

//*****************************************************************************
//
//! @brief Converts a Binary Coded Decimal (BCD) byte to its Decimal form.
//!
//! @param ui8BCDByte is a BCD byte.
//!
//! This function is useful when working with RTC's as they tend to be in BCD
//! format.
//!
//! @returns the BCD byte converted to Decimal
//
//*****************************************************************************
extern uint8_t am_util_math_bcd_to_dec(uint8_t ui8BCDByte);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_MATH_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

