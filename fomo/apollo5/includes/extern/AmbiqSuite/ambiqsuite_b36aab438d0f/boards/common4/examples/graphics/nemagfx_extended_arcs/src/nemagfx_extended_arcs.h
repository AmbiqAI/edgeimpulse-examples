//*****************************************************************************
//
//! @file nemagfx_extended_arcs.h
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_EXTENDED_ARCS_H
#define NEMAGFX_EXTENDED_ARCS_H

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
#include "am_util_stdio.h"
#include "am_hal_global.h"

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
#include "rtos.h"
//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "gui_task.h"
#endif //!< BAREMETAL

//*****************************************************************************
//
// Third-party/devices include files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************

#endif // NEMAGFX_EXTENDED_ARCS_H
