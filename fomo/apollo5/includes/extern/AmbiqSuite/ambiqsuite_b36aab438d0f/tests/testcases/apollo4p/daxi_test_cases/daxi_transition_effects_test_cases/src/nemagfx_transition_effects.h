//*****************************************************************************
//
//! @file nemagfx_transition_effects.h
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

#ifndef NEMAGFX_TRANSITION_H
#define NEMAGFX_TRANSITION_H

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
//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************

#define PANEL_RESX          (g_sDispCfg.ui16ResX/4*4)
#define PANEL_RESY          (g_sDispCfg.ui16ResY/4*4)

#define RESX 256
#define RESY 256

#define PANEL_OFFSET        ((PANEL_RESX-RESX)/2)

//
// use small frame buffer if defined macro SMALLFB
//
//#define SMALLFB

#if defined(SMALLFB)
#define SMALLFB_STRIPES     4
#define FRAME_BUFFERS       2
#else
#define SMALLFB_STRIPES     1
#define FRAME_BUFFERS       1
#endif

#define ANIMATION_STEP_0_1  0.02f
#define MIN_STEP            0.0f
#define MAX_STEP            1.0f
// #### INTERNAL BEGIN ####
//
// !Texture location
//
//#define LOAD_FROM_DSP_SRAM
#define LOAD_FROM_SRAM
//#define LOAD_FROM_MRAM
//#define LOAD_FROM_PSRAM

#define FB_IN_PSRAM
//#define XIP_EXECUTION

#if defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // MSPI_PSRAM_MODULE == 0

#define MAX_XIP_CODE_SIZE 0x400000

#define MSPI_XIPMM_BASE_ADDRESS (MAX_XIP_CODE_SIZE + MSPI_XIP_BASE_ADDRESS)

#endif //defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)

// #### INTERNAL END ####
#define USE_STIMER
#endif