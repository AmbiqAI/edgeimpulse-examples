//*****************************************************************************
//
//! @file flash_task.h
//!
//! @brief Functions and variables related to the SPIFRAM task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FLASH_TASK_H
#define FLASH_TASK_H

//*****************************************************************************
//
// SPIFRAM task handle.
//
//*****************************************************************************
extern TaskHandle_t FlashTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void FlashTaskSetup(void);
extern void FlashTask(void *pvParameters);

#endif // FLASH_TASK_H
