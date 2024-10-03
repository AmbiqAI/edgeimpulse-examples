//*****************************************************************************
//
//! @file mspi_loader_example.h
//!
//! @brief Headers file for the example of MSPI loader.
//! @{
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef _MSPI_LOADER_EXAMPLE_H
#define _MSPI_LOADER_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

//
//! the abstracted marco for the various mspi devices
//
#if defined(APS25616N)
#include "am_devices_mspi_psram_aps25616n.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_abstract_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(APS25616BA)
#include "am_devices_mspi_psram_aps25616ba_1p2v.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_abstract_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(W958D6NW)
#include "am_devices_mspi_psram_w958d6nw.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_abstract_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(ATXP032)
#include "am_devices_mspi_atxp032.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_ATXP032_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_ATXP032_MAX_SECTORS
#define am_abstract_mspi_devices_config_t am_devices_mspi_atxp032_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_atxp032_sdr_timing_config_t
#define REQUIRE_ERASE
#elif defined(IS25WX064)
#include "am_devices_mspi_is25wx064.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_IS25WX064_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_IS25WX064_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_IS25WX064_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_IS25WX064_MAX_SECTORS
#define am_abstract_mspi_devices_config_t am_devices_mspi_is25wx064_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_is25wx064_timing_config_t
#define REQUIRE_ERASE
#endif

//
//! the universal struct for the various mspi devices struct
//! each internal function pointer members refer to their different drivers
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_abstract_mspi_devices_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
    uint32_t (*mspi_term)(void *pHandle);

    uint32_t (*mspi_read_id)(void *pHandle);

    uint32_t (*mspi_read)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_read_adv)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_read_callback)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes);

    uint32_t (*mspi_write)(void *pHandle, uint8_t *ui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_write_adv)(void *pHandle,
                           uint8_t *puiTxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_mass_erase)(void *pHandle);
    uint32_t (*mspi_sector_erase)(void *pHandle, uint32_t ui32SectorAddress);

    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);
    uint32_t (*mspi_scrambling_enable)(void *pHandle);
    uint32_t (*mspi_scrambling_disable)(void *pHandle);
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        am_abstract_mspi_devices_config_t *pDevCfg,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
} mspi_device_func_t;

typedef struct
{
  const am_hal_mspi_device_e    eDeviceMode;
  const char                    *string;
}mspi_device_mode_str_t;

typedef struct
{
  const am_hal_mspi_clock_e     eFreq;
  const char                    *string;
} mspi_speed_t;

typedef struct
{
  uint32_t      XIPBase;
  uint32_t      XIPMMBase;
} MSPIBaseAddr_t;

#ifdef __cplusplus
}
#endif

#endif // _MSPI_HEX_DDR_PSRAM_BANDWIDTH_EXAMPLE_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
