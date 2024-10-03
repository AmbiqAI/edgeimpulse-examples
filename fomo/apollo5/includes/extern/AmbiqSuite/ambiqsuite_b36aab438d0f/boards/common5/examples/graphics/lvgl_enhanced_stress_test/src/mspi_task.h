//*****************************************************************************
//
//! @file mspi_task.h
//!
//! @brief Functions and variables related to the mspi task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_TASK_H
#define MSPI_TASK_H

//*****************************************************************************
//
// MSPI task handle.
//
//*****************************************************************************
extern TaskHandle_t MspiTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void MspiTaskSetup(void);
extern void MspiTask(void *pvParameters);

#endif // MSPI_TASK_H
