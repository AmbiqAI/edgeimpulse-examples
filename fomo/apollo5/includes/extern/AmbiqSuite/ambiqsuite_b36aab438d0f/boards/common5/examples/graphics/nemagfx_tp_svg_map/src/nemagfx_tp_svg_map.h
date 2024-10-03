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

#ifndef _NEMAGFX_TP_SVG_MAP
#define _NEMAGFX_TP_SVG_MAP

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

#include "am_devices_chsc5816_ap5.h"
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

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_utils.h"
#include "nema_programHW.h"
#include "nema_matrix3x3.h"

#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"
#include "nema_vg_paint.h"
#include "nema_vg_context.h"

#define RESX 468
#define RESY 468

extern int touch_test(void);

#endif
