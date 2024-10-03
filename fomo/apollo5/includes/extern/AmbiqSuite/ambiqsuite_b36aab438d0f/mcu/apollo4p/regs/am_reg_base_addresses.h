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
// Information about the memory map comes from FALC-349, obtained 6/4/21.
// #### INTERNAL END ####
// ****************************************************************************
// RAM Memory Map:
//  TCM:                             384KB      0x10000000 - 0x1005FFFF
//  Shared System SRAM (SSRAM):     1024KB      0x10060000 - 0x1015FFFF
//  Extended RAM:                    384KB      0x10160000 - 0x101BFFFF
//  Shared System SRAM (SSRAM):     1024KB      0x101C0000 - 0x102BFFFF
// ****************************************************************************
//
// TCM address space
//
#define SRAM_BASEADDR                           (0x10000000UL)
#define TCM_BASEADDR                            SRAM_BASEADDR
#define TCM_MAX_SIZE                            (384UL * 1024UL)

//
// The two non-contiguous SSRAM spaces are referred to as SSRAM0 and SSRAM1.
// SSRAM0 address space.
//
#define SSRAM0_BASEADDR                         (SRAM_BASEADDR + TCM_MAX_SIZE)
#define SSRAM0_MAX_SIZE                         (1024UL * 1024UL)

//
// Extended RAM address space.
// DSP0 layout is exactly as Apollo4b. DSP1RAMs were replaced in Apollo4p with SSRAM1.
//
#define EXTRAM_BASEADDR                         (SSRAM0_BASEADDR + SSRAM0_MAX_SIZE)
#define DSP0IRAM_BASEADDR                       EXTRAM_BASEADDR
#define DSP0IRAM_MAX_SIZE                       (128UL * 1024UL)
#define DSP0DRAM_BASEADDR                       (DSP0IRAM_BASEADDR + DSP0IRAM_MAX_SIZE)
#define DSP0DRAM_MAX_SIZE                       (256UL * 1024UL)
#define EXTRAM_MAX_SIZE                         (DSP0IRAM_MAX_SIZE + DSP0DRAM_MAX_SIZE)

//
// SSRAM1 address space
//
#define SSRAM1_BASEADDR                         (EXTRAM_BASEADDR + EXTRAM_MAX_SIZE)
#define SSRAM1_MAX_SIZE                         (1024UL * 1024UL)

//
// RAM sizes.
// NONTCM_MAX_SIZE is the total of contiguous RAM after TCM (mix of SSRAM and Extended)
// SSRAM_MAX_SIZE  is the total of Shared System RAM, but is not necessarily contiguous.
// RAM_TOTAL_SIZE  is the grand total of all contiguous RAM.
//
#define SSRAM_MAX_SIZE                          (SSRAM0_MAX_SIZE + SSRAM1_MAX_SIZE)
#define NONTCM_MAX_SIZE                         (EXTRAM_MAX_SIZE + SSRAM_MAX_SIZE)
#define RAM_TOTAL_SIZE                          (TCM_MAX_SIZE + NONTCM_MAX_SIZE)

// ****************************************************************************
// MRAM Address Space
// ****************************************************************************
#define MRAM_BASEADDR                           (0x00000000UL)
#define MRAM_MAX_SIZE                           (2048UL * 1024UL)

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

