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

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//Frame buffer size
#define FB_RESX (400U)
#define FB_RESY (400U)

//*****************************************************************************
//
// Debug pin.
//
//*****************************************************************************
#if defined(apollo4b_eb)
#define DEBUG_PIN_1    22
#define DEBUG_PIN_2    23
#define DEBUG_PIN_3    24
#define DEBUG_PIN_4    25
#define DEBUG_PIN_5    26
#define DEBUG_PIN_6    27
#elif defined(apollo4b_bga_evb_disp_shld)
#define DEBUG_PIN_1    25
#define DEBUG_PIN_2    74
#define DEBUG_PIN_3    75
#define DEBUG_PIN_4    76
#define DEBUG_PIN_5    77
#define DEBUG_PIN_6    78
#elif defined(apollo4p_bga_sys_test)
#define DEBUG_PIN_1    74
#define DEBUG_PIN_2    75
#define DEBUG_PIN_3    76
#define DEBUG_PIN_4    77
#define DEBUG_PIN_5    78
#define DEBUG_PIN_6    79
#else
#define DEBUG_PIN_1    22
#define DEBUG_PIN_2    23
#define DEBUG_PIN_3    24
#define DEBUG_PIN_4    25
#define DEBUG_PIN_5    26
#define DEBUG_PIN_6    27
#endif

//*****************************************************************************
//
// Gui task handle.
//
//*****************************************************************************
extern TaskHandle_t GuiTaskHandle;

extern SemaphoreHandle_t g_semDCStart;
extern SemaphoreHandle_t g_semGPUStart;
extern SemaphoreHandle_t g_semDCEnd;
extern SemaphoreHandle_t g_semGPUEnd;

//Pointer to frame buffer
extern img_obj_t* g_pFrameBufferGPU;
extern img_obj_t* g_pFrameBufferDC;

//Texture
extern img_obj_t g_sRef;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTask(void *pvParameters);

#endif // GUI_TASK_H
