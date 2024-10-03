//*****************************************************************************
//
//! @file render_task.h
//!
//! @brief Functions and variables related to the compose task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RENDER_TASK_H
#define RENDER_TASK_H

//*****************************************************************************
//
// compose task handle.
//
//*****************************************************************************
extern TaskHandle_t render_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RenderTaskSetup(void);
extern void RenderTask(void *pvParameters);

#endif // RENDER_TASK_H
