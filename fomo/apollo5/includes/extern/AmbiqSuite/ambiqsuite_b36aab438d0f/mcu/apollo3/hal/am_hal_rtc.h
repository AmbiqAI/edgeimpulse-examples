//*****************************************************************************
//
//! @file am_hal_rtc.h
//!
//! @brief Functions for interfacing and accessing the Real-Time Clock (RTC).
//!
//! @addtogroup rtc3 RTC - Real-Time Clock
//! @ingroup apollo3_hal
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
#ifndef AM_HAL_RTC_H
#define AM_HAL_RTC_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name OSC Start and Stop
//! @brief OSC Start and Stop defines.
//!
//! OSC Start and Stop defines to be used with \e am_hal_clkgen_osc_x().
//! @{
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
#if 0
//
// CSV-349, https://ambiqmicro.atlassian.net/browse/CSV-349
// As of 6/18/19, the validation team discovered that this switch of the
// RTC to LFRC would lock up some A1 and B0 parts. Until the problem
// is root-caused and a solid workaround determined, we must skip it.
// We'll leave it defined for internal use.
//
#define AM_HAL_RTC_OSC_LFRC          0x1
#endif
// #### INTERNAL END ####
#define AM_HAL_RTC_OSC_XT            0x0
//! @}

//*****************************************************************************
//
//! @name RTC Interrupts
//! @brief Macro definitions for RTC interrupt status bits.
//!
//! These macros correspond to the bits in the RTC interrupt status register.
//! They may be used with any of the \e am_hal_rtc_int_x() functions.
//!
//! @{
//
//*****************************************************************************
#define AM_HAL_RTC_INT_ALM                  RTC_INTEN_ALM_Msk
//! @}

//*****************************************************************************
//
//! @name RTC Alarm Repeat Interval.
//! @brief Macro definitions for the RTC alarm repeat interval.
//!
//! These macros correspond to the RPT bits in the RTCCTL register.
//! They may be used with the \e am_hal_rtc_alarm_interval_set() function.
//!
//! Note: AM_HAL_RTC_ALM_RPT_10TH and AM_HAL_RTC_ALM_RPT_100TH do not
//! correspond to the RPT bits but are used in conjunction with setting the
//! ALM100 bits in the ALMLOW register.
//!
//! @{
//
//*****************************************************************************
#define AM_HAL_RTC_ALM_RPT_DIS              0x0
#define AM_HAL_RTC_ALM_RPT_YR               0x1
#define AM_HAL_RTC_ALM_RPT_MTH              0x2
#define AM_HAL_RTC_ALM_RPT_WK               0x3
#define AM_HAL_RTC_ALM_RPT_DAY              0x4
#define AM_HAL_RTC_ALM_RPT_HR               0x5
#define AM_HAL_RTC_ALM_RPT_MIN              0x6
#define AM_HAL_RTC_ALM_RPT_SEC              0x7
#define AM_HAL_RTC_ALM_RPT_10TH             0x8
#define AM_HAL_RTC_ALM_RPT_100TH            0x9
//! @}

//*****************************************************************************
//
//! @name RTC Alarm 100 Interval.
//! @brief Macro definitions for the RTC alarm ms intervals.
//!
//! These macros are used inside the #am_hal_rtc_alarm_interval_set function
//! when 10ms and 100ms repeated alarm intervals are desired.
//!
//! @{
//
//*****************************************************************************
#define AM_HAL_RTC_ALM100_DEFAULT           0x00
#define AM_HAL_RTC_ALM100_10TH              0xF0
#define AM_HAL_RTC_ALM100_100TH             0xFF
//! @}

