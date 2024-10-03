//*****************************************************************************
//
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

// ****************************************************************************
// RAM Memory Map:
//  TCM:                             384KB      0x10000000 - 0x1005FFFF
//  System SRAM (SRAM):             1024KB      0x10060000 - 0x1015FFFF
//  Extended RAM:                    480KB      0x10160000 - 0x101D7FFF
// ****************************************************************************
//
// SRAM address space
//
#define SRAM_BASEADDR                           (0x10000000UL)
#define TCM_BASEADDR                            SRAM_BASEADDR
#define TCM_MAX_SIZE                            (384UL * 1024UL)

//
// SSRAM address space
//
#define SSRAM_BASEADDR                          (0x10060000UL)
#define SSRAM0_BASEADDR                         SSRAM_BASEADDR
#define SSRAM0_MAX_SIZE                         (0x80000UL)
#define SSRAM1_BASEADDR                         (SSRAM_BASEADDR + SSRAM0_MAX_SIZE)
#define SSRAM1_MAX_SIZE                         (0x80000UL)
#define SSRAM_MAX_SIZE                          (SSRAM0_MAX_SIZE + SSRAM1_MAX_SIZE)

//
// Extended RAM address space
//
#define EXRAM_BASEADDR                          (0x10160000UL)
#define DSP0IRAM_BASEADDR                       EXRAM_BASEADDR
#define DSP0IRAM_MAX_SIZE                       (0x20000UL)
#define DSP0DRAM_BASEADDR                       (0x10180000UL)
#define DSP0DRAM_MAX_SIZE                       (0x40000UL)
#define DSP1DRAM_BASEADDR                       (0x101C0000UL)
#define DSP1DRAM_MAX_SIZE                       (0x10000UL)
#define DSP1IRAM_BASEADDR                       (0x101D0000UL)
#define DSP1IRAM_MAX_SIZE                       (0x8000UL)
#define EXRAM_MAX_SIZE                          (0x78000UL)
#define EXTRAM_MAX_SIZE                         (EXRAM_MAX_SIZE)

//
// RAM sizes.
// NONTCM_MAX_SIZE is the total of contiguous RAM after TCM (mix of SSRAM and Extended)
// SSRAM_MAX_SIZE  is the total of Shared System RAM, but is not necessarily contiguous.
// RAM_TOTAL_SIZE  is the grand total of all contiguous RAM.
//
#define NONTCM_MAX_SIZE                         (EXTRAM_MAX_SIZE + SSRAM_MAX_SIZE)
#define RAM_TOTAL_SIZE                          (TCM_MAX_SIZE + NONTCM_MAX_SIZE)

// ****************************************************************************
// MRAM Address Space
// ****************************************************************************
#define MRAM_BASEADDR                           (0x00000000UL)
#define MRAM_MAX_SIZE                           (0x200000UL)

//
// MSPI Aperature address ranges
//
#define MSPI0_APERTURE_START_ADDR               (0x14000000UL)
#define MSPI0_APERTURE_END_ADDR                 (0x18000000UL)
#define MSPI1_APERTURE_START_ADDR               (0x18000000UL)
#define MSPI1_APERTURE_END_ADDR                 (0x1C000000UL)
#define MSPI2_APERTURE_START_ADDR               (0x1C000000UL)
#define MSPI2_APERTURE_END_ADDR                 (0x20000000UL)

#endif // AM_REG_BASE_ADDRESSES_H

