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
#ifndef GPU_DC_PWRCTRL_TEST_COMMON_H
#define GPU_DC_PWRCTRL_TEST_COMMON_H

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
#include "am_devices_dc_xspi_raydium.h"
#include "am_devices_dc_dsi_raydium.h"

extern void setUp(void);
extern void tearDown(void);
extern void globalSetUp(void);
extern void globalTearDown(void);
extern uint32_t am_nemadc_read(uint32_t eCmd, uint32_t ui32Length);

#endif // GPU_DC_PWRCTRL_TEST_COMMON_H
