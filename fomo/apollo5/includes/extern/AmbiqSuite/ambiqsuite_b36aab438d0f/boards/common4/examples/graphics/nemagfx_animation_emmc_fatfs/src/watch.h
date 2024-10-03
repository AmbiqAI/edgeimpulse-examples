//*****************************************************************************
//
//! @file watch.h
//!
//! @brief Global includes for the NemaGFX example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef WATCH_H
#define WATCH_H

#include "nema_core.h"
#include "nema_utils.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESOLUTION_X        456
#define RESOLUTION_Y        456
#define STYLE_ONE           0
#define STYLE_TWO           1

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // MSPI_PSRAM_MODULE == 0

//*****************************************************************************
//
//! @brief watchhand pivot point struct
//!
//! this struct support pivot point information for hour,minute and second hands
//!
//
//*****************************************************************************
typedef struct
{
    int hand_x;
    int hand_y;
}pivot_t;

typedef struct
{
    pivot_t hour;
    pivot_t min;
    pivot_t sec;
}watchPivot_t;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern uintptr_t g_pWatchAddress;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
void g_load_watch_images(void);
nema_cmdlist_t *g_draw_watch_hands(float time, uint8_t style);
#endif // WATCH_H
