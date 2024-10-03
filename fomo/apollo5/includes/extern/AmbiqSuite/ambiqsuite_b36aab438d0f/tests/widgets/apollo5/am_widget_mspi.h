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

#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032) || defined(CYPRESS_S25FS064S) || \
     defined(DOSILICON_DS35X1GA) || defined(ISSI_IS25WX064) || defined(TOSHIBA_TC58CYG0) || defined(MACRONIX_MX25UM51245G) || \
     defined(WINBOND_W25N02KW))
#define AM_WIDGET_MSPI_FLASH_DEVICE
#elif defined(APS6404L)
#define AM_WIDGET_MSPI_PSRAM_DEVICE
#elif defined(APS12808L)
#define AM_WIDGET_MSPI_DDR_PSRAM_DEVICE
#elif defined(APS25616N)
#define AM_WIDGET_MSPI_HEX_DDR_PSRAM_DEVICE
#elif defined(APS25616BA)
#define AM_WIDGET_MSPI_HEX_DDR_APM_PSRAM_1P2V_DEVICE
#elif defined(W958D6NW)
#define AM_WIDGET_MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE
#elif defined(MB85RS64V)
#define AM_WIDGET_MSPI_FRAM_DEVICE
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
#elif defined(ISSI_IS25WX064)
#include "am_devices_mspi_is25wx064.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_IS25WX064_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_IS25WX064_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_IS25WX064_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_IS25WX064_MAX_SECTORS
#define am_widget_mspi_devices_config_t am_devices_mspi_is25wx064_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_is25wx064_timing_config_t
#elif defined(MACRONIX_MX25UM51245G)
#include "am_devices_mspi_mx25um51245g.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE AM_DEVICES_MSPI_MX25UM51245G_SECTOR_SIZE
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS AM_DEVICES_MSPI_MX25UM51245G_MAX_SECTORS
#define am_widget_mspi_devices_config_t am_devices_mspi_mx25um51245g_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_mx25um51245g_timing_config_t
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
#elif defined(AM_WIDGET_MSPI_HEX_DDR_PSRAM_DEVICE)
#include "am_devices_mspi_psram_aps25616n.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(AM_WIDGET_MSPI_HEX_DDR_APM_PSRAM_1P2V_DEVICE)
#include "am_devices_mspi_psram_aps25616ba_1p2v.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(AM_WIDGET_MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE)
#include "am_devices_mspi_psram_w958d6nw.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_widget_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(AM_WIDGET_MSPI_FRAM_DEVICE)
#include "am_devices_mb85rs64v_mspi.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MB85RS64V_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MB85RS64V_STATUS_ERROR
#define am_widget_mspi_devices_config_t am_devices_mspi_fram_config_t
#elif defined(AMBT52)
#include "am_devices_ambt52.h"
#define AM_WIDGET_MSPI_SUCCESS AM_DEVICES_MSPI_AMBT52_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR   AM_DEVICES_MSPI_AMBT52_STATUS_FAIL
#define am_widget_mspi_devices_config_t am_devices_mspi_ambt52_config_t
#elif defined(DOSILICON_DS35X1GA)
#include "am_devices_mspi_ds35x1ga.h"
#define AM_WIDGET_MSPI_SUCCESS                  AM_DEVICES_MSPI_DS35X1GA_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR                    AM_DEVICES_MSPI_DS35X1GA_STATUS_ERROR
#define AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE    AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE
#define AM_DEVICES_MSPI_NAND_FLASH_MAX_PAGES    AM_DEVICES_MSPI_DS35X1GA_MAX_PAGES
#define am_widget_mspi_devices_config_t         am_devices_mspi_ds35x1ga_config_t
#define am_widget_mspi_devices_timing_config_t  am_devices_mspi_ds35x1ga_sdr_timing_config_t
#elif defined(TOSHIBA_TC58CYG0)
#include "am_devices_mspi_tc58cyg0.h"
#define AM_WIDGET_MSPI_SUCCESS                  AM_DEVICES_MSPI_TC58CYG0_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR                    AM_DEVICES_MSPI_TC58CYG0_STATUS_ERROR
#define AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE    AM_DEVICES_MSPI_TC58CYG0_PAGE_DATA_SIZE
#define AM_DEVICES_MSPI_NAND_FLASH_MAX_PAGES    AM_DEVICES_MSPI_TC58CYG0_MAX_PAGES
#define am_widget_mspi_devices_config_t         am_devices_mspi_tc58cyg0_config_t
#define am_widget_mspi_devices_timing_config_t  am_devices_mspi_tc58cyg0_sdr_timing_config_t
#elif defined(WINBOND_W25N02KW)
#include "am_devices_mspi_w25n02kw.h"
#define AM_WIDGET_MSPI_SUCCESS                  AM_DEVICES_MSPI_W25N02KW_STATUS_SUCCESS
#define AM_WIDGET_MSPI_ERROR                    AM_DEVICES_MSPI_W25N02KW_STATUS_ERROR
#define AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE    AM_DEVICES_MSPI_W25N02KW_PAGE_DATA_SIZE
#define AM_DEVICES_MSPI_NAND_FLASH_MAX_PAGES    AM_DEVICES_MSPI_W25N02KW_MAX_PAGES
#define am_widget_mspi_devices_config_t         am_devices_mspi_w25n02kw_config_t
#define am_widget_mspi_devices_timing_config_t  am_devices_mspi_w25n02kw_sdr_timing_config_t
#endif

