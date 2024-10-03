//*****************************************************************************
//
//! @file nemagfx_blit.h
//!
//! @brief Global includes for the nemagfx rotating cube app.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_BLIT_H
#define NEMAGFX_BLIT_H

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
#include "nema_core.h"
#include "nema_utils.h"
#include "am_hal_global.h"
#include "nema_programHW.h"
#include "nema_shaderSpecific.h"
//#include "tsi_malloc.h"

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

#define ENABLE_XIPMM
#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS MSPI0_APERTURE_START_ADDR
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS MSPI1_APERTURE_START_ADDR
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS MSPI2_APERTURE_START_ADDR
#endif // #if (MSPI_PSRAM_MODULE == 0)

#define MSPI_XIPMM_BASE_ADDRESS MSPI_XIP_BASE_ADDRESS

#define DDR_PSRAM_TARGET_ADDRESS 0

#define NEMA_CONF_POS_AXIM       (31)
#define NEMA_CONF_POS_TEXFILTER  (30)
#define NEMA_CONF_POS_TSC6       (29)
#define NEMA_CONF_POS_BLENDER    (28)
#define NEMA_CONF_POS_ASYNC      (27)
#define NEMA_CONF_POS_DIRTY      (26)
#define NEMA_CONF_POS_TYPES      (22)
#define NEMA_CONF_POS_MMU        (21)
#define NEMA_CONF_POS_ZCOMPR     (20)
#define NEMA_CONF_POS_VRX        (19)
#define NEMA_CONF_POS_ZBUF       (18)
#define NEMA_CONF_POS_TSC        (17)
#define NEMA_CONF_POS_CG         (16)
#define NEMA_CONF_POS_CORES      ( 8)
#define NEMA_CONF_POS_THREADS    ( 0)

#define NEMA_CONF_MASK_BLENDER   ( 0x1  << NEMA_CONF_POS_BLENDER   )

#define check_bits(x, mask, pos)    ( (x & mask) >> pos )


#define SHOW_INTERVAL 100

#ifndef USE_1BIT_FONT
#define USE_1BIT_FONT 1
#endif
#ifndef USE_4BIT_FONT
#define USE_4BIT_FONT 1
#endif
#ifndef USE_8BIT_FONT
#define USE_8BIT_FONT 1
#endif

#define OPAQUE(color)         ((color) | 0xFF000000 )
#define TRANSP(alpha, color)  ((color & 0x00FFFFFF) | ((alpha & 0xff) << 24) )


#define RESX 360
#define RESY 360

#endif // NEMAGFX_BLIT_H
