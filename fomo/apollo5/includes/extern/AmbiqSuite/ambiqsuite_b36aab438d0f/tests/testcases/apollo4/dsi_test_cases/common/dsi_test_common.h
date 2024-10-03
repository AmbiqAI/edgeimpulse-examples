//*****************************************************************************
//
//! @file dsi_test_common.h
//!
//! @brief DSI test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef NEMAGFX_TEST_COMMON_H
#define NEMAGFX_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "nema_dc_regs.h"
#include "tsi_malloc.h"
#include "string.h"
#include "am_devices_dsi_rm67162.h"

typedef enum PIXEL_FORMAT
{
    FMT_RGB565 = 0,
    FMT_RGB888,
    FMT_RGB666
} pixel_format_enum;

extern void setUp(void);
extern void tearDown(void);
extern void globalSetUp(void);
extern void globalTearDown(void);

#endif // NEMAGFX_TEST_COMMON_H