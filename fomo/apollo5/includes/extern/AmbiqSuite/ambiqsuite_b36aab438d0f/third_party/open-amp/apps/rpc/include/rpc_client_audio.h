//*****************************************************************************
//
//! @file rpc_client_audio.h
//!
//! @brief Functions and variables related to the audio module client role.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef RPC_CLIENT_AUDIO_H_H
#define RPC_CLIENT_AUDIO_H_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * rpc_client_audio_ep_get
 *
 * Function to get audio endpoint structure's pointer
 *
 * @param NA
 *
 * return - pointer to the audio endpoint structure(refer to struct ipc_ept in ipc_service.h)
 */
void* rpc_client_audio_ep_get(void);
/**
 * rpc_client_audio_send_packet
 *
 *  RPC client send audio packet.
 *
 * @param type     - request type of audio packet
 * @param packet   - packet to send
 * @param len      - length of packet
 *
 * return - >=0 on SUCCESS(length of sent out audio packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int rpc_client_audio_send_packet(uint8_t type, uint8_t *packet, uint16_t len);

/**
 * rpc_client_audio_rx
 *
 * process the received audio endpoint RPMsg from RPC server.
 *
 * @param msg - pointer to the received message
 *
 * return - none
 */
void rpc_client_audio_rx(uint8_t * msg);

/**
 * rpc_client_audio_cb_set
 *
 *  Used to set application callback.
 *
 * @id   - identifier of the callback function
 * @cb   - specific application callback function
 *
 */
void rpc_client_audio_cb_set(p_cb_t cb);
#ifdef __cplusplus
}
#endif
#endif

