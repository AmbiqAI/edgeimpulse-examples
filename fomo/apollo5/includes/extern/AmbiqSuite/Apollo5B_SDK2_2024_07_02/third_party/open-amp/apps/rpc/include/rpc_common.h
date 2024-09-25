//*****************************************************************************
//
//! @file rpc_common.h
//!
//! @brief Functions and variables related to RPC client and server.
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

#ifndef RPC_COMMON_H_H
#define RPC_COMMON_H_H

#include <stdint.h>
#include "compiler.h"
#include "ipc_service.h"
#include "open_amp.h"
#include "event_groups.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */

/// Application RPMsg data header structure length(see structure #RPMsgCommandHdr_t)
#define RPMSG_PDU_HEADER_LEN        (4)
/// Application RPMsg data header reserved field length (see structure #RPMsgCommandHdr_t)
#define RPMSG_PDU_HEADER_RSVD_LEN   (1)
/// maximum NVM database length
#define NVM_DB_LENGTH_MAX           (200)
// RPMsg header length
#define RPRPMSG_PDU_HEADER_LEN      (16)

/// Application RPMsg payload default length
#define RPMSG_PAYLOAD_LEN           (1)
/// Bluetooth address length
#define BD_ADDR_LEN                 (6)
/// Application RPMsg audio endpoint name
#define RPMSG_AUDIO_EP_NAME         "rpmsg_audio_ep"
/// Application RPMsg syscntl endpoint name
#define RPMSG_SYS_CNTL_EP_NAME      "rpmsg_syscntl_ep"
/// Application RPMsg debug endpoint name
#define RPMSG_DEBUG_EP_NAME         "rpmsg_debug_ep"
#define IPC_DATA_QUEUE_SIZE   (16)

/*
 * Macros
 ****************************************************************************************
 */


/// Converting a little endian byte buffer to integers.
#define BYTES_TO_UINT16(n, p)     {n = ((uint16_t)(p)[0] + ((uint16_t)(p)[1] << 8));}

#define BYTES_TO_UINT24(n, p)     {n = ((uint16_t)(p)[0] + ((uint16_t)(p)[1] << 8) + \
                                        ((uint16_t)(p)[2] << 16));}

#define BYTES_TO_UINT32(n, p)     {n = ((uint32_t)(p)[0] + ((uint32_t)(p)[1] << 8) + \
                                        ((uint32_t)(p)[2] << 16) + ((uint32_t)(p)[3] << 24));}


/// Converting a little endian byte stream to integers, with increment.
#define BSTREAM_TO_INT8(n, p)     {n = (int8_t)(*(p)++);}
#define BSTREAM_TO_UINT8(n, p)     {n = (uint8_t)(*(p)++);}
#define BSTREAM_TO_UINT16(n, p)    {BYTES_TO_UINT16(n, p); p += 2;}
#define BSTREAM_TO_UINT32(n, p)    {BYTES_TO_UINT32(n, p); p += 4;}

#define BSTREAM_TO_BDADDR(bda, p)   {uint8_t ijk; uint8_t *pbda = (uint8_t *)bda + BD_ADDR_LEN - 1; for (ijk = 0; ijk < BD_ADDR_LEN; ijk++) *pbda-- = *p++;}


/// Converting integers to a little endian byte stream, with increment.
#define UINT8_TO_BSTREAM(p, n)     {*(p)++ = (uint8_t)(n);}
#define UINT16_TO_BSTREAM(p, n)    {*(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8);}
#define UINT32_TO_BSTREAM(p, n)    {*(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                    *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24);}

#define BDADDR_TO_STREAM(p, bda)    {uint8_t ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (uint8_t) bda[BD_ADDR_LEN - 1 - ijk];}

/// Locate to offset position of p
#define LOCATE_RPMSG_DATA(p, offset) ((uint8_t *)(p) + offset)

/// Start of component message enumeration
#define RPMSG_BTDM_MSG_START(id)     ((id) << 8)
///  Get the component ID from a message ID
#define RPMSG_BTDM_ID_FROM_MSG(msg)  ((msg) >> 8)

