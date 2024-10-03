//*****************************************************************************
//
//! @file nemagfx_earth_nasa.h
//!
//! @brief Global includes for the nemagfx_earth_nasa app.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_EARTH_NASA_H
#define NEMAGFX_EARTH_NASA_H

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

//*****************************************************************************
//
// Third-party/Devices include files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_programHW.h"
#include "nema_event.h"
#include "nema_math.h"
#include "nema_graphics.h"
#include "nema_programHW.h"
#include "nema_regs.h"
#include "nema_vg.h"
#include "nema_vg_tsvg.h"

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
// Device drivers.
//
//*****************************************************************************
#include "am_devices_display_generic.h"
#include "am_devices_chsc5816_ap5.h"

//*****************************************************************************
//
// miscellaneous.
//
//*****************************************************************************

#define GPU_FORMAT_RGB888

#define FRAME_BUFFERS                   1

#define RESX                            468
#define RESY                            468

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

#endif // NEMAGFX_EARTH_NASA_H
