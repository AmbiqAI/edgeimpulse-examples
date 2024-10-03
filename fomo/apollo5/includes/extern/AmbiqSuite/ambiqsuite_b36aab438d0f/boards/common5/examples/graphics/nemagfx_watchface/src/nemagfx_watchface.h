//*****************************************************************************
//
//! @file nemagfx_watchface.h
//!
//! @brief Global includes for the nemagfx_watchface app.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_WATCHFACE_H
#define NEMAGFX_WATCHFACE_H

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
#include "am_devices_tma525.h"

//*****************************************************************************
//
// miscellaneous.
//
//*****************************************************************************

#define GPU_FORMAT_RGB888

#define FB_RESX         400
#define FB_RESY         400

#endif // NEMAGFX_WATCHFACE_H
