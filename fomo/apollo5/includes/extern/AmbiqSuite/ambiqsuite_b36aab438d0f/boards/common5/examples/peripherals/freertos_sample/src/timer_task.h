//*****************************************************************************
//
//! @file timer_task.h
//!
//! @brief Functions and variables related to the timer task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef TIMER_TASK_H
#define TIMER_TASK_H

//*****************************************************************************
//
// Timer task handle.
//
//*****************************************************************************
extern TaskHandle_t timer_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void TimerTaskSetup(void);
extern void TimerTask(void *pvParameters);

#endif // TIMER_TASK_H
