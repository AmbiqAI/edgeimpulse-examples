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
#include "nema_core.h"
#include "nema_matrix4x4.h"
#include "nema_math.h"
#include "nema_utils.h"
#include "nema_graphics.h"

//*****************************************************************************
//
// miscellaneous.
//
//*****************************************************************************

#define RESX                320
#define RESY                320

#ifndef COLOR_CUBE
#define COLOR_CUBE          0
#endif

#define FRAME_BUFFERS       2

#define CRATE_GPU_FORMAT    NEMA_RGB24

#endif // NEMAGFX_ROTATING_CRATE_H
