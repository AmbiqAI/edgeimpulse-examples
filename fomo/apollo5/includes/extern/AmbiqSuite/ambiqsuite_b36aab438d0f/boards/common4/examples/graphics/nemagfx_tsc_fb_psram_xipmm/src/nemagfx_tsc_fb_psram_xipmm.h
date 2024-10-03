//*****************************************************************************
//
//! @file nemagfx_tsc_fb_psram_xipmm.h
//!
//! @brief Global includes for the NemaGFX example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_TSC_FB_PSRAM_XIPMM_H
#define NEMAGFX_TSC_FB_PSRAM_XIPMM_H

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

#include "nema_core.h"
#include "nema_utils.h"
#include "am_hal_global.h"
#include "nema_dc.h"
#include "nema_hal.h"

#ifndef BAREMETAL
//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "gui_task.h"
#endif // BAREMETAL

//#define ENABLE_XIPMM
#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // #if (MSPI_PSRAM_MODULE == 0)

#define MSPI_XIPMM_BASE_ADDRESS MSPI_XIP_BASE_ADDRESS

#endif // NEMAGFX_TSC_FB_PSRAM_XIPMM_H
