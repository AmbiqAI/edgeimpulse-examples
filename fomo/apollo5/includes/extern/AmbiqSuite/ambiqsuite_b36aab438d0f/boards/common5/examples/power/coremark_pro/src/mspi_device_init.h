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

#include "coremark_pro.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(NOR_FLASH_ADESTO_ATXP032)
    #include "am_devices_mspi_atxp032.h"
    #define MSPI_FLASH_STATUS_SUCCESS               AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
    #define MSPI_FLASH_STATUS_ERROR                 AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
    #define MSPI_FLASH_PAGE_SIZE                    AM_DEVICES_MSPI_ATXP032_PAGE_SIZE
    #define MSPI_FLASH_SECTOR_SIZE                  AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE
    #define MSPI_FLASH_MAX_SECTORS                  AM_DEVICES_MSPI_ATXP032_MAX_SECTORS
    #define am_devices_mspi_flash_config_t          am_devices_mspi_atxp032_config_t
    #define am_devices_mspi_flash_timing_config_t   am_devices_mspi_atxp032_sdr_timing_config_t

#elif defined(NAND_FLASH_DOSILICON_DS35X1GA)
    #include "am_devices_mspi_ds35x1ga.h"
    #define MSPI_FLASH_STATUS_SUCCESS               AM_DEVICES_MSPI_DS35X1GA_STATUS_SUCCESS
    #define MSPI_FLASH_STATUS_ERROR                 AM_DEVICES_MSPI_DS35X1GA_STATUS_ERROR
    #define MSPI_FLASH_PAGE_SIZE                    AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE
    #define MSPI_FLASH_SECTOR_SIZE                  AM_DEVICES_MSPI_DS35X1GA_BLOCK_SIZE
    #define MSPI_FLASH_MAX_SECTORS                  AM_DEVICES_MSPI_DS35X1GA_MAX_BLOCKS
    #define am_devices_mspi_flash_config_t          am_devices_mspi_ds35x1ga_config_t
    #define am_devices_mspi_flash_timing_config_t   am_devices_mspi_ds35x1ga_sdr_timing_config_t
#else
    #define FLASH_ENABLE        0
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
#elif defined(PSRAM_APS25616BA)
    #include "am_devices_mspi_psram_aps25616ba_1p2v.h"
    #define MSPI_PSRAM_STATUS_SUCCESS               AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
    #define MSPI_PSRAM_STATUS_ERROR                 AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
    #define am_devices_mspi_psram_config_t         am_devices_mspi_psram_config_t
    #define am_devices_mspi_psram_timing_config_t  am_devices_mspi_psram_ddr_timing_config_t
#else

#endif

//
// Typedef - to encapsulate device driver functions
//
#if  (FLASH_ENABLE != 0)
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_devices_mspi_flash_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
    uint32_t (*mspi_term)(void *pHandle);


    uint32_t (*mspi_read)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);


    uint32_t (*mspi_write)(void *pHandle, uint8_t *ui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);



    uint32_t (*mspi_mass_erase)(void *pHandle);
    uint32_t (*mspi_sector_erase)(void *pHandle, uint32_t ui32SectorAddress);

    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);

#if defined(am_devices_mspi_flash_timing_config_t)

#if defined(NAND_FLASH_DOSILICON_DS35X1GA)
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        const am_devices_mspi_flash_config_t *pDevCfg,
                                        am_devices_mspi_flash_timing_config_t *pDevSdrCfg);
#else
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        am_devices_mspi_flash_config_t *pDevCfg,
                                        am_devices_mspi_flash_timing_config_t *pDevSdrCfg);
#endif
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_devices_mspi_flash_timing_config_t *pDevSdrCfg);
#endif
} mspi_flash_device_func_t;

#endif
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
extern uint32_t mspi_psram_init(void);
extern void mspi_psram_config_print(void);
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
