//*****************************************************************************
//
//! @file rpc_server_dbglog.h
//!
//! @brief Functions and variables related to the debug module server role.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef RPC_SERVER_DEBUG_H_H
#define RPC_SERVER_DEBUG_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_server_dbglog_ep_get
 *
 * Function to get dbglog server endpoint structure's pointer
 *
 * @param NA
 *
 * return - pointer to the dbglog server endpoint structure(refer to struct ipc_ept in ipc_service.h)
 */
void* rpc_server_dbglog_ep_get(void);

/**
 * rpc_server_dbglog_ep_get
 *
 * Function to get deblog client endpoint structure's pointer
 *
 * @param NA
 *
 * return - pointer to the deblog server endpoint structure(refer to struct ipc_ept in ipc_service.h)
 */
/**
 * rpc_server_dump_debug_log
 *
 *  RPC server dump debug log to MCU.
 *
 * @param packet   - pointer to debug packet
 * @param len      - length of debug packet
 *
 * return - >=0 on SUCCESS(length of sent out HCI packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int rpc_server_dump_debug_log(uint8_t *packet, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
