//*****************************************************************************
//
//! @file nemagfx_power_profiling_test_cases.h
//!
//! @brief GFX and DC power profiling tests definitions
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef NEMAGFX_POWER_PROFILING_TEST_CASES_H
#define NEMAGFX_POWER_PROFILING_TEST_CASES_H

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define TEX_RESX 256 // corresponds to lines of 1024 bytes of RGBA8888
#define TEX_RESY 1024 // lines transferred per test
#define FB_RESX 456 // LCD resolution
#define FB_RESY 456
#define GPIO_MEASURE 14 // toggles high when transfer measurement begins, low when it completes
#define FB_BURST_SIZE GPU_MSPI_BURST_SIZE_64
#define TEX_BURST_SIZE GPU_MSPI_BURST_SIZE_64


#endif //NEMAGFX_POWER_PROFILING_TEST_CASES_H