//*****************************************************************************
//
//! @file rpc_server_sys_cntl.h
//!
//! @brief Functions and variables related to the sys_cntl endpoint server.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RPC_SERVER_SYS_CNTL_H_H
#define RPC_SERVER_SYS_CNTL_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_server_sys_cntl_rx
 *
 * process the received system control endpoint RPMsg from RPC client.
 *
 * @param msg - pointer to the received message
 *
 * return - none
 */
void rpc_server_sys_cntl_rx(uint8_t * msg);


#ifdef __cplusplus
}
#endif
#endif
