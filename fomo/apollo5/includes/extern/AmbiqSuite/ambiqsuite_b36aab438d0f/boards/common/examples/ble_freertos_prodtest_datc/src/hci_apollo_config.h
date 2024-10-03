//*****************************************************************************
//
//! @file hci_apollo_config.h
//!
//! @brief This file describes the physical aspects of the HCI conection.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include "am_bsp.h"

#ifndef HCI_APOLLO_CONFIG_H
#define HCI_APOLLO_CONFIG_H

//*****************************************************************************
//
// Pin numbers and configuration.
//
// NOTE: RTS, CTS, and RESET are implemented as GPIOs, so no "CFG" field is
// needed.
//
//*****************************************************************************
//#define HCI_APOLLO_POWER_PIN            AM_BSP_GPIO_EM9304_POWER
//#define HCI_APOLLO_POWER_CFG            AM_BSP_GPIO_CFG_EM9304_POWER

#define HCI_APOLLO_RESET_PIN            AM_BSP_GPIO_EM9304_RESET

//*****************************************************************************
//
// Other options.
//
// These options are provided in case your board setup is a little more
// unusual. Most boards shouldn't need these features. If in doubt, leave all
// of these features disabled.
//
//*****************************************************************************
#define HCI_APOLLO_CFG_OVERRIDE_ISR         0 // Override the exactle UART ISR
// #### INTERNAL BEGIN ####
//
// CAJ: Internal because they aren't implemented yet.
//
#define HCI_APOLLO_CFG_INVERT_RESET         0 // Use active LOW reset
#define HCI_APOLLO_CFG_INVERT_POWER         0 // Use LOW-side power gate
#define HCI_APOLLO_CFG_RADIO_SELF_POWERED   0 // Don't use a power pin at all
#define HCI_APOLLO_CFG_RADIO_USING_OTP      0 // Don't load the radio firmware
// #### INTERNAL END ####

#ifndef USE_LED
#define TEST_START_GPIO               1
#define TEST_STATUS_GPIO              2
#else
#define TEST_START_GPIO               AM_BSP_GPIO_LED0
#define TEST_STATUS_GPIO              AM_BSP_GPIO_LED1
#endif

#endif // HCI_APOLLO_CONFIG_H
