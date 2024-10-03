//*****************************************************************************
//
//  am_reg_base_addresses.h
//! @file
//!
//! @brief Register defines for all module base addresses
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_BASE_ADDRESSES_H
#define AM_REG_BASE_ADDRESSES_H

#include "stdint.h"

//
// SRAM address space
//
#define SRAM_BASEADDR                           (0x10000000UL)

//
// TCM address space
//
#define TCM_BASEADDR                            SRAM_BASEADDR
#define TCM_MAX_SIZE                            (64UL * 1024UL)

//
// Flash address space
//
#define FLASH_BASEADDR                          (0x00000000UL)

#endif // AM_REG_BASE_ADDRESSES_H

