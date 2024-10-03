//
//*****************************************************************************
//! @file sdio_rs9116_emmc_multiple_instances_test_cases.h
//!
//! @brief Global includes for the sdio_rs9116_emmc_multiple_instances_test_cases.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef SDIO_RS9116_EMMC_MULTIPLE_INSTANCES_TEST_CASES_H
#define SDIO_RS9116_EMMC_MULTIPLE_INSTANCES_TEST_CASES_H

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
#include "unity.h"


//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW.h"
#endif

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "rtos.h"
#include "am_widget_sdio.h"
#include "sdio_test_common.h"

#if defined(BAREMETAL) && defined(SYSTEM_VIEW)
#error SYSTEM_VIEW cannot be used in baremetal!
#endif

#define TEST_NUM                5
#define TEST_CONFIG_NUM         25
#define EMMC_MODE_INDEX_MAX     24
#define EMMC_USE_SDIO_HOST1     0

//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (4)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)
#define TASK_PRIORITY_HIGHEST           (TASK_PRIORITY_MIDDLE + 2)

extern am_widget_timing_scan_hashtable_t eMMC_timing_table[DUT_NUM][SCAN_NUM];
extern am_widget_timing_scan_hashtable_t rs9116_timing_table[DUT_NUM][SCAN_NUM];

extern void globalTearDown(void);
#endif // SDIO_RS9116_EMMC_MULTIPLE_INSTANCES_TEST_CASES_H