//*****************************************************************************
//
//! @file usb_timer_utils.h
//!
//! @brief This will manage the timer for the example
//!
//! @brief contains the common board init function
//!
//! @addtogroup usb_examples USB Examples
//!
//! @defgroup tinyusb_cdc_msc_hfrc2 TinyUSB CDC MSC HFRC2 Example
//! @ingroup usb_examples
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
#include "usb_timer_utils.h"

#define TIMER_NUM 0

const IRQn_Type gc_TimerIrq = (IRQn_Type) (TIMER0_IRQn + TIMER_NUM);

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

#if TIMER_NUM < 10
#define am_timer_isr(n)  am_timer0 ## n ## _isr
#else
#define am_timer_isr(n)  am_timer ## n ## _isr
#endif

#define am_iom_timerx(n) am_timer_isr(n)
#define timer_isr am_iom_timerx(TIMER_NUM)

//*****************************************************************************
//
//! @brief timer ISR
//
//*****************************************************************************
void
timer_isr(void)
{
    //
    // Clear Timer Interrupt.
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_NUM, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(TIMER_NUM);

    if (pfnTimerCallbackFcn)
    {
        pfnTimerCallbackFcn();
    }
}

//*****************************************************************************
//
// Init and start timer interrupt
//
//*****************************************************************************
uint32_t
timer_init(uint32_t ui32TimerCounts, tmr_callback_fcn_t pfnCallbackFcnParam)
{
    am_hal_timer_config_t TimerConfig;

    pfnTimerCallbackFcn = pfnCallbackFcnParam;

    //
    // Set up the desired TIMER.
    // The default config parameters include:
    //  eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16
    //  eFunction = AM_HAL_TIMER_FN_EDGE
    //  Compare0 and Compare1 maxed at 0xFFFFFFFF
    //
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_XT_DIV32;

    //
    // Modify the default parameters.
    // Configure the timer to a ui32LfrcCounts period via ui32Compare1.
    //
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32Compare1 = ui32TimerCounts; // one per second

    //
    // Configure the timer
    //
    uint32_t status = am_hal_timer_config(TIMER_NUM, &TimerConfig);
    if (status != AM_HAL_STATUS_SUCCESS)
    {
        return status;
    }

    //
    // Clear the timer and its interrupt
    //
    am_hal_timer_clear(TIMER_NUM);
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_NUM, AM_HAL_TIMER_COMPARE1));
    //
    // Enable the timer Interrupt.
    //
    NVIC_SetPriority(gc_TimerIrq, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(gc_TimerIrq);

    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(TIMER_NUM, AM_HAL_TIMER_COMPARE1));

    am_hal_timer_enable(TIMER_NUM);

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
    am_hal_timer_disable(TIMER_NUM);
    NVIC_DisableIRQ(gc_TimerIrq);

}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

