//*****************************************************************************
//
//! @file gui_task.h
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef GUI_TASK_H
#define GUI_TASK_H

extern TaskHandle_t gui_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTaskSetup(void);
extern void GuiTask(void *pvParameters);

#endif
