//*****************************************************************************
//
//! @file am_util_time.h
//!
//! @brief Functions useful for RTC, calendar, time, etc. computations.
//!
//! @addtogroup time Time - RTC Time Computations
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
#ifndef AM_UTIL_TIME_H
#define AM_UTIL_TIME_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief Compute the day of the week given the month, day, and year.
//!
//! @param iYear  - The year of the desired date (e.g. 2016).
//! @param iMonth - The month of the desired date (1-12).
//! @param iDay   - The day of the month of the desired date (1-31).
//!
//! This function is general in nature, but is designed to be used with the RTC.
//!
//! @returns An index value indicating the day of the week.
//! 0-6 indicate  Sun, Mon, Tue, Wed, Thu, Fri, Sat, respectively.
//! 7   indicates that the given date is invalid (e.g. 2/29/2015).
//
//*****************************************************************************
extern int am_util_time_computeDayofWeek(int iYear, int iMonth, int iDay);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_TIME_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

