//*****************************************************************************
//
//! @file pwr_timer_utils.h
//!
//! @brief This will manage the timer for the example
//!
//! @addtogroup power_examples Power Examples
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup uart_async UART Asynchronous Example
//! @ingroup peripheral_examples
//! @{
//!
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "timer_utils.h"

#define TIMER_NUM 0

const IRQn_Type gc_TimerIrq = (IRQn_Type) (CTIMER_IRQn);

static tmr_callback_fcn_t pfnTimerCallbackFcn;

//*****************************************************************************
//
//! @brief Interrupt handler for the timer
//! This provides a clock to time the HFRC2 clock refresh activities
//! It is recommended that the HFRC2 refresh be run every ten seconds and
//! it it best if it is not run in an ISR, simply because it has a 1 msec delay
//! when run in fault mode.
//
//*****************************************************************************

#if TMR_TICKS_PER_SECOND == 512
#define TIMER_CONFIG_VAL (AM_HAL_CTIMER_TIMERA | AM_HAL_CTIMER_LFRC_512HZ | AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE)
#else
#define TIMER_CONFIG_VAL (AM_HAL_CTIMER_TIMERA | AM_HAL_CTIMER_LFRC_512HZ | AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE)
#endif

//*****************************************************************************
//
// timer isr
//
//*****************************************************************************
void am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt.
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);
    if ( pfnTimerCallbackFcn){
        pfnTimerCallbackFcn() ;
    }
}

//*****************************************************************************
//
// Init and start timer interrupt
//
//*****************************************************************************
am_hal_ctimer_config_t g_sTimer0 =
{
    // Don't link timers.
    0,

    // Set up Timer0A.
    (AM_HAL_CTIMER_FN_REPEAT    |
     AM_HAL_CTIMER_INT_ENABLE   |
     #if USE_XTAL
     AM_HAL_CTIMER_XT_256HZ),
     #else
     AM_HAL_CTIMER_LFRC_512HZ),
#endif

    // No configuration for Timer0B.
    0,
};


uint32_t
timer_init(uint32_t ui32TimerCounts, tmr_callback_fcn_t pfnCallbackFcnParam)
{
    if ( ui32TimerCounts < 2 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

#if USE_XTAL
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);
#else
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);
#endif

    //
    // Set up timer A0.
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(0, &g_sTimer0);

    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, ui32TimerCounts-1, 0);

    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    //
    // Start the timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);

    pfnTimerCallbackFcn = pfnCallbackFcnParam;

    NVIC_ClearPendingIRQ(CTIMER_IRQn);
    NVIC_SetPriority((IRQn_Type) CTIMER_IRQn, 0x04);
    NVIC_EnableIRQ(CTIMER_IRQn);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Disable timer and timer interrupt
//
//*****************************************************************************
void
timer_disable(void)
{
    am_hal_ctimer_stop(TIMER_NUM,AM_HAL_CTIMER_TIMERA);
    NVIC_DisableIRQ(gc_TimerIrq);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
