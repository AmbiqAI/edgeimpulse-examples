//*****************************************************************************
//
//! @file timer_utils.c
//!
//! @brief
//!
//! @addtogroup ap3x_power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup ap3x_power_examples
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

#include "timer_utils.h"

//*****************************************************************************
//
// enable the timer
//
//*****************************************************************************
static tmr_callback_fcn_t g_pCallbackFcn;

//*****************************************************************************
//
// timer isr
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt.
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);
    
    if (g_pCallbackFcn)
    {
        g_pCallbackFcn(0);
    }
}

//*****************************************************************************
//
// enable the timer
//
//*****************************************************************************
uint32_t
timer_init(uint32_t ui32LfrcCounts, tmr_callback_fcn_t callbackFcnParam)
{

    g_pCallbackFcn = callbackFcnParam;
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERA,
                                AM_HAL_CTIMER_LFRC_512HZ |
                                AM_HAL_CTIMER_FN_REPEAT |
                                AM_HAL_CTIMER_INT_ENABLE);

    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, ui32LfrcCounts, 0);

    //
    // Start the timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);

    NVIC_SetPriority(CTIMER_IRQn, LOCAL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(CTIMER_IRQn);
    NVIC_EnableIRQ(CTIMER_IRQn);
    

    return 0;
}

//*****************************************************************************
//
// disble the timer
//
//*****************************************************************************
void
timer_disable(void) 
{
  
    NVIC_DisableIRQ(CTIMER_IRQn);
    am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
    
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

