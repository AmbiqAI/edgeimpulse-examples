//*****************************************************************************
//
//! @file am_util_time.c
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
#include <stdint.h>
#include <stdbool.h>
#include "am_util_time.h"

//*****************************************************************************
//
//! @brief Calculate whether passed in year is a leap year
//!
//! @param year - year to check for leap year
//
//*****************************************************************************
#define AM_UTIL_TIME_IS_LEAP_YEAR(year) \
    (year % 4 == 0 && ((year % 100 != 0) || (year % 400 != 0)))

//*****************************************************************************
//
// Local variables.
//
//*****************************************************************************

//*****************************************************************************
//
//! Numer of days in each month in a standard year.
//
//*****************************************************************************
const static uint32_t g_iDaysPerMonth[] =
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//*****************************************************************************
//
//! Weekday drift numbers for each month.
//
//*****************************************************************************
const static int g_iMonthOffsets[] =
    {4, 0, 0, 3, 5, 1, 3, 6, 2, 4, 0, 2};

//*****************************************************************************
//
// Compute the day of the week given the month, day, and year.
//
//*****************************************************************************
int
am_util_time_computeDayofWeek(int iYear, int iMonth, int iDay)
{
    bool bInvalidDay;
    int iYearOffset;
    int iMonthOffset;
    int iWeekday;

    int iLeapYearOffset = 0;

    //
    // Validate inputs.  Return 7 if any are out-of-bounds.
    //
    if ( (iMonth < 1) || (iMonth > 12) || (iYear < 0) || (iDay < 1) )
    {
        return 7;
    }

    //
    // Make sure this day actually exists in this month. Make sure to include
    // an exception for leap years.
    //
    if (iDay > g_iDaysPerMonth[iMonth - 1])
    {
        if (iMonth == 2 && AM_UTIL_TIME_IS_LEAP_YEAR(iYear) && iDay == 29)
        {
            bInvalidDay = false;
        }
        else
        {
            bInvalidDay = true;
        }
    }
    else
    {
        bInvalidDay = false;
    }

    if (bInvalidDay)
    {
        return 7;
    }

    iYearOffset = 2 + iYear + iYear / 4 - iYear / 100 + iYear / 400;
    iMonthOffset = g_iMonthOffsets[iMonth - 1];

    if (AM_UTIL_TIME_IS_LEAP_YEAR(iYear) && (iMonth < 3))
    {
        iLeapYearOffset = -1;
    }

    iWeekday = iDay + iYearOffset + iMonthOffset + iLeapYearOffset;

    return iWeekday % 7;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

