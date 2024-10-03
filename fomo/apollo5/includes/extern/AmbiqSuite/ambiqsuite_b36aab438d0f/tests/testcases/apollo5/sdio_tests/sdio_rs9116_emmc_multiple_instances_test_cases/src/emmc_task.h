//*****************************************************************************
//
//! @file emmc_task.h
//!
//! @brief Functions and variables related to the emmc task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef EMMC_TASK_H
#define EMMC_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
//*****************************************************************************
//
// EMMC task handle.
//
//*****************************************************************************
extern TaskHandle_t emmc_task_handle;
extern SemaphoreHandle_t g_semEMMCWrite;
extern SemaphoreHandle_t g_semEMMCRead;
extern SemaphoreHandle_t g_semEMMCStart;
//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
void emmc_test_cfg_init(void);
extern void EmmcTask(void *pvParameters);
#endif // EMMC_TASK_H