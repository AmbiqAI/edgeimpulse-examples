//*****************************************************************************
//
//! @file render_task.h
//!
//! @brief Functions and variables related to the render task.
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
// render task handle.
//
//*****************************************************************************
extern TaskHandle_t RenderTaskHandle;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern img_obj_t g_sFrameBuffer[];
//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RenderTask(void *pvParameters);

extern float angle_dx;
extern float angle_dy;
extern float trans_dz;
extern bool bDrawWire;
#endif // RENDER_TASK_H
