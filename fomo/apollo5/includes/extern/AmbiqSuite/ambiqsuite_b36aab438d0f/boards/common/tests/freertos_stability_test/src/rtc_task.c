//*****************************************************************************
//
//! @file rtc_task.c
//!
//! @brief Task to handle 1Hz RTC I/O.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices.h"
#include "am_util.h"

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

//*****************************************************************************
//
// Stability Test include files.
//
//*****************************************************************************
#include "rtc_task.h"
#include "freertos_stability_test.h"

//*****************************************************************************
//
// Defines.
//
//*****************************************************************************
//! Input buffer set to this value when empty.
#define INBUFFER_EMPTY      (0xEE)
//! Output buffer set to this value when empty.
#define OUTBUFFER_EMPTY     (0xED)
//*****************************************************************************
//
// RTC task handle.
//
//*****************************************************************************
TaskHandle_t RTCTaskHandle;

//*****************************************************************************
//
// Handle for RTC Task related events.
//
//*****************************************************************************
EventGroupHandle_t xRTCEventHandle;

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************

//*****************************************************************************
//
// RTC ISR
//
//*****************************************************************************
void
am_clkgen_isr(void)
{
  //
  // Clear the RTC alarm interrupt.
  //
  am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);

  BaseType_t xHigherPriorityTaskWoken, xResult;

  xHigherPriorityTaskWoken = pdFALSE;

  //
  // Send event to the RTC task.
  //
  xResult = xEventGroupSetBitsFromISR(xRTCEventHandle, 1,
                                      &xHigherPriorityTaskWoken);
  //
  // If the RTC task is higher-priority than the context we're currently
  // running from, we should yield now and run the radio task.
  //
  if (xResult != pdFAIL)
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

}

//*****************************************************************************
//
// Perform initial setup for the RTC task.
//
//*****************************************************************************
void
RTCTaskSetup(void)
{
  //
  // Print the banner
  //
  am_util_debug_printf("RTC Task: setup\r\n");

  //
  // Create an event handle for our wake-up events.
  //
  xRTCEventHandle = xEventGroupCreate();

  //
  // Make sure we actually allocated space for the events we need.
  //
  while (xRTCEventHandle == NULL);

  //
  // Enable the XT for the RTC.
  //
  am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);

  //
  // Select XT for RTC clock source
  //
  am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);

  //
  // Enable the RTC.
  //
  am_hal_rtc_osc_enable();

  //
  // Set the alarm repeat interval to be every second.
  //
  am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC);

  //
  // Clear the RTC alarm interrupt.
  //
  am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);

  //
  // Enable the RTC alarm interrupt.
  //
  am_hal_rtc_int_enable(AM_HAL_RTC_INT_ALM);

  am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CLKGEN, configMAX_SYSCALL_INTERRUPT_PRIORITY);

  //
  // Enable GPIO interrupts to the NVIC.
  //
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_CLKGEN);

}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************

void
RTCTask(void *pvParameters)
{

  am_util_debug_printf("RTC Task: running\r\n");
  while (1)
  {
    //
    // Wait for an event to be posted to the SPI FRAM Event Handle.
    //
    xEventGroupWaitBits(xRTCEventHandle, 1, pdTRUE,
                        pdFALSE, portMAX_DELAY);

    am_util_debug_printf("&");

  }
}
