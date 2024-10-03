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

#ifndef _TOUCH_TEST_H
#define _TOUCH_TEST_H

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
#include "am_devices_chsc5816.h"
#include "am_devices_display_generic.h"
#include "DejaVuSerif8pt1b.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************

#include "nema_core.h"
#include "nema_font.h"
#include "nema_event.h"
#include "nema_utils.h"

#define RESX (g_sDispCfg.ui16ResX)
#define RESY (g_sDispCfg.ui16ResY)

typedef enum
{
    TOUCH_INIT  = 0,
    TOUCH_PRESSED,
    TOUCH_RELEASED,
    TOUCH_PRESSED_AGAIN
} touch_state_t;

extern int touch_test(void);

#endif
