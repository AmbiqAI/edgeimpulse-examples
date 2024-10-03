//*****************************************************************************
//
//! @file am_hal_debug.h
//!
//! @brief Useful Functions for Debugging.
//!
//! These functions and macros were created to assist with debugging. They are
//! intended to be as unintrusive as possible and designed to be removed from
//! the compilation of a project when they are no longer needed.
//!
//! @addtogroup haldebug3p Debug - HAL Debug/Assert Utilities
//! @ingroup apollo3p_hal
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
#ifndef AM_HAL_DEBUG_H
#define AM_HAL_DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif


//*****************************************************************************
//
//! Determine DBG_FILENAME
//
//*****************************************************************************
//
// By spec and convention, the standard __FILE__ compiler macro includes a full
// path (absolute or relative) to the file being compiled. This makes recreating
// binaries virtually impossible unless rebuilt on the same or identically
// configured system.
//
// To be able to build consistent binaries on different systems, we want to make
// sure the full pathname is not included in the binary.  Only IAR EWARM provides
// an easy mechanism to provide only the filename without the path.  For other
// platforms, we will simply use a generic pathname.
//
#if defined (__IAR_SYSTEMS_ICC__)
//
// With EWARM the --no_path_in_file_macros option reduces __FILE__ to only the
//  module name.  Therefore this define assumes the option is being used.
//
#define DBG_FILENAME    __FILE__
#elif defined(__KEIL__)
//
// Keil provides __MODULE__ which is simply the module name portion of __FILE__.
//
#define DBG_FILENAME    __MODULE__
#elif defined(__ARMCC_VERSION)
#if (__ARMCC_VERSION >= 6000000)
#define DBG_FILENAME    __FILE_NAME__
#else
#define DBG_FILENAME    __MODULE__
#endif
#else
//
// With GCC, we're out of luck.
//
#define DBG_FILENAME    "debug_filename.ext"
//#define DBG_FILENAME    __FILE__
#endif

//*****************************************************************************
//
//! Debug assert macros.
//
//*****************************************************************************
#ifndef AM_HAL_DEBUG_NO_ASSERT

#define am_hal_debug_assert_msg(bCondition, pcMessage)                        \
    if ( !(bCondition))  am_hal_debug_error(DBG_FILENAME, __LINE__, pcMessage)

#define am_hal_debug_assert(bCondition)                                       \
    if ( !(bCondition))  am_hal_debug_error(DBG_FILENAME, __LINE__, 0)

#else

#define am_hal_debug_assert_msg(bCondition, pcMessage)
#define am_hal_debug_assert(bCondition)

#endif // AM_DEBUG_ASSERT

//*****************************************************************************
//
//! @brief Default implementation of a failed ASSERT statement.
//!
//! @param pcFile is the name of the source file where the error occurred.
//! @param ui32Line is the line number where the error occurred.
//! @param pcMessage is an optional message describing the failure.
//!
//! This function is called by am_hal_debug_assert() macro when the supplied
//! condition is not true. The implementation here simply halts the application
//! for further analysis. Individual applications may define their own
//! implementations of am_hal_debug_error() to provide more detailed feedback
//! about the failed am_hal_debug_assert() statement.
//!
//! @note this function never returns
//
//*****************************************************************************
extern void am_hal_debug_error(const char *pcFile, uint32_t ui32Line,
                               const char *pcMessage);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_DEBUG_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
