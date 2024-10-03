//*****************************************************************************
//
//! @file mspi_xip_task.h
//!
//! @brief Functions and variables related to the mspi xip task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_XIPMM_TASK_H
#define MSPI_XIPMM_TASK_H

//*****************************************************************************
//
// XipMM task handle.
//
//*****************************************************************************
extern TaskHandle_t MpsiXipMmTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void XipMmTask(void *pvParameters);

#endif // MSPI_XIPMM_TASK_H
