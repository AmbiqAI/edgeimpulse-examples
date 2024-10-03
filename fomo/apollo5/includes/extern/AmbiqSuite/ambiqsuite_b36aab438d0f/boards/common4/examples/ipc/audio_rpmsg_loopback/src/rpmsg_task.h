//*****************************************************************************
//
//! @file rpmsg_task.h
//!
//! @brief Functions and variables related to the rpmsg task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RPMSG_TASK_H
#define RPMSG_TASK_H

#include "ipc_service.h"
#include "rpmsg_virtio.h"
#include "am_devices_ambt53.h"


//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
extern metal_thread_t rpmsg_recv_task_handle;
extern metal_thread_t rpmsg_send_task_handle;


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RPMsgTaskSetup(void);
extern void RPMsgReceiveTask(void *pvParameters);
extern void RPMsgSendTask(void *pvParameters);


#endif // RPMSG_TASK_H
