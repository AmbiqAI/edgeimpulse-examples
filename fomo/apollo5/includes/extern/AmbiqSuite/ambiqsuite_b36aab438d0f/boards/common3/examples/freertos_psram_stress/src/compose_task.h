//*****************************************************************************
//
//! @file compose_task.h
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

#ifndef COMPOSE_TASK_H
#define COMPOSE_TASK_H

//*****************************************************************************
//
// compose task handle.
//
//*****************************************************************************
extern TaskHandle_t compose_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void ComposeTaskSetup(void);
extern void ComposeTask(void *pvParameters);

#endif // COMPOSE_TASK_H
