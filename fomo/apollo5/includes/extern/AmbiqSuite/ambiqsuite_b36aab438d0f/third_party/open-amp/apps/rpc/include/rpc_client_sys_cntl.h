//*****************************************************************************
//
//! @file rpc_client_sys_cntl.h
//!
//! @brief Functions and variables related to the sys_cntl endpoint client.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RPC_CLIENT_SYS_CNTL_H_H
#define RPC_CLIENT_SYS_CNTL_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_client_syscntl_ep_get
 *
 * Function to get sys cntl endpoint structure's pointer
 *
 * @param NA
 *
 * return - pointer to the sys cntl endpoint structure(refer to struct ipc_ept in ipc_service.h)
 */
void* rpc_client_syscntl_ep_get(void);


/**
 * rpc_client_sys_cntl_rx
 *
 * process the received system control endpoint RPMsg from RPC server.
 *
 * @param msg - pointer to the received message
 *
 * return - none
 */
void rpc_client_sys_cntl_rx(uint8_t * msg);


#ifdef __cplusplus
}
#endif
#endif

