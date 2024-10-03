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

//Frame buffer size
#define FB_RESX (454U)
#define FB_RESY (454U)

#define ANIMATION_SPEED (0.03F)
#define SWITCH_SCALE_ROTATE ((uint32_t)(360.0f/ANIMATION_SPEED))

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
extern img_obj_t* g_pFrameBufferA;
extern img_obj_t* g_pFrameBufferB;

//Texture
extern img_obj_t g_sMask;
extern img_obj_t g_sScottyImage;
extern img_obj_t g_sScottyXmasImage;
extern img_obj_t g_sScottyMortonImage;
extern img_obj_t g_sScottyXmasMortonImage;
extern img_obj_t g_sAmbiqLogo;
extern img_obj_t g_sMaskAmbiqLogo;

extern uint32_t g_ui32FrameTime;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTask(void *pvParameters);

#endif // RADIO_TASK_H
