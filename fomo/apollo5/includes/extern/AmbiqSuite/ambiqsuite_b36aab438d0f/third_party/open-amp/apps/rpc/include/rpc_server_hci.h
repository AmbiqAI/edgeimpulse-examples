//*****************************************************************************
//
//! @file rpc_server_hci.h
//!
//! @brief Functions and variables related to HCI module rpc server role.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef RPC_SERVER_HCI_H_H
#define RPC_SERVER_HCI_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_server_hci_ep_get
 *
 * Function to get HCI server endpoint structure's pointer
 *
 * @param NA
 *
 * return - pointer to the HCI server endpoint structure(refer to struct ipc_ept in ipc_service.h)
 */
void* rpc_server_hci_ep_get(void);
/**
 * rpc_server_hci_handler
 *
 * RPC server handle received HCI packet.
 *
 * @param msg - pointer to the received HCI packet
 *
 * return - None
 */
void rpc_server_hci_handler(uint8_t *msg);

/**
 * rpc_server_hci_send_packet
 *
 *  RPC server send HCI packet.
 *
 * @param hci_type - type of HCI packet
 * @param packet   - pointer to HCI packet
 * @param len      - length of HCI packet
 *
 * return - >=0 on SUCCESS(length of sent out HCI packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int rpc_server_hci_send_packet(uint8_t hci_type, uint8_t *packet, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
