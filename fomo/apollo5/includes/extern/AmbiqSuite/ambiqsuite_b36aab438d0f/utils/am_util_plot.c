//*****************************************************************************
//
//! @file am_util_plot.c
//!
//! @brief A few useful plot functions to be used with AMFlash.
//! @note  AMFlash utility application is no longer supported by Ambiq.
//!
//! Functions for providing AMFlash with the correct data enabling real-time
//! plotting.
//!
//! @addtogroup plot Plot Functions
//! @ingroup utils
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
#include <stdarg.h>
#include "hal/am_hal_itm.h"
#include "am_util_stdio.h"
#include "am_util_plot.h"

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
//
//! variable to keep track if we need to send a sync packet.
//
uint32_t g_ui32Sync = 0;

//*****************************************************************************
//
// Initializes the plot interface (ITM)
//
//*****************************************************************************
void
am_util_plot_init(void)
{
    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Initialize the printf interface for ITM/SWO output
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_hal_itm_print);
}

//*****************************************************************************
//
// Plots an integer using AMFlash as the viewer.
//
//*****************************************************************************
void
am_util_plot_int(uint32_t ui32Trace, int32_t i32Value)
{
    if (g_ui32Sync == 0)
    {
        //
        // Send Sync.
        //
        am_hal_itm_sync_send();

        //
        // Reset sync count.
        //
        g_ui32Sync = AM_UTIL_PLOT_SYNC_SEND;
    }
    else
    {
        g_ui32Sync--;
    }

    //
    // Write to the stimulus register.
    //
    am_hal_itm_stimulus_reg_word_write(ui32Trace, i32Value);
}

//*****************************************************************************
//
// Plots an byte using AMFlash as the veiwer.
//
//*****************************************************************************
void
am_util_plot_byte(uint32_t ui32Trace, uint8_t ui8Value)
{
    if (g_ui32Sync == 0)
    {
        //
        // Send Sync.
        //
        am_hal_itm_sync_send();

        //
        // Reset sync count.
        //
        g_ui32Sync = AM_UTIL_PLOT_SYNC_SEND;
    }
    else
    {
        g_ui32Sync--;
    }

    //
    // Write to the stimulus register.
    //
    am_hal_itm_stimulus_reg_byte_write(ui32Trace, ui8Value);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

