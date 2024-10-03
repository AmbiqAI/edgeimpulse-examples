//*****************************************************************************
//
//! @file radio_task.h
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

#ifndef RADIO_TASK_H
#define RADIO_TASK_H

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
extern TaskHandle_t radio_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RadioTaskSetup(void);
extern void RadioTask(void *pvParameters);

#endif // RADIO_TASK_H
