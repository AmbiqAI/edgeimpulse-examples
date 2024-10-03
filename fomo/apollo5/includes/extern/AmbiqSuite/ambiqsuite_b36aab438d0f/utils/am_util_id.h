//*****************************************************************************
//
//! @file am_util_id.h
//!
//! @brief Identification of the Ambiq Micro device.
//!
//! This module contains functions for run time identification of Ambiq Micro
//! devices.
//!
//! @addtogroup id ID - Identification
//! @ingroup utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_UTIL_ID_H
#define AM_UTIL_ID_H

#include "am_mcu_apollo.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Define the devices to be included in identification.
// This is useful for limiting coding to the desired device.
//
//*****************************************************************************
//
// Define AM_ID_APOLLO_ALL to define all Apollo family
//
//#define AM_ID_APOLLO_ALL

//
//! New defines to be used in am_util_id
//
#if defined(AM_PART_APOLLO)
#define AM_ID_APOLLO
#endif
#if defined(AM_PART_APOLLO2)
#define AM_ID_APOLLO2
#endif
#if defined(AM_PART_APOLLO3)
#define AM_ID_APOLLO3
#endif
#if defined(AM_PART_APOLLO3P)
#define AM_ID_APOLLO3P
#endif
#if defined(AM_PART_APOLLO4)
#define AM_ID_APOLLO4A
#endif
#if defined(AM_PART_APOLLO4B)
#define AM_ID_APOLLO4B
#endif
#if defined(AM_PART_APOLLO4P)
#define AM_ID_APOLLO4P
#endif
#if defined(AM_PART_APOLLO4L)
#define AM_ID_APOLLO4L
#endif
#if defined(AM_PART_APOLLO5A)
#define AM_ID_APOLLO5A
#endif
#if defined(AM_PART_APOLLO5B)
#define AM_ID_APOLLO5B
#endif
#if defined(AM_PART_APOLLO510L)
#define AM_ID_APOLLO510L
#endif


//
//! Handle AM_ID_APOLLO_ALL
//
#if defined(AM_ID_APOLLO_ALL)
#ifndef AM_ID_APOLLO
#define AM_ID_APOLLO
#endif
#ifndef AM_ID_APOLLO2
#define AM_ID_APOLLO2
#endif
#ifndef AM_ID_APOLLO3
#define AM_ID_APOLLO3
#endif
#ifndef AM_ID_APOLLO3P
#define AM_ID_APOLLO3P
#endif
#ifndef AM_ID_APOLLO4A
#define AM_ID_APOLLO4A
#endif
#ifndef AM_ID_APOLLO4B
#define AM_ID_APOLLO4B
#endif
#ifndef AM_ID_APOLLO4P
#define AM_ID_APOLLO4P
#endif
#ifndef AM_ID_APOLLO4L
#define AM_ID_APOLLO4L
#endif
#ifndef AM_ID_APOLLO5A
#define AM_ID_APOLLO5A
#endif
#ifndef AM_ID_APOLLO5B
#define AM_ID_APOLLO5B
#endif
#ifndef AM_ID_APOLLO510L
#define AM_ID_APOLLO510L
#endif
#endif // AM_ID_APOLLO_ALL

//*****************************************************************************
//
//! ID structure
//
//*****************************************************************************
typedef struct
{
    //
    //! Contains the HAL hardware information about the device.
    //
    am_hal_mcuctrl_device_t sMcuCtrlDevice;

    //
    //! Contains the HAL hardware information about the device.
    //
    am_hal_mcuctrl_feature_t sMcuCtrlFeature;

    //
    //! Device type (derived value, not a hardware value)
    //
    uint32_t ui32Device;

    //
    //! Vendor name from the MCUCTRL VENDORID register and stringized here.
    //
    const uint8_t *pui8VendorName;

    //
    //! Device name (derived value, not a hardware value)
    //
    const uint8_t *pui8DeviceName;

    //
    //! Major chip revision (e.g. char 'A' or 'B')
    //
    uint8_t ui8ChipRevMaj;

    //
    //! Minor chip revision (e.g. char '0', '1', ' ')
    //
    uint8_t ui8ChipRevMin;

    //
    //! Package Type (defined at factory)
    //
    const uint8_t *pui8PackageType;

    //
    //! Temperature Range
    //
    const uint8_t *pui8TempRange;

// #### INTERNAL BEGIN ####
//
// This is not always going to be correct for each part
// Until Manufacturing decides to flesh it out properly, we will keep it internaled
//
    //
    //! Number of Pins in the package
    //
    uint8_t ui8NumPins;
// #### INTERNAL END ####
}
am_util_id_t;

//*****************************************************************************
//
//! @name Macros for MCUCTRL CHIP INFO field.
//! @note these macros are derived from CHIPPN definitions.
//! @{
//
//*****************************************************************************
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO510L  0x10000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO5B    0x10000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO5A    0x10000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO4L    0x09000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO4     0x08000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO3P    0x07000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO3     0x06000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLOBL    0x05000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_VOYAGER     0x04000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO2     0x03000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLOHC    0x02000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO      0x01000000
#define AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_PN_M        0xFF000000
//! @}

//*****************************************************************************
//
//! @name Macros for silicon identification
//! @{
//
//*****************************************************************************
#define AM_UTIL_ID_UNKNOWN      0
#define AM_UTIL_ID_APOLLO       0x0001
#define AM_UTIL_ID_APOLLO2      0x0002
#define AM_UTIL_ID_APOLLO3      0x0003      // Apollo3 Blue
#define AM_UTIL_ID_APOLLO3P     0x0103      // Apollo3 Blue Plus
#define AM_UTIL_ID_APOLLO4      0x0004      // Apollo4
#define AM_UTIL_ID_APOLLO4P     0x0104      // Apollo4 Plus
#define AM_UTIL_ID_APOLLO4L     0x0204      // Apollo4 Lite
#define AM_UTIL_ID_APOLLO5A     0x0005      // Apollo5 revA
#define AM_UTIL_ID_APOLLO5B     0x0105      // Apollo5 revB
#define AM_UTIL_ID_APOLLO510L   0x0205      // Apollo5 Lite
//! @}

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Device identification.
//!
//! @param psIDDevice - ptr to a device ID structure (am_util_id_t*) to be
//! filled in by the function.
//!
//! This function provides additional information about the currently running
//! Ambiq Micro MCU device.
//!
//! @returns The ui32Device value, which is a value corresponding to the
//! device type.
//
//*****************************************************************************
extern uint32_t am_util_id_device(am_util_id_t *psIDDevice);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_ID_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

