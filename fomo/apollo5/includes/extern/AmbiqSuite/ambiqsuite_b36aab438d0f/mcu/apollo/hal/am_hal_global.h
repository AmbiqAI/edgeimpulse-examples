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
// Device definitions
//
//*****************************************************************************
#define AM_HAL_DEVICE_NAME      "Apollo"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//******************************************************************************
//
// Macros used to access the bit fields in the flags variable.
//
//******************************************************************************
#define AM_HAL_FLAGS_BFR(flagnm)                                            \
    ((g_ui32HALflags & AM_HAL_FLAGS_##flagnm##_M) >> AM_HAL_FLAGS_##flagnm##_S)

#define AM_HAL_FLAGS_BFW(flagnm, value)                                     \
    g_ui32HALflags = ((g_ui32HALflags & (~(AM_HAL_FLAGS_##flagnm##_M)))  |    \
     ((value << AM_HAL_FLAGS_##flagnm##_S) & (AM_HAL_FLAGS_##flagnm##_M)) )

//******************************************************************************
//
// ITMSKIPENABLEDISABLE - Set when the ITM is not to be disabled.  This is
//                        typically needed by Keil debug.ini.
//
//******************************************************************************
#define AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_S         0
#define AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_M         (1 << AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_S)
#define AM_HAL_FLAGS_ITMSKIPENABLEDISABLE(n)        (((n) << AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_S) & AM_HAL_FLAGS_ITMSKIPENABLEDISABLE_M)

//******************************************************************************
//
// ITMBKPT - Breakpoint at the end of itm_enable(), which is needed by
//           Keil debug.ini.
//
//******************************************************************************
#define AM_HAL_FLAGS_ITMBKPT_S                      1
#define AM_HAL_FLAGS_ITMBKPT_M                      (1 << AM_HAL_FLAGS_ITMBKPT_S)
#define AM_HAL_FLAGS_ITMBKPT(n)                     (((n) << AM_HAL_FLAGS_ITMBKPT_S) & AM_HAL_FLAGS_ITMBKPT_M)

//******************************************************************************
//
// Next available flag or bit field.
//
//******************************************************************************
#define AM_HAL_FLAGS_NEXTBITFIELD_S                 2
#define AM_HAL_FLAGS_NEXTBITFIELD_M                 (1 << AM_HAL_FLAGS_NEXTBITFIELD_S)
#define AM_HAL_FLAGS_NEXTBITFIELD(n)                (((n) << AM_HAL_FLAGS_NEXTBITFIELD_S) & AM_HAL_FLAGS_NEXTBITFIELD_M)

//*****************************************************************************
//
// Global Variables extern declarations.
//
//*****************************************************************************
extern volatile uint32_t g_ui32HALflags;

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
__asm void
am_hal_triple_read( uint32_t ui32TimerAddr, uint32_t ui32Data[]);
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
void
am_hal_triple_read(uint32_t ui32TimerAddr, uint32_t ui32Data[]);
#elif defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
void
am_hal_triple_read(uint32_t ui32TimerAddr, uint32_t ui32Data[]);
#elif defined(__IAR_SYSTEMS_ICC__)
__stackless void
am_hal_triple_read( uint32_t ui32TimerAddr, uint32_t ui32Data[]);
#else
#error Compiler is unknown, please contact Ambiq support team
#endif

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_GLOBAL_H
