//*****************************************************************************
//
//! @file gui_task.h
//!
//! @brief Functions and variables related to the gui task.
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

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define ENABLE_LABEL         1
#define WALLPAPER_FRAME_NUM  90


//*****************************************************************************
//
// Gui task handle.
//
//*****************************************************************************
extern TaskHandle_t GuiTaskHandle;
extern SemaphoreHandle_t g_semBLEAdv;
extern SemaphoreHandle_t g_semBLECon;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTask(void *pvParameters);

#endif // RADIO_TASK_H