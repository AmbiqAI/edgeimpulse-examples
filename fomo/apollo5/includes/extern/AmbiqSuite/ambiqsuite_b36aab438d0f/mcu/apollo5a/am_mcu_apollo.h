//*****************************************************************************
//
//! @file am_mcu_apollo.h
//!
//! @brief Top Include for Apollo5a class devices.
//!
//! This file provides all the includes necessary for an apollo device.
//!
//! @addtogroup hal mcu
//
//! @defgroup apollo5a_hal apollo5
//! @ingroup hal
//! @{
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
// Arm architecture intrinsics and options
//  Security extensions
//  Vector extension
//
//*****************************************************************************
#include <arm_cmse.h>               // Include ARM Security Extensions

#if   (__ARM_FEATURE_MVE & 3) == 3  // MVE integer and floating point intrinsics available
#include <arm_mve.h>                // Include M-Profile Vector Extension aka Helium
#elif (__ARM_FEATURE_MVE & 1) == 1  // MVE integer intrinsics available
#include <arm_mve.h>                // Include M-Profile Vector Extension aka Helium
#else
// Compiler options for vector extensions are not enabled.
#ifndef AM_HAL_SKIP_NO_MVE_WARNING
#warning MVE intrinsics not available
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FPGA-specific defines.
//
//*****************************************************************************
//
// Specify that the Apollo5 FPGA is in use.
// If defined, it is assumed to designate the target SOF frequency in MHz.
// For example, if the SOF frequency is designated as 12MHz or 48MHz, then
//  APOLLO5_FPGA should be set to the value of 12 or 48, respectively.
// If the value is changed, the HAL should be rebuilt.
// This define used to support FPGA-specific differences in the HAL as well as
//  modify timings within the HAL for the speed of the FPGA.
//
//
// Some notes about FPGA target speeds.
// - The SOF designated as 48MHz actually outputs HFRC at 25MHz.
// - The SOF designated as 12MHz actually outputs HFRC at 6.25MHz.
// - HFRC can be measured on designated pins by doing 2 things:
//   1. Configuring CLKGEN->CLKOUT with CLKGEN_CLKOUT_CKSEL_HFRC and
//      CLKGEN_CLKOUT_CKEN_EN.
//   2. Configuring the GPIO with FNCSEL=CLKOUT.
//
//
// #warning "am_mcu_apollo.h: APOLLO5_FPGA is defined here. Must be removed for silicon."
//
// It was defined here (as opposed to config.ini) for those instances when the
// HAL is pulled into a debug (IDE) environment and needs to be defined there.
// While defining it in config.ini is preferred, it does not work in the IDE.
//
//#define APOLLO5_FPGA            8      // FPGA SOF target frequency (in MHz)
// #### INTERNAL END ####
//*****************************************************************************
//
//! AM_PART_APOLLO5_API indicates that this device uses the Apollo5 API.
//
//*****************************************************************************
#define AM_PART_APOLLO5_API     1

//*****************************************************************************
//
//! Define AM_CMSIS_REGS to indicate that CMSIS registers are supported.
//
//*****************************************************************************
#define AM_CMSIS_REGS           1

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//*****************************************************************************
//
// Apollo CMSIS peripheral registers
//
//*****************************************************************************
#include "apollo5a.h"

//*****************************************************************************
//
// Global HAL
//
//*****************************************************************************
//
// Define this macro to disable and remove parameter validation in functions
// throughout the HAL.
//
//#define AM_HAL_DISABLE_API_VALIDATION

//*****************************************************************************
//
// Registers
//
//*****************************************************************************
#include "regs/am_reg_base_addresses.h"
#include "regs/am_reg_macros.h"
#include "regs/am_reg.h"
#include "regs/am_reg_jedec.h"

//*****************************************************************************
//
// HAL
//
//*****************************************************************************
#include "hal/am_hal_global.h"
#include "hal/am_hal_pin.h"
#include "hal/am_hal_status.h"
#include "hal/am_hal_sysctrl.h"

//
// HAL MCU includes
//
#include "hal/mcu/am_hal_bootrom_helper.h"
#include "hal/mcu/am_hal_cachectrl.h"
#include "hal/mcu/am_hal_card_host.h"
#include "hal/mcu/am_hal_card.h"
#include "hal/mcu/am_hal_clkgen.h"
#include "hal/mcu/am_hal_cmdq.h"
#include "hal/mcu/am_hal_dsi.h"
#include "hal/mcu/am_hal_iom.h"
#include "hal/mcu/am_hal_ios.h"
#include "hal/mcu/am_hal_itm.h"
#include "hal/mcu/am_hal_mcu.h"
#include "hal/mcu/am_hal_mcuctrl.h"
#include "hal/mcu/am_hal_mcu_sysctrl.h"
#include "hal/mcu/am_hal_mram.h"
#include "hal/mcu/am_hal_mspi.h"
#include "hal/mcu/am_hal_reset.h"
#include "hal/mcu/am_hal_rtc.h"
#include "hal/mcu/am_hal_sdhc.h"
#include "hal/mcu/am_hal_secure_ota.h"
#include "hal/mcu/am_hal_systick.h"
#include "hal/mcu/am_hal_tpiu.h"
#include "hal/mcu/am_hal_uart.h"
#include "hal/mcu/am_hal_mpu.h"

//
// HAL common includes
//
#include "hal/am_hal_access.h"
#include "hal/am_hal_adc.h"
#include "hal/am_hal_audadc.h"
#include "hal/am_hal_dcu.h"
// #### INTERNAL BEGIN ####
#include "hal/am_hal_gpdma.h"
// #### INTERNAL END ####
#include "hal/am_hal_gpio.h"
#include "hal/am_hal_i2s.h"
#include "hal/am_hal_info.h"
#include "hal/am_hal_infoc.h"
#include "hal/am_hal_pdm.h"
#include "hal/am_hal_pwrctrl.h"
#include "hal/am_hal_queue.h"
#include "hal/am_hal_security.h"
#include "hal/am_hal_stimer.h"
// #### INTERNAL BEGIN ####
//#include "hal/am_hal_shmem.h"
//#include "hal/am_hal_system.h"
// #### INTERNAL END ####
#include "hal/am_hal_timer.h"
#include "hal/am_hal_usb.h"
#include "hal/am_hal_usbcharger.h"
#include "hal/am_hal_utils.h"
#include "hal/am_hal_wdt.h"

//
// INFO includes
//
#include "regs/am_mcu_apollo5a_mraminfo0.h"
#include "regs/am_mcu_apollo5a_mraminfo1.h"
#include "regs/am_mcu_apollo5a_mraminfoc.h"
#include "regs/am_mcu_apollo5a_otpinfo0.h"
#include "regs/am_mcu_apollo5a_otpinfo1.h"
#include "regs/am_mcu_apollo5a_otpinfoc.h"

#ifdef __cplusplus
}
#endif

#endif // AM_MCU_APOLLO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
