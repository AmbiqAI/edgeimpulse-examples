//*****************************************************************************
//
//! @file mspi_nand_flash_example.h
//!
//! @brief Headers file for Example of the MSPI operation with Octal SPI Flash.
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
#ifndef _MSPI_NAND_FLASH_EXAMPLE_H
#define _MSPI_NAND_FLASH_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(AM_BSP_MSPI_FLASH_DEVICE_DS35X1GA)
#include "am_devices_mspi_ds35x1ga.h"
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_TC58CYG0)
#include "am_devices_mspi_tc58cyg0.h"
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_W25N02KW)
#include "am_devices_mspi_w25n02kw.h"
#endif

//
//! the abstracted marco for the various mspi devices
//
#if defined(AM_BSP_MSPI_FLASH_DEVICE_DS35X1GA)
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_ds35x1ga_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_ds35x1ga_sdr_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_DS35X1GA_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_DS35X1GA_STATUS_ERROR
    #define AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE      AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE
    #define AM_DEVICES_MSPI_NAND_FLASH_OOB_SIZE       AM_DEVICES_MSPI_DS35X1GA_PAGE_OOB_SIZE
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_TC58CYG0)
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_tc58cyg0_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_tc58cyg0_sdr_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_TC58CYG0_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_TC58CYG0_STATUS_ERROR
    #define AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE      AM_DEVICES_MSPI_TC58CYG0_PAGE_DATA_SIZE
    #define AM_DEVICES_MSPI_NAND_FLASH_OOB_SIZE       AM_DEVICES_MSPI_TC58CYG0_PAGE_OOB_SIZE
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_W25N02KW)
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_w25n02kw_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_w25n02kw_sdr_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_W25N02KW_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_W25N02KW_STATUS_ERROR
    #define AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE      AM_DEVICES_MSPI_W25N02KW_PAGE_DATA_SIZE
    #define AM_DEVICES_MSPI_NAND_FLASH_OOB_SIZE       AM_DEVICES_MSPI_W25N02KW_PAGE_OOB_SIZE
#endif
//
//! the universal struct for the various mspi devices struct
//! each internal function pointer members refer to their different drivers
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, const am_abstract_mspi_devices_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
    uint32_t (*mspi_term)(void *pHandle);

    uint32_t (*mspi_read_id)(void *pHandle, uint32_t *pui32DeviceID);

    uint32_t (*mspi_read)(void *pHandle, uint32_t ui32PageNum,
                           uint8_t *pui8DataBuffer,
                           uint32_t ui32DataLen,
                           uint8_t *pui8OobBuffer,
                           uint32_t ui32OobLen,
                           uint8_t *pui32EccResult);

    uint32_t (*mspi_write)(void *pHandle, uint32_t ui32PageNum,
                           uint8_t *pui8DataBuffer,
                           uint32_t ui32DataLen,
                           uint8_t *pui8OobBuffer,
                           uint32_t ui32OobLen);

    uint32_t (*mspi_block_erase)(void *pHandle, uint32_t ui32BlockAddress);

    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        const am_abstract_mspi_devices_config_t *pDevCfg,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
} mspi_device_func_t;

#ifdef __cplusplus
}
#endif

#endif // _MSPI_NAND_FLASH_EXAMPLE_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
