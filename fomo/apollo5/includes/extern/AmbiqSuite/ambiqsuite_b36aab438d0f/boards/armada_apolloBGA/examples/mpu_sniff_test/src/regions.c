//*****************************************************************************
//
//! @file regions.c
//!
//! @brief Provides MPU region structures for the mpu_sniff_test example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "mpu.h"
#include "regions.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
tMPURegion sFlash =
{
    1,                          // Region number: 1
    0x00000000,                 // Base address: 0x00000000
    17,                         // Size: 256K (2^(17+1))
    PRIV_RW_PUB_RW,             // Full Access
    false,                      // Don't prevent execution
    0x0                         // Don't disable any subregions.
};

tMPURegion sSRAM =
{
    2,                          // Region number: 2
    0x10000000,                 // Base address: 0x00000000
    15,                         // Size: 64K (2^(15+1))
    PRIV_RW_PUB_RW,             // Full Access
    false,                      // Don't prevent execution
    0x0                         // Don't disable any subregions.
};

tMPURegion sROM =
{
    3,                          // Region number: 3
    0x20000000,                 // Base address: 0x00000000
    7,                          // Size: 256K (2^(7+1))
    PRIV_RW_PUB_RW,             // Full Access
    false,                      // Don't prevent execution
    0x0                         // Don't disable any subregions.
};

//
// This covers much more of the address space than it needs to, but should
// ensure that all peripheral registers are taken care of.
//
tMPURegion sPeripherals =
{
    4,                          // Region number: 4
    0x40000000,                 // Base address: 0x00000000
    28,                         // Size: 2M (2^(28+1))
    // PRIV_RW_PUB_RW,             // Full Access
    NO_ACCESS,                  // No Access
    true,                       // Do prevent execution
    0xF                         // Disable all subregions.
};
