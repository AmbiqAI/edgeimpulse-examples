//*****************************************************************************
//
//! @file am_util_delay.h
//!
//! @brief A few useful delay functions.
//!
//! Functions for fixed delays.
//!
//! @addtogroup delay Delay Functionality
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
#ifndef AM_UTIL_DELAY_H
#define AM_UTIL_DELAY_H

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
//! @brief Delays for a desired amount of loops.
//!
//! This function will delay for a number of cycle loops.
//!
//! @note - the number of cycles each loops takes to execute is approximately 3.
//! Therefore the actual number of cycles executed will be ~3x ui32Iterations.
//!
//! For example, a ui32Iterations value of 100 will delay for 300 cycles.
//!
//! @param ui32Iterations - Desired number of cycle loops to delay for.
//
//*****************************************************************************
extern void am_util_delay_cycles(uint32_t ui32Iterations);

//*****************************************************************************
//
//! @brief Delays for a desired amount of milliseconds.
//!
//! This function will delay for a number of milliseconds.
//!
//! @param ui32MilliSeconds - number of milliseconds to delay for.
//
//*****************************************************************************
extern void am_util_delay_ms(uint32_t ui32MilliSeconds);

//*****************************************************************************
//
//! @brief Delays for a desired amount of microseconds.
//!
//! This function will delay for a number of microseconds.
//!
//! @param ui32MicroSeconds - number of microseconds to delay for.
//
//*****************************************************************************
extern void am_util_delay_us(uint32_t ui32MicroSeconds);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_DELAY_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

