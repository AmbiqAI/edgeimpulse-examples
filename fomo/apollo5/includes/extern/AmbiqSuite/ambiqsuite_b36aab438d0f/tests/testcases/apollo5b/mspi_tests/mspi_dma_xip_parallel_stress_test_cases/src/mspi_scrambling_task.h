//*****************************************************************************
//
//! @file mspi_scrambling_task.h
//!
//! @brief Functions and variables related to the mspi scrambling task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_SCRAMBLING_TASK_H
#define MSPI_SCRAMBLING_TASK_H

//*****************************************************************************
//
// mspi scrambling task handle.
//
//*****************************************************************************
extern TaskHandle_t MspiScramblingTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void MspiScramblingTask(void *pvParameters);

#endif // MSPI_SCRAMBLING_TASK_H
