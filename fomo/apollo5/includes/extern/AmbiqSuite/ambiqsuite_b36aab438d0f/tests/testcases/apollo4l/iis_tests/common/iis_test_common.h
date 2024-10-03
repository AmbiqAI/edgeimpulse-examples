//*****************************************************************************
//
//! @file iis_test_common.h
//!
//! @brief IIS test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef IIS_TEST_COMMON_H
#define IIS_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_iis.h"
#include "am_util.h"
#include "am_bsp.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "string.h"

// print interface
#define DEBUG_PRINT        am_util_stdio_printf

extern void setUp(void);
extern void tearDown(void);
extern void globalSetUp(void);
extern void globalTearDown(void);

#endif // IIS_TEST_COMMON_H
