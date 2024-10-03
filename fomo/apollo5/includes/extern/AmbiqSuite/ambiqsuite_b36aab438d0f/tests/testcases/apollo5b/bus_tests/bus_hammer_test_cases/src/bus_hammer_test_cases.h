//*****************************************************************************
//
//! @file bus_hammer_test_cases.h
//!
//! @brief Constants and includes for the AXI bus hammer test
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef BUS_HAMMER_TEST_CASES_H
#define BUS_HAMMER_TEST_CASES_H

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
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_programHW.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#include "nema_dc_regs.h"
#include "nema_regs.h"
#include "nema_cmdlist.h"
#include "nema_error.h"

#include "unity.h"

//*****************************************************************************
//
// Devices.
//
//*****************************************************************************
#include "am_devices_dc_dsi_raydium.h"
#include "am_devices_dc_xspi_raydium.h"

//*****************************************************************************
//
// Local includes
//
//*****************************************************************************

#include "mspi.h"
#include "patterns.h"
#include "targets.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

// *** test configuration ***

#define LOG_INTERVAL 100 // print log message every LOG_INTERVAL transfers
#define PEAK_SSRAM_ITERATIONS 5000 // number of loops through all permutations for peak SSRAM hammer
#define SSRAM_HAMMER_MULTI_BLOCK_SINGLE_BANK_ITERATIONS 5000 // number of loops through all permutations for SSRAM multi/single
#define SSRAM_HAMMER_MULTI_BLOCK_MULTI_BANK_ITERATIONS 2 // number of loops through all permutations for SSRAM multi/multi
#define MSPI_HAMMER_ITERATIONS 5000 // number of loops through all permutations for MSPI hammer
#define ALL_HAMMER_ITERATIONS 2 // number of loops through all permutations for MSPI hammer

// Uncomment test case names below to exclude from the suite
//#define SKIP_PEAK_SSRAM_TEST_CASES
//#define SKIP_SSRAM_HAMMER_MULTI_BLOCK_SINGLE_BANK_TEST_CASES
//#define SKIP_SSRAM_HAMMER_MULTI_BLOCK_MULTI_BANK_TEST_CASES
//#define SKIP_MSPI_HAMMER_TEST_CASES
//#define SKIP_ALL_HAMMER_TEST_CASES

// *** end test configuration ***

// NOTE: changing the FB size or the fill pattern will change the display CRC
#define DISP_CRC    0x987633C9


#define GFX_BLEND_CYCLES 16 // # of clear B / blend A->B / clear A / blend B->A cycles per xfer

#define GFX_BYTES_PER_BLEND (TEX_RESX * TEX_RESY * 4 /*RGBA8888*/ * 3 /*read src + read/modify/write dest*/)
#define GFX_BYTES_PER_CLEAR (TEX_RESX * TEX_RESY * 4)
#define GFX_BYTES_PER_ISSUE ((GFX_BLEND_CYCLES * (GFX_BYTES_PER_BLEND + GFX_BYTES_PER_CLEAR) * 2 /*A->B then B->A*/) + CL_SIZE)
#define DISP_BYTES_PER_ISSUE (FB_RESX * FB_RESY * 4 /*RGBA8888*/)

// Burst size config for GPU source (TEX) and destination (FB) operations
// This is max 64 on revA and 128 on revB
#define FB_BURST_SIZE   GPU_BURST_SIZE_64
#define TEX_BURST_SIZE  GPU_BURST_SIZE_64

//
// Maximum read frequency of RM67162 is about 3MHz
//
#define TARGET_FREQ 3
//
// Format clock frequency is half of DISPCLK.
//
#define FORMAT_CLK_DIV 2
//
// Assume DISPCLK is 48MHz or 96MHz on actual silicon, the proper division ratio is 16.
//
#define SILICON_FORMAT_DIV (16U)
//
// DC option clock divider position
//
#define DC_DIV2_POS 27

// 2MB aligned to 512k spans all 3 SSRAMs with at least 2 128k banks in each
#define SSRAM_POOL_SIZE (1024*2048)

#endif // BUS_HAMMER_TEST_CASES_H