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

#define ARM_MATH_CM4
#include <arm_math.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_sdio.h"
#include "am_util.h"
#include "am_bsp.h"

#ifndef  SDCARD_TEST
#define FMC_BOARD_EMMC_TEST
#endif

#define ENABLE_SDIO_POWER_SAVE

#define ALIGN(x) __attribute__((aligned(1<<x)))

#define DELAY_MAX_COUNT   10000  // was 1000, need more time for async reads/writes

// #### INTERNAL BEGIN ####
//
// bumped up the volume of data being written and read
// first is original
// In addition COLLECT_BENCHMARKS is defined if the user
// wishes to collect write and read benchmarks for both PIO
// and SDIO operations
//

#if 0
#define BLK_NUM 64//128
#else
#define BLK_NUM 256
#endif

#define COLLECT_BENCHMARKS

// #### INTERNAL END ####

#define BUF_LEN 512*BLK_NUM

#define WIDTH_START_INDEX 0
#ifdef  SDCARD_TEST
#define WIDTH_END_INDEX   2
#else
#define WIDTH_END_INDEX   3
#endif
#define N_WIDTHS          (WIDTH_END_INDEX - WIDTH_START_INDEX + 1)

#define SPEED_START_INDEX 0
#define SPEED_END_INDEX   6
#define N_SPEEDS          (SPEED_END_INDEX - SPEED_START_INDEX + 1)

#define MODE_START_INDEX  0
#define MODE_END_INDEX    1 // Don't run DDR tests.  Feature has been deprecated.
#define N_MODES           (MODE_END_INDEX - MODE_START_INDEX + 1)

#define N_BENCHMARKS    N_WIDTHS*N_SPEEDS*N_MODES*3

#define VOLTAGE_START_INDEX  0
#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
#define VOLTAGE_END_INDEX    2
#else
#define VOLTAGE_END_INDEX    1
#endif

// at the moment results under 1ms are not reliable.
#define WAKE_INTERVAL_IN_MS     1  
                      
#define XT_PERIOD               32768

#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000)


extern uint8_t ui8RdBuf[BUF_LEN] AM_BIT_ALIGNED(128);
extern uint8_t ui8WrBuf[BUF_LEN] AM_BIT_ALIGNED(128);

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

#ifdef COLLECT_BENCHMARKS

// Benchmark test criteria for synchronous data is:
// Synchronous Read Speed (MBPS) > ((Speed_MHz * Bits) / BENCHMARKS_CONSTANT)
// Measured benchmark data has shown a typical constant of ~8.33.  
// For pass/fail criteria, we increase this to allow for a generous margin of error
// (since there is no established criteria or characterization data, we just
// want to verify that we're in the ballpark of previous measurements)
#define BENCHMARKS_CONSTANT 11.0 // increase this value to loosen pass/fail criteria

typedef enum
{
    eRW_PIO = 0,
    eRW_SYNC,
    eRW_ASYNC
} sdio_rw_type_e;

typedef struct
{
    bool            bIsValid;
    sdio_rw_type_e  eRWType;
    sdio_speed_t*   psSpeed;
    sdio_width_t*   psWidth;
    sdio_mode_t*    psMode;
    uint32_t        ui32WriteMs;
    uint32_t        ui32WriteVolume;
    uint32_t        ui32ReadMs;
    uint32_t        ui32ReadVolume;  
    float32_t       f32WriteMBPS;
    float32_t       f32ReadMBPS;
} sdio_benchmark_data_t;

extern sdio_benchmark_data_t g_BenchmarkData[N_BENCHMARKS]; 

void init_benchmark_data(void);
void store_benchmark_data(sdio_rw_type_e eRWType, sdio_speed_t* psSpeed, sdio_width_t* psWidth, sdio_mode_t* psMode, uint32_t ui32WriteMs, uint32_t ui32WriteVolume, uint32_t ui32ReadMs, uint32_t ui32ReadVolume);
void display_benchmark_data(sdio_rw_type_e eRWType);
#endif

#endif // SDIO_TEST_COMMON_H
