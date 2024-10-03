//*****************************************************************************
//
//  am_reg_base_addresses.h
//! @file am_reg_base_addresses.h
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

// #### INTERNAL BEGIN ####
// Information about the memory map comes from the Confluence page Cayenne
// Specifications (RevA)->CPU Subsystem, cDec 2022.
// #### INTERNAL END ####
//
// The following table is based on maximum memory sizing of Apollo5,
// specific SKUs may vary.
// ****************************************************************************
// RAM and NVRAM Memory Map:
//  ITCM:                            256KB      0x00000000 - 0x0003FFFF
//  NVM (MRAM):                     4096MB      0x00400000 - 0x007FFFFF
//  MSPIx External memory:          varies      0x60000000 - 0x8FFFFFFF
//  DTCM:                            512KB      0x20000000 - 0x2007FFFF
//  System SRAM:                    3072KB      0x20080000 - 0x2037FFFF
// ****************************************************************************
//
// TCM address space.
//
#define ITCM_BASEADDR                           (0x00000000UL)
#define ITCM_MAX_SIZE                           (256UL * 1024UL)
#define DTCM_BASEADDR                           (0x20000000UL)
#define DTCM_MAX_SIZE                           (512UL * 1024UL)

//
// The Apollo5 maximum SSRAM space is 3MB.
//
#define SSRAM_BASEADDR                          (0x20080000)
#define SSRAM_MAX_SIZE                          (3072UL * 1024UL)

// ****************************************************************************
// MRAM Address Space
// ****************************************************************************
#define MRAM_BASEADDR                           (0x00400000UL)
#define MRAM_MAX_SIZE                           (4096UL * 1024UL)

// ****************************************************************************
// INFO base addresses
// ****************************************************************************
#define AM_REG_INFO0_BASEADDR           0x42000000
#define AM_REG_INFO1_BASEADDR           0x42002000
// #### INTERNAL BEGIN ####
//#define AM_REG_INFOC_BASEADDR           0x400C2000
// #### INTERNAL END ####
#define AM_REG_OTP_INFO0_BASEADDR       0x42004000
#define AM_REG_OTP_INFO1_BASEADDR       0x42006000
//
// INFOC base address is the same for either OTP or MRAM.
//
#define AM_REG_OTP_INFOC_BASEADDR       0x400C2000

//
// INFO sizes
//
#define AM_REG_INFO0_SIZE               2048
#define AM_REG_INFO1_SIZE               6144
#define AM_REG_INFOC_SIZE               8192
#define AM_REG_OTP_INFO0_SIZE           256
#define AM_REG_OTP_INFO1_SIZE           2816
#define AM_REG_OTP_INFOC_SIZE           1024


// ****************************************************************************
// MSPI aperture address ranges
// ****************************************************************************
// #### INTERNAL BEGIN ####
// Information about the MSPI aperatures comes from the Confluence page Cayenne
// Specifications (RevA)->CPU Subsystem, cDec 2022.
// #### INTERNAL END ####
//
// Please note that the END_ADDR as used here is actually the (end address + 1).
//
#define MSPI0_XIP_BASEADDR                      (0x04000000UL)
#define MSPI1_XIP_BASEADDR                      (0x08000000UL)
#define MSPI3_XIP_BASEADDR                      (0x0C000000UL)

#define MSPI0_APERTURE_START_ADDR               (0x60000000UL)
#define MSPI0_APERTURE_END_ADDR                 (0x70000000UL)
#define MSPI1_APERTURE_START_ADDR               (0x80000000UL)
#define MSPI1_APERTURE_END_ADDR                 (0x84000000UL)
#define MSPI2_APERTURE_START_ADDR               (0x84000000UL)
#define MSPI2_APERTURE_END_ADDR                 (0x88000000UL)
#define MSPI3_APERTURE_START_ADDR               (0x88000000UL)
#define MSPI3_APERTURE_END_ADDR                 (0x90000000UL)

#endif // AM_REG_BASE_ADDRESSES_H

