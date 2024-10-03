//*****************************************************************************
//
//  am_hal_global.c
//! @file
//!
//! @brief Locate global variables here.
//!
//! This module contains global variables that are used throughout the HAL.
//!
//! One use in particular is that it uses a global HAL flags variable that
//! contains flags used in various parts of the HAL.
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


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
// Version information
//
//*****************************************************************************
const uint8_t  g_ui8HALcompiler[] = COMPILER_VERSION;
const am_hal_version_t g_ui32HALversion =
{
    .s.bAMREGS  = false,
    .s.Major    = AM_HAL_VERSION_MAJ,
    .s.Minor    = AM_HAL_VERSION_MIN,
    .s.Revision = AM_HAL_VERSION_REV
};

// #### INTERNAL BEGIN ####
#ifdef APOLLO4_FPGA
uint32_t g_ui32FPGAfreqMHz = APOLLO4_FPGA;


//
// Use this function to set an FPGA frequency at run-time - avoids
// having to rebuild the entire HAL.
//
void am_hal_global_FPGAfreqSet(uint32_t ui32FPGAfreqMHz)
{
    if ( ui32FPGAfreqMHz >= 1000000 )
    {
        ui32FPGAfreqMHz /= 1000000;
    }

    g_ui32FPGAfreqMHz = ui32FPGAfreqMHz;
} // am_hal_global_FFGAfreqSet()

#endif // APOLLO4_FPGA
// #### INTERNAL END ####
