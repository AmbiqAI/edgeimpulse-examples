//*****************************************************************************
//
//! @file nemagfx_guiBuilder_animated_screen.h
//!
//! @brief Global includes for the freertos_fit app.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_GAUGE_H
#define NEMAGFX_GAUGE_H

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

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_core.h"
#include "nema_font.h"
#include "nema_easing.h"
#include "nema_transitions.h"
#include "nema_event.h"
#include "nema_utils.h"

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

#endif // NEMAGFX_GAUGE_H
