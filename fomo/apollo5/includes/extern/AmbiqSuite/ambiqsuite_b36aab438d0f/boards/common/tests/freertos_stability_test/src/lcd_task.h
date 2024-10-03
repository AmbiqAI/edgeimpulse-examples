//*****************************************************************************
//
//! @file lcd_task.h
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

#ifndef LCD_TASK_H
#define LCD_TASK_H

//*****************************************************************************
//
// SPIFRAM task handle.
//
//*****************************************************************************
extern TaskHandle_t LCDTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void LCDTaskSetup(void);
extern void LCDTask(void *pvParameters);

#endif // LCD_TASK_H
