//*****************************************************************************
//
//! @file rpc_client_hci.h
//!
//! @brief Functions and variables related to HCI module rpc client role.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RPC_CLIENT_HCI_H_H
#define RPC_CLIENT_HCI_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * rpc_client_hci_handler
 *
 * RPC client handle received HCI packet.
 *
 * @param msg - pointer to the received HCI packet
 *
 * return - None
 */
void rpc_client_hci_handler(uint8_t *packet, uint16_t len);

/**
 * rpc_client_hci_send_packet
 *
 *  RPC client send HCI packet.
 *
 * @param packet   - pointer to HCI packet
 * @param len      - length of HCI packet
 *
 * return - >=0 on SUCCESS(length of sent out HCI packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int rpc_client_hci_send_packet(uint8_t *packet, uint16_t len);

/**
 *  Register the rpc_client_hci_handler callback function to process the received HCI packet
 *
 * @param cb   - pointer to the rpc_client_hci_handler callback function
 *
 */
void rpc_client_hci_msg_handler_cb_register(hci_handler_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif

