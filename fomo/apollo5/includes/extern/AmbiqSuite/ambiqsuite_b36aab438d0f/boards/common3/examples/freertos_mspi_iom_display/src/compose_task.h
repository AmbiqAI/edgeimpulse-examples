//*****************************************************************************
//
//! @file compose_task.h
//!
//! @brief Functions and variables related to the compose task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef COMPOSE_TASK_H
#define COMPOSE_TASK_H

// Multiple of lines
//#define COMPOSE_FRAGMENT_SIZE     (FRAME_SIZE/4)
#define COMPOSE_FRAGMENT_SIZE     ((TEMP_BUFFER_SIZE / COLUMN_NUM) * COLUMN_NUM)
#define NUM_ROW_PER_FRAGMENT      (COMPOSE_FRAGMENT_SIZE/COLUMN_NUM)
#define NUM_COMPOSE_FRAGMENTS     ((FRAME_SIZE + COMPOSE_FRAGMENT_SIZE - 1)/COMPOSE_FRAGMENT_SIZE)

#define BACKGROUND_COLOR          0x00
#define BAND_COLOR                0xfc // white
#define COLOR_MAX                 0xfc
#define BAND_WIDTH                16
// Extract color from a 4p word
#define COL(word, pix)            (((word) >> ((pix) << 3)) & COLOR_MAX)
#define WRAP_COL(num, max)        (((num) > (max)) ? 0 : (num))
// Increment color
#define INC_COL(color, inc)       WRAP_COL((color) + (inc))

// 4 pixel worth of data
#define COLOR_4P(color)           ((color) | ((color) << 8) | ((color) << 16) | ((color) << 24))
// Increment color for each pixel
#define INC_COL_4P(w, inc)       (INC_COL(COL((w), 0), (inc)) | (INC_COL(COL((w), 1), (inc)) << 8) | (INC_COL(COL((w), 2), (inc)) << 16) | (INC_COL(COL((w), 3), (inc)) << 24))

//*****************************************************************************
//
// compose task handle.
//
//*****************************************************************************
extern TaskHandle_t compose_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void ComposeTaskSetup(void);
extern void ComposeTask(void *pvParameters);

#endif // COMPOSE_TASK_H
