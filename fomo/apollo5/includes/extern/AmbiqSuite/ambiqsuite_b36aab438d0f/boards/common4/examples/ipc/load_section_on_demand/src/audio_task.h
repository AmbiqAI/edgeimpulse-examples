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

typedef enum
{
    LOADER_NONE         = 0,
    LOADER_SBC_DECODER  = 1,
    LOADER_SBC_ENCODER  = 2,
    LOADER_MSBC         = 3,

    LOADER_TYPE_NUMBER  = 4,
} actrl_loader_type_t;

typedef enum
{
    LOADER_CODE       = 0,
    LOADER_DATA       = 1,

    LOADER_NUMBER     = 2,
} actrl_loader_t;

typedef enum
{
    LOADER_FAIL       = 0,
    LOADER_SUC        = 1,
} actrl_loader_status_t;

#ifdef __cplusplus
extern "C" {
#endif

void audio_task_setup(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_TASK_H
