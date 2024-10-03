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

#ifndef MAIN_TASK_H
#define MAIN_TASK_H

#define MAIN_EVENT_NEW_FRAME_READY  0x1
#define MAIN_EVENT_DISPLAY_DONE     0x2

//*****************************************************************************
//
// Main task handle.
//
//*****************************************************************************
extern TaskHandle_t main_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void MainTaskSetup(void);
extern void MainTask(void *pvParameters);

#endif // MAIN_TASK_H
