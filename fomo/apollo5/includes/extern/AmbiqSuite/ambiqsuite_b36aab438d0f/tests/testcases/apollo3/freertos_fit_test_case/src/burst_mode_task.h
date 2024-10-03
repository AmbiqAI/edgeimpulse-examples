//*****************************************************************************
//
//! @file burst_mode_task.h
//!
//! @brief Functions and variables related to the radio task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BURST_MODE_TASK_H
#define BURST_MODE_TASK_H

//*****************************************************************************
//
// Burst mode task handle.
//
//*****************************************************************************
extern TaskHandle_t Burst_mode_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void BurstmodeSetup(void);
extern void BurstmodeTask(void *pvParameters);

#endif // BURST_MODE_TASK_H

