//*****************************************************************************
//
//! @file nemagfx_test_common.h
//!
//! @brief NemaGFX test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef NEMAGFX_TEST_COMMON_H
#define NEMAGFX_TEST_COMMON_H

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
// NemaSDK include files.
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

#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_font.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_context.h"

#include "unity.h"

//*****************************************************************************
//
// Devices.
//
//*****************************************************************************
#include "am_devices_dc_dsi_raydium.h"
#include "am_devices_dc_xspi_raydium.h"
#include "am_devices_mspi_psram_aps25616n.h"

#ifdef NEMA_USE_CUSTOM_MALLOC
#include "simple_malloc.h"

/*Size of the memory available for PSRAM heap in bytes (>= 2kB)*/
#  define LV_MEM_TCM_SIZE (0x40000)          /*[bytes]*/
/*Set an address for the TCM heap.*/
#  define LV_MEM_TCM_ADR (0x20040000UL)   /*0: unused*/

/*Size of the memory available for SSRAM heap in bytes*/
#  define LV_MEM_SSRAM_SIZE (0x100000)          /*[bytes]*/
/*Set an address for the SSRAM heap.*/
#  define LV_MEM_SSRAM_ADR (0x20080000UL + 64)     /*0: unused*/

#  define LV_MEM_PSRAM_SIZE (0x200000)          /*[bytes]*/
/*Set an address for the SSRAM heap.*/
#  define LV_MEM_PSRAM_ADR (0x60000000UL)     /*0: unused*/

#endif

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
#ifdef USE_PSRAM
    #define MSPI_PSRAM_MODULE               0

    #if (MSPI_PSRAM_MODULE == 0)
    #define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
    #elif (MSPI_PSRAM_MODULE == 1)
    #define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
    #elif (MSPI_PSRAM_MODULE == 2)
    #define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
    #endif // #if (MSPI_PSRAM_MODULE == 0)

    #define PSRAM_ISR_PRIORITY              (4)
    extern am_devices_mspi_psram_config_t g_sMspiPsramConfig;
#endif

extern void globalSetUp(void);
extern void globalTearDown(void);

#ifdef USE_NEMADC
extern uint32_t am_nemadc_read(uint32_t eCmd, uint32_t ui32Length);
extern uint32_t dc_common_interface(uint16_t ui16ResX,uint16_t ui16ResY);
#endif

#endif // NEMAGFX_TEST_COMMON_H
