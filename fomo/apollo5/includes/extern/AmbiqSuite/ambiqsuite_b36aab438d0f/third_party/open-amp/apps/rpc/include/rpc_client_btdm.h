//*****************************************************************************
//
//! @file rpc_client_btdm.h
//!
//! @brief Functions and variables related to BTDM module rpc client role.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RPC_CLIENT_BTDM_H
#define RPC_CLIENT_BTDM_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_client_btdm_handler
 *
 * RPC client handle received bluetooth packet.
 *
 * @param msg - pointer to the received bluetooth packet
 * @param pdu_len - length of pdu, including type length and payload length
 *
 * return - None
 */
void rpc_client_btdm_handler(uint8_t *msg, uint16_t pdu_len);

/**
 * rpc_client_btdm_send_packet
 *
 *  RPC client send bluetooth packet.
 *
 * @param type     - request type of BTDM packet
 * @param packet   - packet to send
 * @param len      - length of packet
 *
 * return - >=0 on SUCCESS(length of sent out HCI packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int rpc_client_btdm_send_packet(uint8_t type, uint8_t *packet, uint16_t len);

/**
 * rpc_client_btdm_cb_set
 *
 *  Used to set application callback.
 *
 * @id   - identifier of the callback function
 * @cb   - specific application callback function
 *
 */
void rpc_client_btdm_cb_set(p_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // RPC_CLIENT_BTDM_H

