//*****************************************************************************
//
//! @file am_hal_systick.c
//!
//! @brief Functions for interfacing with the SYSTICK
//!
//! @addtogroup systick4 SYSTICK - System Tick Timer
//! @ingroup bronco_hal
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SYSTICK_MAX_TICKS   ((1 << 24)-1)
#define MAX_U32             (0xffffffff)

//*****************************************************************************
//
// Start the SYSTICK.
//
// This function starts the systick timer.
//
//*****************************************************************************
void
am_hal_systick_start(void)
{
    //
    // Start the systick timer.
    //
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

//*****************************************************************************
//
// Stop the SYSTICK.
//
// This function stops the systick timer.
//
//*****************************************************************************
void
am_hal_systick_stop(void)
{
    //
    // Stop the systick timer.
    //
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//*****************************************************************************
//
// Enable the interrupt in the SYSTICK.
//
// This function enables the interupt in the systick timer.
//
//*****************************************************************************
void
am_hal_systick_int_enable(void)
{
    //
    // Enable the systick timer interrupt.
    //
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

//*****************************************************************************
//
// Disable the interrupt in the SYSTICK.
//
// This function disables the interupt in the systick timer.
//
//*****************************************************************************
void
am_hal_systick_int_disable(void)
{
    //
    // Disable the systick timer interrupt.
    //
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

//*****************************************************************************
//
// Reads the interrupt status.
//
// This function reads the interrupt status in the systick timer.
//
//*****************************************************************************
uint32_t
am_hal_systick_int_status_get(void)
{
    //
    // Return the systick timer interrupt status.
    //
    return SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
}

//*****************************************************************************
//
// Reset the interrupt in the SYSTICK.
//
// This function resets the systick timer by clearing out the configuration
// register.
//
//*****************************************************************************
void
am_hal_systick_reset(void)
{
    //
    // Reset the systick timer interrupt.
    //
    SysTick->CTRL = 0x0;
}

//*****************************************************************************
//
// Load the value into the SYSTICK.
//
// This function loads the desired value into the systick timer.
//
//*****************************************************************************
void
am_hal_systick_load(uint32_t ui32LoadVal)
{
    //
    //  The proper SysTick initialization sequence is: (p 4-36 of the M4 UG).
    //      1. Program reload value
    //      2. Clear current value
    //      3. Program CSR
    // Write the given value to the reload register.
    // Write the Current Value Register to clear it to 0.
    //
    SysTick->LOAD = ui32LoadVal;
    SysTick->VAL = 0;
}

//*****************************************************************************
//
// Get the current count value in the SYSTICK.
//
// This function gets the current count value in the systick timer.
//
//*****************************************************************************
uint32_t
am_hal_systick_count(void)
{
    //
    // Return the current systick timer count value.
    //
    return SysTick->VAL;
}

//*****************************************************************************
//
// Wait the specified number of ticks.
//
// This function delays for the given number of SysTick ticks.
//
//*****************************************************************************
uint32_t
am_hal_systick_wait_ticks(uint32_t ui32Ticks)
{

    if ( ui32Ticks == 0 )
    {
        ui32Ticks++;                // Make sure we get the COUNTFLAG
    }

    //
    //  The proper SysTick initialization sequence is: (p 4-36 of the M4 UG).
    //      1. Program reload value
    //      2. Clear current value
    //      3. Program CSR
    //
    // Set the reload value to the required number of ticks.
    //
    SysTick->LOAD = ui32Ticks;

    //
    // Clear the current count.
    //
    SysTick->VAL = 0x0;

    //
    // Set to use the processor clock, but don't cause an exception (we'll poll).
    //
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;

    //
    // Poll till done
    //
    while ( !(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) );

    //
    // And disable systick before exiting.
    //
    SysTick->CTRL = 0x0;

    return 0;
}

//*****************************************************************************
//
// Delay the specified number of microseconds.
//
// This function will use the SysTick timer to delay until the specified
//  number of microseconds have elapsed.  It uses the processor clocks and
//  takes into account the current CORESEL setting.
//
//*****************************************************************************
uint32_t
am_hal_systick_delay_us(uint32_t ui32NumUs)
{
#if defined(CLKGEN_DEFINED)
    uint32_t ui32nLoops, ui32Ticks, uRet;
    uint32_t ui32ClkFreq, ui32TicksPerMHz;
//    uint32_t ui32CoreSel = CLKGEN->CCTRL_b.CORESEL;
    uint32_t ui32CoreSel = AM_HAL_CLKGEN_CORESEL_MAXDIV;

    ui32nLoops = 0;
    if ( (ui32CoreSel <= AM_HAL_CLKGEN_CORESEL_MAXDIV)  &&  (ui32NumUs >= 2) )
    {
        //
        // Determine clock freq, then whether we need more than 1 iteration.
        //
        ui32ClkFreq = AM_HAL_CLKGEN_FREQ_MAX_MHZ >> ui32CoreSel;
//##### INTERNAL BEGIN #####
#if defined(AM_PART_APOLLO)
        if ( ui32CoreSel > 1 )
        {
            ui32ClkFreq = AM_HAL_CLKGEN_FREQ_MAX_MHZ / (ui32CoreSel + 1);
        }
#endif
#if !defined(AM_PART_APOLLO) && !defined(AM_PART_APOLLO2)
//##### INTERNAL END #####
//       ui32ClkFreq <<= (am_hal_burst_mode_status() == AM_HAL_BURST_MODE)? 1 : 0;
//##### INTERNAL BEGIN #####
#endif
//##### INTERNAL END #####

        ui32TicksPerMHz = SYSTICK_MAX_TICKS / ui32ClkFreq;
        if ( ui32NumUs > ui32TicksPerMHz )
        {
            //
            // Get number of required loops, as well as additional ticks.
            //
            ui32nLoops = ui32NumUs / ui32TicksPerMHz;
            ui32NumUs  = ui32NumUs % ui32TicksPerMHz;
        }

        //
        // Compute the number of ticks required.
        // Allow for about 2us of call overhead.
        //
        ui32Ticks = (ui32NumUs - 2) * ui32ClkFreq;
    }
    else
    {
        ui32Ticks = 1;
    }

    uRet = (ui32nLoops * SYSTICK_MAX_TICKS) + ui32Ticks;
    while ( ui32nLoops )
    {
        am_hal_systick_wait_ticks(SYSTICK_MAX_TICKS);
        ui32nLoops--;
    }
    am_hal_systick_wait_ticks(ui32Ticks);
    return uRet;
#else
    return AM_HAL_STATUS_SUCCESS;
#endif
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
