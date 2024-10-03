//*****************************************************************************
//
//! @file usr_include.h
//!
//! @brief Global includes for freertos_kwd.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef USR_INCLUDE_H
#define USR_INCLUDE_H

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
//#include "am_devices.h"
#include "am_util.h"

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
//*****************************************************************************
//
// User hardware config include files.
//
//*****************************************************************************
#include "vole_board_config.h"
//#include "am_app_KWD_AWE.h"
//#include "am_app_KWD_ble.h"
#include "am_app_KWD_init.h"
//#include "am_app_KWD_rtt_recorder.h"
//*****************************************************************************

#if configUSE_AWE
#include "Errors.h"
#include "Framework.h"
#include "StandardDefs.h"
#include "PlatformTuningHandler.h"
#include "MathHelper.h"
#include "ControlDriver.h"
#include "PlatformAPI.h"
#endif
//*****************************************************************************
//
// Application Tasks include files.
//
//*****************************************************************************
//#include "am_app_KWD_isr.h"
//#include "am_app_KWD_task.h"
//#include "radio_task.h"
//*****************************************************************************
//
// User utils include files.
//
//*****************************************************************************
#include "am_app_utils.h"

//*****************************************************************************
//
// Codec include files.
//
//*****************************************************************************

#endif // FREERTOS_KWD_H