#define AM_WIDGET_MSPI_TIMEOUT  1000000

#ifndef NO_PWR_TEST

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

#endif

typedef enum
{
  AM_WIDGET_MSPI_FLASH_ATXP032,
  AM_WIDGET_MSPI_FLASH_IS25WX064,
  AM_WIDGET_MSPI_FLASH_MX25UM51245G,
  AM_WIDGET_MSPI_PSRAM_APS6404L,
  AM_WIDGET_MSPI_PSRAM_APS6408L,
  AM_WIDGET_MSPI_DDR_PSRAM_APS12808L,
  AM_WIDGET_MSPI_DDR_PSRAM_APS25616N,
  AM_WIDGET_MSPI_DDR_PSRAM_APS25616BA,
  AM_WIDGET_MSPI_DDR_PSRAM_W958D6NW,
} am_widget_mspi_device_model_e;

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_mspi_status_t;

typedef struct
{
  uint32_t                          ui32Module;             // MSPI instance.
  am_widget_mspi_device_model_e     eDeviceModel;           // Device Model.
  am_widget_mspi_devices_config_t   MSPIConfig;             // MSPI configuration.
  void                              *pHandle;               // MSPI instance handle.
  void                              *pDevHandle;            // MSPI device handle.
  uint32_t                          ui32DMACtrlBufferSize;  // DMA control buffer size.
  uint32_t                          *pDMACtrlBuffer;        // DMA control buffer.

  am_hal_mspi_timing_scan_t         sTimingResult;
} am_widget_mspi_config_t;

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

typedef enum
{
  AM_WIDGET_MSPI_PIO_ACCESS,
  AM_WIDGET_MSPI_DMA_ACCESS,
  AM_WIDGET_MSPI_XIPMM_ACCESS_32B,
  AM_WIDGET_MSPI_XIPMM_ACCESS_16B,
  AM_WIDGET_MSPI_XIPMM_ACCESS_8B,
  AM_WIDGET_MSPI_MEMCPY_XIPMM_ACCESS,
  AM_WIDGET_MSPI_ACCESS_MAX,
}am_widget_mspi_access_e;

typedef struct
{
  bool                          bMemContinous;
  float                         f32WriteBandwidth;
  float                         f32ReadBandwidth;
}am_widget_mspi_bandwidth_t;

typedef struct
{
  bool         bScanTxNeg;
  uint32_t     ui32ScanTxNegStart;
  uint32_t     ui32ScanTxNegEnd;

  bool         bScanRxNeg;
  uint32_t     ui32ScanRxNegStart;
  uint32_t     ui32ScanRxNegEnd;

  bool         bScanRxCap;
  uint32_t     ui32ScanRxCapStart;
  uint32_t     ui32ScanRxCapEnd;

  bool         bScanTurnAround;
  uint32_t     i32ScanTurnAroundStart;
  uint32_t     i32ScanTurnAroundEnd;

  bool         bScanTxDQSDelay;
  uint32_t     ui32ScanTxDQSDelayStart;
  uint32_t     ui32ScanTxDQSDelayEnd;

  bool         bScanRxDQSDelay;
  uint32_t     ui32ScanRxDQSDelayStart;
  uint32_t     ui32ScanRxDQSDelayEnd;

  uint32_t     ui32SectorAddress;
  am_widget_mspi_access_e   eMSPIAccess;
  uint32_t     ui32TimingScanMinAcceptLen;
}am_widget_mspi_timing_scan_t;


#if defined(DOSILICON_DS35X1GA) || defined(TOSHIBA_TC58CYG0) || defined(WINBOND_W25N02KW)
typedef struct
{
  uint32_t                      NumBytes;               // Number of bytes to read/write.
  uint32_t                      ByteOffset;             // Byte offset in the RX/TX buffers.
  uint32_t                      PageNum;                // Page Number.
  uint32_t                      PageOffset;             // Offset into Page to write/read.
} am_widget_mspi_test_t;
#else
typedef struct
{
  uint32_t                      NumBytes;               // Number of bytes to read/write.
  uint32_t                      ByteOffset;             // Byte offset in the RX/TX buffers.
  uint32_t                      SectorAddress;          // Sector address.
  uint32_t                      SectorOffset;           // Offset into Sector to write/read.
  bool                          TurnOnCaching;          // Turn on D/I caching.
  uint32_t                      UnscrambledSector;      // Address of the unscrambled sector.
  uint32_t                      ReadBlockSize;          // Number of bytes to read in each block.
  mspi_xipmm_access_e           eXIPMMAccess;
  uint8_t *                     pTxBuffer;
  uint8_t *                     pRxBuffer;
  am_widget_mspi_bandwidth_t    bandwidth;

  am_widget_mspi_access_e       eMSPIAccess;

} am_widget_mspi_test_t;
#endif

