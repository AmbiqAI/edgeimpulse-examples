//*****************************************************************************
//
//! @file prefetch_bench_test_cases.h
//!
//! @brief Test cases to measure prefetch performance for SSRAM and MSPI XIPMM
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef PREFETCH_BENCH_TEST_CASES_H
#define PREFETCH_BENCH_TEST_CASES_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_hal_global.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"

//*****************************************************************************
//
// Third-party/Devices include files.
//
//*****************************************************************************

#include "unity.h"
#include "am_devices_mspi_psram_aps25616n.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define BUF_ALIGN 1024*64 // 16k buffer alignment
#define BUF_SIZE 32768 // 32k buffer, needs to be smaller than Apollo5 D$
#define LOOPS 1000 // read loops to average over

#define TIMER_NUM 0 // CTIMER to use for measurement

// NOTE: redefine on FPGA if HFRC is < 96M!
#define TIMER_TICKS_PER_SEC 24000000 // 96M HFRC / 4x prescaler


//
// PSRAM address
//
#define MSPI_PSRAM_MODULE               0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

#define PSRAM_ISR_PRIORITY              (4)
#define MSPI_PSRAM_SIZE                 (0x800000)


#endif // PREFETCH_BENCH_TEST_CASES_H
