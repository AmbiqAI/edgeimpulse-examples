//*****************************************************************************
//
//! @file am_widget_sdio.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_WIDGET_SDIO_H
#define AM_WIDGET_SDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_devices_emmc_rpmb.h"

#define DUT_NUM   2
#define SCAN_NUM  4

typedef enum
{
    AM_WIDGET_SDIO_SUCCESS,
    AM_WIDGET_SDIO_ERROR
} am_widget_sdio_status_e;

typedef enum
{
  AM_WIDGET_SDIO_96M,
  AM_WIDGET_SDIO_48M,
  AM_WIDGET_SDIO_24M,
  AM_WIDGET_SDIO_12M,
  AM_WIDGET_SDIO_3M,
  AM_WIDGET_SDIO_750K,
  AM_WIDGET_SDIO_375K,
}am_widget_sdio_clock_e;

typedef enum
{
  AM_WIDGET_HOST_XFER_PIO_SYNC,
  AM_WIDGET_HOST_XFER_ADMA_SYNC,
  AM_WIDGET_HOST_XFER_ADMA_ASYNC,
  AM_WIDGET_HOST_XFER_SDMA_SYNC,
  AM_WIDGET_HOST_XFER_SDMA_ASYNC,
} am_widget_host_xfer_mode_e;

typedef struct
{
  uint32_t ui32TxDelay;
  uint32_t ui32RxDelay;
  bool     bvalid;
  bool     bcalibrate;
}am_widget_timing_scan_t;

typedef struct
{
  uint32_t key;
  am_widget_timing_scan_t value;
}am_widget_timing_scan_hashtable_t;

//*****************************************************************************
// eMMC Card definitions
//*****************************************************************************
#define EMMC_NUM  3
typedef struct
{
  const char          *manufacturer;
  uint8_t             manufacturerID;
  char                productName[6];
  uint8_t             productRevision;
  uint8_t             OEM;
  uint32_t            emmc_csize;
  uint32_t            max_enh_size_mult;
  uint32_t            sector_count;
}am_widget_emmc_device_info_t;

typedef struct
{
  am_hal_card_pwr_ctrl_policy_e   eCardPwrCtrlPolicy;
  am_hal_card_pwr_ctrl_func       pCardPwrCtrlFunc;
  volatile bool                   bAsyncWriteIsDone;
  volatile bool                   bAsyncReadIsDone;
}am_widget_emmc_devices_config_t;

//*****************************************************************************
// SDIO Card definitions
//*****************************************************************************
#if defined(WIFI_DEVICE_RX9116)
//#define am_widget_sdio_devices_config_t   //add device config here from wifi device driver
#endif 

typedef struct
{
  am_hal_card_pwr_ctrl_policy_e   eCardPwrCtrlPolicy;
  am_hal_card_pwr_ctrl_func       pCardPwrCtrlFunc;
  volatile bool                   bAsyncWriteIsDone;
  volatile bool                   bAsyncReadIsDone;
}am_widget_sdio_devices_config_t;
typedef struct
{
  am_hal_card_host_t                *pSdhcHost;             // SDIO host handle.
  void                              *pDevHandle;            // SDIO device handle.
} am_widget_sdio_config_t;

//*****************************************************************************
// global test definition
//*****************************************************************************
typedef struct
{
  //host config
  am_hal_host_inst_index_e    eHost;
  am_widget_host_xfer_mode_e  eXferMode;
  am_widget_sdio_clock_e      eClock;
  am_hal_host_bus_width_e     eBusWidth;
  am_hal_host_bus_voltage_e   eBusVoltage;
  am_hal_host_uhs_mode_e      eUHSMode;
  am_hal_card_erase_type_t    eEraseType;
  am_widget_timing_scan_t     *sTimingParam;

  //device config
  am_hal_card_type_e          eCardType;
  void                        *device_config;

  //test config
  uint32_t ui32StartBlock;
  uint32_t ui32BlockCount;
  uint32_t ui32SectorCount;
  uint32_t ui32RpmbCount;
  uint32_t ui32RpmbBlkCnt;
  uint32_t ui32IoVectorCnt;
} am_widget_sdio_test_t;

extern am_widget_sdio_config_t g_DUTs[AM_HAL_CARD_HOST_NUM];
extern am_widget_sdio_test_t   DUTs_setting[DUT_NUM];
extern am_widget_emmc_device_info_t emmcs_info[EMMC_NUM];

extern bool am_widget_check_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len);

extern bool am_widget_sdio_setup(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDIOTestConfig);
extern bool am_widget_sdio_cleanup(am_widget_sdio_config_t *pHandles);

extern void am_widget_host0_event_cb(am_hal_host_evt_t *pEvt);
extern void am_widget_host1_event_cb(am_hal_host_evt_t *pEvt);

extern void am_widget_prepare_testdata(uint8_t *pui8WrBuf, uint32_t ui32Len, uint32_t ui32Seed);

extern void am_widget_setupTimingScanTable(am_widget_timing_scan_hashtable_t *table);

extern am_widget_timing_scan_t *am_widget_findTimingParam(am_widget_timing_scan_hashtable_t *table, am_widget_sdio_test_t *pSDHCTestConfig);

extern uint32_t am_widget_get_emmc_identifier(am_hal_card_t *pCard);

extern bool am_widget_sdio_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *ui8WriteBuf, uint8_t *ui8ReadBuf);
extern bool am_widget_sdio_benchmark_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf);

bool am_widget_emmc_card_init(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig);
bool am_widget_sdio_card_init(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig);
bool am_widget_sd_card_init(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig);

bool am_widget_sdio_rsi_read(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8RdBuf);
bool am_widget_sdio_rsi_write(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8WrBuf);
bool am_widget_sdio_rsi_single_write(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint8_t ui8Data);
bool am_widget_sdio_rsi_single_read(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint8_t *pui8Data);
bool am_widget_prepare_data_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len);
extern char *dummy_key_hash;
extern bool am_widget_check_key(am_hal_card_t *pCard);
extern bool am_widget_rpmb_partition_switch(am_hal_card_t *pCard, am_devices_emmc_partiton_access_e ePartionMode);
extern bool am_widget_rpmb_test_write_read(am_widget_sdio_config_t *pHandles, am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf);
extern bool am_widget_emmc_test_erase(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf);

uint32_t am_widget_sd_card_power_config(am_hal_card_pwr_e eCardPwr);
extern bool am_widget_sd_card_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf);
extern bool am_widget_sd_card_benchmark_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf);
extern bool am_widget_emmc_scatter_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, am_hal_card_iovec_t *pWriteVec, am_hal_card_iovec_t *pReadVec, uint32_t ui32IoVectorCount);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_SDIO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
