//*****************************************************************************
//
//! @file nemagfx_blend.h
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

#ifndef NEMAGFX_BLEND_H
#define NEMAGFX_BLEND_H

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
#include "am_util_delay.h"
#include "am_util_stdio.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_regs.h"
#include "nema_core.h"
#include "nema_utils.h"

//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************
#define RESX            400
#define RESY            400

#define RED             (0x000000FFU)
#define OPAQUE(color)   ((color) | 0xFF000000)

#define TRANSP(alpha, color)    ((color & 0x00FFFFFF) | ((alpha & 0xff) << 24))

#define LOAD_TO_SSRAM
#define CPU_DEEP_SLEEP
#define DISPLAY_ENABLE

#endif // NEMAGFX_BLEND_H