/// Check if pointer parameter is NULL
#define check_pointer_return(p)       \
    if ((p) == NULL)                  \
    {                                 \
        metal_assert(false);          \
        return RPMSG_ERR_PARAM;       \
    }

/// Check if pointer parameter is NULL, without return value
#define check_pointer_void(p)         \
    if((p) == NULL)                   \
    {                                 \
        metal_assert(false);          \
        return;                       \
    }

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

typedef enum
{
    // data queue for btdm IPC request
    IPC_DATA_QUEUE_BTDM_REQ_TX,
    // Nvm IPC data
    IPC_DATA_QUEUE_NVM_REQ_TX,
    // Audio IPC request
    IPC_DATA_QUEUE_AUDIO_REQ_TX,
    // HCI MSG Received from controller
    IPC_DATA_QUEUE_HCI_MSG_RX,

    IPC_DATA_QUEUE_TYPE_MAX
} ipc_data_queue_t;

typedef enum
{
    // received data from remote IPC
    IPC_WAIT_EVENT_MBOX_RX,
    // send out IPC data
    IPC_WAIT_EVENT_BTDM_TX,
    // nvm IPC sending event
    IPC_EVENT_NVM_TX,
    // audio sending event
    IPC_EVENT_AUDIO_TX,
    // hci sending event
    IPC_EVENT_HCI_TX,
    // hci msg received
    IPC_EVENT_HCI_RX,

    IPC_EVENT_MAX
}ipc_wait_event_group_t;

#define IPC_MBOX_RX_BIT     (1 << IPC_WAIT_EVENT_MBOX_RX)
#define IPC_BTDM_TX_BIT     (1 << IPC_WAIT_EVENT_BTDM_TX )
#define IPC_NVM_TX_BIT      (1 << IPC_EVENT_NVM_TX)
#define IPC_AUDIO_TX_BIT    (1 << IPC_EVENT_AUDIO_TX)
#define IPC_HCI_TX_BIT      (1 << IPC_EVENT_HCI_TX)
#define IPC_HCI_RX_BIT      (1 << IPC_EVENT_HCI_RX)

#define IPC_WAITING_ALL_BITS    (IPC_MBOX_RX_BIT|IPC_BTDM_TX_BIT \
                                |IPC_NVM_TX_BIT|IPC_AUDIO_TX_BIT \
                                |IPC_HCI_TX_BIT|IPC_HCI_RX_BIT)

/// Application RPMsg module
typedef enum
{
    RPMSG_AUDIO_MODULE             = 0x01,

    RPMSG_SYSTEM_MODULE            = 0x02,
    RPMSG_BTDM_MODULE              = 0x03,
    RPMSG_NVM_DB_MODULE            = 0x04,
    RPMSG_HCI_MODULE               = 0x05,

    RPMSG_DEBUG_MODULE             = 0x06,
}eRpmsgModule;

/// Application RPMsg packet type for audio module
typedef enum
{
    RPMSG_AUDIO_REQ_ACK,

    RPMSG_AUDIO_RECORD_PLAYBACK_BASE = 1,
    RPMSG_AUDIO_RECORD_START         = RPMSG_AUDIO_RECORD_PLAYBACK_BASE,
    RPMSG_AUDIO_RECORD_STOP,
    RPMSG_AUDIO_PLAYBACK_START,
    RPMSG_AUDIO_PLAYBACK_STOP,
    RPMSG_AUDIO_RECORD_PLAYBACK_END,

    RPMSG_AUDIO_DATA_FLOW_BASE = 10,
    RPMSG_AUDIO_DATA_TRANS     = RPMSG_AUDIO_DATA_FLOW_BASE,
    RPMSG_AUDIO_DSP_NEED_DATA,
    RPMSG_AUDIO_DATA_FLOW_END,

    RPMSG_AUDIO_LOADER_BASE = 20,
    RPMSG_AUDIO_LOADER_REQ  = RPMSG_AUDIO_LOADER_BASE,
    RPMSG_AUDIO_LOADER_RSP,
    RPMSG_AUDIO_LOADER_END,

    RPMSG_AUDIO_REQ_TYPE_MAX
} eAudioReq;

