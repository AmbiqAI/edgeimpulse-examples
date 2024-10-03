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
#include "am_mcu_apollo.h"
#include "regions.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
tMPURegion sMRAM =
{
    .ui8RegionNumber = 1,                  // Region number: 1
    .ui32BaseAddress = 0x00000000,         // Base address: 0x00000000
    .ui8Size = 20,                         // Size: 2MB (2^(20+1))
    .eAccessPermission = PRIV_RW_PUB_RW,   // Full Access
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0x0            // Don't disable any subregions.
};

tMPURegion sTCM =
{
    .ui8RegionNumber = 2,                  // Region number: 2
    .ui32BaseAddress = 0x10000000,         // Base address: 0x10000000
    .ui8Size = 18,                         // Size: 512KB (2^(18+1)) TCM size = 384KB
    .eAccessPermission = PRIV_RW_PUB_RW,   // Full Access
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0xC0           // Disable subregions number 7,8
};

tMPURegion sSSRAM_A =
{
    .ui8RegionNumber = 3,                  // Region number: 3
    .ui32BaseAddress = 0x10000000,         // Base address: 0x10060000 Protection from 0x10060000
    .ui8Size = 19,                         // Size: 1MB (2^(19+1)) Protection Size = 1MB
    .eAccessPermission = PRIV_RO_PUB_RO,   // Read Only
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0x7            // Disable subregions number 1,2,3
};

tMPURegion sSSRAM_B =
{
    .ui8RegionNumber = 4,                  // Region number: 6
    .ui32BaseAddress = 0x10100000,         // Base address: 0x10060000 Protection from 0x10060000
    .ui8Size = 18,                         // Size: 512KB (2^(18+1)) Protection Size = 512KB
    .eAccessPermission = PRIV_RO_PUB_RO,   // Read Only
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0xC0           // Disable subregions number 7,8
};

tMPURegion sDSPRAM_A =
{
    .ui8RegionNumber = 5,                  // Region number: 5
    .ui32BaseAddress = 0x10100000,         // Base address: 0x10160000
    .ui8Size = 18,                         // Size: 512KB (2^(18+1)) Protection Size = 512KB
    .eAccessPermission = PRIV_RO_PUB_RO,   // Read only
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0x3F           // Don't disable any subregions.
};

tMPURegion sDSPRAM_B =
{
    .ui8RegionNumber = 6,                  // Region number: 5
    .ui32BaseAddress = 0x10180000,         // Base address: 0x10160000
    .ui8Size = 18,                         // Size: 512KB (2^(18+1)) Protection Size = 512KB
    .eAccessPermission = PRIV_RO_PUB_RO,   // Read only
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0x0            // Don't disable any subregions.
};

tMPURegion sPSRAM =
{
    .ui8RegionNumber = 7,                  // Region number: 7
    .ui32BaseAddress = 0x14000000,         // Base address: 0x14000000 for MSPI0
    .ui8Size = 25,                         // Size: 64MB (2^(25+1))
    .eAccessPermission = PRIV_RO_PUB_RO,   // Read only
    .bExecuteNever = false,                // Don't prevent execution
    .ui16SubRegionDisable = 0x0            // Don't disable any subregions.
};
