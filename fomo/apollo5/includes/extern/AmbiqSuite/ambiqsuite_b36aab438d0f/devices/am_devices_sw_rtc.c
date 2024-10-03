//*****************************************************************************
//
//! @file am_devices_sw_rtc.c
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

#include "am_mcu_apollo.h"
#include "am_devices_sw_rtc.h"
#include "am_util_stdio.h"
#include "am_bsp.h"
#include "am_util_delay.h"
#include "am_util_time.h"

//
// STIMER Compare function definitions.  Using Compare 7/H.
// Note: These must be consistent for SW RTC to function.
//
#define STIMER_COMPARE_INSTANCE         7
#define STIMER_COMPARE_ENABLE           AM_HAL_STIMER_CFG_COMPARE_H_ENABLE
#define STIMER_COMPARE_INT              AM_HAL_STIMER_INT_COMPAREH
#define STIMER_COMPARE_NVIC             STIMER_CMPR7_IRQn

//
// GPIO Timing Enable.
//
//#define SW_RTC_GPIO_TIMING

//*****************************************************************************
//
// Type definitions.
//
//*****************************************************************************

/* ========================================  Start of section using anonymous unions  ======================================== */
#if defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language = extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
  #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
  #pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

//
// This type overlays the STIMER->SNVR0 to SNVR3 registers.
//
typedef struct
{
  __IOM uint32_t             STTMRVAL;  // Saved STTMR register value.
  union
  {
    __IOM uint32_t           LOWER;
    struct
    {
      __IOM uint32_t HUNDREDTHS : 7;    // 0..99
      __IOM uint32_t            : 1;    // Spare bits
      __IOM uint32_t SECONDS    : 6;    // 0..59
      __IOM uint32_t            : 2;    // Spare bits
      __IOM uint32_t MINUTES    : 6;    // 0..59
      __IOM uint32_t            : 2;    // Spare bits
      __IOM uint32_t HOURS      : 5;    // 0..23
            uint32_t            : 3;    // Spare bits
    } LOWER_b;
  } ;
  union
  {
    __IOM uint32_t           UPPER;
    struct
    {
      __IOM uint32_t DAY        : 5;    // 1..31
      __IOM uint32_t            : 3;    // Spare bits
      __IOM uint32_t MONTH      : 4;    // 1..12
      __IOM uint32_t            : 4;    // Spare bits
      __IOM uint32_t YEAR       : 7;    // 0..99
      __IOM uint32_t            : 1;    // Spare bits
      __IOM uint32_t CENTURY    : 1;    // 0 = 2000's; 1 = 2100's
      __IOM uint32_t            : 3;    // Spare bits
      __IOM uint32_t WEEKDAY    : 3;    // 0..6 = 0: Sun, 1: Mon ... 6: Sat
            uint32_t            : 1;    // Spare bits
    } UPPER_b;
  } ;
  union
  {
    __IOM uint32_t           STATUS;
    struct
    {
      __IOM uint32_t ALARMOVER  : 8;    // 0..255
      __IOM uint32_t ALARMREM   : 24;   // Upper 24 bits of yields minor error in callback.
    } STATUS_b;
  } ;
} am_hal_sw_rtc_time_t;

/* =========================================  End of section using anonymous unions  ========================================= */
#if defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#endif

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
const uint8_t ui8DaysPerMonth[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},     // Non-Leap Year.
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}      // Leap Year.
};

am_devices_sw_rtc_callback_t g_pAlarmCallback;

