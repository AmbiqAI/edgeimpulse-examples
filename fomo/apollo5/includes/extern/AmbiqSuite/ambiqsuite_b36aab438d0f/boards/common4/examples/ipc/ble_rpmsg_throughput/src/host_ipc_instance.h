//*****************************************************************************
//
//! @file host_ipc_instance.h
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

#ifndef HOST_IPC_INSTANCE_H_H
#define HOST_IPC_INSTANCE_H_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * host_ipc_instance_setup
 *
 * Create host instance and register endpoints.
 *
 * @param - None
 *
 * return - None
 */
void host_ipc_instance_setup(void);

#ifdef __cplusplus
}
#endif

#endif // HOST_IPC_INSTANCE_H_H
