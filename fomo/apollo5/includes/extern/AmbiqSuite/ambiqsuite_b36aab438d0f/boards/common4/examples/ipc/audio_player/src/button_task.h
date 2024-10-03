//*****************************************************************************
//
//! @file button_task.h
//!
//! @brief Functions and variables related to the button task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BUTTON_TASK_H
#define BUTTON_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ButtonsTimeTag
{
    B_INVALID,
    B_SHORT,
    B_LONG,
    B_VERY_LONG,
    B_DOUBLE,
    B_REPEAT,
    B_LOW_TO_HIGH,
    B_HIGH_TO_LOW,
    B_SHORT_SINGLE,
    B_LONG_RELEASE,
    B_VERY_LONG_RELEASE,
    B_VERY_VERY_LONG,
    B_VERY_VERY_LONG_RELEASE,
    B_TRIPLE
} ButtonsTime_t;

void button_task_setup(void);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_TASK_H