//*****************************************************************************
//
//! @brief The basic time structure used by the HAL for RTC interaction.
//!
//! All values are positive whole numbers. The HAL routines convert back and
//! forth to BCD.
//
//*****************************************************************************
typedef struct am_hal_rtc_time_struct
{
    uint32_t ui32ReadError;
    uint32_t ui32CenturyEnable;
    uint32_t ui32Weekday;
    uint32_t ui32Century;
    uint32_t ui32Year;
    uint32_t ui32Month;
    uint32_t ui32DayOfMonth;
    uint32_t ui32Hour;
    uint32_t ui32Minute;
    uint32_t ui32Second;
    uint32_t ui32Hundredths;
}am_hal_rtc_time_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Selects the clock source for the RTC.
//!
//! This function selects the clock source for the RTC.
//!
//! @param ui32OSC The clock source for the RTC.\n
//! Valid values for ui32OSC are:
//!
// #### INTERNAL BEGIN ####
//!     - AM_HAL_RTC_OSC_LFRC
// #### INTERNAL END ####
//!     - AM_HAL_RTC_OSC_XT
//!
//!
//! @note After selection of the RTC oscillator, a 2 second delay occurs before
//! the new setting is reflected in status. Therefore the CLKGEN.STATUS.OMODE
//! bit will not reflect the new status until after the 2s wait period.
//!
//
//*****************************************************************************
extern void am_hal_rtc_osc_select(uint32_t ui32OSC);

//*****************************************************************************
//
//! @brief Enable/Start the RTC oscillator.
//!
//! Starts the RTC oscillator.
//
//*****************************************************************************
extern void am_hal_rtc_osc_enable(void);

//*****************************************************************************
//
//! @brief Disable/Stop the RTC oscillator.
//!
//! Stops the RTC oscillator.
//
//*****************************************************************************
extern void am_hal_rtc_osc_disable(void);

//*****************************************************************************
//
//! @brief Configures the RTC for 12 or 24 hour time keeping.
//!
//! Configures the RTC for 12 (true) or 24 (false) hour time keeping.
//!
//! @param b12Hour - A 'true' configures the RTC for 12 hour time keeping.
//
//*****************************************************************************
extern void am_hal_rtc_time_12hour(bool b12Hour);

//*****************************************************************************
//
//! @brief Set the Real Time Clock counter registers.
//!
//! Sets the RTC counter registers to the supplied values.
//!
//! @param *pTime - A pointer to the time structure.
//
//*****************************************************************************
extern uint32_t am_hal_rtc_time_set(am_hal_rtc_time_t *pTime);

//*****************************************************************************
//
//! @brief Get the Real Time Clock current time.
//!
//! Gets the RTC's current time
//!
//! @param *pTime - A pointer to the time structure to store the current time.
//!
//! @return 0 for success and 1 for error.
//
//*****************************************************************************
extern uint32_t am_hal_rtc_time_get(am_hal_rtc_time_t *pTime);

//*****************************************************************************
//
//! @brief Sets the alarm repeat interval.
//!
//! Sets the alarm repeat interval.
//!
//! @param ui32RepeatInterval
//! @parblock
//! the desired repeat interval.
//!
//! Valid values for ui32RepeatInterval:
//!
//!     - AM_HAL_RTC_ALM_RPT_DIS
//!     - AM_HAL_RTC_ALM_RPT_YR
//!     - AM_HAL_RTC_ALM_RPT_MTH
//!     - AM_HAL_RTC_ALM_RPT_WK
//!     - AM_HAL_RTC_ALM_RPT_DAY
//!     - AM_HAL_RTC_ALM_RPT_HR
//!     - AM_HAL_RTC_ALM_RPT_MIN
//!     - AM_HAL_RTC_ALM_RPT_SEC
//!     - AM_HAL_RTC_ALM_RPT_10TH
//!     - AM_HAL_RTC_ALM_RPT_100TH
//! @endparblock
//
//*****************************************************************************
extern void am_hal_rtc_alarm_interval_set(uint32_t ui32RepeatInterval);

