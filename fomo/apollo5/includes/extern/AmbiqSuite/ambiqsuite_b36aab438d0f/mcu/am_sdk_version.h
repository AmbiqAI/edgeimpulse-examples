//*****************************************************************************
//
//! @file am_sdk_version.h
//!
//! @brief Defines SDK version.
//!
//! @addtogroup ambiqsuite Ambiqsuite SDK
//
//! @defgroup hal mcu
//! @ingroup ambiqsuite
//! @{

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_SDK_VERSION_H
#define AM_SDK_VERSION_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macros to define HAL SDK version.
//
//*****************************************************************************
//
// Define the current HAL version.
//
#ifndef AM_HAL_VERSION_MAJ
#if defined(AM_PART_APOLLO3_API)
#define AM_HAL_VERSION_MAJ      3
#define AM_HAL_VERSION_MIN      2
#define AM_HAL_VERSION_REV      0
#elif defined(AM_PART_APOLLO4_API) && !defined(AM_PART_BRONCO)
#define AM_HAL_VERSION_MAJ      4
#define AM_HAL_VERSION_MIN      5
#define AM_HAL_VERSION_REV      0
#elif defined(AM_PART_APOLLO5_API)
#define AM_HAL_VERSION_MAJ      5
#define AM_HAL_VERSION_MIN      0
#define AM_HAL_VERSION_REV      0
#else
#warning Please define AM_HAL_VERSION_MAJ, AM_HAL_VERSION_MIN, AM_HAL_VERSION_REV
#define AM_HAL_VERSION_MAJ      0
#define AM_HAL_VERSION_MIN      0
#define AM_HAL_VERSION_REV      0
#endif
#endif // AM_HAL_VERSION_MAJ

// #### INTERNAL BEGIN ####
//
// History:
//  08/12/2024  3.2.0  Ready for R3.2.0 code freeze.
//  05/28/2024  4.5.0  Ready for R4.5.0 code freeze.
//  07/12/2023  4.4.1  Ready for R4.4.1, retroactively updated R3.1.1 
//  06/07/2023  4.4.0  Ready for R4.4.0 code freeze.
//  03/06/2023  3.1.0  Ready for R3.1.0 code freeze.
//  08/05/2022  4.3.0  Ready for R4.3.0 code freeze.
//  05/12/2022  4.2.0  Ready for R4.2.0 code freeze.
//  03/24/2022  4.1.1  Bump before R4.2.0.
//  02/15/2022  4.1.0  Feb 2022 Apollo4p beta release.
//  12/15/2021  3.0.0  Apollo3 and 3p SDK. Release target of 12/15/21.
//  11/09/2021  4.0.2  Apollo4p SDK update to 4.0.1 (wk of Nov 9).
//  10/18/2021  4.0.1  Initial Apollo4p silicon SDK release (wk of Oct 18).
//  08/05/2021  4.0.0  Release 4.0.0.
//  07/01/2021  2.7.1  Bumped version for next release.
//  06/30/2021  2.7.0  Apollo4p FPGA initial release.
//  06/15/2021  2.7.0  Apollo4b SDK release. (aka 0614).
//  04/xx/2021  2.6.3  SDK update.
//  03/30/2021  2.6.2  Apollo4b release also supporting B2.
//  03/16/2021  2.6.1  Apollo4b general release.
//  02/25/2021  2.6.1  Apollo4b Crypto workaround.
//  02/08/2021  2.6.0  Apollo4b general release.
//  12/10/2020  2.5.5  Apollo4 SDK updates for crypto, GPIO drive strengths, etc.
//  12/04/2020  2.5.4  Release Apollo4 SDK to major customer.
//  11/16/2020  2.5.3  Release Apollo4 SDK to other customers.
//  11/10/2020  2.5.2  Release Apollo4 SDK to major customer.
//  09/16/2020  2.5.0  Major SDK release.
//  05/22/2020  2.4.3  Release Apollo4 SDK to major customer.
//  02/07/2020  2.4.2  Fix Keil project files, release Keil Pack 1.2.0.
//  01/22/2020  2.4.1  Adds some missing files and fixes some Makefiles.
//  01/21/2020  2.4.0  First official release supporting Apollo3 Blue Plus.
//  11/06/2019  2.3.2  Removes some Stingray files.
//  11/05/2019  2.3.1  Aborted release.
//  10/31/2019  2.3.0  Cordio stack, GPIO mask, TurboSpot to Binary Counter.
//  06/xx/2019  2.2.0  Support for Fast GPIO, FreeRTOS 10.
//  03/xx/2019  2.1.0  Supports Apollo3 A1 and B0, BLE improvements, TurboSpot.
//  01/xx/2019  2.0.0  Support for Apollo3 Blue, defect fixes.
//  04/xx/2018  1.2.12 Resolves defects for Apollo1/2.

// #### INTERNAL END ####
#ifdef __cplusplus
}
#endif

#endif // AM_SDK_VERSION_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
