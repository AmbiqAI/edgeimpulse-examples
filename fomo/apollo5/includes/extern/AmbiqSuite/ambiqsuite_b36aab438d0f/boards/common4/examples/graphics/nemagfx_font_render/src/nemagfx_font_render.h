//*****************************************************************************
//
//! @file nemagfx_font_render.h
//!
//! @brief Global includes for the app.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_FONT_RENDER_H
#define NEMAGFX_FONT_RENDER_H

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
#include "am_util_delay.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_font.h"
#include "nema_programHW.h"

//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************
#ifndef USE_1BIT_FONT
#define USE_1BIT_FONT 1
#endif
#ifndef USE_2BIT_FONT
#define USE_2BIT_FONT 1
#endif
#ifndef USE_4BIT_FONT
#define USE_4BIT_FONT 1
#endif
#ifndef USE_8BIT_FONT
#define USE_8BIT_FONT 1
#endif

#define NEMA_FONT_IMPLEMENTATION
#if USE_1BIT_FONT != 0
    #include "DejaVuSerif8pt1b.h"
#endif
#if USE_2BIT_FONT != 0
    #include "DejaVuSerif8pt2b.h"
#endif
#if USE_4BIT_FONT != 0
    #include "DejaVuSerif8pt4b.h"
#endif
#if USE_8BIT_FONT != 0
    #include "DejaVuSerif8pt8b.h"
#endif

#undef NEMA_FONT_IMPLEMENTATION

#define RESX 390
#define RESY 390

#endif // NEMAGFX_FONT_RENDER_H
