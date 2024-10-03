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
extern tMPURegion sFlash;
extern tMPURegion sSRAM;
extern tMPURegion sROM;
extern tMPURegion sPeripherals;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // REGIONS_H

