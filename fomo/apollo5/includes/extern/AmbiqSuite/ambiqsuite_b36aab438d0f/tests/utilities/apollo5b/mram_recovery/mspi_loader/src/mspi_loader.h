//*****************************************************************************
//
//! @file mspi_loader.h
//!
//! @brief Headers file for MSPI flash loader.
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
#ifndef _MSPI_LOADER_H
#define _MSPI_LOADER_H


#ifdef __cplusplus
extern "C"
{
#endif


//
//! abstracted macros for the various mspi devices
//
#if defined(AM_BSP_MSPI_FLASH_DEVICE_ATXP032)
    #include "am_devices_mspi_atxp032.h"
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_atxp032_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_atxp032_sdr_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
    #define AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT        16
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    #include "am_devices_mspi_is25wx064.h"
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_is25wx064_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_is25wx064_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_IS25WX064_STATUS_ERROR
    #define AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT        17
#else
    #error "Must define a flash device!"
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

#ifdef __cplusplus
}
#endif

#endif // _MSPI_LOADER_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
