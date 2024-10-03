//*****************************************************************************
//
//! @file am_widget_mspi.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef MSPI_DEVICE_INIT_H
#define MSPI_DEVICE_INIT_H

#include "nemagfx_enhanced_stress_test_config.h"

#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    #include "am_devices_mspi_is25wx064.h"
#else
    #include "am_devices_mspi_atxp032.h"
#endif

#define IOM_MODULE_TOTAL                8

extern AM_SHARED_RW uint32_t FlashDMATCBBuffer[2560];
extern AM_SHARED_RW uint32_t ui32DMATCBBufferPSRAMXIP[2560];
extern AM_SHARED_RW uint32_t ui32DMATCBBuffer2[IOM_MODULE_TOTAL][1024];

#ifdef __cplusplus
extern "C"
{
#endif


#if defined(AM_BSP_MSPI_FLASH_DEVICE_ATXP032)
    #define am_devices_mspi_flash_config_t            am_devices_mspi_atxp032_config_t
    #define am_devices_mspi_flash_timing_config_t     am_devices_mspi_atxp032_sdr_timing_config_t
    #define MSPI_FLASH_STATUS_SUCCESS                 AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
    #define MSPI_FLASH_STATUS_ERROR                   AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
    #define AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT        16
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    #define am_devices_mspi_flash_config_t            am_devices_mspi_is25wx064_config_t
    #define am_devices_mspi_flash_timing_config_t     am_devices_mspi_is25wx064_timing_config_t
    #define MSPI_FLASH_STATUS_SUCCESS                 AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS
    #define MSPI_FLASH_STATUS_ERROR                   AM_DEVICES_MSPI_IS25WX064_STATUS_ERROR
    #define AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT        17
#endif


#if defined(PSRAM_APS12808L)
#include "am_devices_mspi_psram_aps12808l.h"
#define MSPI_PSRAM_STATUS_SUCCESS               AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define MSPI_PSRAM_STATUS_ERROR                 AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_devices_mspi_psram_config_t          am_devices_mspi_psram_config_t


#elif defined(PSRAM_APS25616N)
#include "am_devices_mspi_psram_aps25616n.h"
#define MSPI_PSRAM_STATUS_SUCCESS               AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define MSPI_PSRAM_STATUS_ERROR                 AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_devices_mspi_psram_config_t         am_devices_mspi_psram_config_t
#define am_devices_mspi_psram_timing_config_t  am_devices_mspi_psram_ddr_timing_config_t

#endif

//
//! the universal struct for the various mspi devices struct
//! each internal function pointer members refer to their different drivers
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_devices_mspi_flash_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
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
                                        am_devices_mspi_flash_config_t *pDevCfg,
                                        am_devices_mspi_flash_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_devices_mspi_flash_timing_config_t *pDevSdrCfg);
} mspi_flash_device_func_t;


//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_devices_mspi_psram_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
    uint32_t (*mspi_term)(void *pHandle);


    uint32_t (*mspi_read)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_nonblocking_read)(void *pHandle,
                                             uint8_t *pui8RxBuffer,
                                             uint32_t ui32ReadAddress,
                                             uint32_t ui32NumBytes,
                                             am_hal_mspi_callback_t pfnCallback,
                                             void *pCallbackCtxt);

    uint32_t (*mspi_write)(void *pHandle, uint8_t *ui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);

#if defined(am_devices_mspi_psram_timing_config_t)
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        am_devices_mspi_psram_config_t *pDevCfg,
                                        am_devices_mspi_psram_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_devices_mspi_psram_timing_config_t *pDevSdrCfg);
#endif
} mspi_psram_device_func_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern uint32_t mspi_flash_init(void);
extern uint32_t mspi_psram_init(void);
extern uint32_t mspi_psram_xip_code_init(void);
#ifdef __cplusplus
}
#endif

#endif // MSPI_DEVICE_INIT_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
