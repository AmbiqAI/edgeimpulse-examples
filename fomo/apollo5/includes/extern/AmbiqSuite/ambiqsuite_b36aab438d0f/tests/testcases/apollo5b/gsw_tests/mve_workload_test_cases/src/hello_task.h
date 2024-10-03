//*****************************************************************************
//
//! @file hello_task.h
//!
//! @brief Functions and variables related to the hello task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef HELLO_TASK_H
#define HELLO_TASK_H

//*****************************************************************************
//
// Hello task handle.
//
//*****************************************************************************
extern TaskHandle_t hello_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void HelloTaskSetup(void);
extern void HelloTask(void *pvParameters);

#endif // HELLO_TASK_H
