//*****************************************************************************
//
//! @file nemagfx_test_common.h
//!
//! @brief NemaGFX test cases common definitions.
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
#include "am_devices_nemadc_rm67162.h"

//
// Maximum read frequency of RM67162 is about 3MHz
//
#define TARGET_FREQ 3
//
// Format clock frequency is half of DISPCLK.
//
#define FORMAT_CLK_DIV 2
//
// Assume DISPCLK is 48MHz or 96MHz on actual silicon, the proper division ratio is 16.
//
#define SILICON_FORMAT_DIV 16
//
// DC option clock divider position
//
#define DC_DIV2_POS 27

extern void setUp(void);
extern void tearDown(void);
extern void globalSetUp(void);
extern void globalTearDown(void);
extern uint32_t am_nemadc_read(uint32_t eCmd, uint32_t ui32Length);

#endif // NEMAGFX_TEST_COMMON_H
