//
//*****************************************************************************
//! @file emmc_multiple_instances_test_cases.h
//!
//! @brief Global includes for the emmc_multiple_instances_test_cases.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef EMMC_MULTIPLE_INSTANCES_TEST_CASES_H
#define EMMC_MULTIPLE_INSTANCES_TEST_CASES_H

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
#include "emmc0_task.h"
#include "emmc1_task.h"
#include "rtos.h"
#include "am_widget_sdio.h"

#if defined(BAREMETAL) && defined(SYSTEM_VIEW)
#error SYSTEM_VIEW cannot be used in baremetal!
#endif

#define TEST_NUM                5
#define TEST_CONFIG_NUM         24

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

extern void globalTearDown(void);
#endif // EMMC_MULTIPLE_INSTANCES_TEST_CASES_H