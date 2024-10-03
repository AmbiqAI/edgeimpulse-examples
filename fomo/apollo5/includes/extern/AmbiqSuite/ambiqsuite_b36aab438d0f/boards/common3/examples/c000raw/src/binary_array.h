//*****************************************************************************
//
//! @file binary_array.h
//!
//! @brief This is a generated file.
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

#ifndef BINARY_ARRAY_H
#define BINARY_ARRAY_H

//*****************************************************************************
//
// Length of the binary array in bytes.
//
//*****************************************************************************
#define BINARY_ARRAY_LENGTH                 9188

//*****************************************************************************
//
// CRC-32C value calculated over the binary array.
//
//*****************************************************************************
#define BINARY_ARRAY_CRC                    0xB0DFE7A0

//*****************************************************************************
//
// Extracted binary array.
//
//*****************************************************************************
#define ABS_LOCATION    0x0000C000

#if defined(__IAR_SYSTEMS_ICC__)
#define DECLARE_LOCATION    @ABS_LOCATION
#define EXTERN_LOCATION
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
#define DECLARE_LOCATION
#define EXTERN_LOCATION      __attribute__((at(ABS_LOCATION)))
#else
#error c000raw.c Compiler not yet supported.
#endif

extern const uint8_t g_pui8BinaryArray[BINARY_ARRAY_LENGTH] EXTERN_LOCATION;

#endif // BINARY_ARRAY_H