//*****************************************************************************
//
//! @brief Sets the RTC's Alarm.
//!
//! Set the Real Time Clock Alarm Parameters.
//!
//! @param *pTime - A pointer to the time structure.
//! @param ui32RepeatInterval
//! @parblock
//! the desired alarm repeat interval.\n
//! Valid values for ui32RepeatInterval:
//!
//!     - AM_HAL_RTC_ALM_RPT_DIS
//!     - AM_HAL_RTC_ALM_RPT_YR
//!     - AM_HAL_RTC_ALM_RPT_MTH
//!     - AM_HAL_RTC_ALM_RPT_WK
//!     - AM_HAL_RTC_ALM_RPT_DAY
//!     - AM_HAL_RTC_ALM_RPT_HR
//!     - AM_HAL_RTC_ALM_RPT_MIN
//!     - AM_HAL_RTC_ALM_RPT_SEC
//!     - AM_HAL_RTC_ALM_RPT_10TH
//!     - AM_HAL_RTC_ALM_RPT_EVERY_100TH
//! @endparblock
//
//*****************************************************************************
extern void am_hal_rtc_alarm_set(am_hal_rtc_time_t *pTime,
                                 uint32_t ui32RepeatInterval);

//*****************************************************************************
//
//! @brief Get the Real Time Clock Alarm Parameters
//! @note Gets the RTC's Alarm time
//!
//! @param *pTime - A pointer to the time structure to store the current alarm.
//
//*****************************************************************************
extern void am_hal_rtc_alarm_get(am_hal_rtc_time_t *pTime);

//*****************************************************************************
//
//! @brief Enable selected RTC interrupts.
//!
//! Enables the RTC interrupts.
//!
//! @param ui32Interrupt desired interrupts
//! @parblock
//!
//! ui32Interrupt should be the following:
//!
//!     AM_HAL_RTC_INT_ALM
//! @endparblock
//
//*****************************************************************************
extern void am_hal_rtc_int_enable(uint32_t ui32Interrupt);

//*****************************************************************************
//
//! @brief Return the enabled RTC interrupts.
//!
//! @return enabled RTC interrupts. Return is 0 or AM_HAL_RTC_INT_ALM
//
//*****************************************************************************
extern uint32_t am_hal_rtc_int_enable_get(void);

//*****************************************************************************
//
//! @brief Disable selected RTC interrupts.
//!
//! Disables RTC interrupts.
//!
//! @param ui32Interrupt
//! @parblock
//! desired interrupts
//!
//! ui32Interrupt should be the following:
//!
//!     AM_HAL_RTC_INT_ALM
//! @endparblock
//
//*****************************************************************************
extern void am_hal_rtc_int_disable(uint32_t ui32Interrupt);

//*****************************************************************************
//
//! @brief Clear selected RTC interrupts.
//!
//! Clears the RTC interrupts.
//!
//! @param ui32Interrupt
//! @parblock
//! desired interrupts
//!
//! ui32Interrupt should be the following:
//!
//!     AM_HAL_RTC_INT_ALM
//! @endparblock
//
//*****************************************************************************
extern void am_hal_rtc_int_clear(uint32_t ui32Interrupt);

//*****************************************************************************
//
//! @brief Sets the selected RTC interrupts.
//!
//! Sets the RTC interrupts causing them to immediately trigger.
//!
//! @param ui32Interrupt
//! @parblock
//! desired interrupts
//!
//! ui32Interrupt should be the following:
//!
//!     AM_HAL_RTC_INT_ALM
//! @endparblock
//
//*****************************************************************************
extern void am_hal_rtc_int_set(uint32_t ui32Interrupt);

//*****************************************************************************
//
//! @brief Returns the RTC interrupt status.
//!
//! Returns the RTC interrupt status.
//!
//! @param bEnabledOnly - return the status of only the enabled interrupts.
//!
//! @return Bitwise representation of the current interrupt status.
//! The return value will be 0 or the following:
//!    - AM_HAL_RTC_INT_ALM
//
//*****************************************************************************
extern uint32_t am_hal_rtc_int_status_get(bool bEnabledOnly);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_RTC_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
