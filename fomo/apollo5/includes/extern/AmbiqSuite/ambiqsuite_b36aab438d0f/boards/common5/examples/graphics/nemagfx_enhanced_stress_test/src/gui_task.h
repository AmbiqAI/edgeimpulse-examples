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
//#define SMALLFB

#ifdef SMALLFB
#ifndef SMALLFB_STRIPES
#define SMALLFB_STRIPES (4)
#endif
#else
#undef SMALLFB_STRIPES
#define SMALLFB_STRIPES (1)
#endif

//Frame buffer size
#define FB_RESX (360U)
#define FB_RESY (360U)

#define  USE_RGB888  1

#if USE_RGB888
    #define DISPLAY_FORMAT              COLOR_FORMAT_RGB888
    #define FB_FORMAT                   NEMA_RGB24
#elif USE_RGB565
    #define DISPLAY_FORMAT              COLOR_FORMAT_RGB565
    #define FB_FORMAT                   NEMA_RGB565
#else
    #error "Display format not supported!"
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

extern SemaphoreHandle_t g_semDCStart;
extern SemaphoreHandle_t g_semGPUStart;
extern SemaphoreHandle_t g_semDCEnd;
extern SemaphoreHandle_t g_semGPUEnd;

extern img_obj_t g_sHourHand;
extern img_obj_t g_sMinHand;
extern img_obj_t g_sSecHand;

//Pointer to frame buffer
extern img_obj_t* g_pFrameBufferGPU;
extern img_obj_t* g_pFrameBufferDC;

//Pointer to texture buffer
extern img_obj_t* g_pClockDialBufferDMA;
extern img_obj_t* g_pClockDialBufferGPU;

extern img_obj_t* g_pSportsDialBufferDMA;
extern img_obj_t* g_pSportsDialBufferGPU;

extern int g_intTexturePart;
extern int g_intRenderPart;
extern int g_intDisplayPart;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTask(void *pvParameters);

#endif // RADIO_TASK_H
