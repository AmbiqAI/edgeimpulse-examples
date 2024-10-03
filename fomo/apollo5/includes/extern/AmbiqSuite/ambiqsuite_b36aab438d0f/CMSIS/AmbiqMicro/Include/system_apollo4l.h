//*****************************************************************************
//
//! @file system_Apollo4l.h
//!
//! @brief Ambiq Micro Apollo4 Lite MCU specific functions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef SYSTEM_APOLLO4L_H
#define SYSTEM_APOLLO4L_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern uint32_t SystemCoreClock;     // System Clock Frequency (Core Clock)

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void SystemInit (void);
extern void SystemCoreClockUpdate (void);

#ifdef __cplusplus
}
#endif

#endif  // SYSTEM_APOLLO4L_H

