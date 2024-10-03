//*****************************************************************************
//
//  am_mcu_apollo.h
//! @file
//!
//! @brief Top Include for Apollo1 class devices.
//!
//! This file provides all the includes necessary for an apollo device.
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//
//! @defgroup apollo1hal HAL for Apollo
//! @ingroup hal
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_MCU_APOLLO_H
#define AM_MCU_APOLLO_H


//*****************************************************************************
//
// Define AM_CMSIS_REGS to indicate that AM_REGS registers are supported.
//
//*****************************************************************************
#define AM_CMSIS_REGS       0       // 0 = Use AM_REGS

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __IAR_SYSTEMS_ICC__
#include "intrinsics.h"     // __CLZ() and other intrinsics
#endif

//*****************************************************************************
//
// Registers
//
//*****************************************************************************
#include "regs/am_reg_adc.h"
#include "regs/am_reg_base_addresses.h"
#include "regs/am_reg_clkgen.h"
#include "regs/am_reg_ctimer.h"
// ##### INTERNAL BEGIN #####
#include "regs/am_reg_flashctrl.h"
// ##### INTERNAL END #####
#include "regs/am_reg_gpio.h"
#include "regs/am_reg_iomstr.h"
#include "regs/am_reg_ioslave.h"
#include "regs/am_reg_itm.h"
#include "regs/am_reg_jedec.h"
#include "regs/am_reg_macros.h"
#include "regs/am_reg_mcuctrl.h"
#include "regs/am_reg_nvic.h"
#include "regs/am_reg_rtc.h"
#include "regs/am_reg_rstgen.h"
#include "regs/am_reg_sysctrl.h"
#include "regs/am_reg_systick.h"
#include "regs/am_reg_tpiu.h"
#include "regs/am_reg_uart.h"
#include "regs/am_reg_vcomp.h"
#include "regs/am_reg_wdt.h"

// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
// RTOS
//
//*****************************************************************************
#include "am_rtos_abstraction.h"

// ##### INTERNAL END #####
//*****************************************************************************
//
// HAL
//
//*****************************************************************************
#include "hal/am_hal_adc.h"
#include "hal/am_hal_cachectrl.h"
#include "hal/am_hal_clkgen.h"
#include "hal/am_hal_ctimer.h"
#include "hal/am_hal_debug.h"
#include "hal/am_hal_flash.h"
#include "hal/am_hal_global.h"
#include "hal/am_hal_gpio.h"
#include "hal/am_hal_i2c_bit_bang.h"
#include "hal/am_hal_interrupt.h"
#include "hal/am_hal_iom.h"
#include "hal/am_hal_ios.h"
#include "hal/am_hal_itm.h"
#include "hal/am_hal_mcuctrl.h"
#include "hal/am_hal_otp.h"
#include "hal/am_hal_pin.h"
#include "hal/am_hal_pwrctrl.h"
#include "hal/am_hal_queue.h"
#include "hal/am_hal_reset.h"
#include "hal/am_hal_rtc.h"
#include "hal/am_hal_sysctrl.h"
#include "hal/am_hal_systick.h"
#include "hal/am_hal_tpiu.h"
#include "hal/am_hal_uart.h"
#include "hal/am_hal_vcomp.h"
#include "hal/am_hal_wdt.h"


#endif // AM_MCU_APOLLO_H

