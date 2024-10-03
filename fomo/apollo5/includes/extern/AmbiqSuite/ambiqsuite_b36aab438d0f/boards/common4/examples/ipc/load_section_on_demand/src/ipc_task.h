//*****************************************************************************
//
//! @file ipc_task.h
//!
//! @brief This file describes the host ipc instance API
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IPC_TASK_H
#define IPC_TASK_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 * ipc_task_setup
 *
 * Create host instance and register endpoints.
 *
 * @param - None
 *
 * @return - None
 */
void ipc_task_setup(void);

/**
 * host_ipc_ep_send_sync_packet
 *
 * Send 1 byte synchronization pakcet to remote
 *
 * @param - ep_idx, endpoint identifier
 * @param - data, pointer to data to send
 * @param - len, length of data to send
 *
 * return - status to indicate if sending fail or the sent out data length
 */
int host_ipc_ep_send_sync_packet(eAppRPMsgEpIndex ep_idx, const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // IPC_TASK_H
