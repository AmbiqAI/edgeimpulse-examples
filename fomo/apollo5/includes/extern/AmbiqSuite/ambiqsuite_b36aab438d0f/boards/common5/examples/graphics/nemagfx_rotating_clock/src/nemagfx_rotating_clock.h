//*****************************************************************************
//
//! @file nemagfx_rotating_clock.h
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

#ifndef NEMAGFX_ROTATING_CLOCK_H
#define NEMAGFX_ROTATING_CLOCK_H

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

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_easing.h"
#include "nema_matrix4x4.h"

#include "nema_programHW.h"

//*****************************************************************************
//
// Miscellaneous
//
//*****************************************************************************
#define RESX            384
#define RESY            192

#define PLEURES         (10)
#define ALTITUDE        (0.8f)
#define WIDTH2          (0.259936f)

#define CHAR_ANGLE      (360.f / (PLEURES))

#define ZNEAR           (1)
#define ZFAR            (15)

#define TEX_DIM         80

#define EASE            nema_ez_bounce_out

#endif // NEMAGFX_ROTATING_CLOCK_H
