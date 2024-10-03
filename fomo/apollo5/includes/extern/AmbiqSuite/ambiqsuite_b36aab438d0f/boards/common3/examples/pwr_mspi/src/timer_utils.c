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

static tmr_callback_fcn_t callbackFcn;
void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt.
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);
    
    if (callbackFcn) 
    {
        callbackFcn(0);
    }
}


uint32_t
timer_init(uint32_t ui32TimerCounts, tmr_callback_fcn_t callbackFcnParam)
{
    //
    // Start a timer to trigger the ADC periodically. This timer won't actually
    // be connected to the ADC (as can be done with Timer 3). Instead, we'll
    // generate interrupts to the CPU, and then use the CPU to trigger the ADC
    // in the CTIMER interrupt handler.
    //
  
    callbackFcn = callbackFcnParam;
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERA,
                                AM_HAL_CTIMER_LFRC_512HZ |
                                AM_HAL_CTIMER_FN_REPEAT |
                                AM_HAL_CTIMER_INT_ENABLE);

    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, ui32TimerCounts, 0);


    //
    // Start the timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
    
    NVIC_EnableIRQ(CTIMER_IRQn);
    

    return 0;
}

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

