//*****************************************************************************
//
//! @file crc32.h
//!
//! @brief Brief description of the header. No need to get fancy here.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AMOTAS_CRC32_H
#define AMOTAS_CRC32_H

#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// function definitions
//
//*****************************************************************************
uint32_t AmotaCrc32(uint32_t crcInit, uint32_t len, uint8_t *pBuf);


#ifdef __cplusplus
}
#endif

#endif // AMOTAS_CRC32_H
