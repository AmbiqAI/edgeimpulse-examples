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

// ARM standard register space (needed for macros)
#define REG_ITM_BASEADDR                         (0x00000000UL)
#define REG_JEDEC_BASEADDR                       (0x00000000UL)
#define REG_NVIC_BASEADDR                        (0x00000000UL)
#define REG_SYSCTRL_BASEADDR                     (0x00000000UL)
#define REG_SYSTICK_BASEADDR                     (0x00000000UL)
#define REG_TPIU_BASEADDR                        (0x00000000UL)

// Peripheral register space
#define REG_ADC_BASEADDR                         (0x50010000UL)
#define REG_CACHECTRL_BASEADDR                   (0x40018000UL)
#define REG_CLKGEN_BASEADDR                      (0x40004000UL)
#define REG_CTIMER_BASEADDR                      (0x40008000UL)
// #### INTERNAL BEGIN ####
#define REG_FLASHCTRL_BASEADDR                   (0x40014000UL)
// #### INTERNAL END ####
#define REG_GPIO_BASEADDR                        (0x40010000UL)
#define REG_IOMSTR_BASEADDR                      (0x50004000UL)
#define REG_IOSLAVE_BASEADDR                     (0x50000000UL)
#define REG_MCUCTRL_BASEADDR                     (0x40020000UL)
// #### INTERNAL BEGIN ####
#define REG_MCUCTRLPRIV_BASEADDR                 (0x40040000UL)
// #### INTERNAL END ####
#define REG_PDM_BASEADDR                         (0x50011000UL)
#define REG_PWRCTRL_BASEADDR                     (0x40021000UL)
#define REG_RSTGEN_BASEADDR                      (0x40000000UL)
#define REG_RTC_BASEADDR                         (0x40004040UL)
#define REG_UART_BASEADDR                        (0x4001C000UL)
#define REG_VCOMP_BASEADDR                       (0x4000C000UL)
#define REG_WDT_BASEADDR                         (0x40024000UL)

// SRAM address space
#define SRAM_BASEADDR                            (0x10000000UL)

#endif // AM_REG_BASE_ADDRESSES_H