typedef struct
{
  uint32_t                    ui32NumBytes;                // Number of bytes to read/write.
  uint32_t                    ui32ScramblingStartAddr;     // Scrambling Start Address
  uint32_t                    ui32ScramblingEndAddr;       // Scrambling End Address
  uint32_t                    ui32APStartAddr;             // Aperture Base Address
  am_hal_mspi_ap_e            eAPMode;                     // Aperture Mode
  am_hal_mspi_ap_size_e       eAPSize;                     // Aperature Size
  bool                        bScramble;
}am_widget_mspi_xip_range_test_t;

typedef uint32_t (*mspi_xip_test_function_t)(uint32_t, uint32_t, uint32_t);

typedef struct
{
    uint32_t binAddr;
    uint32_t funcOffset;
    uint32_t binSize;
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t result;
} mspi_xip_test_funcinfo_t;

//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[30];

    uint32_t (*mspi_init)(uint32_t ui32Module, ...);
    uint32_t (*mspi_term)(void *pHandle, ...);

    uint32_t (*mspi_read_id)(void *pHandle, ...);

    uint32_t (*mspi_read)(void *pHandle, ...);

    uint32_t (*mspi_read_adv)(void *pHandle, ...);

    uint32_t (*mspi_read_callback)(void *pHandle, ...);

    uint32_t (*mspi_write)(void *pHandle, ...);

    uint32_t (*mspi_write_adv)(void *pHandle, ...);

    uint32_t (*mspi_mass_erase)(void *pHandle);
    uint32_t (*mspi_sector_erase)(void *pHandle, ...);
    uint32_t (*mspi_block_erase)(void *pHandle, ...);

    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);
    uint32_t (*mspi_xip_config)(void *pHandle, ...);
    uint32_t (*mspi_scrambling_enable)(void *pHandle);
    uint32_t (*mspi_scrambling_disable)(void *pHandle);
#if defined(am_widget_mspi_devices_timing_config_t)
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module, ...);
    uint32_t (*mspi_init_timing_apply)(void *pHandle, ...);
#endif
} mspi_device_func_t;

extern mspi_device_func_t mspi_device_func;
typedef struct
{
  uint32_t      XIPBase;
  uint32_t      XIPMMBase;
  uint32_t      XIPMMEnd;
} MSPIBaseAddr_t;

extern const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES];
extern bool am_widget_mspi_setup(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr);
extern bool am_widget_mspi_init(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr);
extern bool am_widget_mspi_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_mspi_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);
extern bool am_widget_mspi_test_get_eflash_id(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_mspi_nand_flash_bad_block_check(void *pWidget, void *pTestCfg, char *pErrStr);
#if defined(AMBT52)
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
extern bool am_widget_mspi_test_xipmm(void *pWidget, void *pTestCfg, char *pErrStr);

extern bool am_widget_mspi_test_write_read_bandwidth(void *pWidget, void *pTestCfg, char *pErrStr);
extern bool am_widget_mspi_xipmm_bandwidth(void *pWidget, void *pTestCfg, char *pErrStr);

extern bool am_widget_mspi_xip_range_test(void *pWidget, void *pTestCfg, char *pErrStr);

extern bool am_widget_mspi_timing_scan(void *pWidget, void *pTestCfg, char *pErrStr);

extern bool am_widget_mspi_write_read(void *pWidget, void *pTestCfg, char *pErrStr);

extern void am_widget_mspi_global_teardown(void);

extern void stimer_init(void);
extern void stimer_deinit(void);

extern uint32_t am_widget_mspi_power_save_and_restore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);

#if defined (AM_PART_APOLLO510L)
extern AM_SHARED_RW uint8_t                 g_SectorTXBuffer[512*1024];      // 512K SRAM TX buffer
extern AM_SHARED_RW uint8_t                 g_SectorRXBuffer[512*1024];      // 512K SRAM RX buffer.
#else
extern AM_SHARED_RW uint8_t                 g_SectorTXBuffer[1024*1024];      // 1M SRAM TX buffer
extern AM_SHARED_RW uint8_t                 g_SectorRXBuffer[1024*1024];      // 1M SRAM RX buffer.
#endif

#if defined(AM_WIDGET_MSPI_FAST_SCAN)
extern bool bFirst;
#endif

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
