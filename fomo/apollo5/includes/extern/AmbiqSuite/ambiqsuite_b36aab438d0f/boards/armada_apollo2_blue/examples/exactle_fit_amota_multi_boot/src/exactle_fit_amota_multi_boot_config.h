//*****************************************************************************
//
//! @file multi_boot_config.h
//!
//! @brief Global bootloader information.
//!
//! This is a bootloader program that supports flash programming over UART,
//! SPI, and I2C. The correct protocol is selected automatically at boot time.
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_bootloader.h"

#ifndef MULTI_BOOT_CONFIG_H
#define MULTI_BOOT_CONFIG_H

//*****************************************************************************
//
// Location of the flag page.
//
//*****************************************************************************
#define FLAG_PAGE_LOCATION                 0x00006000

//*****************************************************************************
//
// Run without flag page.
//
//*****************************************************************************
#define USE_FLAG_PAGE                       1//0

//*****************************************************************************
//
// Ignore the configured location, and use the last available flash page as the
// flag page.
//
//*****************************************************************************
#define USE_LAST_PAGE_FOR_FLAG              0   // 0 = Normal flag page location.
                                                // 1 = Flag page at end of flash.

//*****************************************************************************
//
// Default settings.
//
//*****************************************************************************
#define DEFAULT_LINK_ADDRESS                ((uint32_t *) 0x00006000)
#define DEFAULT_OVERRIDE_GPIO               1
#define DEFAULT_OVERRIDE_POLARITY           AM_BOOTLOADER_OVERRIDE_HIGH

#define INTERRUPT_PIN                       4

//*****************************************************************************
//
// Boot Loader Version Number
//
//*****************************************************************************
#define AM_BOOTLOADER_VERSION_NUM           0x00000001

//*****************************************************************************
//
// Boot messages.
//
//*****************************************************************************
#define AM_BOOTLOADER_ACK                   0x00000000
#define AM_BOOTLOADER_NAK                   0x00000001
#define AM_BOOTLOADER_READY                 0x00000002
#define AM_BOOTLOADER_IMAGE_COMPLETE        0x00000003
#define AM_BOOTLOADER_BAD_CRC               0x00000004
#define AM_BOOTLOADER_ERROR                 0x00000005
#define AM_BOOTLOADER_BL_VERSION            0x00000006
#define AM_BOOTLOADER_FW_VERSION            0x00000007

//*****************************************************************************
//
// Boot Commands.
//
//*****************************************************************************
#define AM_BOOTLOADER_ACK_CMD               0x00000000
#define AM_BOOTLOADER_NAK_CMD               0x00000001
#define AM_BOOTLOADER_NEW_IMAGE             0x00000002
#define AM_BOOTLOADER_NEW_PACKET            0x00000003
#define AM_BOOTLOADER_RESET                 0x00000004
#define AM_BOOTLOADER_SET_OVERRIDE_CMD      0x00000005
#define AM_BOOTLOADER_BL_VERSION_CMD        0x00000006
#define AM_BOOTLOADER_FW_VERSION_CMD        0x00000007
#define AM_BOOTLOADER_NEW_ENCRYPTED_IMAGE   0x00000008
#define AM_BOOTLOADER_RESTART               0x00000009

#endif // MULTI_BOOT_CONFIG_H
