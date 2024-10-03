//*****************************************************************************
//
//! @file regions.h
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

#ifndef REGIONS_H
#define REGIONS_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern tMPURegion sMRAM;
extern tMPURegion sTCM;
extern tMPURegion sROM;
extern tMPURegion sPeripherals;
extern tMPURegion sDSPRAM_A;
extern tMPURegion sDSPRAM_B;
extern tMPURegion sPSRAM;
extern tMPURegion sSSRAM_A;
extern tMPURegion sSSRAM_B;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // REGIONS_H

