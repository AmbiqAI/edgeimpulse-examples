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
#include "am_util.h"
#include "am_bsp.h"

#define FMC_BOARD_EMMC_TEST
#define ENABLE_SDIO_POWER_SAVE

#define ALIGN(x) __attribute__((aligned(1<<x)))

#define DELAY_MAX_COUNT   1000

#define WIDTH_START_INDEX 0
#define WIDTH_END_INDEX   3

#define SPEED_START_INDEX 0
#define SPEED_END_INDEX   6

#define MODE_START_INDEX  0
#define MODE_END_INDEX    1 // Don't run DDR tests.  Feature has been deprecated.


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



#endif // SDIO_TEST_COMMON_H
