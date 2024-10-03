//*****************************************************************************
//
//! @file am_hal_debug.c
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

#if defined (__IAR_SYSTEMS_ICC__)
__weak void
#else
void __attribute__((weak))
#endif
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
am_hal_debug_error(const char *pcFile, uint32_t ui32Line, const char *pcMessage)
{
    //
    // Halt for analysis.
    //
    while(1);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
