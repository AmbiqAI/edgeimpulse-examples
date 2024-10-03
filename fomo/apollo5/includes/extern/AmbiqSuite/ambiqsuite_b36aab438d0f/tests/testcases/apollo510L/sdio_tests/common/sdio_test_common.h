//*****************************************************************************
//
//! @file sdio_test_common.h
//!
//! @brief SDIO test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef SDIO_TEST_COMMON_H
#define SDIO_TEST_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_sdio.h"
#include "am_util.h"
#include "am_bsp.h"
#include "am_devices_emmc_rpmb.h"

#define FMC_BOARD_EMMC_TEST
#define ENABLE_SDIO_POWER_SAVE

#define ALIGN(x) __attribute__((aligned(1<<x)))

//empirical value, timeout is dependent on clk freq, bit width and transfer size
#if defined (APOLLO5_FPGA)
#define DELAY_MAX_COUNT   100000
#else
#define DELAY_MAX_COUNT   10000
#endif

#if defined(EMMC_DMA_MEMORY_TEST)
#define BLK_NUM 9
#elif  defined(COLLECT_BENCHMARKS)
#define BLK_NUM 128
#else
#define BLK_NUM 64
#endif

#define ERASE_TEST_START_BLK_OFFSET  20
#define ERASE_BLK_NUM                64
#define TRIM_ERASE_BLK_NUM           20
#define ERASE_TIME_OUT               100
#define SECURE_ERASE_TIME_OUT        10000

#define BUF_LEN 512*BLK_NUM
#define DCACHE_SIZE     64*1024

#define IO_VECTOR_NUM     10

#define MEM_OFFSET_SZ       16
#define MEM_TEST_SZ         512

#define XFER_MODE_START_INDEX 0
#define XFER_MODE_END_INDEX   3

#define WIDTH_START_INDEX 0
#define WIDTH_END_INDEX   3

#define SPEED_START_INDEX 1
#define SPEED_END_INDEX   3

#define MODE_START_INDEX  0
#define MODE_END_INDEX    2

#define DATA_ERROR_CHECK_NUM        10
#define SDIO_TEST_PATTERN_NUMBER    5       // 5 patterns

#define SDIO_START_INISTANCE        1
#define SDIO_END_INISTANCE          2
#define SDIO_TEST_INCREACE_LENGTH   20
#define SDIO_TEST_INCREACE_BLK      1
#define SDIO_BYTES_TEST_START_NUM   4

#define SDIO_EMMC_RPMB_BLK_NUM      16

#define RPMB_PRTITION_START_INDEX   0
#define RPMB_PRTITION_END_INDEX     5

#define ERASE_START_TYPE            0
#define ERASE_END_TYPE              6

#define VOLTAGE_START_INDEX  0
#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
#define VOLTAGE_END_INDEX    2
#else
#define VOLTAGE_END_INDEX    1
#endif

#define SDCARD_TEST_MODULE    1
//*****************************************************************************
//
// RS9116 macro definitions
//
//*****************************************************************************
#define RSI_PING_BUFFER_ADDR 0x18000
#define RSI_PONG_BUFFER_ADDR 0x19000

#define RSI_HOST_INTF_REG_OUT     0x4105003C
#define RSI_HOST_INTF_REG_IN      0x41050034
#define RSI_HOST_INTF_STATUS_REG  0x41050004

#define RSI_PONG_AVAIL           'O'
#define RSI_PING_AVAIL           'I'

#define RSI_MIN_CHUNK_SIZE   4096
#define RSI_FW_START_OF_FILE BIT(0)
#define RSI_FW_END_OF_FILE   BIT(1)

#define RSI_SD_MASTER_ACCESS     1<<16

#define RSI_HOST_INTERACT_REG_VALID    (0xAB << 8)
#define RSI_HOST_INTERACT_REG_VALID_FW (0xA0 << 8)

#define RSI_BLOCK_SIZE                256
#define RSI_TEST_BUF_LEN              RSI_BLOCK_SIZE*32     //256*BLK_NUM
#define RSI_TEST_USE_DEFAULT_MODE     1

#define RSI_XFER_MODE_START_INDEX 0
#define RSI_XFER_MODE_END_INDEX   5

#define RSI_WIDTH_START_INDEX     0
#define RSI_WIDTH_END_INDEX       2

#define RSI_BLKCOUNT_START_INDEX     1
#define RSI_BLKCOUNT_END_INDEX       32

#ifdef EMMC_SCATTER_TEST
#define BLK_SIZE          512

extern am_hal_card_iovec_t WriteVec[IO_VECTOR_NUM];
extern am_hal_card_iovec_t ReadVec[IO_VECTOR_NUM];

extern AM_SHARED_RW uint8_t ui8WrBufSSRAM[BUF_LEN];
extern AM_SHARED_RW uint8_t ui8RdBufSSRAM[BUF_LEN];

extern uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
extern uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

extern uint8_t         *pIoVectorWrBuf[IO_VECTOR_NUM];
extern uint8_t         *pIoVectorRdBuf[IO_VECTOR_NUM];
#else
extern AM_SHARED_RW uint8_t ui8RdBuf[BUF_LEN];
extern AM_SHARED_RW uint8_t ui8WrBuf[BUF_LEN];
#endif

#ifdef COLLECT_BENCHMARKS
extern uint32_t g_ui32Count;
#define WAKE_INTERVAL_IN_MS     1
#define TIMER_NUM               0
#define XT_PERIOD               32768

#if defined APOLLO5_FPGA
#define WAKE_INTERVAL           347
#define TIME_DIVIDER            (16/(APOLLO5_FPGA*1.0))    //Chose HFRC_DIV16 as timer clk source when HFRC=8M
#else
#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000)
#define TIME_DIVIDER            (16/96.0)                  //Chose HFRC_DIV16 as timer clk source when HFRC=96M
#endif
#endif

typedef struct
{
  const uint32_t     speed;
  const char         *string;
} sdio_speed_t;

typedef struct
{
  const am_hal_host_bus_width_e width;
  const char                    *string;
} sdio_width_t;

typedef struct
{
  const am_hal_host_uhs_mode_e mode;
  const char                   *string;
} sdio_mode_t;

typedef struct
{
  const am_widget_host_xfer_mode_e mode;
  const char                       *string;
} sdio_xfer_mode_t;

typedef struct
{
  const am_hal_card_pwr_ctrl_policy_e mode;
  const char                       *string;
} sdio_pwr_ctrl_policy_t;

typedef struct
{
  const am_devices_emmc_partiton_access_e partition_mode;
  const char                   *string;
} sdio_partition_mode_t;

typedef struct
{
  const am_hal_card_erase_type_t type;
  const char                   *string;
}sdio_erase_type_t;

typedef struct
{
  const am_hal_host_bus_voltage_e voltage;
  const char                   *string;
} sdio_voltage_t;

extern sdio_speed_t           sdio_test_speeds[];
extern sdio_width_t           sdio_test_widths[];
extern sdio_mode_t            sdio_test_modes[];
extern sdio_xfer_mode_t       sdio_test_xfer_modes[];
extern sdio_pwr_ctrl_policy_t sdio_test_pwrctrl_policies[];
extern sdio_partition_mode_t  sdio_test_partition_modes[];
extern sdio_erase_type_t      sdio_test_erase_types[];
extern sdio_voltage_t         sdio_test_voltages[];
extern uint32_t  DMATCBBuffer[2560];

#ifdef COLLECT_BENCHMARKS
extern sdio_xfer_mode_t sdio_benchmark_xfer_modes[];
#endif

#endif // SDIO_TEST_COMMON_H
