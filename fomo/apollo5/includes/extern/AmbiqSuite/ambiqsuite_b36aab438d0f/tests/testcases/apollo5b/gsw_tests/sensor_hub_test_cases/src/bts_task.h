//*****************************************************************************
//
//! @file bts_task.h
//!
//! @brief Functions and variables related to the Bring to See task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BTS_TASK_H
#define BTS_TASK_H

//*****************************************************************************
//
// Bring to See task handle.
//
//*****************************************************************************
extern TaskHandle_t BtsTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void BtsTaskSetup(void);
extern void BtsTask(void *pvParameters);

#endif // BTS_TASK_H
