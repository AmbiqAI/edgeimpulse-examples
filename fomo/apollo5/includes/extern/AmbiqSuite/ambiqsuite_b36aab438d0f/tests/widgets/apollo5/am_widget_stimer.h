//*****************************************************************************
//
//! @file am_widget_stimer.h
//
//*****************************************************************************

#ifndef __AM_WIDGET_STIMER_H__
#define __AM_WIDGET_STIMER_H__

extern uint32_t am_widget_stimer_int_test(void);
extern uint32_t am_widget_stimer_speed_test(uint32_t ui32Speed);
extern uint32_t am_widget_stimer_capture_test(uint32_t ui32Speed, uint32_t ui32CaptureNum);
extern int32_t  am_widget_stimer_compare_test(uint32_t ui32Speed);
extern bool am_widget_stimer_counter_test(uint32_t ui32Speed);
extern bool am_widget_stimer_compare_int_test(uint32_t ui32Speed, uint32_t tolerance);
extern void am_widget_stimer_sleep_wakeup_test(void);

#endif