//*****************************************************************************
//
//! @file radio_task.h
//!
//! @brief Functions and variables related to the radio task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RADIO_TASK_H
#define RADIO_TASK_H

#include "rpmsg_service.h"

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
extern metal_thread_t radio_task_handle;
extern metal_thread_t rpmsg_recv_task_handle;
extern metal_thread_t audio_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RadioTaskSetup(void);
extern void RadioTask(void *pvParameters);
extern void RPMsgRecvTask(void *pvParameters);
extern void AudioTask(void *pvParameters);

#endif // RADIO_TASK_H
