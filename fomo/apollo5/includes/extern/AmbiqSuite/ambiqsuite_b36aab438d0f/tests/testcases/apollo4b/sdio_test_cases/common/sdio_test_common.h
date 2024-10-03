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

#ifndef SDCARD_TEST
#define FMC_BOARD_EMMC_TEST
#endif

#define ENABLE_SDIO_POWER_SAVE

#define ALIGN(x) __attribute__((aligned(1<<x)))

#define DELAY_MAX_COUNT   10000

#define BLK_NUM 64//128
#define BUF_LEN 512*BLK_NUM

#define WIDTH_START_INDEX 0
#ifdef SDCARD_TEST
#define WIDTH_END_INDEX   2
#else
#define WIDTH_END_INDEX   3
#endif

#define SPEED_START_INDEX 0
#define SPEED_END_INDEX   6

#define MODE_START_INDEX  0
#ifdef FMC_BOARD_EMMC_TEST
#define MODE_END_INDEX    2
#else
#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
#define MODE_END_INDEX    2
#else
#define MODE_END_INDEX    1
#endif
#endif

#define VOLTAGE_START_INDEX  0
#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
#define VOLTAGE_END_INDEX    2
#else
#define VOLTAGE_END_INDEX    1
#endif

#define SDIO_TEST_PATTERN_NUMBER    5       // 5 patterns
#define SDIO_EMMC_RPMB_BLK_NUM      16

#define RPMB_PRTITION_START_INDEX   0
#define RPMB_PRTITION_END_INDEX     5

extern uint8_t ui8RdBuf[BUF_LEN] AM_BIT_ALIGNED(128);
extern uint8_t ui8WrBuf[BUF_LEN] AM_BIT_ALIGNED(128);

#ifdef EMMC_SCATTER_TEST
#define IO_VECTOR_NUM     16
#define BLK_SIZE          512

extern am_hal_card_iovec_t WriteVec[IO_VECTOR_NUM];
extern am_hal_card_iovec_t ReadVec[IO_VECTOR_NUM];

extern AM_SHARED_RW uint8_t ui8WrBufSSRAM[BUF_LEN];
extern AM_SHARED_RW uint8_t ui8RdBufSSRAM[BUF_LEN];

extern uint8_t         *pIoVectorWrBuf[IO_VECTOR_NUM];
extern uint8_t         *pIoVectorRdBuf[IO_VECTOR_NUM];
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
  const am_hal_host_bus_voltage_e voltage;
  const char                   *string;
} sdio_voltage_t;

extern sdio_speed_t sdio_test_speeds[];
extern sdio_width_t sdio_test_widths[];
extern sdio_mode_t  sdio_test_modes[];
extern sdio_voltage_t sdio_test_voltages[];

#endif // SDIO_TEST_COMMON_H
