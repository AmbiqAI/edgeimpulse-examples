//*****************************************************************************
//
//! @file rpc_server_dbglog.h
//!
//! @brief Functions and variables related to the debug module server role.
//!
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
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
