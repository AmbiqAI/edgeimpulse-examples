//*****************************************************************************
//
//! @file extflash.h
//!
//! @brief Brief description of the header. No need to get fancy here.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AMOTAS_EXTFLASH_H
#define AMOTAS_EXTFLASH_H

#include "am_mcu_apollo.h"
#include "am_devices.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_multi_boot.h"

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//
// Fixed external SPI Flash storage start address to be defined by user.
//
#define AMOTA_EXT_FLASH_STORAGE_START_ADDRESS 0


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_multiboot_flash_info_t g_extFlash;


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// function definitions
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // AMOTAS_EXTFLASH_H
