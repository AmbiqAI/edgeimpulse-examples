//*****************************************************************************
//
//! @file am_time.c
//!
//! @brief Systick support functions for coremark.
//!
//! Implements a systick interrupt for coremark.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_time.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile uint32_t g_ui32SysTickWrappedTime = 0;

//*****************************************************************************
//
// Systick ISR.
//
//*****************************************************************************
void
am_systick_isr(void)
{
    //
    // Add enough cycles to account for one full cycle of systick.
    //
    g_ui32SysTickWrappedTime += 0x01000000;
}
