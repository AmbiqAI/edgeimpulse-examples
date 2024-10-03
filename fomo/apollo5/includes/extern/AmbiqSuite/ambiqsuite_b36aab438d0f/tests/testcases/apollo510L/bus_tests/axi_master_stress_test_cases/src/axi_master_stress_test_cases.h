//*****************************************************************************
//
//! @file axi_master_stress_test_cases.h
//!
//! @brief Constants and includes for the AXI master stress test
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AXI_MASTER_STRESS_TEST_CASES_H
#define AXI_MASTER_STRESS_TEST_CASES_H

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

#include "unity.h"

//*****************************************************************************
//
// Devices.
//
//*****************************************************************************
#include "am_devices_dc_dsi_raydium.h"
#include "am_devices_dc_xspi_raydium.h"
#include "am_devices_mspi_psram_aps25616n.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define STRESS_TEST_TRANSFERS 32768 // number of transfer cycles to run across all interfaces
#define LOG_INTERVAL 1000 // print log message every LOG_INTERVAL transfers

// NOTE: changing the FB size or the fill pattern will change the display CRC
#define DISP_CRC    0x987633C9
#define FB_RESX     64 // corresponds to lines of 256 bytes of RGBA8888
#define FB_RESY     64  // lines transferred per test - 16k
#define TEX_RESX    256 // corresponds to lines of 1024 bytes of RGBA8888
#define TEX_RESY    16  // lines transferred per test - 16k
#define CL_SIZE     16384 // size of command list - 16k
#define DMA_SIZE    16384 // size of DMA buffer - 16k
#define CM55_SIZE   16384 // size of CM55 buffer - 16k

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

#endif // AXI_MASTER_STRESS_TEST_CASES_H