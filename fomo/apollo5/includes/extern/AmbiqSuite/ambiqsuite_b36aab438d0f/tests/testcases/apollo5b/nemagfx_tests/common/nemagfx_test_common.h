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
// NemaSDK.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_programHW.h"
#include "nema_regs.h"
#include "nema_cmdlist.h"
#include "nema_error.h"
#include "nema_font.h"

#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_font.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_context.h"

//*****************************************************************************
//
// Test suite.
//
//*****************************************************************************
#include "unity.h"

//*****************************************************************************
//
// Devices.
//
//*****************************************************************************
#ifdef USE_DISPLAY
    #include "am_devices_display_generic.h"
#endif

#ifdef USE_PSRAM
    #include "am_devices_mspi_psram_aps25616n.h"
#endif

//*****************************************************************************
//
// Customized malloc.
//
//*****************************************************************************
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

//*****************************************************************************
//
// PSRAM Address.
//
//*****************************************************************************
#ifdef USE_PSRAM
    #define MSPI_PSRAM_MODULE               0

    #if (MSPI_PSRAM_MODULE == 0)
    #define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
    #elif (MSPI_PSRAM_MODULE == 1)
    #define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
    #elif (MSPI_PSRAM_MODULE == 2)
    #define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
    #endif // #if (MSPI_PSRAM_MODULE == 0)
	extern am_devices_mspi_psram_config_t g_sMspiPsramConfig;
#endif

//*****************************************************************************
//
// setup and tear down operation.
//
//*****************************************************************************
extern void globalSetUp(void);
extern void globalTearDown(void);

#endif // NEMAGFX_TEST_COMMON_H
