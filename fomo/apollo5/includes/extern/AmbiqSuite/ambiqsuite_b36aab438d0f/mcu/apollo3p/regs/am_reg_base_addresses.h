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

//
// MSPI XIP & XIPMM addresses
//
#define MSPI0_XIP_BASEADDR                      (0x02000000UL)
#define MSPI0_XIPMM_BASEADDR                    (0x52000000UL)
#define MSPI1_XIP_BASEADDR                      (0x04000000UL)
#define MSPI1_XIPMM_BASEADDR                    (0x54000000UL)
#define MSPI2_XIP_BASEADDR                      (0x06000000UL)
#define MSPI2_XIPMM_BASEADDR                    (0x56000000UL)

#endif // AM_REG_BASE_ADDRESSES_H

