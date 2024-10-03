//*****************************************************************************
//
//! @file nemagfx_vg_test.h
//!
//! @brief Global includes for the nemagfx_vg_paint.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_VG_TEST_H
#define NEMAGFX_VG_TEST_H

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

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_error.h"
#include "nema_raster.h"
#include "nema_programHW.h"
#include "nema_matrix3x3.h"

#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"
#include "nema_vg_paint.h"
#include "nema_vg_context.h"

#include "am_devices_dc_xspi_raydium.h"
#include "am_devices_dc_dsi_raydium.h"

#include "am_devices_mspi_psram_aps25616n.h"

//*****************************************************************************
//
// Avaliable demos.
//
//*****************************************************************************
//#define RUN_MASKING_EXAMPLE
//#define RUN_PAINT_EXAMPLE
//#define RUN_RENDER_VG_FONT
//#define RUN_SHAPE
//#define RUN_TEXT_TRANSFORMATION
//#define RUN_PAINT_LUT_EXAMPLE
//#define RUN_TSVG_BENCHMARK
//#define RUN_TSVG_MEASURE
//#define RUN_TSVG_RENDER_EXAMPLE
//#define RUN_RASTER_ARC
//#define RUN_RENDER_RAW_TTF
#define RUN_JOINS_CAPS

//*****************************************************************************
//
// PSRAM address.
//
//*****************************************************************************
#define MSPI_PSRAM_MODULE               0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

extern int display_setup(uint32_t fb_w, uint32_t fb_h);
extern void display_refresh_start(void);
extern void display_refresh_end(void);

#endif // NEMAGFX_VG_PAINT_H
