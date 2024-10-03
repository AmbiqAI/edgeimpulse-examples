//*****************************************************************************
//
//! @file emmc1_task.h
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

#ifndef EMMC1_TASK1_H
#define EMMC1_TASK1_H

#include "emmc0_task.h"
//*****************************************************************************
//
// EMMC1 task handle.
//
//*****************************************************************************
extern TaskHandle_t emmc1_task_handle;
extern SemaphoreHandle_t g_semEMMC1Write;
extern SemaphoreHandle_t g_semEMMC1Read;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void emmc1_test_cfg_init(void);
extern void Emmc1Task(void *pvParameters);

#endif // EMMC1_TASK_H