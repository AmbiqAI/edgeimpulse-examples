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
#ifndef AM_WIDGET_MSPI_H
#define AM_WIDGET_MSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MSPI_TEST_POWERSAVE_MODE AM_HAL_SYSCTRL_DEEPSLEEP

#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032) || defined(CYPRESS_S25FS064S))
#define AM_WIDGET_MSPI_FLASH_DEVICE
#elif defined(APS6404L)
#define AM_WIDGET_MSPI_PSRAM_DEVICE
#elif defined(APS12808L)
#define AM_WIDGET_MSPI_DDR_PSRAM_DEVICE
#elif defined(MB85RS64V)
#define AM_WIDGET_MSPI_FRAM_DEVICE
#else
#endif

#if defined(MICRON_N25Q256A)
#include "am_devices_mspi_n25q256a.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_N25Q256A_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_N25Q256A_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_N25Q256A_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_N25Q256A_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_N25Q256A_MAX_SECTORS
#define am_widget_mspi_devices_config_t am_devices_mspi_n25q256a_config_t
#elif defined(MACRONIX_MX25U12835F)
#include "am_devices_mspi_mx25u12835f.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_MX25U12835F_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_MX25U12835F_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_MX25U12835F_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_MX25U12835F_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_MX25U12835F_MAX_SECTORS
#define am_widget_mspi_devices_config_t am_devices_mspi_mx25u12835f_config_t
#elif defined(ADESTO_ATXP032)
#include "am_devices_mspi_atxp032.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_ATXP032_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_ATXP032_MAX_SECTORS
#define am_widget_mspi_devices_config_t am_devices_mspi_atxp032_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_atxp032_sdr_timing_config_t
#elif defined(CYPRESS_S25FS064S)
#include "am_devices_mspi_s25fs064s.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_S25FS064S_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_S25FS064S_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_S25FS064S_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_S25FS064S_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_S25FS064S_MAX_SECTORS
#define am_widget_mspi_devices_config_t am_devices_mspi_s25fs064s_config_t
#elif defined(AM_WIDGET_MSPI_PSRAM_DEVICE)
#include "am_devices_mspi_psram_aps6404l.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_psram_sdr_timing_config_t
#elif defined(AM_WIDGET_MSPI_DDR_PSRAM_DEVICE)
#include "am_devices_mspi_psram_aps12808l.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(AM_WIDGET_MSPI_FRAM_DEVICE)
#include "am_devices_mb85rs64v_mspi.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MB85RS64V_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MB85RS64V_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_fram_config_t
#elif defined(AMBT53)
#include "am_devices_ambt53.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_AMBT53_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_AMBT53_STATUS_FAIL
#define am_widget_mspi_devices_config_t am_devices_mspi_ambt53_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_ambt53_remote_timing_config_t
#endif
    
// Whether to put IOM in low power mode between the tests
#ifndef MSPI_TEST_POWER_SAVE_RESTORE
#define MSPI_TEST_POWER_SAVE_RESTORE
#endif

#ifndef MSPI_TEST_POWER_MSPI_ENABLE_DISABLE
//#define MSPI_TEST_POWER_MSPI_ENABLE_DISABLE
#endif

#ifndef MSPI_TEST_POWER_SAVE_RESTORE_SLEEP
//#define MSPI_TEST_POWER_SAVE_RESTORE_SLEEP
#endif

typedef enum
{
  AM_WIDGET_MSPI_PSRAM_ATXP032,
  AM_WIDGET_MSPI_PSRAM_APS6404L,
  AM_WIDGET_MSPI_PSRAM_APS6408L,
  AM_WIDGET_MSPI_DDR_PSRAM_APS12808L
} am_widget_mspi_psram_e;

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_mspi_status_t;

typedef struct
{
  uint32_t                      ui32Module;             // MSPI instance.
  am_widget_mspi_psram_e        ePSRAM;                 // PSRAM type.
  am_widget_mspi_devices_config_t MSPIConfig;           // MSPI configuration.
  void                          *pHandle;               // MSPI instance handle.
  void                          *pDevHandle;            // MSPI device handle.
  uint32_t                      ui32DMACtrlBufferSize;  // DMA control buffer size.
  uint32_t                      *pDMACtrlBuffer;        // DMA control buffer.
} am_widget_mspi_config_t;

typedef struct
{
  uint32_t                      NumBytes;               // Number of bytes to read/write.
  uint32_t                      ByteOffset;             // Byte offset in the RX/TX buffers.
  uint32_t                      SectorAddress;          // Sector address.
  uint32_t                      SectorOffset;           // Offset into Sector to write/read.
  bool                          TurnOnCaching;          // Turn on D/I caching.
  uint32_t                      UnscrambledSector;      // Address of the unscrambled sector.
} am_widget_mspi_test_t;

//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_widget_mspi_devices_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
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
#if defined(am_widget_mspi_devices_timing_config_t)
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        am_widget_mspi_devices_config_t *pDevCfg,
                                        am_widget_mspi_devices_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_widget_mspi_devices_timing_config_t *pDevSdrCfg);
#endif
} mspi_device_func_t;

extern mspi_device_func_t mspi_device_func;
typedef struct
{
  uint32_t      XIPBase;
  uint32_t      XIPMMBase;
} MSPIBaseAddr_t;

extern const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES];
extern bool am_widget_mspi_setup(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr);
extern bool am_widget_mspi_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_mspi_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);
extern bool am_widget_mspi_test_get_eflash_id(void *pWidget, void *pTestCfg, char *pErrStr);
#if defined(AMBT53)
extern bool am_widget_mspi_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr, am_hal_mspi_callback_t pfnCallback, void *pCallbackCtxt);
#else
extern bool am_widget_mspi_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
#endif
extern bool am_widget_mspi_test_xip_databus(void *pWidget, void *pTestCfg, char *pErrStr);
extern bool am_widget_mspi_test_xip_instrbus(void *pWidget, void *pTestCfg, char *pErrStr);
#if defined(AM_WIDGET_MSPI_FLASH_DEVICE)
extern bool am_widget_mspi_test_xip_mixedmode(void *pWidget, void *pTestCfg, char *pErrStr);
#endif
extern bool am_widget_mspi_test_scrambling(void *pWidget, void *pTestCfg, char *pErrStr);
extern bool am_widget_mspi_test_write_queue_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern bool am_widget_mspi_test_b2b(void *pWidget, void *pTestCfg, char *pErrStr);

extern void stimer_init(void);
extern void stimer_deinit(void);

extern uint32_t am_widget_mspi_power_save_and_restore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_MSPI_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
