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

//
// ARM standard register space (needed for macros)
//
#define REG_ITM_BASEADDR                        (0x00000000UL)
#define REG_JEDEC_BASEADDR                      (0x00000000UL)
#define REG_NVIC_BASEADDR                       (0x00000000UL)
#define REG_SYSCTRL_BASEADDR                    (0x00000000UL)
#define REG_SYSTICK_BASEADDR                    (0x00000000UL)
#define REG_TPIU_BASEADDR                       (0x00000000UL)

//
// Peripheral register space
//
#define REG_ADC_BASEADDR                        (0x40038000UL)
#define REG_APBDMA_BASEADDR                     (0x40011000UL)
#define REG_AUDADC_BASEADDR                     (0x40210000UL)
#define REG_CLKGEN_BASEADDR                     (0x40004000UL)
#define REG_CPU_BASEADDR                        (0x48000000UL)
#define REG_DC_BASEADDR                         (0x400A0000UL)
#define REG_DSI_BASEADDR                        (0x400A8000UL)
#define REG_I2S_BASEADDR                        (0x40208000UL)
#define REG_FPIO_BASEADDR                       (0x40010800UL)
#define REG_GPIO_BASEADDR                       (0x40010000UL)
#define REG_GPU_BASEADDR                        (0x40090000UL)
#define REG_IOM_BASEADDR                        (0x40050000UL)
#define REG_IOSLAVE_BASEADDR                    (0x40034000UL)
#define REG_MCUCTRL_BASEADDR                    (0x40020000UL)
#define REG_MRAM_BASEADDR                       (0x40014000UL)
#define REG_MSPI_BASEADDR                       (0x40060000UL)
#define REG_PDM_BASEADDR                        (0x40201000UL)
#define REG_PWRCTRL_BASEADDR                    (0x40021000UL)
#define REG_RSTGEN_BASEADDR                     (0x40000000UL)
#define REG_RTC_BASEADDR                        (0x40004800UL)
#define REG_SECURITY_BASEADDR                   (0x40030000UL)
#define REG_STIMER_BASEADDR                     (0x40008800UL)
#define REG_TIMER_BASEADDR                      (0x40008000UL)
#define REG_UART_BASEADDR                       (0x4001C000UL)
#define REG_VCOMP_BASEADDR                      (0x4000C000UL)
#define REG_WDT_BASEADDR                        (0x40024000UL)
#define REG_SSC_BASEADDR                        (0x400D0000UL)

// #### INTERNAL BEGIN ####
// Information about the memory map comes from FALC-349, obtained 6/4/21.
// #### INTERNAL END ####
// ****************************************************************************
// RAM Memory Map:
//  TCM:                             768KB      0x10000000 - 0x100BFFFF
//  Shared System SRAM0 (SSRAM):    1024KB      0x100C0000 - 0x101BFFFF
//  Shared System SRAM1 (SSRAM):    1024KB      0x101C0000 - 0x102BFFFF
//  Shared System SRAM2 (SSRAM):    1024KB      0x102C0000 - 0x103BFFFF
// ****************************************************************************
//
// TCM address space
//
#define SRAM_BASEADDR                           (0x10000000UL)
#define TCM_BASEADDR                            SRAM_BASEADDR
#define TCM_MAX_SIZE                            (768UL * 1024UL)

//
// The two non-contiguous SSRAM spaces are referred to as SSRAM0 and SSRAM1.
// SSRAM0 address space.
//
#define SSRAM0_BASEADDR                         (SRAM_BASEADDR + TCM_MAX_SIZE)
#define SSRAM0_MAX_SIZE                         (1024UL * 1024UL)

//
// SSRAM1 address space
//
#define SSRAM1_BASEADDR                         (SSRAM0_BASEADDR + SSRAM0_MAX_SIZE)
#define SSRAM1_MAX_SIZE                         (1024UL * 1024UL)

//
// SSRAM2 address space
//
#define SSRAM2_BASEADDR                         (SSRAM1_BASEADDR + SSRAM1_MAX_SIZE)
#define SSRAM2_MAX_SIZE                         (1024UL * 1024UL)

//
// RAM sizes.
// NONTCM_MAX_SIZE is the total of contiguous RAM after TCM (mix of SSRAM and Extended)
// SSRAM_MAX_SIZE  is the total of Shared System RAM, but is not necessarily contiguous.
// RAM_TOTAL_SIZE  is the grand total of all contiguous RAM.
//
#define SSRAM_MAX_SIZE                          (SSRAM0_MAX_SIZE + SSRAM1_MAX_SIZE + SSRAM2_MAX_SIZE)
#define NONTCM_MAX_SIZE                         (SSRAM_MAX_SIZE)
#define RAM_TOTAL_SIZE                          (TCM_MAX_SIZE + NONTCM_MAX_SIZE)

// ****************************************************************************
// MRAM Address Space
// ****************************************************************************
#define MRAM_BASEADDR                           (0x00000000UL)
#define MRAM_MAX_SIZE                           (4096UL * 1024UL)

//
// MSPI Aperature address ranges
//
#define MSPI0_APERTURE_START_ADDR               (0x14000000UL)
#define MSPI0_APERTURE_END_ADDR                 (0x18000000UL)
#define MSPI1_APERTURE_START_ADDR               (0x18000000UL)
#define MSPI1_APERTURE_END_ADDR                 (0x1C000000UL)
#define MSPI2_APERTURE_START_ADDR               (0x1C000000UL)
#define MSPI2_APERTURE_END_ADDR                 (0x20000000UL)
#define MSPI3_APERTURE_START_ADDR               (0x20000000UL)
#define MSPI3_APERTURE_END_ADDR                 (0x24000000UL)


#endif // AM_REG_BASE_ADDRESSES_H

