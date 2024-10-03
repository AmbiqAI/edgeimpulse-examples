//*****************************************************************************
//
//! @file rpc_client_dbglog.h
//!
//! @brief Functions and variables related to the debug module client role.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RPC_CLIENT_DEBUG_H_H
#define RPC_CLIENT_DEBUG_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_client_debug_ep_get
 *
 * Function to get debug endpoint structure's pointer
 *
 * @param NA
 *
 * return - pointer to the debug endpoint structure(refer to struct ipc_ept in ipc_service.h)
 */
void* rpc_client_debug_ep_get(void);

/**
 * rpc_client_debug_log_handler
 *
 * RPC client handle received debug log packet.
 *
 * @param msg - pointer to the received debug log packet
 * @len       - length of msg
 *
 * return - None
 */
void rpc_client_debug_log_handler(uint8_t *msg, size_t len);


#ifdef __cplusplus
}
#endif
#endif
