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

#include "nema_utils.h"
#include "nema_graphics.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SMALLFB
#define SMALLFB_STRIPES (4)

#ifdef SMALLFB
#ifndef SMALLFB_STRIPES
#define SMALLFB_STRIPES (4)
#endif
#else
#undef SMALLFB_STRIPES
#define SMALLFB_STRIPES (1)
#endif

//*****************************************************************************
//
// Debug pin.
//
//*****************************************************************************

//*****************************************************************************
//
// Gui task handle.
//
//*****************************************************************************
extern TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTask(void *pvParameters);

#endif // RADIO_TASK_H
