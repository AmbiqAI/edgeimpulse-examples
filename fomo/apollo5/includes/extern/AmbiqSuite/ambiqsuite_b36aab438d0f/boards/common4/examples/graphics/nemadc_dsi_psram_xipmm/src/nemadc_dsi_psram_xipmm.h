//*****************************************************************************
//
//! @file nemadc_dsi_psram_xipmm.h
//!
//! @brief The nemadc_dsi_psram_xipmm includes headers for the example.
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

#ifndef NEMADC_DSI_PSRAM_XIPMM_H
#define NEMADC_DSI_PSRAM_XIPMM_H

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
#include "tsi_malloc.h"

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
#endif // BAREMETAL

#endif // NEMADC_DSI_PSRAM_XIPMM_H
