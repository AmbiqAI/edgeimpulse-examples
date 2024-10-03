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
//
// Specify the number of iterations.
// For Apollo5, the iterations value had to be increased from 2000 to 2250
// in order to get the benchmark to run at least 10s.
//
#define ITERATIONS                          2250

// #### INTERNAL BEGIN ####
#define INTERNAL_OPTIMIZATIONS  0
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//
// AM_OUTPUT_HCLK puts the test into an endless, 3-stage loop.  Each stage
//  runs the number of specified ITERATIONS.
//  1. The first stage is the standard Coremark test with most peripherals
//     turned off.
//  2. The second stage runs Coremark with HCLK/4 output to pin 12 so that it
//     can be verified.  This stage utilizes CTIMER A0 using HCLK as a source.
//  3. The third stage turns off the pad used in step 2, but leaves the ctimer
//     running so the power used by the timer itself can be determined.
//
#define AM_OUTPUT_HCLK                      0
// #### INTERNAL END ####
//
// AM_PRINTF_RESULTS will print the normal Coremark results to the console upon
//  completion. The printing is done to a RAM buffer during execution, and
//  afterwards, the ITM is turned on and the buffer is dumped to the console.
//
// #### INTERNAL BEGIN ####
// Note: typically AM_PRINT_RESULTS is only used mutually exclusive of
//  AM_OUTPUT_HCLK. Since AM_OUTPUT_HCLK never ends, nothing is ever printed
//  out and thus eventually the buffer will overflow (which is handled and
//  doesn't cause a problem, it's simply not useful).
// #### INTERNAL END ####
#define ALL_RETAIN 0 // 0 for min TCM retain (default), 1 for all retain

#define AM_PRINT_RESULTS                    1

#define ENABLE_BURST_MODE                   0

#define COREMARK_GPIO                       0

// #### INTERNAL BEGIN ####
// DISABLE_UPPER_MRAM is defined for apollo5a only
// #define DISABLE_UPPER_MRAM                  1
// #### INTERNAL END ####
//*****************************************************************************
//
// Coremark configuration
// These parameters generally need not be modified.
//
//*****************************************************************************
#define AMBIQ_SPECIFIC                      1
#define HAS_TIME_H                          0
#define HAS_STDIO                           0
#define HAS_PRINTF                          1
#define HAS_FLOAT                           1           // Indicates that printf %f is available
#define MAIN_HAS_NOARGC                     1
#define MEM_METHOD                          MEM_STACK   // MEM_STACK is default.  See core_portme.h

//*****************************************************************************
//
// Specify print output.
//
//*****************************************************************************
#define AM_CORECLK_HZ                       AM_HAL_CLKGEN_FREQ_MAX_HZ
#define AM_CORECLK_MHZ                      AM_HAL_CLKGEN_FREQ_MAX_MHZ
#define AM_CORECLK_250MHZ                   (AM_HAL_CLKGEN_FREQ_HP250_MHZ)

#define COMPILER_FLAGS                      "flags: -oMax"

#if MEM_METHOD == MEM_STACK
#define MEM_LOCATION                        "STACK"
#elif MEM_METHOD == MEM_STATIC
#define MEM_LOCATION                        "STATIC"
#elif MEM_METHOD == MEM_MALLOC
#define MEM_LOCATION                        "MALLOC"
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
int am_sprintf(char*, ...);

#endif // AMBIQ_CORE_CONFIG_H