/// Application RPMsg packet type for system module
typedef enum
{
    RPMSG_SYSTEM_VOLUME_SETTING     = 0x01,
    RPMSG_SYSTEM_POWER_RESET        = 0x02,
    RPMSG_SYSTEM_POWER_OFF          = 0x03,

} eSystemReq;

/// Packet type for RPMsg request from RPC client to RPC server
typedef enum
{
    // BLE GAP message definition
    RPMSG_BTDM_BLE_ADV_START,
    RPMSG_BTDM_BLE_ADV_STOP,//TODDO implement RPMsg request API
    RPMSG_BTDM_BLE_SCAN_START,//TODDO implement RPMsg request API
    RPMSG_BTDM_BLE_SCAN_STOP,//TODDO implement RPMsg request API
    RPMSG_BTDM_BLE_CONN_CREATE,//TODDO implement RPMsg request API
    RPMSG_BTDM_BLE_CONN_CANCEL,//TODDO implement RPMsg request API
    RPMSG_BTDM_BLE_UPDATE_BAS_LVL,

    // BT GAP message definition
    RPMSG_BTDM_BT_INQUIRY_START,
    RPMSG_BTDM_BT_INQUIRY_STOP,
    RPMSG_BTDM_BT_CONN_CREATE,
    RPMSG_BTDM_BT_CONN_CANCEL,
    RPMSG_BTDM_BT_INQ_SCAN_START,
    RPMSG_BTDM_BT_INQ_SCAN_STOP,
    RPMSG_BTDM_BT_PAGE_SCAN_START,
    RPMSG_BTDM_BT_PAGE_SCAN_STOP,

    // Device configure message definition
    RPMSG_BTDM_APP_START,

    RPMSG_BTDM_HFP_ANSWER_INCOMING_CALL,
    RPMSG_BTDM_HFP_REJECT_INCOMING_CALL,
    RPMSG_BTDM_HFP_INIT_OUTGOING_CALL,
    RPMSG_BTDM_HFP_HANGUP_CALL,
    RPMSG_BTDM_HFP_SET_SPEAKER_VOL,
    RPMSG_BTDM_HFP_UPDATE_BATTERY_LVL,

    RPMSG_BTDM_BT_AVRCP_STOP,
    RPMSG_BTDM_BT_AVRCP_PLAY,
    RPMSG_BTDM_BT_AVRCP_SET_VOL,
    RPMSG_BTDM_BT_AVRCP_PLAY_FORWARD,
    RPMSG_BTDM_BT_AVRCP_PLAY_BACKWARD,

    RPMSG_BTDM_BT_A2DP_SRC_START,
    RPMSG_BTDM_BT_A2DP_SRC_SUSPEND,
    RPMSG_BTDM_BT_A2DP_SRC_CLOSE,

    // throughput profile request message
    RPMSG_BTDM_THRPUT_DATA_SEND,

} eBTDMReq;

