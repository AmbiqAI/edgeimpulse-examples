//*****************************************************************************
//
//! @file iom_psram_task.h
//!
//! @brief Functions and variables related to the iom_psram task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IOM_PSRAM_TASK_H
#define IOM_PSRAM_TASK_H

//*****************************************************************************
//
// IOM_PSRAM task handle.
//
//*****************************************************************************
extern TaskHandle_t IomPsramTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void IomPsramTask(void *pvParameters);

#endif // IOM_PSRAM_TASK_H
