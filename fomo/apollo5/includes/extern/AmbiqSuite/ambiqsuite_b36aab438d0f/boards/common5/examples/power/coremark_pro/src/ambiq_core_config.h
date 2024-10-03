//*****************************************************************************
//
//! @file ambiq_core_config.h
//!
//! @brief Configuration options for running coremark on Ambiq devices.
//!
//! By default, Coremark is optimized for performance.
//! By default, CoremarkLP is optimized for power.
//!
//! The biggest differences between the two versions are that CoremarkLP uses
//! -O0 and -mlong-calls optimization options, while Coremark uses -o3 and
//! -funroll-loops.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AMBIQ_CORE_CONFIG_H
#define AMBIQ_CORE_CONFIG_H

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Ambiq configuration
//
//*****************************************************************************

#if ENABLE_CPU_HP_250
    #define MY_CORECLK_HZ                   ( AM_HAL_CLKGEN_FREQ_HP250_HZ )
#else
    #define MY_CORECLK_HZ                   ( AM_CORECLK_HZ )
#endif
#define MY_CORECLK_MHZ                      ( MY_CORECLK_HZ / 1000000 )

//
// Specify the number of iterations.
//
#if ENABLE_CPU_HP_250
    #define ITERATIONS                      3000
#else
    #define ITERATIONS                      2000
#endif

//*****************************************************************************
//
// Specify print output.
//
//*****************************************************************************
#define AM_CORECLK_HZ                       AM_HAL_CLKGEN_FREQ_MAX_HZ
#define AM_CORECLK_MHZ                      (AM_CORECLK_HZ/1000000)

//
// Since the stringize operator itself does not first expand macros, two levels
//  of indirection are required in order to fully resolve the pre-defined
//  compiler (integer) macros.  The 1st level expands the macro, and the 2nd
//  level actually stringizes it.
// This method will also work even if the argument is not a macro. However, if
//  the argument is already a string, the string will end up with inserted quote
//   marks.
//
#define STRINGIZE_VAL(n)                    STRINGIZE_VAL2(n)
#define STRINGIZE_VAL2(n)                   #n

#endif // AMBIQ_CORE_CONFIG_H