/// Packet type for RPMsg event from RPC server to RPC client
typedef enum
{
    // acknowledgement for synchronous reqeust from RPC client
    RPMSG_REQ_ACK,
    RPMSG_BTDM_BLE_ADV_REPORT,
    RPMSG_BTDM_BLE_OTA_TRANSFER,
    RPMSG_BTDM_BLE_OTA_RCV_HEADER,
    RPMSG_BTDM_BLE_UPDATE_OTA_FALG,
    RPMSG_BTDM_BLE_OTA_CMPLT,
    RPMSG_BTDM_BLE_ANCS_MSG,

    RPMSG_BTDM_LE_CONN_CMPLT,
    RPMSG_BTDM_LE_DISCONNECTED,

    RPMSG_BTDM_BT_INQUIRY_STOPPED,
    RPMSG_BTDM_BT_INQ_REPORT,
    RPMSG_BTDM_BT_CONN_CMPLT,
    RPMSG_BTDM_BT_DISCONNECTED,
    RPMSG_BTDM_BONDED,

    // A2DP sink state
    RPMSG_BTDM_A2DP_SINK_STREAM_STARTED,
    RPMSG_BTDM_A2DP_SINK_STREAM_SUSPENDED,
    RPMSG_BTDM_A2DP_SINK_STREAM_CLOSED,
    // A2DP source state
    RPMSG_BTDM_A2DP_SRC_STREAM_STARTED,
    RPMSG_BTDM_A2DP_SRC_STREAM_SUSPENDED,
    RPMSG_BTDM_A2DP_SRC_STREAM_CLOSED,

    RPMSG_BTDM_HFP_SVC_LVL_ESTABLISHED,
    RPMSG_BTDM_HFP_INCOMING_CALL_RCVD,
    RPMSG_BTDM_HFP_CALL_STARTED,
    RPMSG_BTDM_HFP_CALL_TERMINATED,
    RPMSG_BTDM_HFP_AUDIO_TERMINATED,
    // AVRCP state
    RPMSG_BTDM_AVRCP_VOL_CHANGED,

    // throughput profile messages
    RPMSG_BTDM_THRPUT_DATA_REQ,
    RPMSG_BTDM_THRPUT_RCV_DATA,
}eBTDMEvt;

/// Application RPMsg packet type for NVM module
typedef enum
{
    RPMSG_NVM_DB_LOAD               = 0x01,
    RPMSG_NVM_DB_UPDATE             = 0x02,
} eNvmReq;

typedef enum
{
    NVM_ITEM_DEV_BOND_INFO,
    NVM_ITEM_HFP_VOL,
    NVM_ITEM_AVRCP_VOL,
}nvm_item_type_t;

/// Application RPMsg packet type for HCI module
typedef enum
{
    RPMSG_HCI_CMD_TYPE              = 0x01,
    RPMSG_HCI_ACL_TYPE              = 0x02,
    RPMSG_HCI_EVT_TYPE              = 0x04,
    RPMSG_HCI_ISO_TYPE              = 0x05,
} eHciReq;

/// Application RPMsg packet type for debug module
typedef enum
{
    RPMSG_SYSTEM_DUMP_LOG           = 0x01,
} eDebugReq;

/// Application RPMsg endpoints
typedef enum {
    // audio stream endpoint
    APP_RPMSG_AUDIO_STREAM_EP,
    // system control endpoint, BT/BLE message/NVM/HCI
    APP_RPMSG_SYS_CNTL_EP,
    // debug endpoint
    APP_RPMSG_DEBUG_EP,

    APP_RPMSG_EP_MAX
}eAppRPMsgEpIndex;


/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
    uint8_t     type;
    uint16_t    len;
    uint8_t     *data;
} ipc_data_t;

/// Application RPMsg data header structure
typedef struct
{
    // RPMsg request module
    uint8_t    rpmsg_module;
    // reserved byte
    uint8_t    reserved;
    // PDU payload length
    uint16_t   rpmsg_payload_len;
}RPMsgCommandHdr_t;

/// Application HCI message structure
/// HCI message header length(see structure #hciRPMsgHdr_t)
#define RPMSG_HCI_PDU_HEADER_LEN (sizeof(struct hciRPMsgHdr_t))
/// Define maximum data payload of HCI module
#define RPMSG_HCI_PAYLOAD_MAX      (RPMSG_BUFFER_SIZE-RPRPMSG_PDU_HEADER_LEN-RPMSG_HCI_PDU_HEADER_LEN)

METAL_PACKED_BEGIN
struct hciRPMsgHdr_t {
    // rpmsg_payload_len includes packet type and payload length
    RPMsgCommandHdr_t rpmsg_hdr;
}METAL_PACKED_END;

typedef struct
{
    struct hciRPMsgHdr_t hdr;
    uint8_t       payload[RPMSG_PAYLOAD_LEN];
}hciRPMsgType;

/// Application BTDM message structure
// BTDM message type length
#define RPMSG_BTDM_PDU_TYPE_LEN     (1)
/// BTDM message header length(see structure #btDmRPMsgHdr_t)
#define RPMSG_BTDM_PDU_HEADER_LEN   (sizeof(struct btDmRPMsgHdr_t))
/// Define maximum data payload of BTDM module
#define RPMSG_BTDM_PAYLOAD_MAX      (RPMSG_BUFFER_SIZE-RPRPMSG_PDU_HEADER_LEN-RPMSG_BTDM_PDU_HEADER_LEN)