//*****************************************************************************
//
// Local functions.
//
//*****************************************************************************
static void add_delta_to_rtc (uint32_t StimerDiff)
{
  uint32_t              ui32Accum, ui32Month, ui32Year;
  am_hal_sw_rtc_time_t *pSwRtcState = (am_hal_sw_rtc_time_t *)&(STIMER->SNVR0);
  am_hal_rtc_time_t     RelativeTime;

  //
  // Covert the StimerDiff value into a Relative Time value.
  // Maximum time value of (2^32)/375000 which is ~3 hours, 10 minutes; 54 seconds.
  //
  RelativeTime.ui32Hundredths   = StimerDiff / STIMER_CLK_PER_HUNDREDTH % 100;
  RelativeTime.ui32Second       = StimerDiff / STIMER_CLK_PER_SECOND % 60;
  RelativeTime.ui32Minute       = StimerDiff / STIMER_CLK_PER_MINUTE % 60;
  RelativeTime.ui32Hour         = StimerDiff / STIMER_CLK_PER_HOUR % 24;
  RelativeTime.ui32DayOfMonth   = 0;
  RelativeTime.ui32Month        = 0;
  RelativeTime.ui32Year         = 0;
  RelativeTime.ui32Century      = 0;
  RelativeTime.ui32Weekday      = 0;
  RelativeTime.ui32CenturyEnable = 0;
  RelativeTime.ui32ReadError    = 0;

  // Determine if it is a Leap Year.
  // Assumes that RelativeTime can never roll into a new year AND move us into month of Feb.
  bool bIsLeapYear = ((pSwRtcState->UPPER_b.YEAR % 4) == 0) &&
    (((pSwRtcState->UPPER_b.YEAR % 100) != 0) || ((pSwRtcState->UPPER_b.YEAR % 400) == 0));

  // Accumulate hundredths of a second.
  ui32Accum = RelativeTime.ui32Hundredths + pSwRtcState->LOWER_b.HUNDREDTHS;
  pSwRtcState->LOWER_b.HUNDREDTHS = ui32Accum % 100;

  // Accumulate seconds.
  ui32Accum /= 100;
  ui32Accum += RelativeTime.ui32Second + pSwRtcState->LOWER_b.SECONDS;
  pSwRtcState->LOWER_b.SECONDS = ui32Accum % 60;

  // Accumulate minutes.
  ui32Accum /= 60;
  ui32Accum += RelativeTime.ui32Minute + pSwRtcState->LOWER_b.MINUTES;
  pSwRtcState->LOWER_b.MINUTES = ui32Accum % 60;

  // Accumulate hours.
  ui32Accum /= 60;
  ui32Accum += RelativeTime.ui32Hour + pSwRtcState->LOWER_b.HOURS;
  pSwRtcState->LOWER_b.HOURS = ui32Accum % 24;

  // Accumulate days and months.
  ui32Accum /= 24;
  ui32Accum += RelativeTime.ui32DayOfMonth + pSwRtcState->UPPER_b.DAY;
  ui32Month = pSwRtcState->UPPER_b.MONTH;
  ui32Year  = 0;
  while (ui32Accum > ui8DaysPerMonth[bIsLeapYear][ui32Month-1])
  {
    ui32Accum -= ui8DaysPerMonth[bIsLeapYear][ui32Month - 1];
    ui32Month = (ui32Month + 1);
    if (ui32Month == 13)  // Wrap to January.
    {
      ui32Month = 1;
      ui32Year++;
    }
  }
  pSwRtcState->UPPER_b.DAY = ui32Accum;
  pSwRtcState->UPPER_b.MONTH = ui32Month;

  // Accumulate years.
  ui32Accum = pSwRtcState->UPPER_b.YEAR + ui32Year;
  pSwRtcState->UPPER_b.YEAR = (ui32Accum % 100);

  // Accumulate centuries.
  ui32Accum /= 100;
  pSwRtcState->UPPER_b.CENTURY += ui32Accum;

  //
  // Update Day of Week using utility.
  //
  DIAG_SUPPRESS_VOLATILE_ORDER()
  pSwRtcState->UPPER_b.WEEKDAY =
    am_util_time_computeDayofWeek(2000 + pSwRtcState->UPPER_b.YEAR,
                                  pSwRtcState->UPPER_b.MONTH,
                                  pSwRtcState->UPPER_b.DAY);
  DIAG_DEFAULT_VOLATILE_ORDER()

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

//
// Public functions.
//
//*****************************************************************************
//*****************************************************************************
//
// Initialize the SW RTC.
//
//*****************************************************************************
uint32_t
am_devices_sw_rtc_initialize(am_devices_sw_rtc_config_t *pConfig)
{
  //
  // Checks for resource conflicts and avoids overwriting STIMER
  //
  if (am_hal_stimer_is_running())
  {
    //
    // Check that the current STIMER clock selection will work for SW RTC.
    //
    uint32_t    ui32ClockSel = STIMER->STCFG_b.CLKSEL;
#if defined(AM_PART_APOLLO4)
    if (ui32ClockSel != STIMER_STCFG_CLKSEL_HFRC_187KHZ)  // Clock must be the slowest HFRC
    {
      return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#elif defined(AM_PART_APOLLO4B)
    if (ui32ClockSel != STIMER_STCFG_CLKSEL_HFRC_375KHZ)  // Clock must be the slowest HFRC
    {
      return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif
  }
  else
  {
    //
    // Clear and start STIMER if it isn't already running.
    //
    am_hal_stimer_reset_config();
#if defined(AM_PART_APOLLO4)
    am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_187KHZ |
                          AM_HAL_STIMER_CFG_THAW |
                           AM_HAL_STIMER_CFG_RUN);
#elif defined(AM_PART_APOLLO4B)
    am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_375KHZ |
                          AM_HAL_STIMER_CFG_THAW |
                           AM_HAL_STIMER_CFG_RUN);
#endif
  }

#if defined(SW_RTC_GPIO_TIMING)
  am_hal_gpio_pinconfig(0, am_hal_gpio_pincfg_output);
  am_hal_gpio_pinconfig(1, am_hal_gpio_pincfg_output);
  am_hal_gpio_pinconfig(2, am_hal_gpio_pincfg_output);
#endif

  return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Set the SW RTC current time.
//
//*****************************************************************************
uint32_t
am_devices_sw_rtc_time_set(am_hal_rtc_time_t *pTime)
{
  am_hal_sw_rtc_time_t *pSwRtcState = (am_hal_sw_rtc_time_t *)&(STIMER->SNVR0);

  //
  //Takes a snapshot of running STIMER
  //
  pSwRtcState->STTMRVAL = STIMER->STTMR;

  //
  //Store the pTime in SNVRx structure.
  //
  pSwRtcState->UPPER_b.CENTURY          = pTime->ui32Century;
  pSwRtcState->UPPER_b.YEAR             = pTime->ui32Year;
  pSwRtcState->UPPER_b.MONTH            = pTime->ui32Month;
  pSwRtcState->UPPER_b.DAY              = pTime->ui32DayOfMonth;
  pSwRtcState->UPPER_b.WEEKDAY          = pTime->ui32Weekday;
  pSwRtcState->LOWER_b.HOURS            = pTime->ui32Hour;
  pSwRtcState->LOWER_b.MINUTES          = pTime->ui32Minute;
  pSwRtcState->LOWER_b.SECONDS          = pTime->ui32Second;
  pSwRtcState->LOWER_b.HUNDREDTHS       = pTime->ui32Hundredths;

  // Enable OVERFLOW interrupt.
  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_OVERFLOW);
  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
#ifdef AM_IRQ_PRIORITY_DEFAULT
    NVIC_SetPriority(STIMER_OVF_IRQn, AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
  NVIC_EnableIRQ(STIMER_OVF_IRQn);

  am_hal_interrupt_master_enable();

  return AM_HAL_STATUS_SUCCESS;
}
//*****************************************************************************

//
// Get the SW RTC current time.
//
//*****************************************************************************
uint32_t
am_devices_sw_rtc_time_get(am_hal_rtc_time_t *pTime)
{
  am_hal_sw_rtc_time_t *pSwRtcState = (am_hal_sw_rtc_time_t *)&(STIMER->SNVR0);
  uint64_t      ui32StimerVal, ui32StimerDiff;

#if defined(SW_RTC_GPIO_TIMING)
  am_hal_gpio_output_set(0);
#endif
  //
  // Update the current SW RTC time.
  //

  // Calculate the difference between saved value and current STIMER value.
  ui32StimerVal = STIMER->STTMR;
  ui32StimerDiff = ui32StimerVal - pSwRtcState->STTMRVAL;
  pSwRtcState->STTMRVAL = ui32StimerVal;
  add_delta_to_rtc(ui32StimerDiff);

  // Return the updated time value.
  pTime->ui32Hundredths = pSwRtcState->LOWER_b.HUNDREDTHS;
  pTime->ui32Second     = pSwRtcState->LOWER_b.SECONDS;
  pTime->ui32Minute     = pSwRtcState->LOWER_b.MINUTES;
  pTime->ui32Hour       = pSwRtcState->LOWER_b.HOURS;
  pTime->ui32DayOfMonth = pSwRtcState->UPPER_b.DAY;
  pTime->ui32Month      = pSwRtcState->UPPER_b.MONTH;
  pTime->ui32Year       = pSwRtcState->UPPER_b.YEAR;
  pTime->ui32Century    = pSwRtcState->UPPER_b.CENTURY;
  pTime->ui32Weekday    = pSwRtcState->UPPER_b.WEEKDAY;

#if defined(SW_RTC_GPIO_TIMING)
  am_hal_gpio_output_clear(0);
#endif

  //Reads STIMER, calculate current time, save it, return it.
  return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Set the RTC Alarm time and callback function.
//
//*****************************************************************************
uint32_t
am_devices_sw_rtc_alarm_set(am_hal_rtc_time_t *pTime, am_devices_sw_rtc_callback_t pCallback)
{
  am_hal_rtc_time_t     sCurrentTime;
  uint32_t              sCurrentStimer = STIMER->STTMR;
  uint64_t              ui64ElapsedTime;
  uint32_t              ui32Overflows, ui32Remainder;
  am_hal_sw_rtc_time_t *pSwRtcState = (am_hal_sw_rtc_time_t *)&(STIMER->SNVR0);

#if defined(SW_RTC_GPIO_TIMING)
  am_hal_gpio_output_set(1);
#endif
  // Get current time
  am_devices_sw_rtc_time_get(&sCurrentTime);

  // Calculate extended time based on current STIMER value.
  ui64ElapsedTime = elapsed_time_ms(&sCurrentTime, pTime) / 10 * STIMER_CLK_PER_HUNDREDTH;

  // Calculate NumOverflows and Remainder
  ui32Overflows = ui64ElapsedTime >> 32;
  ui32Remainder = (ui64ElapsedTime & 0xFFFFFFFF);

  //
  // this value must ui32Overflows be a byte value
  // see pSwRtcState->STATUS_b.ALARMOVER
  //
  if (ui32Overflows > 0xFF )
  {
    return AM_HAL_STATUS_OUT_OF_RANGE;
  }

  // Register the callback function.
  g_pAlarmCallback = pCallback;

  // If (NumOverflows == 0) && (Remainder > STTMR), then call callback.
  // Else record NumOverflows and Remainder
  if (ui32Overflows == 0)
  {
    if ((ui32Remainder + sCurrentStimer) < STIMER->STTMR)
    {
      g_pAlarmCallback();
      g_pAlarmCallback = NULL;
    }
    else
    {
#if defined(AM_PART_APOLLO4)
      am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_187KHZ |
                           STIMER_COMPARE_ENABLE |
                           AM_HAL_STIMER_CFG_THAW |
                             AM_HAL_STIMER_CFG_RUN);
#elif defined(AM_PART_APOLLO4B)
      am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_375KHZ |
                           STIMER_COMPARE_ENABLE |
                           AM_HAL_STIMER_CFG_THAW |
                             AM_HAL_STIMER_CFG_RUN);
#endif
      am_hal_stimer_compare_delta_set(STIMER_COMPARE_INSTANCE, ui32Remainder);
      am_hal_stimer_int_clear(STIMER_COMPARE_INT);
      am_hal_stimer_int_enable(STIMER_COMPARE_INT);
#ifdef AM_IRQ_PRIORITY_DEFAULT
      NVIC_SetPriority(STIMER_COMPARE_NVIC, AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
      NVIC_EnableIRQ(STIMER_COMPARE_NVIC);
    }
  }
  else
  {
    pSwRtcState->STATUS_b.ALARMOVER = ui32Overflows;
    pSwRtcState->STATUS_b.ALARMREM = (ui32Remainder >> 8);
  }

#if defined(SW_RTC_GPIO_TIMING)
  am_hal_gpio_output_clear(1);
#endif

  //Set Alarm and registers callback function.
  return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Service the SW RTC Alarm and Overflows.
//
//*****************************************************************************
void
am_devices_sw_rtc_interrupt_service(uint32_t ui32STimerInts)
{
  uint32_t      ui32StimerSaved, ui64StimerNow, ui32StimerDiff;
  am_hal_sw_rtc_time_t *pSwRtcState = (am_hal_sw_rtc_time_t *)&(STIMER->SNVR0);

  //
  // Alarm has expired.  Execute callback function and disable interrupts.
  //
  if (ui32STimerInts & STIMER_COMPARE_INT)
  {
      am_hal_stimer_int_clear(STIMER_COMPARE_INT);
      g_pAlarmCallback();
      am_hal_stimer_int_disable(STIMER_COMPARE_INT);
      NVIC_DisableIRQ(STIMER_COMPARE_NVIC);
#if defined(AM_PART_APOLLO4)
      am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_187KHZ |
                           AM_HAL_STIMER_CFG_THAW |
                             AM_HAL_STIMER_CFG_RUN);
#elif defined(AM_PART_APOLLO4B)
      am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_375KHZ |
                           AM_HAL_STIMER_CFG_THAW |
                             AM_HAL_STIMER_CFG_RUN);
#endif
  }

  // Overflow has occured.  Check if ALARMOVER has expired and set up for
  // compare interrupt if necessary.
  if (ui32STimerInts & AM_HAL_STIMER_INT_OVERFLOW)
  {
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_OVERFLOW);
#if defined(SW_RTC_GPIO_TIMING)
    am_hal_gpio_output_set(2);
#endif
    if (pSwRtcState->STATUS_b.ALARMOVER > 0)
    {
      pSwRtcState->STATUS_b.ALARMOVER--;
      if (pSwRtcState->STATUS_b.ALARMOVER == 0)
      {
#if defined(AM_PART_APOLLO4)
        am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_187KHZ |
                             STIMER_COMPARE_ENABLE |
                               AM_HAL_STIMER_CFG_THAW |
                                 AM_HAL_STIMER_CFG_RUN);
#elif defined(AM_PART_APOLLO4B)
        am_hal_stimer_config(STIMER_STCFG_CLKSEL_HFRC_375KHZ |
                             STIMER_COMPARE_ENABLE |
                               AM_HAL_STIMER_CFG_THAW |
                                 AM_HAL_STIMER_CFG_RUN);
#endif
        am_hal_stimer_compare_delta_set(STIMER_COMPARE_INSTANCE, (pSwRtcState->STATUS_b.ALARMREM << 8));
        am_hal_stimer_int_clear(STIMER_COMPARE_INT);
        am_hal_stimer_int_enable(STIMER_COMPARE_INT);
#ifdef AM_IRQ_PRIORITY_DEFAULT
        NVIC_SetPriority(STIMER_COMPARE_NVIC, AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
        NVIC_EnableIRQ(STIMER_COMPARE_NVIC);
      }
    }

    // Calculate the difference between saved value and current STIMER value, including overflow.
    ui32StimerSaved = pSwRtcState->STTMRVAL;
    ui64StimerNow   = STIMER->STTMR;
    ui32StimerDiff = ui64StimerNow + 0xFFFFFFFF - ui32StimerSaved;
    add_delta_to_rtc(ui32StimerDiff);

#if defined(SW_RTC_GPIO_TIMING)
    am_hal_gpio_output_clear(2);
#endif

  }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

