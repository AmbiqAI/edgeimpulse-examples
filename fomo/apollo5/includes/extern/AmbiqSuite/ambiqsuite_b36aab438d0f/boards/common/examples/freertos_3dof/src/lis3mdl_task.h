//*****************************************************************************
//
//! @file lis3mdl_task.h
//!
//! @brief Functions and variables related to the lis3mdl task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef LIS3MDL_TASK_H
#define LIS3MDL_TASK_H

//*****************************************************************************
//
// Lis3mdl task handle.
//
//*****************************************************************************
extern TaskHandle_t lis3mdl_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void LIS3MDLTaskSetup(void);
extern void LIS3MDLTask(void *pvParameters);
extern void lis3mdl_timer_handler(uint32_t ui32Status);

#endif // LIS3MDL_TASK_H
