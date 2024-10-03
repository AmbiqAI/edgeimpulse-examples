//*****************************************************************************
//
//! @file image_boot_handlers.h
//!
//! @brief
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
#include <stdbool.h>

#ifndef IMAGE_BOOT_HANDLERS_H
#define IMAGE_BOOT_HANDLERS_H

#ifdef __cplusplus
extern "C"
{
#endif

extern void image_load_from_internal_flash( uint32_t* pui32TargetAddress,
                                            uint32_t* pui32StorageAddress,
                                            uint32_t ui32NumberBytes);
extern void image_load_from_external_flash( uint32_t* pui32TargetAddress,
                                            uint32_t ui32StorageAddress,
                                            uint32_t ui32NumberBytes);
extern bool image_flash_write_from_sram( uint32_t* pui32DstAddr, uint32_t* pui32SrcAddr,
                                            uint32_t ui32NumberBytes);
#ifdef __BATCH_ERASE_INTERNAL__
extern bool image_flash_erase( uint32_t* pui32DstAddr, uint32_t ui32NumberBytes);
#endif

//spiflash support functions
extern void configure_spiflash_pins(void);
extern void image_spiflash_erase(uint32_t ui32DstAddr, uint32_t ui32NumberBytes);

extern am_devices_spiflash_t g_sSpiFlash;
extern am_hal_iom_config_t g_sIOMConfig;
extern uint32_t g_ui32FlashLoadingBuffer[];

#ifdef __cplusplus
}
#endif

#endif // IMAGE_BOOT_HANDLERS_H
