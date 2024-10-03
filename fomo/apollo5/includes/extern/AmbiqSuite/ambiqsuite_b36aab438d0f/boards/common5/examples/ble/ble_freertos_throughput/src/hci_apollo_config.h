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
#define HCI_APOLLO_RESET_PIN            AM_BSP_GPIO_EM9305_RESET

//*****************************************************************************
//
// Other options.
//
// These options are provided in case your board setup is a little more
// unusual. Most boards shouldn't need these features. If in doubt, leave all
// of these features disabled.
//
//*****************************************************************************
#define HCI_APOLLO_CFG_OVERRIDE_ISR         1 // Override the exactle UART ISR

#endif // HCI_APOLLO_CONFIG_H
