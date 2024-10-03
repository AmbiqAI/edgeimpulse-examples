//*****************************************************************************
//
//! @file low_power_config.h
//!
//! @brief low power config example code
//!
//! Purpose: This example shows how to setup and start a pwm output
//!
//! Printing takes place over the ITM at 1M Baud.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef LOW_PWER_CONFIG_H
#define LOW_PWER_CONFIG_H

#include "am_mcu_apollo.h"

//*****************************************************************************
//
//! @brief standard cache setup
//!
//! @return Standard Hal status
//
//*****************************************************************************
extern uint32_t std_cache_setup(void);

//*****************************************************************************
//
//! @brief standard very low power config
//!
//! @return Standard Hal status
//
//*****************************************************************************
extern uint32_t deepsleep_config(void);
//*****************************************************************************
//
//! @brief setup minimal memory for low power
//!
//! @return Standard Hal status
//
//*****************************************************************************
extern uint32_t lowpower_memory_config(void);

#endif //LOW_PWER_CONFIG_H
