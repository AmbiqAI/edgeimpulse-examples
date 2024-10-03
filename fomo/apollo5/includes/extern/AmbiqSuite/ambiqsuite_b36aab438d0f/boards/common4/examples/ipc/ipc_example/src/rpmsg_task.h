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

/* Generic IPC event index definitions. */
#define IPC_EP0_SEND_IDX    (INST0_EP0)
#define IPC_EP1_SEND_IDX    (INST0_EP1)
#define IPC_MBOX_IDX        (INST0_EP_NUM)
/* Generic bit definitions. */
#define IPC_EP0_SEND_BIT    ( 1 << IPC_EP0_SEND_IDX )
#define IPC_EP1_SEND_BIT    ( 1 << IPC_EP1_SEND_IDX )
#define IPC_MBOX_BIT        ( 1 << IPC_MBOX_IDX )

/* Combinations of bits used in IPC. */
#define IPC_ALL_BITS ( IPC_MBOX_BIT | IPC_EP0_SEND_BIT | IPC_EP1_SEND_BIT)

/* Generic bit definitions for each instance's kick callback. */
#define IPC_INST0_KICK_BIT    ( 1 << 0 )
//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
extern metal_thread_t rpmsg_proc_task_handle;


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RPMsgTaskSetup(void);
extern void RPMsgReceiveTask(void *pvParameters);
extern void RPMsgProcessTask(void *pvParameters);

#endif // RPMSG_TASK_H
