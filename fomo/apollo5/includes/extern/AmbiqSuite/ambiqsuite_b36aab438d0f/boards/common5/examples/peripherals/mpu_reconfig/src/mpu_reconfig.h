//*****************************************************************************
//
//! @file mpu_reconfig.h
//!
//! @brief Global includes for the mpu_reconfig.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MPU_RECONFIG
#define MPU_RECONFIG

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

extern int mpu_reconfig_demo(void);

#endif // MPU_RECONFIG
