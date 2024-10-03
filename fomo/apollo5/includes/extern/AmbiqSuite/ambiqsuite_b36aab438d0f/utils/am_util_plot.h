//*****************************************************************************
//
//! @file am_util_plot.h
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
#ifndef AM_UTIL_PLOT_H
#define AM_UTIL_PLOT_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Trace defines
//! @brief Trace defines for valid plot traces
//!
//! These macros should be used to specify which trace to update for plotting.
//! @{
//
//*****************************************************************************
#define AM_UTIL_PLOT_0              24
#define AM_UTIL_PLOT_1              25
#define AM_UTIL_PLOT_2              26
#define AM_UTIL_PLOT_3              27
//! @}

//
//! Define for the frequency of sync packets.
//
#define AM_UTIL_PLOT_SYNC_SEND      64

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initializes the plot interface (ITM)
//!
//! This function initializes the ITM to allow for plotting.
//
//*****************************************************************************
extern void am_util_plot_init(void);

//*****************************************************************************
//
//! @brief Plots an integer using AMFlash as the viewer.
//!
//! @param ui32Trace - trace number.
//! @param i32Value - value to plot.
//!
//! This function will plot a value to the desired trace. Valid values for
//! ui32Trace are:
//!
//! AM_UTIL_PLOT_0
//! AM_UTIL_PLOT_1
//! AM_UTIL_PLOT_2
//! AM_UTIL_PLOT_3
//
//*****************************************************************************
extern void am_util_plot_int(uint32_t ui32Trace, int32_t i32Value);

//*****************************************************************************
//
//! @brief Plots an byte using AMFlash as the veiwer.
//!
//! @param ui32Trace - trace number.
//! @param ui8Value - byte value to plot.
//!
//! This function will plot a byte value to the desired trace. If your plot
//! value fits into a byte, use this function as the ITM traffic can be reduced
//! by a factor of 4 over am_util_plot_int().  Valid values for ui32Trace
//! are:
//!
//! AM_UTIL_PLOT_0
//! AM_UTIL_PLOT_1
//! AM_UTIL_PLOT_2
//! AM_UTIL_PLOT_3
//
//*****************************************************************************
extern void am_util_plot_byte(uint32_t ui32Trace, uint8_t ui8Value);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_PLOT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

