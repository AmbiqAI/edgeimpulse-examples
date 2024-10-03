//*****************************************************************************
//
//! @file nemagfx_watchface.h
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


//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_core.h"
#include "nema_matrix4x4.h"
#include "nema_math.h"
#include "nema_utils.h"
#include "nema_graphics.h"

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
#endif //!< BAREMETAL
//*****************************************************************************
//
// miscellaneous.
//
//*****************************************************************************




#endif // NEMAGFX_WATCHFACE_H
