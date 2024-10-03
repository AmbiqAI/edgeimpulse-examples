//*****************************************************************************
//
//! @file dynamic_load_codec_task.h
//!
//! @brief Functions and variables related to the dynamic load codec task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef DYNAMIC_LOAD_CODEC_TASK_H
#define DYNAMIC_LOAD_CODEC_TASK_H

#include "rpmsg_virtio.h"
#include "am_devices_ambt53.h"

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
extern metal_thread_t load_mp3_task_handle;
extern metal_thread_t load_aac_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern int init_ambt53(void);
extern void LoadMP3Task(void *pvParameters);
extern void LoadAACTask(void *pvParameters);

#endif // RPMSG_TASK_H
