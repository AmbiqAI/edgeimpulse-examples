//*****************************************************************************
//
//! @file dsi_rw_command.h
//!
//! @brief The dsi_rw_command includes headers for the example.
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

#ifndef DSI_RW_COMMAND_H
#define DSI_RW_COMMAND_H

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
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "am_devices_dc_dsi_raydium.h"

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
#endif /* BAREMETAL */

#endif // DSI_RW_COMMAND_H
