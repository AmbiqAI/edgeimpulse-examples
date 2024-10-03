//*****************************************************************************
//
//! @file system_apollo5b.h
//!
//! @brief Ambiq Micro Apollo5 RevB MCU specific functions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef SYSTEM_APOLLO5B_H
#define SYSTEM_APOLLO5B_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//
// Exception / Interrupt Handler Function Prototype
//
typedef void(*VECTOR_TABLE_Type)(void);

//
// System Clock Frequency (Core Clock)
//
extern uint32_t SystemCoreClock;     // System Clock Frequency (Core Clock)

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//
// Initialize the System and update the SystemCoreClock variable.
//
extern void SystemInit (void);
//
// Updates the SystemCoreClock with current core clock retrieved from CPU registers.
//
extern void SystemCoreClockUpdate (void);

#ifdef __cplusplus
}
#endif

#endif  // SYSTEM_APOLLO5B_H

