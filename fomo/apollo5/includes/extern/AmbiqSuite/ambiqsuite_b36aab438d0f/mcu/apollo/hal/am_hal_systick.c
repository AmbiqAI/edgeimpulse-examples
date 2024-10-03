//*****************************************************************************
//
//  am_hal_systick.c
//! @file
//!
//! @brief Functions for interfacing with the SYSTICK
//!
//! @addtogroup systick1 System Timer (SYSTICK)
//! @ingroup apollo1hal
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
//! @brief Start the SYSTICK.
//!
//! This function starts the systick timer.
//!
//! @note This timer does not run in deep-sleep mode as it runs from the core
//! clock, which is gated in deep-sleep. If a timer is needed in deep-sleep use
//! one of the ctimers instead. Also to note is this timer will consume higher
//! power than the ctimers.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_systick_start(void)
{
    //
    // Start the systick timer.
    //
    AM_REG(SYSTICK, SYSTCSR) |= AM_REG_SYSTICK_SYSTCSR_ENABLE_M;
}

//*****************************************************************************
//
//! @brief Stop the SYSTICK.
//!
//! This function stops the systick timer.
//!
//! @note This timer does not run in deep-sleep mode as it runs from the core
//! clock, which is gated in deep-sleep. If a timer is needed in deep-sleep use
//! one of the ctimers instead. Also to note is this timer will consume higher
//! power than the ctimers.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_systick_stop(void)
{
    //
    // Stop the systick timer.
    //
    AM_REG(SYSTICK, SYSTCSR) &= ~AM_REG_SYSTICK_SYSTCSR_ENABLE_M;
}

//*****************************************************************************
//
//! @brief Enable the interrupt in the SYSTICK.
//!
//! This function enables the interupt in the systick timer.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_systick_int_enable(void)
{
    //
    // Enable the systick timer interrupt.
    //
    AM_REG(SYSTICK, SYSTCSR) |= AM_REG_SYSTICK_SYSTCSR_TICKINT_M;
}

//*****************************************************************************
//
//! @brief Disable the interrupt in the SYSTICK.
//!
//! This function disables the interupt in the systick timer.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_systick_int_disable(void)
{
    //
    // Disable the systick timer interrupt.
    //
    AM_REG(SYSTICK, SYSTCSR) &= ~AM_REG_SYSTICK_SYSTCSR_TICKINT_M;
}

//*****************************************************************************
//
//! @brief Reads the interrupt status.
//!
//! This function reads the interrupt status in the systick timer.
//!
//! @return the interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_systick_int_status_get(void)
{
    //
    // Return the systick timer interrupt status.
    //
    return AM_REG(SYSTICK, SYSTCSR) & AM_REG_SYSTICK_SYSTCSR_COUNTFLAG_M;
}

//*****************************************************************************
//
//! @brief Reset the interrupt in the SYSTICK.
//!
//! This function resets the systick timer by clearing out the configuration
//! register.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_systick_reset(void)
{
    //
    // Reset the systick timer interrupt.
    //
    AM_REG(SYSTICK, SYSTCSR) = 0x0;
}

//*****************************************************************************
//
//! @brief Load the value into the SYSTICK.
//!
//! @param ui32LoadVal the desired load value for the systick. Maximum value is
//! 0x00FF.FFFF.
//!
//! This function loads the desired value into the systick timer.
//!
//! @return None.
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
    AM_REG(SYSTICK, SYSTRVR) = ui32LoadVal;
    AM_REG(SYSTICK, SYSTCVR) = 0;
}

//*****************************************************************************
//
//! @brief Get the current count value in the SYSTICK.
//!
//! This function gets the current count value in the systick timer.
//!
//! @return Current count value.
//
//*****************************************************************************
uint32_t
am_hal_systick_count(void)
{
    //
    // Return the current systick timer count value.
    //
    return AM_REG(SYSTICK, SYSTCVR);
}

//*****************************************************************************
//
//! @brief Wait the specified number of ticks.
//!
//! This function delays for the given number of SysTick ticks.
//!
//! @note If the SysTick timer is being used elsewhere, it will be corrupted
//! by calling this function.
//!
//! @return 0 if successful.
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
    AM_REG(SYSTICK, SYSTRVR) = ui32Ticks;

    //
    // Clear the current count.
    //
    AM_REG(SYSTICK, SYSTCVR) = 0x0;

    //
    // Set to use the processor clock, but don't cause an exception (we'll poll).
    //
    AM_REG(SYSTICK, SYSTCSR) = AM_REG_SYSTICK_SYSTCSR_ENABLE_M;

    //
    // Poll till done
    //
    while ( !(AM_REG(SYSTICK, SYSTCSR) & AM_REG_SYSTICK_SYSTCSR_COUNTFLAG_M) );

    //
    // And disable systick before exiting.
    //
    AM_REG(SYSTICK, SYSTCSR) = 0;

    return 0;
}

//*****************************************************************************
//
//! @brief Delay the specified number of microseconds.
//!
//! This function will use the SysTick timer to delay until the specified
//!  number of microseconds have elapsed.  It uses the processor clocks and
//!  takes into account the current CORESEL setting.
//!
//! @note If the SysTick timer is being used elsewhere, it will be corrupted
//! by calling this function.
//!
//! @return Total number of SysTick ticks delayed.
//
//*****************************************************************************
uint32_t
am_hal_systick_delay_us(uint32_t ui32NumUs)
{
    uint32_t ui32nLoops, ui32Ticks, uRet;
    uint32_t ui32ClkFreq, ui32TicksPerMHz;
    uint32_t ui32CoreSel = AM_BFR(CLKGEN, CCTRL, CORESEL);

    ui32nLoops = 0;
    if ( (ui32CoreSel <= AM_HAL_CLKGEN_CORESEL_MAXDIV)  &&  (ui32NumUs >= 2) )
    {
        //
        // Determine clock freq, then whether we need more than 1 iteration.
        //
        ui32ClkFreq = AM_HAL_CLKGEN_FREQ_MAX_MHZ >> ui32CoreSel;
//##### INTERNAL BEGIN #####
#if defined(AM_PART_APOLLO)
//##### INTERNAL END #####
        if ( ui32CoreSel > 1 )
        {
            ui32ClkFreq = AM_HAL_CLKGEN_FREQ_MAX_MHZ / (ui32CoreSel + 1);
        }
//##### INTERNAL BEGIN #####
#endif
#if !defined(AM_PART_APOLLO) && !defined(AM_PART_APOLLO2)
        ui32ClkFreq <<= (am_hal_burst_mode_status() == AM_HAL_BURST_MODE)? 1 : 0;
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
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************