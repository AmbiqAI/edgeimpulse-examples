//*****************************************************************************
//
//! @file playback_task.h
//!
//! @brief Functions and variables related to the MP3 playback task
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef PLAYBACK_TASK_H
#define PLAYBACK_TASK_H

//*****************************************************************************
//
// XIPMM task handles.
//
//*****************************************************************************
extern TaskHandle_t playback_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void PlaybackTaskSetup(void);
extern void PlaybackTask(void *pvParameters);

#endif // PLAYBACK_TASK_H
