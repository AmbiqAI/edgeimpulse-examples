//*****************************************************************************
//
//! @file touch_test.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef _NEMAGFX_TOUCH_TEST_H
#define _NEMAGFX_TOUCH_TEST_H

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

#include "am_devices_tma525.h"
#include "am_devices_display_generic.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************

#include "nema_core.h"
#include "nema_font.h"
#include "nema_event.h"
#include "nema_utils.h"

#define RESX 452
#define RESY 452

typedef enum
{
    TOUCH_INIT  = 0,
    TOUCH_PRESSED,
    TOUCH_RELEASED,
    TOUCH_PRESSED_AGAIN
} touch_state_t;

extern int touch_test(void);

#endif//_NEMAGFX_TOUCH_TEST_H
