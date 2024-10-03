//*****************************************************************************
//
//! @file am_widget_stimer.h
//
//*****************************************************************************

extern uint32_t am_widget_stimer_int_test(void);
extern uint32_t am_widget_stimer_speed_test(uint32_t ui32Speed);
extern uint32_t am_widget_stimer_capture_test(uint32_t ui32Speed);
extern int32_t  am_widget_stimer_compare_test(uint32_t ui32Speed);
extern bool am_widget_stimer_compare_int_test(uint32_t ui32Speed, uint32_t tolerance);
#if (defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L))
extern bool am_widget_stimer_counter_test(uint32_t ui32Speed);
#endif