METAL_PACKED_BEGIN
struct btDmRPMsgHdr_t {
    // rpmsg_payload_len includes packet type and payload length
    RPMsgCommandHdr_t rpmsg_hdr;
    uint8_t          type;
}METAL_PACKED_END;

typedef struct
{
    struct btDmRPMsgHdr_t    hdr;
    uint8_t           payload[RPMSG_PAYLOAD_LEN];
}btDmRPMsgType;

/// Application nvm message structure
// NVM message type length
#define RPMSG_NVM_PDU_TYPE_LEN (1)
/// BTDM message header length(see structure #nvmRPMsgHdr_t)
#define RPMSG_NVM_PDU_HEADER_LEN (sizeof(struct nvmRPMsgHdr_t))

/// Define maximum data payload of NVM module
#define RPMSG_NVM_PAYLOAD_MAX                                                  \
    (RPMSG_BUFFER_SIZE - RPRPMSG_PDU_HEADER_LEN - RPMSG_NVM_PDU_HEADER_LEN)

    METAL_PACKED_BEGIN
    struct nvmRPMsgHdr_t
    {
        // rpmsg_payload_len includes packet type and payload length
        RPMsgCommandHdr_t rpmsg_hdr;
        uint8_t              type;
    } METAL_PACKED_END;

    typedef struct
    {
        struct nvmRPMsgHdr_t hdr;
        uint8_t              payload[RPMSG_PAYLOAD_LEN];
    } nvmRPMsgType;

/// Application system message structure
/// System message header length(see structure #sysRPMsgHdr_t)
#define RPMSG_SYSTEM_PDU_HEADER_LEN (sizeof(struct sysRPMsgHdr_t))

METAL_PACKED_BEGIN
struct sysRPMsgHdr_t {
    // rpmsg_payload_len includes packet type and payload length
    RPMsgCommandHdr_t rpmsg_hdr;
    uint8_t          type;
}METAL_PACKED_END;

typedef struct
{
    struct sysRPMsgHdr_t     hdr;
    uint8_t           payload[RPMSG_PAYLOAD_LEN];
}systemRPMsgType;

/// Application debug message structure
typedef struct
{
    RPMsgCommandHdr_t rpmsg_hdr;

    uint8_t       payload[RPMSG_PAYLOAD_LEN];
}debugRPMsgType;

/// Application audio message structure
// Audio message type length
#define RPMSG_AUDIO_PDU_TYPE_LEN     (1)
/// Audio message header length(see structure #audioRPMsgHdr_t)
#define RPMSG_AUDIO_PDU_HEADER_LEN   (sizeof(struct audioRPMsgHdr_t))
#define RPMSG_AUDIO_BUF_SIZE         (RPMSG_BUFFER_SIZE-RPRPMSG_PDU_HEADER_LEN-RPMSG_AUDIO_PDU_HEADER_LEN)

METAL_PACKED_BEGIN
struct audioRPMsgHdr_t {
    // rpmsg_payload_len includes packet type and payload length
    RPMsgCommandHdr_t rpmsg_hdr;
    uint8_t          type;
}METAL_PACKED_END;

/// Application audio message structure
typedef struct
{
    struct audioRPMsgHdr_t   hdr;
    uint8_t           payload[RPMSG_PAYLOAD_LEN];
}audioRPMsgType;

typedef void (*p_cb_t)(uint8_t type, uint8_t *evt_pkt, uint16_t len);
typedef void (*hci_handler_cb_t)(uint8_t* pui8Data, uint32_t ui32Length);

// call return value decoded by user decoder handler.
extern uint16_t m_return_value;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
int rpc_send_packet_sync(struct ipc_ept*ep, uint8_t *buf, uint16_t len);

/*
 * rpc_request_rsp_wait
 *
 * Function called while waiting for RPC request response. It handles incoming events.
 *
 */
