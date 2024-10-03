//*****************************************************************************
//
//! @file mspi_psram_task.h
//!
//! @brief Functions and variables related to the mspi psram task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_PSRAM_TASK_H
#define MSPI_PSRAM_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
#include "am_devices_mspi_psram_aps25616n.h"

#define MSPI_PSRAM_TASK_BLOCK_SIZE            1024
#define MSPI_PSRAM_TASK_TEST_BLOCK_SIZE       20*1024
#define MSPI_PSRAM_TEST_NUM                   80

#define MAX_MSPI_PSRAM_TASK_DELAY             (100 * 2)

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
#elif defined(PSRAM_W958D6NW)
#include "am_devices_mspi_psram_w958d6nw.h"
#define MSPI_PSRAM_STATUS_SUCCESS               AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define MSPI_PSRAM_STATUS_ERROR                 AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_devices_mspi_psram_config_t         am_devices_mspi_psram_config_t
#define am_devices_mspi_psram_timing_config_t  am_devices_mspi_psram_ddr_timing_config_t
#endif

#define MSPI_PSRAM_TASK_SUCCESS      AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define MSPI_PSRAM_TASK_ERROR        AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define MSPI_TEST_POWERSAVE_MODE     AM_HAL_SYSCTRL_DEEPSLEEP

//*****************************************************************************
//
// MSPI psram task handle.
//
//*****************************************************************************
extern TaskHandle_t mspi_psram_task_handle;
extern AM_SHARED_RW uint32_t PSRAMDMATCBBuffer[4096];
typedef enum
{
  MSPI_TASK_PSRAM_ATXP032,
  MSPI_TASK_PSRAM_APS6404L,
  MSPI_TASK_PSRAM_APS6408L,
  MSPI_TASK_DDR_PSRAM_APS12808L,
  MSPI_TASK_MSPI_DDR_PSRAM_APS25616N
} mspi_task_psram_e;


typedef struct
{
  uint32_t                      ui32Module;             // MSPI instance.
  mspi_task_psram_e             ePSRAM;                 // PSRAM type.
  am_devices_mspi_psram_config_t MSPIConfig;           // MSPI configuration.
  void                          *pHandle;               // MSPI instance handle.
  void                          *pDevHandle;            // MSPI device handle.
  uint32_t                      ui32DMACtrlBufferSize;  // DMA control buffer size.
  uint32_t                      *pDMACtrlBuffer;        // DMA control buffer.
} mspi_psram_config_t;

typedef struct
{
  uint32_t                    ui32Module;
  void                        *pMspiHandle;
  am_hal_mspi_device_e        eDeviceConfig;
  bool                        bOccupied;
} mspi_task_psram_t;

typedef enum
{
  MSPI_XIPMM_WORD_ACCESS,
  MSPI_XIPMM_SHORT_ACCESS,
  MSPI_XIPMM_BYTE_ACCESS,
  MSPI_XIPMM_SHORT_ACCESS_UNALIGNED,
  MSPI_XIPMM_WORD_ACCESS_UNALIGNED,
  MSPI_XIPMM_OWORD_ACCESS,
  MSPI_XIPMM_MEMCPY_ACCESS,
} mspi_xipmm_access_e;

typedef struct
{
  const am_hal_mspi_clock_e     speed;
  const char                    *string;
} mspi_speed_t;

extern mspi_speed_t mspi_test_speeds[];

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
extern void MspiPsramTask(void *pvParameters);

#endif // MSPI_PSRAM_TASK_H