//*****************************************************************************
//
//! @file audio_task.h
//!
//! @brief Functions and variables related to the audio task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H


#ifdef __cplusplus
extern "C" {
#endif

void audio_task_setup(void);

void audio_record_start(void);

void audio_record_stop(void);

void audio_playback_start(void);

void audio_playback_stop(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_TASK_H
