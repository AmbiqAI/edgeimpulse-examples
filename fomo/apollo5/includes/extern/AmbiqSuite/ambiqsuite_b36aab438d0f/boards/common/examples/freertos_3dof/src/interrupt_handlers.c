//*****************************************************************************
//
//! @file interrupt_handlers.c
//!
//! @brief Interrupt handlers.
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
// Global includes for this project.
//
//*****************************************************************************
#include "freertos_3dof.h"

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint64_t ui64Status;

    //
    // Read and clear the GPIO interrupt status.
    //
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);

    //
    // Call the individual pin interrupt handlers for any pin that triggered an
    // interrupt.
    //
    am_hal_gpio_int_service(ui64Status);
}

//*****************************************************************************
//
// Events associated with Timer A0
//
//*****************************************************************************
void
timer_a0_handler(uint32_t ui32Status)
{
    //
    // Only run this handler if the interrupt is for Timer A0.
    //
    if (ui32Status & AM_HAL_CTIMER_INT_TIMERA0)
    {
        //
        // Use Timer A0 for the RTOS Tick event.
        //
        rtos_tick();
    }
}

//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    //
    // Check the timer interrupt status.
    //
    ui32Status = am_hal_ctimer_int_status_get(false);
    am_hal_ctimer_int_clear(ui32Status);

    //
    // Run handlers for the various possible timer events.
    //
    timer_a0_handler(ui32Status);
}
