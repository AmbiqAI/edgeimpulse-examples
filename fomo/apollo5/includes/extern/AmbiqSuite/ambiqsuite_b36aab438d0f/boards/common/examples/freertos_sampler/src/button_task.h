//*****************************************************************************
//
//! @file button_task.h
//!
//! @brief Functions and variables related to the BUTTON task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BUTTON_TASK_H
#define BUTTON_TASK_H

//*****************************************************************************
//
// LED task handle.
//
//*****************************************************************************
extern TaskHandle_t button_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void ButtonTaskSetup(void);
extern void ButtonTask(void *pvParameters);

#endif // RADIO_TASK_H
