//*****************************************************************************
//
//! @file display_task.h
//!
//! @brief Functions and variables related to the display task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

//*****************************************************************************
//
// Debug pin.
//
//*****************************************************************************
#define DEBUG_PIN_1    86
#define DEBUG_PIN_2    87
#define DEBUG_PIN_3    80
#define DEBUG_PIN_4    81
#define DEBUG_PIN_5    82
#define DEBUG_PIN_6    83

//*****************************************************************************
//
// Display task handle.
//
//*****************************************************************************
extern TaskHandle_t DisplayTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void DisplayTask(void *pvParameters);

#endif //DISPLAY_TASK_H
