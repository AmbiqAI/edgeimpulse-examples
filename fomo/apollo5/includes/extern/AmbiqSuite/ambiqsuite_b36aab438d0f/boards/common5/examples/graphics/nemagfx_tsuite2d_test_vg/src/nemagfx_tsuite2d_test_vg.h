//*****************************************************************************
//
//! @file nemagfx_rotating_crate.h
//!
//! @brief Global includes for the nemagfx rotating crate example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_ROTATING_CRATE_H
#define NEMAGFX_ROTATING_CRATE_H

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
#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_programHW.h"
#include "nema_matrix3x3.h"

#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"
#include "nema_vg_paint.h"
#include "nema_vg_context.h"

#include "nema_dc.h"
#include "nema_dc_mipi.h"


//*****************************************************************************
//
// miscellaneous.
//
//*****************************************************************************
static const unsigned long OWHITE    = 0xFFFFFFFF;
static const unsigned long WHITE_A1  = 0xFFFFFFFF;
static const unsigned long WHITE     = 0x00FFFFFF;
static const unsigned long GRAY      = 0x00A0A0A0;
static const unsigned long GRAY7     = 0x00707070;
static const unsigned long BLACK     = 0x00000000;
static const unsigned long BLUE      = 0x00FF0000;
static const unsigned long GREEN     = 0x0000FF00;
static const unsigned long RED       = 0x000000FF;
static const unsigned long YELLOW    = 0x0000FFFF;
static const unsigned long MAGENTA   = 0x00FF00FF;
static const unsigned long CYAN      = 0x00FFFF00;
static const unsigned long PURPLE    = 0x00800080;

#endif // NEMAGFX_ROTATING_CRATE_H
