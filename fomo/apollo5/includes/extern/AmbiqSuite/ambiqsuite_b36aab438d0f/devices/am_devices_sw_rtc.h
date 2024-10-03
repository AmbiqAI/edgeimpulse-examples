//*****************************************************************************
//
//! @file am_devices_sw_rtc.h
//!
//! @brief SW based Real Time Clock using STIMER.
//!
//! @addtogroup sw_rtc SW_RTC - SW defined RTC
//! @ingroup devices
//! @{
//
//**************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_DEVICES_SW_RTC_H
#define AM_DEVICES_SW_RTC_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions.
//! @{
//
//*****************************************************************************
#if defined(AM_PART_APOLLO4)
#define STIMER_CLK_PER_HUNDREDTH        1875
#define STIMER_CLK_PER_SECOND           187500
#define STIMER_CLK_PER_MINUTE           11250000
#define STIMER_CLK_PER_HOUR             675000000
#elif defined(AM_PART_APOLLO4B)
#define STIMER_CLK_PER_HUNDREDTH        3750
#define STIMER_CLK_PER_SECOND           375000
#define STIMER_CLK_PER_MINUTE           22500000
#define STIMER_CLK_PER_HOUR             1350000000
#endif
//! @}

//#define USE_SCMPR5_AS_OVERFLOW

//*****************************************************************************
//
//! Global type definitions.
//
//*****************************************************************************
typedef void (*am_devices_sw_rtc_callback_t)(void);

//*****************************************************************************
//
//! RTC configuration structure.
//
//*****************************************************************************
typedef struct am_devices_sw_rtc_config_t
{
    am_hal_rtc_osc_select_e eOscillator;
    bool b12Hour;
}
am_devices_sw_rtc_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the SW RTC.
//!
//! @param pConfig      - pointer to the input configuration structure.
//!
//! This function initializes the SW RTC configuration.  It does not start
//! the SW RTC.  It does check for conflicts with the current STIMER settings.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_sw_rtc_initialize(am_devices_sw_rtc_config_t *pConfig);

//*****************************************************************************
//
//! @brief Set the SW RTC Time.
//!
//! @param pTime        - pointer to the RTC time to establish.
//!
//! This function sets the current RTC time and makes sure the STIMER is running.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_sw_rtc_time_set(am_hal_rtc_time_t *pTime);

//*****************************************************************************
//
//! @brief Get the SW RTC Time.
//!
//! @param pTime        - pointer to the RTC time to return.
//!
//! This function calculates and returns the current RTC time.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_sw_rtc_time_get(am_hal_rtc_time_t *pTime);

//*****************************************************************************
//
//! @brief Set an SW RTC alarm.
//!
//! @param pTime        - pointer to the RTC time for the alarm callback.
//! @param pCallback    - pointer to the function to call.
//!
//! This function sets the alarm and stores the callback.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_sw_rtc_alarm_set(am_hal_rtc_time_t *pTime,
                                            am_devices_sw_rtc_callback_t pCallback);

//*****************************************************************************
//
//! @brief Service the SW RTC Interrupt functions (STIMER)
//!
//! @param ui32STimerInts - pointer to the RTC time for the alarm callback.
//!
//! This function services the STIMER interrupts that may pertain to the SW RTC.
//
//*****************************************************************************
extern void am_devices_sw_rtc_interrupt_service(uint32_t ui32STimerInts);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_N25Q256A_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

