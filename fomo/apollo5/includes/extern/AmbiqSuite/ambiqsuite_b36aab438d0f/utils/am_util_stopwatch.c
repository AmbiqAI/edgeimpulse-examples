//*****************************************************************************
//
//! @file am_util_stopwatch.c
//!
//! @brief Provides functionality to measure elapsed time.
//!
//! Functions for measuring elapsed time. These can be useful for providing
//! 'ticks' where needed.
//!
//! @note These functions require a RTC to function properly. Therefore, if any
//! RTC configuring takes place after calling am_util_stopwatch_start() the
//! resulting elapsed time will be incorrect unless you first call
//! am_util_stopwatch_restart()
//!
//! @addtogroup stopwatch Stopwatch - Measure Elapsed Time
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
#include <stdarg.h>
#include "am_util_stopwatch.h"

//*****************************************************************************
//
// Format time based on resolution.
//
//*****************************************************************************
static uint32_t
time_format(uint64_t ui64TimeMS, uint32_t ui32Resolution)
{
    switch (ui32Resolution)
    {
        case AM_UTIL_STOPWATCH_MS:
            return ui64TimeMS;
        case AM_UTIL_STOPWATCH_SECOND:
            return ui64TimeMS / 1000;
        case AM_UTIL_STOPWATCH_MINUTE:
            return ui64TimeMS / 60000;
        case AM_UTIL_STOPWATCH_HOUR:
            return ui64TimeMS / 3600000;
        case AM_UTIL_STOPWATCH_DAY:
            return ui64TimeMS / 86400000;
        case AM_UTIL_STOPWATCH_MONTH:
             return ui64TimeMS / (uint64_t) 2592000000;
        case AM_UTIL_STOPWATCH_YEAR:
             return ui64TimeMS / (uint64_t) 31536000000;
        default:
            return ui64TimeMS;
    }
}

//*****************************************************************************
//
// Return the absolute time in milliseconds from a RTC structure.
//
//*****************************************************************************
static uint64_t
elapsed_time_ms(am_hal_rtc_time_t *psStartTime, am_hal_rtc_time_t *psStopTime)
{
    int64_t i64DeltaYear = 0;
    int64_t i64DelataMonth = 0;
    int64_t i64DeltaDay = 0;
    int64_t i64DelatHour = 0;
    int64_t i64DeltaMinute = 0;
    int64_t i64DeltaSecond = 0;
    int64_t i64DeltaHundredths = 0;
    uint64_t ui64DeltaTotal = 0;

    i64DeltaYear = (psStopTime->ui32Year - psStartTime->ui32Year) * (uint64_t) 31536000000;
    i64DelataMonth = (psStopTime->ui32Month - psStartTime->ui32Month) * (uint64_t) 2592000000;
    i64DeltaDay = (psStopTime->ui32DayOfMonth - psStartTime->ui32DayOfMonth) * (uint64_t) 86400000;
    i64DelatHour = (psStopTime->ui32Hour - psStartTime->ui32Hour) * (uint64_t) 3600000;
    i64DeltaMinute = (psStopTime->ui32Minute - psStartTime->ui32Minute) * (uint64_t) 60000;
    i64DeltaSecond = (psStopTime->ui32Second - psStartTime->ui32Second) * (uint64_t) 1000;
    i64DeltaHundredths = (psStopTime->ui32Hundredths - psStartTime->ui32Hundredths) * (uint64_t) 10;

    ui64DeltaTotal = (i64DeltaYear + i64DelataMonth + i64DeltaDay + i64DelatHour +
                      i64DeltaMinute + i64DeltaSecond + i64DeltaHundredths);

    return ui64DeltaTotal;
}

//*****************************************************************************
//
// Start the stopwatch.
//
//*****************************************************************************
void
am_util_stopwatch_init(am_util_stopwatch_t *pStopwatch)
{
    //
    // Initialize everything.
    //
    pStopwatch->ui64ElapsedTime = 0;
    pStopwatch->ui64PausedTime = 0;
    pStopwatch->bStarted = false;
    pStopwatch->bPaused = false;
}

//*****************************************************************************
//
// Start the stopwatch.
//
//*****************************************************************************
void
am_util_stopwatch_start(am_util_stopwatch_t *pStopwatch)
{
    am_hal_rtc_time_t rtc_time;

    //
    // If the start time is clear, read the RTC time to get a reference starting
    // time.
    if ( pStopwatch->bPaused == false && pStopwatch->bStarted == false )
    {
        //
        // Clear the timer which gets the current time as well.
        //
        am_util_stopwatch_clear(pStopwatch);
    }

    //
    // We were paused.
    // Now we need to figure out how long we were paused for.
    //
    else if ( pStopwatch->bPaused == true && pStopwatch->bStarted == true )
    {
        //
        // Get the RTC time.
        //
        while ( am_hal_rtc_time_get(&rtc_time) );

        //
        // Add the time we spent paused to the time we already spent paused.
        //
        pStopwatch->ui64PausedTime += elapsed_time_ms(&pStopwatch->sPauseTime, &rtc_time);
    }

    //
    // Set started to true.
    //
    pStopwatch->bStarted = true;

    //
    // Set paused to false.
    //
    pStopwatch->bPaused = false;
}

