//*****************************************************************************
//
//! @file rpmsg_task.h
//!
//! @brief Functions and variables related to the rpmsg task.
//
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
#include "rpc_common.h"
#include "rpc_client_sys_cntl.h"
#include "rpc_client_hci.h"
#include "errno.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define IPC_INST0_IDX           (0)
#define IPC_INST1_IDX           (1)

#define IPC_INST0_KICK_BIT      (1 << IPC_INST0_IDX)
#define IPC_INST1_KICK_BIT      (1 << IPC_INST1_IDX)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */



/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */



/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct ipc_ept ipc_ept_t;
typedef struct ipc_device ipc_dev_t;
typedef struct ipc_ept_cfg ipc_ept_cfg_t;

//*****************************************************************************
//
// RPMSG task handle.
//
//*****************************************************************************
extern metal_thread_t rpmsg_proc_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RPMsgTaskSetup(void);
extern void RPMsgProcessTask(void *pvParameters);
#endif // RPMSG_TASK_H
