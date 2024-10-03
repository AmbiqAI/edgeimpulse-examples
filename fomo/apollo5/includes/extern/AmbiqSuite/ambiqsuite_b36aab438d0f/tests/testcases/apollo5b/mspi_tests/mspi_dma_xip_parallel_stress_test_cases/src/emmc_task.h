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

//*****************************************************************************
//
// EMMC task handle.
//
//*****************************************************************************
extern TaskHandle_t emmc_task_handle;
extern SemaphoreHandle_t g_semEMMCWrite;
extern SemaphoreHandle_t g_semEMMCRead;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

extern void EmmcTask(void *pvParameters);

#endif // EMMC_TASK_H
