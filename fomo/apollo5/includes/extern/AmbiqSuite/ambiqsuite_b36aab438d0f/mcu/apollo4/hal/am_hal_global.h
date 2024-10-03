//*****************************************************************************
//
//  am_hal_global.h
//! @file
//!
//! @brief Locate all HAL global variables here.
//!
//! This module contains global variables that are used throughout the HAL,
//! but not necessarily those designated as const (which typically end up in
//! flash). Consolidating globals here will make it easier to manage them.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_HAL_GLOBAL_H
#define AM_HAL_GLOBAL_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Include the SDK global version information.
//
//*****************************************************************************
#include "../../am_sdk_version.h"

//*****************************************************************************
//
// Device definitions
//
//*****************************************************************************
#define AM_HAL_DEVICE_NAME      "Apollo4"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
// Utility for compile time assertions
// Will cause divide by 0 error at build time
#define _AM_ASSERT_CONCAT_(a, b) a##b
#define _AM_ASSERT_CONCAT(a, b) _AM_ASSERT_CONCAT_(a, b)
#define am_ct_assert(e) enum { _AM_ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

//*****************************************************************************
//
// Keil attribute macros
//
//*****************************************************************************
#if defined(keil) || defined(keil6)
#define AM_SHARED_RW      __attribute__((section("SHARED_RW"))) __attribute__((used))
#define AM_RESOURCE_TABLE __attribute__((section("RESOURCE_TABLE"))) __attribute__((used))
#define AM_USED           __attribute__((used))
#define AM_SECTION(x)     __attribute__((section(x)))
#define AM_BIT_ALIGNED(x) __attribute__((aligned(x>>3)))

//*****************************************************************************
//
// IAR attribute macros
//
//*****************************************************************************
#elif defined(iar)
#define AM_SHARED_RW      __attribute__((section("SHARED_RW"))) __root
#define AM_RESOURCE_TABLE __attribute__((section("RESOURCE_TABLE"))) __root
#define AM_USED           __root
#define AM_SECTION(x)     __attribute__((section(x)))
#define AM_BIT_ALIGNED(x) __attribute__((aligned(x>>3)))

//*****************************************************************************
//
// GCC attribute macros
//
//*****************************************************************************
#elif defined(gcc)
#define AM_SHARED_RW      __attribute__((section(".shared")))
#define AM_RESOURCE_TABLE __attribute__((section(".resource_table")))
#define AM_USED           __attribute__((used))
#define AM_SECTION(x)     __attribute__((section(x)))
#define AM_BIT_ALIGNED(x) __attribute__((aligned(x>>3)))

//*****************************************************************************
//
// XTENSA attribute macros
//
//*****************************************************************************
#elif defined(xtensa)
#define AM_SHARED_RW
#define AM_SECTION(x)

//*****************************************************************************
//
// Attribute macro stubs.
//
//*****************************************************************************
#else
#define AM_SHARED_RW
#define AM_SECTION(x)

#endif // End of tool-specific attribute macros.

//*****************************************************************************
//
// Core ID
//
//*****************************************************************************
typedef enum
{
    AM_HAL_COREID_CM4F,
    AM_HAL_COREID_DSP0,
    AM_HAL_COREID_DSP1,
    AM_HAL_CORE_MAX
}
am_hal_core_e;

//*****************************************************************************
//
// DSP selector
//
//*****************************************************************************
typedef enum
{
    AM_HAL_DSP0,
    AM_HAL_DSP1
}
am_hal_dsp_select_e;

//*****************************************************************************
//
// Macros to determine compiler version information
//
//*****************************************************************************
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

//*****************************************************************************
//
// Utility Macros
//
//*****************************************************************************
// As long as the two values are not apart by more that 2^31, this should give
// correct result, taking care of wraparound
#define AM_HAL_U32_GREATER(val1, val2)     ((int32_t)((int32_t)(val1) - (int32_t)(val2)) > 0)
#define AM_HAL_U32_SMALLER(val1, val2)     ((int32_t)((int32_t)(val1) - (int32_t)(val2)) < 0)

//******************************************************************************
//
// Global typedefs
//
//******************************************************************************
typedef union
{
    uint32_t    u32;
    struct
    {
        uint32_t    resvd       : 7;    // [6:0]
        uint32_t    bAMREGS     : 1;    // [7]
        uint32_t    Revision    : 8;    // [15:8]
        uint32_t    Minor       : 8;    // [23:16]
        uint32_t    Major       : 8;    // [31:24]
    } s;
} am_hal_version_t;

typedef union
{
    uint32_t    u32;
    struct
    {
        uint32_t    magic   : 24;
        uint32_t    bInit   : 1;
        uint32_t    bEnable : 1;
        uint32_t    resv    : 6;
    } s;
} am_hal_handle_prefix_t;

//*****************************************************************************
//
// Global Variables extern declarations.
//
//*****************************************************************************
extern const    uint8_t  g_ui8HALcompiler[];
extern const    am_hal_version_t g_ui32HALversion;
// #### INTERNAL BEGIN ####
#ifdef APOLLO4_FPGA
extern uint32_t g_ui32FPGAfreqMHz;
extern void am_hal_global_FPGAfreqSet(uint32_t ui32FPGAfreqMhz);
#endif // APOLLO4_FPGA
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_GLOBAL_H
