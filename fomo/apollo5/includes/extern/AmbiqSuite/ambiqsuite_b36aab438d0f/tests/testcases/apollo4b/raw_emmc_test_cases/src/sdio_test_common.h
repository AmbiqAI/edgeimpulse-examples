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
// DaveC removal #include "am_widget_sdio.h"
#include "am_util.h"
#include "am_bsp.h"

#define FMC_BOARD_EMMC_TEST
#define ENABLE_SDIO_POWER_SAVE

#define ALIGN(x) __attribute__((aligned(1<<x)))

#define DELAY_MAX_COUNT   1000

#if 1
#define BLK_NUM 64 //128
#define BUF_LEN 512*BLK_NUM
#endif

#define WIDTH_START_INDEX 0
#define WIDTH_END_INDEX   3

#define SPEED_START_INDEX 0
#define SPEED_END_INDEX   6

#define MODE_START_INDEX  0
#define MODE_END_INDEX    2

#define SDR_4bit          0
#define SDR_8bit          1
#define DDR_4bit          2
#define DDR_8bit          3

#if 0  // DaveC removed
extern uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
extern uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);
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

extern sdio_speed_t sdio_test_speeds[];
extern sdio_width_t sdio_test_widths[];
extern sdio_mode_t  sdio_test_modes[];

#if 0  // YD removed
uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);  // DaveC moved heres
uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);
#endif 

// YD removed uint32_t g_ui32Count = 0;

#endif // SDIO_TEST_COMMON_H
