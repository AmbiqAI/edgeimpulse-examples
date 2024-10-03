//*****************************************************************************
//
//! @file am_util_regdump.h
//!
//! @brief Dump specified registers for debug purposes.
//!
//! This module contains functions for real time (debug) printing of registers
//! from peripherals specified in a given bitmask.
//!
//! @addtogroup regdump RegDump  - Debug Functionality
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
#ifndef AM_UTIL_REGDUMP_H
#define AM_UTIL_REGDUMP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_mcu_apollo.h"

//
//! @name Apollo peripherals
//! @{
//
#define AM_UTIL_REGDUMP_ADC             (1 << 0)
#define AM_UTIL_REGDUMP_CLKGEN          (1 << 1)
#define AM_UTIL_REGDUMP_CTIMER          (1 << 2)
#define AM_UTIL_REGDUMP_GPIO            (1 << 3)
#define AM_UTIL_REGDUMP_IOM             (1 << 4)
#define AM_UTIL_REGDUMP_IOS             (1 << 5)
#define AM_UTIL_REGDUMP_MCUCTRL         (1 << 6)
#define AM_UTIL_REGDUMP_RSTGEN          (1 << 7)
#define AM_UTIL_REGDUMP_RTC             (1 << 8)
#define AM_UTIL_REGDUMP_UART            (1 << 9)
#define AM_UTIL_REGDUMP_VCOMP           (1 << 10)
#define AM_UTIL_REGDUMP_WDT             (1 << 11)
//! @}

//
//! @name Apollo2 new peripherals
//! @{
//
#define AM_UTIL_REGDUMP_CACHE           (1 << 12)
#define AM_UTIL_REGDUMP_PDM             (1 << 13)
#define AM_UTIL_REGDUMP_PWRCTRL         (1 << 14)
//! @}

//
//! @name Apollo3 new peripherals
//! @{
//
#define AM_UTIL_REGDUMP_BLE             (1 << 15)
#define AM_UTIL_REGDUMP_MSPI            (1 << 16)
//! @}

#define AM_UTIL_REGDUMP_INFO0           (1 << 24)

//
//! @name ARM Core blocks
//! @{
//
#define AM_UTIL_REGDUMP_ITM             (1 << 25)
#define AM_UTIL_REGDUMP_NVIC            (1 << 26)
#define AM_UTIL_REGDUMP_SYSCTRL         (1 << 27)
#define AM_UTIL_REGDUMP_SYSTICK         (1 << 28)
#define AM_UTIL_REGDUMP_TPIU            (1 << 29)
//! @}

// #### INTERNAL BEGIN ####
//
//! Internal-only peripherals
//
#define AM_UTIL_REGDUMP_FLASH           (1 << 30)
// #### INTERNAL END ####

//*****************************************************************************
//
//! @name Module mask definitions
//! @{
//
//*****************************************************************************
#define AM_UTIL_REGDUMP_APOLLO                  \
        (   AM_UTIL_REGDUMP_ADC     |           \
            AM_UTIL_REGDUMP_CLKGEN  |           \
            AM_UTIL_REGDUMP_CTIMER  |           \
/* #### INTERNAL BEGIN ####  */                 \
            AM_UTIL_REGDUMP_FLASH   |           \
/* #### INTERNAL END ####    */                 \
            AM_UTIL_REGDUMP_GPIO    |           \
            AM_UTIL_REGDUMP_IOM     |           \
            AM_UTIL_REGDUMP_IOS     |           \
            AM_UTIL_REGDUMP_MCUCTRL |           \
            AM_UTIL_REGDUMP_RSTGEN  |           \
            AM_UTIL_REGDUMP_RTC     |           \
            AM_UTIL_REGDUMP_UART    |           \
            AM_UTIL_REGDUMP_VCOMP   |           \
            AM_UTIL_REGDUMP_WDT )

#define AM_UTIL_REGDUMP_APOLLO2                 \
        (   AM_UTIL_REGDUMP_CACHE   |           \
            AM_UTIL_REGDUMP_PDM     |           \
            AM_UTIL_REGDUMP_PWRCTRL )

#define AM_UTIL_REGDUMP_CORE                    \
        (   AM_UTIL_REGDUMP_ITM     |           \
            AM_UTIL_REGDUMP_NVIC    |           \
            AM_UTIL_REGDUMP_SYSCTRL |           \
            AM_UTIL_REGDUMP_SYSTICK |           \
            AM_UTIL_REGDUMP_TPIU )
//! @}

//
//! @name Get a register dump of ALL modules in a block.
//! @{
//
#ifdef AM_PART_APOLLO
#define AM_UTIL_REGDUMP_ALL                     \
        (   AM_UTIL_REGDUMP_APOLLO  |           \
            AM_UTIL_REGDUMP_CORE )
#endif // PART_APOLLO

#if defined(AM_PART_APOLLO2) || defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
#define AM_UTIL_REGDUMP_ALL                     \
        (   AM_UTIL_REGDUMP_APOLLO  |           \
            AM_UTIL_REGDUMP_APOLLO2 |           \
            AM_UTIL_REGDUMP_CORE )
#endif // PART_APOLLO
//! @}

//
//! Get a register dump of ALL modules in a block.
//
#define AM_UTIL_REGDUMP_MOD_ALL             0xFFFFFFFF

//
//! This macro determines a mask given the first and last modules desired. e.g.
//!  REGDUMP_MOD_MASK(2,4)       // Dump regs for modules 2, 3, and 4
//
#define REGDUMP_MOD_MASK(modfirst, modlast)     \
        (((1 << (modlast - modfirst + 1)) - 1) << modfirst)

//
//! @name These macros determine a single module.
//! @{
//!  REGDUMP_MOD2 | REGDUMP_MOD4    // Dump regs for modules 2 and 4 (skip 3)
//
#define REGDUMP_MOD(n)                  (1 << n)
#define REGDUMP_MOD0                    (REGDUMP_MOD(0))
#define REGDUMP_MOD1                    (REGDUMP_MOD(1))
#define REGDUMP_MOD2                    (REGDUMP_MOD(2))
#define REGDUMP_MOD3                    (REGDUMP_MOD(3))
#define REGDUMP_MOD4                    (REGDUMP_MOD(4))
#define REGDUMP_MOD5                    (REGDUMP_MOD(5))
#define REGDUMP_MOD6                    (REGDUMP_MOD(6))
#define REGDUMP_MOD7                    (REGDUMP_MOD(7))
//! @}

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Register dumping for debug purposes.
//!
//! This function dumps register values to the print port for debug purposes.
//!
//! @param ui32PeriphMask = an OR of the mask values to be printed.  e.g.
//! AM_UTIL_REGDUMP_IOM | AM_UTIL_REGDUMP_GPIO
//!
//! @param ui32ModuleMask = A mask representing the modules (for a multi-module
//! block such as IOM) to be dumped.  Bit0 represents module 0, etc.
//! This parameter is ignored for single-module blocks such as GPIO.
//! Pre-defined macros can be used to generate this mask, e.g.
//!     REGDUMP_MOD0 | REGDUMP_MOD1 | REGDUMP_MOD2
//!     or equivalently
//!     REGDUMP_MOD_MAS(0,2)
//!
//*****************************************************************************
extern void am_util_regdump_print(uint32_t ui32PeriphMask, uint32_t ui32ModuleMask);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_REGDUMP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

