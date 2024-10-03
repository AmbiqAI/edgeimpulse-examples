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

// ARM standard register space (needed for macros)
#define REG_ITM_BASEADDR                         0x00000000
#define REG_JEDEC_BASEADDR                       0x00000000
#define REG_NVIC_BASEADDR                        0x00000000
#define REG_SYSCTRL_BASEADDR                     0x00000000
#define REG_SYSTICK_BASEADDR                     0x00000000
#define REG_TPIU_BASEADDR                        0x00000000

// Peripheral register space
#define REG_ADC_BASEADDR                         0x50008000
#define REG_CLKGEN_BASEADDR                      0x40004000
#define REG_MCUCTRL_BASEADDR                     0x40020000
#define REG_CTIMER_BASEADDR                      0x40008000
// #### INTERNAL BEGIN ####
#define REG_FLASHCTRL_BASEADDR                   0x40014000
// #### INTERNAL END ####
#define REG_GPIO_BASEADDR                        0x40010000
#define REG_IOMSTR0_BASEADDR                     0x50004000
#define REG_IOMSTR1_BASEADDR                     0x50005000
#define REG_IOMSTR_BASEADDR                      0x50004000
#define REG_IOSLAVE_BASEADDR                     0x50000000
// #### INTERNAL BEGIN ####
#define REG_MCUCTRLPRIV_BASEADDR                 0x40040000
// #### INTERNAL END ####
#define REG_RSTGEN_BASEADDR                      0x40000000
#define REG_RTC_BASEADDR                         0x40004040
#define REG_UART_BASEADDR                        0x4001C000
#define REG_VCOMP_BASEADDR                       0x4000C000
#define REG_WDT_BASEADDR                         0x40024000


// SRAM address space
#define SRAM_BASEADDR                            (0x10000000UL)


#endif // AM_REG_BASE_ADDRESSES_H

