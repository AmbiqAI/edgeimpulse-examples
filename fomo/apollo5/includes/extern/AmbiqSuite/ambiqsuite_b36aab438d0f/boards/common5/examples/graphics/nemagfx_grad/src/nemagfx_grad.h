//*****************************************************************************
//
//! @file nemagfx_grad.h
//!
//! @brief Global includes for graphics example nemagfx_grad.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_GRAD_H
#define NEMAGFX_GRAD_H

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
// Third-party/Devices include files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_programHW.h"

//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************

#define RESX            320
#define RESY            RESX

#define MARGIN          10
#define GRIDNUM         3
#define COLORSIZE       (RESY - (GRIDNUM - 1)* MARGIN)/(GRIDNUM)
#define GRIDOFFSET      (COLORSIZE + MARGIN)

#endif // NEMAGFX_GRAD_H
