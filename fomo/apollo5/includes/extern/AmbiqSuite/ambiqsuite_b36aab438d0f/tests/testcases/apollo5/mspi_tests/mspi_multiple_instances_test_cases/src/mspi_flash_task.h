//*****************************************************************************
//
//! @file mspi_flash_task.h
//!
//! @brief Functions and variables related to the mspi flash task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_FLASH_TASK_H
#define MSPI_FLASH_TASK_H

#include "mspi_psram_task.h"

#if defined(NOR_FLASH_ADESTO_ATXP032)
#include "am_devices_mspi_atxp032.h"
#define MSPI_FLASH_STATUS_SUCCESS               AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
#define MSPI_FLASH_STATUS_ERROR                 AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
#define MSPI_FLASH_PAGE_SIZE                    AM_DEVICES_MSPI_ATXP032_PAGE_SIZE
#define MSPI_FLASH_SECTOR_SIZE                  AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE
#define MSPI_FLASH_MAX_SECTORS                  AM_DEVICES_MSPI_ATXP032_MAX_SECTORS
#define am_devices_mspi_flash_config_t          am_devices_mspi_atxp032_config_t
#define am_devices_mspi_flash_timing_config_t   am_devices_mspi_atxp032_sdr_timing_config_t

#elif defined(NOR_FLASH_ISSI_IS25WX064)
#include "am_devices_mspi_is25wx064.h"
#define MSPI_FLASH_STATUS_SUCCESS               AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS
#define MSPI_FLASH_STATUS_ERROR                 AM_DEVICES_MSPI_IS25WX064_STATUS_ERROR
#define MSPI_FLASH_PAGE_SIZE                    AM_DEVICES_MSPI_IS25WX064_PAGE_DATA_SIZE
#define MSPI_FLASH_SECTOR_SIZE                  AM_DEVICES_MSPI_IS25WX064_BLOCK_SIZE
#define MSPI_FLASH_MAX_SECTORS                  AM_DEVICES_MSPI_IS25WX064_MAX_BLOCKS
#define am_devices_mspi_flash_config_t          am_devices_mspi_is25wx064_config_t
#define am_devices_mspi_flash_timing_config_t   am_devices_mspi_is25wx064_timing_config_t
#endif

#define MSPI_FLASH_TASK_TEST_BLOCK_SIZE      10*1024
#define MSPI_FLASH_TEST_NUM                  0xA

//
//MSPI task loop delay
//
#define MAX_MSPI_FLASH_TASK_DELAY    (100 * 2)

#define MSPI_TEST_PAGE_INCR     (AM_DEVICES_MSPI_FLASH_PAGE_SIZE*10)

//
// Typedef - to encapsulate device driver functions
//
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

//*****************************************************************************
//
// mspi flash task handle.
//
//*****************************************************************************
extern TaskHandle_t mspi_flash_task_handle;
extern AM_SHARED_RW uint32_t FlashDMATCBBuffer[4096];

typedef struct
{
  uint32_t                      ui32Module;             // MSPI instance.
  am_devices_mspi_flash_config_t MSPIConfig;           // MSPI configuration.
  void                          *pHandle;               // MSPI instance handle.
  void                          *pDevHandle;            // MSPI device handle.
  uint32_t                      ui32DMACtrlBufferSize;  // DMA control buffer size.
  uint32_t                      *pDMACtrlBuffer;        // DMA control buffer.
} mspi_flash_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void MspiFlashTask(void *pvParameters);

extern char* clkfreq_string(am_hal_mspi_clock_e clk);
extern  char* device_mode_string(am_hal_mspi_device_e mode);
#endif // MSPI_FLASH_TASK_H