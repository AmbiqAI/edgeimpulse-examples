//*****************************************************************************
//
//! @file amota_profile_config.h
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

#ifndef AMOTA_PROFILE_CONFIG_H
#define AMOTA_PROFILE_CONFIG_H

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_bootloader.h"

//*****************************************************************************
//
// Location of the new image storage in internal flash.
//
//*****************************************************************************
//
// Note: Internal flash area to be used for OTA temporary storage
// The address must be aligned to flash page
// This should be customized to the desired memory map of the design
//
#define AMOTA_INT_FLASH_OTA_ADDRESS         0x00050000

//
// User specified maximum size of OTA storage area.
// Make sure the size is flash page multiple
// (Default value is determined based on rest of flash from the start)
//
#define AMOTA_INT_FLASH_OTA_MAX_SIZE        (AM_HAL_FLASH_LARGEST_VALID_ADDR - AMOTA_INT_FLASH_OTA_ADDRESS + 1)


// OTA Descriptor address by reserving 256K bytes app image size
// OTA Descriptor only need one page which is 8K bytes
#define OTA_POINTER_LOCATION                0x4C000


#define AMOTAS_SUPPORT_EXT_FLASH            0

#endif // AMOTA_PROFILE_CONFIG_H
