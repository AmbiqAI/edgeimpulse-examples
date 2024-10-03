//*****************************************************************************
//
//! @file am_util_stopwatch.h
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
#ifndef AM_UTIL_STOPWATCH_H
#define AM_UTIL_STOPWATCH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_mcu_apollo.h"
// #### INTERNAL BEGIN ####
#if 0 // Old way
#include "hal/am_hal_rtc.h"
#endif
// #### INTERNAL END ####

//*****************************************************************************
//
//! A data structure for holding the formatted time.
//
//*****************************************************************************
typedef struct am_util_stopwatch_elapsed_t
{
    uint32_t ui32MS;
    uint32_t ui32Second;
    uint32_t ui32Minute;
    uint32_t ui32Hour;
    uint32_t ui32Day;
    uint32_t ui32Month;
    uint32_t ui32Year;
} am_util_stopwatch_elapsed_t;

//*****************************************************************************
//
//! A data structure for tracking the stopwatch state.
//
//*****************************************************************************
typedef struct am_util_stopwatch_t
{
    uint64_t ui64ElapsedTime;          //! Total elapsed time in ms.
    uint64_t ui64PausedTime;           //! Total paused time in ms.
    bool bStarted;                     //! Stopwatch started state.
    bool bPaused;                      //! Stopwatch paused state.
    am_hal_rtc_time_t sStartTime;      //! Start time to determine elapsed time.
    am_hal_rtc_time_t sPauseTime;      // Pause time to determine elapsed time.
} am_util_stopwatch_t;

//*****************************************************************************
//
//! @name Resolution for Elapsed Time
//! @brief Defines to pass to am_util_stopwatch_elapsed_get()
//!
//! These macros should be used to specify what resolution to return the
//! elapsed time in.
//! @{
//
//*****************************************************************************
#define AM_UTIL_STOPWATCH_MS            0x0
#define AM_UTIL_STOPWATCH_SECOND        0x1
#define AM_UTIL_STOPWATCH_MINUTE        0x2
#define AM_UTIL_STOPWATCH_HOUR          0x4
#define AM_UTIL_STOPWATCH_DAY           0x8
#define AM_UTIL_STOPWATCH_MONTH         0x10
#define AM_UTIL_STOPWATCH_YEAR          0x20
//! @}

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Start the stopwatch.
//!
//! This function records the current time from the RTC and sets the start time.
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//
//*****************************************************************************
extern void am_util_stopwatch_init(am_util_stopwatch_t *pStopwatch);

//*****************************************************************************
//
//! @brief Start the stopwatch.
//!
//! This function records the current time from the RTC and sets the start time.
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//
//*****************************************************************************
extern void am_util_stopwatch_start(am_util_stopwatch_t *pStopwatch);

//*****************************************************************************
//
//! @brief Stop the stopwatch.
//!
//! This function stops the stop watch and anytime am_util_stopwatch_elapsed_get()
//! is called it will return the same elapsed time until am_util_stopwatch_start()
//! is called again.
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//
//*****************************************************************************
extern void am_util_stopwatch_stop(am_util_stopwatch_t *pStopwatch);

//*****************************************************************************
//
//! @brief Restart the stopwatch.
//!
//! This function restarts the stopwatch.
//!
//! If the stopwatch was previously stopped this is functionally equivalent
//! calling am_util_stopwatch_clear() followed by am_util_stopwatch_start().
//!
//! If the stopwatch was previously started this is functionally equivalent to
//! am_util_stopwatch_clear().
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//
//*****************************************************************************
extern void am_util_stopwatch_restart(am_util_stopwatch_t *pStopwatch);

//*****************************************************************************
//
//! @brief Clears the stopwatch.
//!
//! This function clears the start time on the stop watch. If the stop watch is
//! running, it will continue to count the elapsed time from the new start time.
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//
//*****************************************************************************
extern void am_util_stopwatch_clear(am_util_stopwatch_t *pStopwatch);

//*****************************************************************************
//
//! @brief Get the elapsed time from the stopwatch.
//!
//! This function returns the elapsed time in the desired resolution as requested
//! from ui32Resolution.
//!
//! Valid values for ui32Resolution:
//!     AM_UTIL_STOPWATCH_MS
//!     AM_UTIL_STOPWATCH_SEC
//!     AM_UTIL_STOPWATCH_MIN
//!     AM_UTIL_STOPWATCH_HOUR
//!     AM_UTIL_STOPWATCH_DAY
//!     AM_UTIL_STOPWATCH_MONTH
//!     AM_UTIL_STOPWATCH_YEAR
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//! @param ui32Resolution - the desired resolution to return the elapsed time in.
//!
//! @return Elapsed Time in ui32Resolution.
//
//*****************************************************************************
extern uint64_t am_util_stopwatch_elapsed_get(am_util_stopwatch_t *pStopwatch,
                                              uint32_t ui32Resolution);

//*****************************************************************************
//
//! @brief Get and format the elapsed time from the stopwatch.
//!
//! This function returns the fills in the am_util_stopwatch_elapsed_t structure
//! with "human readable" elapsed time.
//!
//! @param pStopwatch - the pointer to the am_util_stopwatch_t structure.
//! @param pTime - the pointer to the am_util_stopwatch_elapsed_t structure.
//
//*****************************************************************************
extern void am_util_stopwatch_elapsed_get_formatted(am_util_stopwatch_t *pStopwatch,
                                                    am_util_stopwatch_elapsed_t *pTime);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_STOPWATCH_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

