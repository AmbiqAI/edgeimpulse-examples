//*****************************************************************************
//
//! @file graphic_task.h
//!
//! @brief Functions and variables related to the graphic task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef GRAPHIC_TASK_H
#define GRAPHIC_TASK_H

extern metal_thread_t graphic_store_task_handle;
extern metal_thread_t graphic_compose_task_handle;

extern void GraphicTaskSetup(void);
extern void GraiphicComposeTask(void *pvParameters);
extern void GraiphicStoreTask(void *pvParameters);

#endif // GRAPHIC_TASK_H