void rpc_request_rsp_wait(void);

/**
 * rpc_request_is_busy
 *
 * Check if there is RPC request in progress.
 *
 * return - true if PRC request is pending, false if RPC request is complete.
 */
bool rpc_request_is_busy(void);

/**
 * rpc_update_rsp_wait_flag
 *
 * Function called to update wait flag, set to true when request started, false when receive response.
 *
 * @param flag   - wait flag, false means received response, true means starting to send request
 */
void rpc_update_rsp_wait_flag(bool flag);

/**
 * ipc_create_event
 *
 * Function called to create IPC event
 *
 */
void ipc_create_event(void);

/**
 * ipc_wait_event
 *
 * Function called to wait IPC event
 *
 * return  - the waited event bits
 */
EventBits_t ipc_wait_event(void);

/**
 * ipc_set_event
 *
 * Function called to set IPC event
 *
 * @param event_type the event type to set
 */
void ipc_set_event(uint8_t event_type);

/**
 * ipc_data_queue_init
 *
 * Function called to initiate IPC data queue
 *
 * @param type   - to indicate which type of data queue to initialize
 */
void ipc_data_queue_init(uint8_t type);

/**
 * ipc_data_queue_pop
 *
 * Function called to pop out IPC data from queue
 *
 * @param type   - to indicate which type of data queue to pop, see the packet type from enum #ipc_data_queue_t
 *
 * return - NULL if the specified type of queue is empty,
 *          or the element in the queue
 */
ipc_data_t *ipc_data_queue_pop(uint8_t type);

/**
 * ipc_data_queue_push
 *
 * Function called to push element to IPC data queue
 *
 * @param type   - to indicate which type of data queue to push, see the packet type from enum #ipc_data_queue_t
 * @param p_buf  - pointer to the data element to push into the queue
 */
void ipc_data_queue_push(uint8_t type, ipc_data_t *p_buf);


/**
 * ipc_btdm_req_send
 *
 * Function called to send btdm IPC request
 *
 * @param type     - request type of BTDM packet
 * @param packet   - packet to send
 * @param len      - length of packet
 *
 * return - >=0 on SUCCESS(length of sent out HCI packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int ipc_btdm_req_send(uint16_t type, uint8_t *packet, uint16_t len);

/**
 * ipc_btdm_data_send
 *
 * Function called to send btdm IPC data, this works in asynchronous mode
 *
 * @param type     - request type of BTDM packet
 * @param packet   - packet to send
 * @param len      - length of packet
 *
 * return - >=0 on SUCCESS(length of sent out HCI packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet
 */
int ipc_btdm_data_send(uint16_t type, uint8_t *packet, uint16_t len);

/**
 * @brief Enqueue data, and notify ipc task to send it. This API will block and
 * won't return until receives the remote device's ack within 100ms or time out.
 *
 * @param type   Audio command type.
 * @param packet Audio data packet pointer that will be sent.
 * @param len    Length of audio data @ref packet.
 *
 * @retval RPMSG_SUCCESS.
 * @retval RPMSG_ERR_NO_BUFF If no memory to store the packet.
 * @retval GAP_ERR_TIMEOUT.
 * @retval Other status. The remote device return status.
 */
int ipc_audio_req_send(uint16_t type, uint8_t *packet, uint16_t len);

/**
 * @brief Enqueue data, and notify ipc task to send it. This API won't check the
 * sending status and will return immediately after notifying ipc task.
 *
 * @param type   Audio command type.
 * @param packet Audio data packet pointer that will be sent.
 * @param len    Length of audio data @ref packet.
 *
 * @retval RPMSG_SUCCESS.
 * @retval RPMSG_ERR_NO_BUFF If no memory to store the packet.
 */
int ipc_audio_data_send(uint16_t type, uint8_t *packet, uint16_t len);

/**
 * ipc_set_tx_event
 *
 * Function called to continue send the btdm ipc message after wakeup the ambt53
 *
 */
void ipc_set_tx_event(void);
#ifdef __cplusplus
}
#endif
#endif
