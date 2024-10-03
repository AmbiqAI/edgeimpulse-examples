//*****************************************************************************
//
//! @file emmc0_task.h
//!
//! @brief Functions and variables related to the emmc0 task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef EMMC0_TASK_H
#define EMMC0_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
//*****************************************************************************
//
// EMMC0 task handle.
//
//*****************************************************************************
extern TaskHandle_t emmc0_task_handle;
extern SemaphoreHandle_t g_semEMMC0Write;
extern SemaphoreHandle_t g_semEMMC0Read;
extern SemaphoreHandle_t g_semEMMC0Start;
//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
void emmc0_test_cfg_init(void);
extern void Emmc0Task(void *pvParameters);
#endif // EMMC0_TASK_H