//*****************************************************************************
//
// Stop the stopwatch.
//
//*****************************************************************************
void
am_util_stopwatch_stop(am_util_stopwatch_t *pStopwatch)
{
    //
    // Save the current time so we know how long we've been paused for.
    //
    while ( am_hal_rtc_time_get(&pStopwatch->sPauseTime) );

    //
    // Set the state to paused.
    //
    pStopwatch->bPaused = true;
}

//*****************************************************************************
//
// Clears the stopwatch.
//
//*****************************************************************************
void
am_util_stopwatch_clear(am_util_stopwatch_t *pStopwatch)
{
    //
    // Read the RTC and save in pStopwatch->sStartTime.
    //
    while ( am_hal_rtc_time_get(&pStopwatch->sStartTime) );

    //
    // Reset the paused time.
    //
    pStopwatch->ui64PausedTime = 0;

    //
    // Reset the elapsed time.
    //
    pStopwatch->ui64ElapsedTime = 0;
}

//*****************************************************************************
//
// Restart the stopwatch.
//
//*****************************************************************************
void
am_util_stopwatch_restart(am_util_stopwatch_t *pStopwatch)
{
    //
    // Clear the stopwatch.
    //
    am_util_stopwatch_clear(pStopwatch);

    //
    // Start the stopwatch.
    //
    am_util_stopwatch_start(pStopwatch);
}

//*****************************************************************************
//
// Get the elapsed time from the stopwatch.
//
//*****************************************************************************
uint64_t
am_util_stopwatch_elapsed_get(am_util_stopwatch_t *pStopwatch, uint32_t ui32Resolution)
{
    am_hal_rtc_time_t rtc_time;

    //
    // Stop watch is not paused and is running.
    // Figure out elapsed time.
    //
    if ( pStopwatch->bPaused == false && pStopwatch->bStarted == true )
    {
        //
        // Get the RTC time.
        //
        while ( am_hal_rtc_time_get(&rtc_time) );

        pStopwatch->ui64ElapsedTime = elapsed_time_ms(&pStopwatch->sStartTime, &rtc_time) -
                                pStopwatch->ui64PausedTime;
    }

    //
    // Return the elapsed time.
    //
    return time_format(pStopwatch->ui64ElapsedTime, ui32Resolution);
}

//*****************************************************************************
//
// Get and format the elapsed time from the stopwatch.
//
//*****************************************************************************
 void
 am_util_stopwatch_elapsed_get_formatted(am_util_stopwatch_t *pStopwatch,
                                         am_util_stopwatch_elapsed_t *psTime)
 {
    uint64_t ui64MS;

    //
    // Get the elapsed time in MS.
    //
    ui64MS = am_util_stopwatch_elapsed_get(pStopwatch, AM_UTIL_STOPWATCH_MS);

    //
    // Zero out the structure.
    //
    psTime->ui32Year = 0;
    psTime->ui32Month = 0;
    psTime->ui32Day = 0;
    psTime->ui32Hour = 0;
    psTime->ui32Minute = 0;
    psTime->ui32Second = 0;
    psTime->ui32MS = 0;

    //
    // Years.
    //
    if ( ui64MS >= 31536000000 )
    {
        //
        // Fill in the structure.
        //
        psTime->ui32Year = (ui64MS / 31536000000);

        //
        // Subtract from ui64MS.
        //
        ui64MS -= (psTime->ui32Year * 31536000000);
    }

    //
    // Months.
    //
    if ( ui64MS >= 2592000000 )
    {
        //
        // Fill in the structure.
        //
        psTime->ui32Month = (ui64MS / 2592000000);

        //
        // Subtract from ui64MS.
        //
        ui64MS -= (psTime->ui32Month * 2592000000);
    }

    //
    // Days.
    //
    if ( ui64MS >= 86400000 )
    {
        //
        // Fill in the structure.
        //
        psTime->ui32Day = (ui64MS / 86400000);

        //
        // Subtract from ui64MS.
        //
        ui64MS -= (psTime->ui32Day * 86400000);
    }

    //
    // Hours.
    //
    if ( ui64MS >= 3600000 )
    {
        //
        // Fill in the structure.
        //
        psTime->ui32Hour = (ui64MS / 3600000);

        //
        // Subtract from ui64MS.
        //
        ui64MS -= (psTime->ui32Hour * 3600000);
    }

    //
    // Minutes.
    //
    if ( ui64MS >= 60000 )
    {
        //
        // Fill in the structure.
        //
        psTime->ui32Minute = (ui64MS / 60000);

        //
        // Subtract from ui64MS.
        //
        ui64MS -= (psTime->ui32Minute * 60000);
    }

    //
    // Seconds.
    //
    if ( ui64MS >= 1000 )
    {
        //
        // Fill in the structure.
        //
        psTime->ui32Second = (ui64MS / 1000);

        //
        // Subtract from ui64MS.
        //
        ui64MS -= (psTime->ui32Second * 1000);
    }

    //
    // Milliseconds.
    //

    //
    // Fill in the structure.
    //
    psTime->ui32MS = ui64MS;
 }

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

