//*****************************************************************************
//
//! @file am_widget_timer.h
//
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
//! @file am_widget_stimer.h
//
//*****************************************************************************

extern bool am_widget_timer_speed_check(uint32_t ui32TimerNum,
                                        am_hal_timer_clock_e eTimerClock);

extern bool am_widget_timer_int_check(uint32_t ui32TimerNum,
                                      am_hal_timer_clock_e eTimerClock);

extern bool am_widget_timer_triggering(uint32_t ui32OutTimerNum,
                                       uint32_t ui32InTimerNum);

extern bool am_widget_timer_variable_period(uint32_t ui32TimerNum);

extern uint32_t am_widget_timer_rudimentary_individual_interrupts(uint32_t ux);

extern void am_widget_timer_repeat_count_int(uint32_t ui32TimerNum);

extern void am_widget_timer_pwm(uint32_t ui32TimerNum,
                         am_hal_timer_clock_e eTimerClock);

extern void am_widget_timer_single_pattern(uint32_t ui32TimerNum,
                         am_hal_timer_clock_e eTimerClock);

extern void am_widget_timer_repeat_pattern(uint32_t ui32TimerNum, am_hal_timer_clock_e eTimerClock);

extern void am_widget_timer_gpio_int_init();

extern bool am_widget_timer_chain(uint32_t ui32SourceTimer, uint32_t ui32SinkTimer,
                                        am_hal_timer_clock_e eTimerClock);

