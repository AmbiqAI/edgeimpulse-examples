//*****************************************************************************
//
//! @file am_widget_ctimer.h
//
//*****************************************************************************
//
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Timer configuration structure.
//
//*****************************************************************************
typedef struct
{
    //
    // Link the timers?
    //
    uint32_t ui32Link;

    //
    // Timer A configuration and period information.
    //
    uint32_t ui32TimerAConfig;
    uint32_t ui32TimerAPeriod;
    uint32_t ui32TimerAOnTime;

    //
    // Timer B configuration and period information.
    //
    uint32_t ui32TimerBConfig;
    uint32_t ui32TimerBPeriod;
    uint32_t ui32TimerBOnTime;

    //
    // Timer to focus on for testing.
    //
    uint32_t ui32TestTimer;
    uint32_t ui32TestSegment;
}
am_widget_ctimer_single_config_t;

//*****************************************************************************
//
// Function declarations.
//
//*****************************************************************************
void get_expected_ticks(uint32_t ui32TestTimer, uint32_t ui32TestSegment);

bool check_config(const am_widget_ctimer_single_config_t *psTimerConfig);

uint32_t am_widget_ctimer_main(const void *psSettings, char *pErrorStr);

int32_t am_widget_ctimer_api_test(const void *psSettings, char *pErrorStr);

uint32_t am_widget_ctimer_config_test(const void *psSettings, char *pErrorStr);

uint32_t am_widget_ctimer_config_test2(const void *psSettings, char *pErrorStr);

int32_t get_speed_from_div(const am_widget_ctimer_single_config_t *psCTimerConfig);

int32_t am_widget_ctimer_speed_test(const void *psSettings, char *pErrorStr);
