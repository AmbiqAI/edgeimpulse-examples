//*****************************************************************************
//
//! @file spifram_task.h
//!
//! @brief Functions and variables related to the SPIFRAM task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef HRM_TASK_H
#define HRM_TASK_H

//*****************************************************************************
//
// SPIFRAM task handle.
//
//*****************************************************************************
extern TaskHandle_t HRMTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void HRMTaskSetup(void);
extern void HRMTask(void *pvParameters);

#endif // HRM_TASK_H
