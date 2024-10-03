
/**
 *  \file l2cap.h
 *
 *  \brief This is Module Header File for L2CAP Layer.
 */

/*
 *  Copyright (C) 2014. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_L2CAP_
#define _H_L2CAP_

/* ---------------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_device_queue.h"


/* ---------------------------------------------- Global Definitions */

/** L2CAP Header Sizes */
/** Basic L2CAP Header Length */
#define L2CAP_HDR_LEN                                   4

#ifdef L2CAP_SUPPORT_CBFC_MODE
/** L2CAP SDU Header Length for LE Information Frame used in CBFC Mode */
#define L2CAP_CREDIT_MODE_SDU_HDR_LEN                   2

/** L2CAP Header Length for LE Informaton Frame used in CBFC Mode */
#define L2CAP_CREDIT_MODE_HDR_LEN                       \
        (L2CAP_HDR_LEN + L2CAP_CREDIT_MODE_SDU_HDR_LEN)
#endif /* L2CAP_SUPPORT_CBFC_MODE */

/** L2CAP Connect Results */
#define L2CAP_CONNECTION_SUCCESSFUL                               0x0000
#define L2CAP_CONNECTION_PENDING                                  0x0001
#define L2CAP_CONNECTION_REFUSED_PSM_UNSUPPORTED                  0x0002
#define L2CAP_CONNECTION_REFUSED_SECURITY_BLOCK                   0x0003
#define L2CAP_CONNECTION_REFUSED_NO_RESOURCE                      0x0004
#define L2CAP_CONNECTION_REFUSED_AUTHENTICATION_INSUFFICIENT      0x0005
#define L2CAP_CONNECTION_REFUSED_AUTHORIZATION_INSUFFICIENT       0x0006
#define L2CAP_CONNECTION_REFUSED_ENC_KEY_SIZE_INSUFFICIENT        0x0007
#define L2CAP_CONNECTION_REFUSED_ENC_INSUFFICIENT                 0x0008
#define L2CAP_CONNECTION_REFUSED_INVALID_SRC_CID                  0x0009
#define L2CAP_CONNECTION_REFUSED_SRC_CID_ALREADY_ALLOCATED        0x000A
#define L2CAP_CONNECTION_REFUSED_UNACCEPTABLE_PARAMETERS          0x000B

/** L2CAP Connect Status */
#define L2CAP_CONNECTION_NO_FURTHER_INFORMATION         0x0000
#define L2CAP_CONNECTION_AUTHENTICATION_PENDING         0x0001
#define L2CAP_CONNECTION_AUTHORIZATION_PENDING          0x0002

/** L2CAP MTU for Fixed channel 0x0006 (used for SMP) */
#ifdef SMP_LESC
#define L2CAP_SMP_MTU                                   65
#else /* SMP_LESC */
#define L2CAP_SMP_MTU                                   23
#endif /* SMP_LESC */

/**
 * L2CAP MTU for Fixed channel 0x0005 (Used for LE specific
 * L2CAP commands)
 */
#define L2CAP_LE_MTU                                    23

/** L2CAP Parameter Default Values */
#define L2CAP_SIG_MTU                                   L2CAP_LE_MTU

/** L2CAP ACL Transmission Queue Flow Control */
#define L2CAP_TX_QUEUE_FLOW_OFF                         0x01
#define L2CAP_TX_QUEUE_FLOW_ON                          0x00

/** L2CAP Fixed channel max and min Connection Identifiers (CID) */
#define L2CAP_MIN_FIXED_CHANNEL_CID             L2CAP_ATT_CID
#define L2CAP_MAX_FIXED_CHANNEL_CID             L2CAP_SMP_CID

/** L2CAP Maximum number of fixed channels */
#define L2CAP_MAX_FIXED_CHANNELS                \
                L2CAP_MAX_FIXED_CHANNEL_CID -   \
                L2CAP_MIN_FIXED_CHANNEL_CID + 1

#ifdef L2CAP_SUPPORT_CBFC_MODE
/** Maximum number of credits allowed for each L2CAP CBFC channel */
#define L2CAP_MAX_CREDITS                               0xFFFF
#endif /* L2CAP_SUPPORT_CBFC_MODE */

/* LMP Link types */
/** BR/EDR LMP Link type */
#define L2CAP_ACL_LINK                          0x01
/** LE LMP Link type */
#define L2CAP_LE_LINK                           0x03


/**
 * \addtogroup le_l2cap_group L2CAP Low Energy
 * \{
 */
/**
 * \addtogroup l2cap_le_apis L2CAP Low Energy APIs
 * \{
 */
/* Response codes for Connection parameter update request */
/** Connection Parameters accepted */
#define L2CAP_CONNECTION_PARAMETERS_ACCEPTED   0x0000

/** Connection Parameters rejected */
#define L2CAP_CONNECTION_PARAMETERS_REJECTED   0x0001

/* Response Codes for Invalid Command Packets */
/** Command Not Understood */
#define L2CAP_COMMAND_NOT_UNDERSTOOD           0x0000

/** Signaling MTU exceeded */
#define L2CAP_SIGNALLING_MTU_EXCEEDED          0x0001

/** Invalid Connection Identifier in request */
#define L2CAP_INVALID_CID_IN_REQUEST           0x0002

/**
 * \}
 */
/**
 * \}
 */

#ifdef BT_LE

/* L2CAP Fixed Channel Events */
/**
 * \addtogroup le_l2cap_group L2CAP Low Energy
 * \{
 *  This section describes all the interfaces and APIs of Bluetooth Low Energy
 *  L2CAP.
 */
/**
 * \defgroup le_l2cap_fc_events L2CAP Low Energy Fixed Channel Events
 * \{
 */
/**
 * This event indicates L2CAP Fixed Channel connection from remote device.
 * Fixed Channel Event handler registered with L2CAP for specific fixed channel
 * will receive this event, when the underlying ACL link is established,
 * as there is no separate connection procedure for L2CAP Fixed Channel.
 */
#define L2CAP_FIXED_CH_CONNECTED_EVENT         0x01

/**
 * This event indicates L2CAP Fixed Channel disconnection from remote device.
 * Fixed Channel Event handler registered with L2CAP for specific fixed channel
 * will receive this event, when the underlying ACL link is disconnected,
 * as there is no separate disconnection procedure for L2CAP Fixed Channel.
 */
#define L2CAP_FIXED_CH_DISCONNECTED_EVENT      0x02

/**
 * This event indicates data received on L2CAP Fixed Channel from remote device.
 * Fixed Channel Event handler registered with L2CAP for specific fixed channel
 * will receive this event.
 */
#define L2CAP_FIXED_CH_DATA_RX_EVENT           0x03

/**
 * This event indicates data transmitted to L2CAP Fixed Channel from remote device.
 * Fixed Channel Event handler registered with L2CAP for specific fixed channel
 * will receive this event.
 */
#define L2CAP_FIXED_CH_DATA_TX_EVENT           0x04

#ifdef BT_L2CAP_FIXED_CHANNEL_TX_COMPLETE
#define L2CAP_FIXED_CH_MAX_EVENT           L2CAP_FIXED_CH_DATA_TX_EVENT
#else
#define L2CAP_FIXED_CH_MAX_EVENT           L2CAP_FIXED_CH_DATA_RX_EVENT
#endif /* BT_L2CAP_FIXED_CHANNEL_TX_COMPLETE */
/**
 * \}
 */
/**
 * \}
 */

/* L2CAP LE events */
/**
 * \addtogroup le_l2cap_group L2CAP Low Energy
 * \{
 *  This section describes all the interfaces and APIs of Bluetooth Low Energy
 *  L2CAP.
 */
/**
 * \defgroup le_l2cap_events L2CAP Low Energy events
 * \{
 */
/**
 * This event indicates the connection parameter update received from remote
 * device. The application expected to reply to L2CAP using
 * \ref BT_l2cap_le_connection_param_update_response API, to resond to remote
 * device whether parameters are accepted.
 *
 * If the connection parameter update request is accepted then application is
 * expected to perform physical link connection parameter update using
 * \ref BT_hci_le_connection_update.
 */
#define L2CAP_CONNECTION_UPDATE_REQUEST_EVENT  0x01

/**
 * This event indicates the connection parameter update request response
 * received. The result field indicates the reason for rejection. The possible
 * responses are \ref L2CAP_CONNECTION_PARAMETERS_ACCEPTED and \ref
 * L2CAP_CONNECTION_PARAMETERS_REJECTED.
 */
#define L2CAP_CONNECTION_UPDATE_RESPONSE_EVENT 0x02

/**
 * This event indicates the connection parameter update request is rejected.
 * The reason field indicates the reason for rejection.
 * Possible reasons for rejection are
 * - \ref L2CAP_COMMAND_NOT_UNDERSTOOD,
 * - \ref L2CAP_SIGNALLING_MTU_EXCEEDED
 * - \ref L2CAP_INVALID_CID_IN_REQUEST
 */
#define L2CAP_COMMAND_REJECTED_EVENT           0x03
/**
 * \}
 */
/**
 * \}
 */

/* L2CAP connection update error code */
#define L2CAP_CONNECTION_PARAM_REJECTED        0x0001
#endif /* BT_LE */

/* ---------------------------------------------- Structures/Data Types */

#ifdef L2CAP_SUPPORT_CBFC_MODE
/**
 * \addtogroup le_l2cap_group L2CAP Low Energy
 * \{
 */
/**
 *  \defgroup le_l2cap_structures L2CAP Structures
 *  \{
 *  This Section describes the Data Structures exposed to the application
 */

/**
 *  The structure representing the Connection Parameter for
 *  L2CAP Credit Based Flow Control Mode.
 */
typedef struct l2cap_cbfc_connect_param
{
    /**
     * MTU - Maximum SDU Size
     *
     * The MTU field specifies the maximum SDU size (in octets) that the L2CAP
     * layer entity sending the LE Credit Based Connection Request can receive
     * on this channel. L2CAP implementations shall support a minimum MTU size
     * of 23 octets.
     */
    UINT16   mtu;

    /**
     * MPS - Maximum PDU Size
     *
     * The MPS field specifies the maximum payload size (in octets) that the
     * L2CAP layer entity sending the LE Credit Based Connection Request is
     * capable of receiving on this channel. L2CAP implementations shall support
     * a minimum MPS of 23 octets and may support an MPS up to 65533 octets.
     */
    UINT16   mps;

    /**
     * Initial number of Credits
     *
     * The initial credit value indicates the number of LE-frames that the peer
     * device can send to the L2CAP layer entity sending the LE Credit Based
     * Connection Request. The initial credit value shall be in the range of 0 to
     * 65535.
     */
    UINT16   credit;

}L2CAP_CBFC_CONNECT_PARAM;

/**
 *  The structure representing the PSMs. It stores all the callback
 *  functions. The upper layer uses this structure to register itself
 *  with L2CAP.
 */
typedef struct psm_struct_cbfc
{
    /**
     * Callback to inform application of the incoming L2CAP CBFC Connection Request.
     *
     * l2ca_connect_ind_cb provides following parameters to the application:
     *     1. Pointer to Device Queue Handle: the peer device sending the request.
     *     2. CID: Channel Identifier.
     *     3. PSM: LE Protocol/Service Multiplexer.
     *     4. CBFC connect Request Parameters (MTU, MPS and Initial Credits)
     */
    API_RESULT (* l2ca_connect_ind_cb)(DEVICE_HANDLE *, UINT16, UINT16, L2CAP_CBFC_CONNECT_PARAM *);

    /**
     * Callback to inform application of the L2CAP CBFC Connection Response/Confirmation.
     *
     * l2ca_connect_cnf_cb provides following parameters to the application:
     *     1. Pointer to Device Queue Handle: the peer device sending the request.
     *     2. CID: Channel Identifier.
     *     3. Result: A result value of 0x0000 indicates success while a non-zero value
     *                indicates the connection request was refused.
     *     4. CBFC connect Response Parameters (MTU, MPS and Initial Credits)
     */
    API_RESULT (* l2ca_connect_cnf_cb)(DEVICE_HANDLE *, UINT16, UINT16, L2CAP_CBFC_CONNECT_PARAM *);

    /**
     * Callback to inform application of the L2CAP CBFC Disconnection Request
     * received from the Peer device. This callback is for the responder of
     * the L2CAP Disconnection request.
     *
     * l2ca_disconnect_ind_cb provides following parameters to the application:
     *     1. CID: Channel Identifier of a previously connected channel.
     *
     * Note: On receiving this callback, previously connected channel for which
     * disconnection is initiated by the peer device will be considered closed.
     */
    API_RESULT (* l2ca_disconnect_ind_cb)(UINT16);

    /**
     * Callback to inform application of the L2CAP CBFC Disconnection
     * confirmation/Response received from the Peer device. This callback is
     * for the initiator of the L2CAP Disconnection request.
     *
     * l2ca_disconnect_cnf_cb provides following parameters to the application:
     *     1. CID: Channel Identifier of a previously connected channel
     *             for which disconnection was initiated by local device.
     *     2. Result: A result value of 0x0000 indicates success while a
     *                non-zero value indicates the disconnection request
     *                has failed (timed out).
     *
     * Note: Irrespective of the 'Result' is success/failure, once the local
     * device has initiated L2CAP Disconnection, the channel will be
     * considered closed.
     */
    API_RESULT (* l2ca_disconnect_cnf_cb)(UINT16, UINT16);

    /**
     * Callback to inform application of data received over L2CAP CBFC channel.
     *
     * l2ca_data_read_cb provides following parameters to the application:
     *     1. CID: Channel Identifier of a previously connected channel.
     *     2. Result: A result value of 0x0000 indicates success, while a non-zero value
     *                indicates an error condition
     *                (e.g. peer device violating credit flow, or MTU size limit)
     *     3. Buffer Pointer: Data/payload received from the peer device.
     *     4. Buffer Length: Receieved data/payload length.
     */
    API_RESULT (* l2ca_data_read_cb)(UINT16, UINT16, UCHAR *, UINT16);

    /**
     * Callback to inform application of receive credits reached low water mark or below.
     * After receiving L2CAP data/payload from peer device for a specification Channel,
     * the available credits are calculated. If the credit count reaches or goes below
     * the low water mark, this callback is called to inform application of the condition,
     * so that if application wants can send more credits to the peer device.
     *
     * l2ca_rx_credit_ind provides following parameters :
     *     1. CID: Channel Identifier of a previously connected channel.
     *     2. Receive Credits: The current received credits count.
     */
    API_RESULT (* l2ca_rx_credit_ind) (UINT16, UINT16);

    /**
     * Callback to inform application of having received transmit credits.
     * This callback is called on receiving LE Flow Control Credit from peer device.
     *
     * l2ca_tx_credit_ind provides following parameters to the application:
     *     1. CID: Channel Identifier of a previously connected channel.
     *     2. Result: A result value of 0x0000 indicates success, while a non-zero value
     *                indicates an error condition (e.g. credit overflow, if total number of credits
     *                crosses specification defined maximum limit of 0xFFFF)
     *     2. Transmit Credits: The cumulative transmit credit count. This is the number of LE-frames
     *                          that can be sent to the peer device sending the LE Flow Control
     *                          Credit packet.
     */
    API_RESULT (* l2ca_tx_credit_ind) (UINT16, UINT16, UINT16);

    /** The LE Protocol/Service Multiplexer */
    UINT16 psm;

#ifdef L2CAP_CBFC_SUPPORT_UL_CREDIT_LWM
    /** Upper Layer defined Receive Credit Low Water Mark */
    UINT16 credit_lwm;
#endif /* L2CAP_CBFC_SUPPORT_UL_CREDIT_LWM */

    /**
     * Callback to inform application of data transmission completion over L2CAP CBFC channel.
     *
     * l2ca_data_write_cb provides following parameters to the application:
     *     1. CID: Channel Identifier of a previously connected channel.
     *     2. Result: A result value of 0x0000 indicates success, while a non-zero value
     *                indicates an error condition
     *                (e.g. peer device violating credit flow, or MTU size limit)
     *     3. Buffer Pointer: Currently NULL. For future usage.
     *     4. Buffer Length: Currently 0. For future usage.
     */
    API_RESULT (* l2ca_data_write_cb)(UINT16, UINT16, UCHAR *, UINT16);

} PSM_STRUCT_CBFC, L2CAP_PSM_CBFC;

#endif /* L2CAP_SUPPORT_CBFC_MODE */

/**
 * \}
 */

/**
 * \addtogroup l2cap_le_apis L2CAP Low Energy APIs
 * \{
 */
/**
 *  \defgroup le_l2cap_cb Application Callback
 *  \{
 *  This Section Describes the module Notification Callback interface offered
 *  to the application
 */
/**
 *  L2CAP Fixed channel Packet handler.
 */
typedef void (* L2CAP_FIXED_CHANNEL_EVENT_HANDLER)
             (
                 /* Device Handle */
                 /* IN */ DEVICE_HANDLE *,

                 /* Event Type - Connect/Disconnect/Data Rx */
                 /* IN */ UCHAR,

                 /**
                  *  Event -> Parameter
                  *  ------------------
                  *  Connected -> Peer Address, Connection Identifier
                  *  Disconnected -> Connection Identifier
                  *  Data RX -> PDU received
                  */
                 /* IN */ void *,

                 /**
                  *  Event -> Parameter
                  *  ------------------
                  *  Connected -> Size of structure storing
                  *               Peer Address, Connection Identifier
                  *  Disconnected -> Size of Connection Identifier
                  *  Data RX -> Length of received PDU
                  */
                 /* IN */ UINT16
             ) DECL_REENTRANT;

/**
 * LE L2CAP Application Asynchronous Notification Callback.
 *
 * L2CAP calls the registered callback to indicate events occurred to the
 * application.
 *
 * \param handle Identifies peer device on which event occurred.
 * \param event Any of the Event occurred from the list at \ref le_l2cap_events.
 * \param data Data associated with the event if any or NULL.
 * \param data_len Size of the event data. 0 if event data is NULL.
 */
typedef void (* L2CAP_LE_EVENT_HANDLER)
             (
                 /* IN */ DEVICE_HANDLE *,        /* BD_ADDR and Type */
                 /* IN */ UCHAR,                  /* Event */
                 /* IN */ UCHAR *,                /* PDU received */
                 /* IN */ UINT16                  /* length of PDU received */
             ) DECL_REENTRANT;
/**
 * \}
 */
/**
 * \}
 */
/**
 * \}
 */

/* ---------------------------------------------- Macros */


/* ---------------------------------------------- API Declarations */
#ifdef __cplusplus
extern "C"{
#endif

#ifdef L2CAP
/** L2CAP Initialisation & Shutdown */
void em_l2cap_init (void);
void l2cap_bt_init (void);
void l2cap_bt_shutdown ( void );
#else
#define em_l2cap_init()
#define l2cap_bt_init()
#define l2cap_bt_shutdown()
#endif /* L2CAP */

/**
 * \addtogroup le_l2cap_group L2CAP Low Energy
 * \{
 */
/**
 * \addtogroup l2cap_le_apis L2CAP Low Energy APIs
 * \{
 */
#ifdef L2CAP_TX_QUEUE_FLOW
/**
 *  \brief To register a L2CAP Transmission Queue Flow Control callback.
 *
 *  \par Description:
 *  This API registers the L2CAP ACL transmission queue flow control callback.
 *  If registered, L2CAP calls this callback function whenever number of
 *  available buffers, in the ACL transmission queue, goes above or below
 *  pre-configured limits.
 *
 *  \param [in] callback_fn
 *         The callback function to be called when number of available buffers,
 *         in the ACL transmission queue, goes above or below a pre-set limits.
 *         This parameter cannot be NULL.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2cap_register_tx_queue_flow_cb
           (
               /* IN */ API_RESULT (* callback_fn) (UCHAR, UINT16)
           );

/**
 *  \brief To get number of available buffers in ACL Transmission Queue.
 *
 *  \par Description:
 *  This API provides the number of available buffers, in the L2CAP's ACL
 *  transmission queue, to application. The application may use the number
 *  of buffers to implement its flow control logic.
 *
 *  \param [out] num_buffer
 *         The number of available buffer, in the ACL transmission queue,
 *         at the time of calling this callback. This parameter cannot be NULL.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2cap_get_tx_queue_num_buffer
           (
               /* OUT */ UINT32 *    num_buffer
           );

/**
 *  \brief To set the low & high water marks for the ACL transmission queue
 *  buffers.
 *
 *  \par Description:
 *  This API provides a method for application to dynamically change/control
 *  the low & high water marks for ACL transmission queue, based upon which
 *  L2CAP decides to call the L2CAP Transmission Queue Flow Control Callback.
 *
 *  The above callback mechanism is provided in L2CAP to inform application
 *  when the number of available buffers, in the ACL transmission queue, is
 *  equal or less than the configured low water mark. The application can
 *  decide what to do to stop data transmission temporarily, so as to allow
 *  L2CAP for some time to transmit ACL packets from transmission queue to
 *  the lower layer/Controller (LMP/Baseband).
 *
 *  The L2CAP, using the same callback, informs the application when the number
 *  of available buffer, in the ACL transmission queue, becomes equal of more
 *  than the configured high water mark. At this point, application can
 *  re-start data write/transmission.
 *
 *  \param [in] tx_queue_lwm
 *         The Low Water Mark for number of available buffers in the ACL
 *         transmission queue.
 *
 *  \param [in] tx_queue_hwm
 *         The High Water Mark for number of available buffers in the ACL
 *         transmission queue.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2cap_set_tx_queue_flow_limits
           (
               /* IN */ UINT32    tx_queue_lwm,
               /* IN */ UINT32    tx_queue_hwm
           );
#endif /* L2CAP_TX_QUEUE_FLOW */


/**
 *  \brief To inform L2CAP about new ACL Connection.
 *
 *  \par Description:
 *  This API implements the LP_ConnectIND primitive for the lower layer
 *  HCI or LMP. LP_ConnectIND should be called to inform L2CAP about
 *  establishment of new ACL Connection with a remote Bluetooth device.
 *  Successful ACL Connection also marks creation of L2CAP Signaling
 *  Channel, and a Signaling Entity is created for the purpose.
 *
 *  \param [in] device_handle
 *         The DEVICE_HANDLE of the remote device
 *  \param [in] handle
 *         The LMP Connection Handle for the remote device
 *  \param [in] role
 *         The LMP Role of the local device
 *
 *  \return API_RESULT:
 *          API_SUCCESS on success, or, an Error Code (see BT_error.h)
 *          describing the cause of failure.
 */
API_RESULT l2cap_lp_connect_ind
           (
               /* IN */ DEVICE_HANDLE * device_handle,
               /* IN */ UINT16              handle,
               /* IN */ UCHAR               role
           );

/**
 *  \brief To inform L2CAP about ACL Disconnection.
 *
 *  \par Description:
 *  This API implements the LP_DisconnectIND primitive for the lower layer
 *  HCI or LMP. LP_DisconnectIND should be called to inform L2CAP about
 *  disconnection of ACL Connection with a remote Bluetooth device.
 *  Disconnection of ACL Connection also marks deletion of L2CAP Signalling
 *  Channel.
 *
 *  \param [in] handle
 *         The DEVICE_HANDLE of the remote device
 *  \param [in] reason
 *         The Reason for Disconnection of LMP connection
 *
 *  \return API_RESULT:
 *          API_SUCCESS on success, or, an Error Code (see BT_error.h)
 *          describing the cause of failure.
 */
API_RESULT l2cap_lp_disconnect_ind
           (
               /* IN */ DEVICE_HANDLE * handle,
               /* IN */ UCHAR           reason
           );

/**
 *  \brief To inform L2CAP about incoming ACL Data.
 *
 *  \par Description:
 *  This function handles the incoming ACL fragments from the LMP
 *
 *  \param [in] acl_header
 *         Buffer containing the ACL Header from the LMP
 *  \param [in] packet
 *         Buffer containing the L2CAP Payload from the LMP
 *  \param [in] packet_len
 *         Size of the Buffer containing the ACL packet
 *
 *  \return API_RESULT:
 *          API_SUCCESS on success, or, an Error Code (see BT_error.h)
 *          describing the cause of failure.
 */
API_RESULT l2cap_lp_data_read
           (
               /* IN */ UCHAR *  acl_header,
               /* IN */ UCHAR *  packet,
               /* IN */ UINT16   packet_len
           );

/**
 *  \brief To inform L2CAP about ACL Number of Completed Packets event.
 *
 *  \par Description:
 *  This function handles LP_NumCompletedPackets service primitive for the
 *  HCI/LMP. The HCI/LMP must inform L2CAP submitted LMP Fragments containing
 *  L2CAP PDUs are transmitted over the air, and the corresponding LMP
 *  buffer is now free to use.
 *
 *  \param [in] handle
 *         LMP (ACL) Connection Handle
 *  \param [in] num_completed
 *         Number of LMP Fragments sent (completed) for the LMP Handle
 *
 *  \return void
 */
void l2cap_lp_num_completed_packets
     (
         /* IN */ UINT16  handle,
         /* IN */ UINT16  num_completed
     );

/**
 *  \brief To inform L2CAP about ACL Buffer Sizes of Bluetooth Controller.
 *
 *  \par Description:
 *  This function handles LP_ACLBufferSize service primitive for the
 *  HCI/LMP. The HCI/LMP must inform L2CAP about the ACL Buffer sizes
 *  of LMP, so that L2CAP can perform fragmentation of L2CAP PDUs before
 *  they are transmitted to LMP.
 *
 *  \param [in] lmp_packet_size
 *         Maximum LMP Fragment size
 *  \param [in] lmp_max_packets
 *         Number of LMP Fragments that can be outstanding
 *
 *  \return None
 */
void l2cap_lp_le_buffer_size
     (
         /* IN */ UINT16  lmp_packet_size,
         /* IN */ UINT16  lmp_max_packets
     );

/**
 *  \brief To disconnect an established L2CAP channel.
 *
 *  \par Description:
 *  This service primitive initiates sending of an L2CAP Disconnect Request
 *  command to the remote L2CAP entity to initiate disconnection of
 *  the referred L2CAP channel.
 *  Disconnection of the L2CAP channel always succeeds - either by reception
 *  of the L2CAP Disconnect Response message from the peer, or by timeout.
 *  In any case, L2CAP will confirm disconnection of the channel,
 *  by calling the L2CA_DisconnectCNF callback.
 *
 *  \param [in] local_cid
 *         This parameter specifies the local channel end-point for the L2CAP
 *         channel. For initiator of L2CAP channel establishment, this must be
 *         set to the value indicated by the L2CA_ConnectCNF callback.
 *         For responder, upper layer protocol obtains this value when L2CAP
 *         invokes L2CA_ConnectIND callback.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2ca_disconnect_req
           (
               /* IN */  UINT16  local_cid
           );

/**
 *  \brief Register callback with LE L2CAP
 *
 *  \par Description:
 *       This routine registers callback with LE L2CAP
 *
 *  \param [in] callback
 *         LE L2CAP Application Notification Callback
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT BT_l2cap_register_le_event_cb
           (
               /* IN */ L2CAP_LE_EVENT_HANDLER callback
           );

/**
 *  \brief Send Connection Update Request
 *
 *  \par Description:
 *       This routine sends LE L2CAP Connection Update Request.
 *       LE L2CAP Connection Update Request shall be sent
 *       only in Peripheral (slave) role.
 *
 *  \param  [in] device_handle
 *          Device handle of the peer device.
 *
 *  \param  [in] min_interval
 *          Minimum value for the connection event interval.
 *          This shall be less than or equal to Conn_Interval_Max.
 *
 *  \param  [in] max_interval
 *          Maximum value for the connection event interval.
 *          This shall be greater than or equal to Conn_Interval_Min.
 *
 *  \param  [in] slave_latency
 *          Slave latency for the connection in number of connection events.
 *
 *  \param  [in] supervision_timeout
 *          Supervision Timeout for the LE Link.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT BT_l2cap_le_connection_param_update_request
           (
               /* IN */ DEVICE_HANDLE * device_handle,
               /* IN */ UINT16          min_interval,
               /* IN */ UINT16          max_interval,
               /* IN */ UINT16          slave_latency,
               /* IN */ UINT16          supervision_timeout
           );

/**
 *  \brief Send Connection Update Response
 *
 *  \par Description:
 *       This routine sends LE L2CAP Connection Update Response.
 *       On receiving LE L2CAP Connection Update Request,
 *       only the Central (master) device, shall send Connection Update
 *       Response.
 *
 *  \param  [in] device_handle
 *          Device handle of the peer device.
 *
 *  \param  [in] result
 *          The result field indicates the response to the Connection Parameter
 *          Update Request.
 *          The result value of 0x0000 (\ref L2CAP_CONNECTION_PARAMETERS_ACCEPTED)
 *          indicates that LE master Host has accepted the connection parameters.
 *          Value 0x0001 (\ref L2CAP_CONNECTION_PARAMETERS_REJECTED) indicates that
 *          LE master Host has rejected the connection parameters.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT BT_l2cap_le_connection_param_update_response
           (
               /* IN */ DEVICE_HANDLE * device_handle,
               /* IN */ UINT16          result
           );

/**
 *  \brief To write on a L2CAP fixed channel.
 *
 *  \par Description:
 *      This API is used to send a packet on L2CAP fixed channel.
 *
 *  \param [in] device_handle
 *         Device handle of the peer device.
 *  \param [in] cid
 *         Fixed channel identifier
 *  \param [in] buffer
 *         Buffer containing packet to be sent. See note below.
 *  \param [in] buffer_len
 *         Packet length
 *
 *  \return API_RESULT
 *          - API_SUCCESS: If successful.
 *          - Error Codes: Error code describing cause of failure.
 */
API_RESULT l2cap_fixed_channel_data_write
           (
               /* IN */ DEVICE_HANDLE  * device_handle,
               /* IN */ UINT16           cid,
               /* IN */ UCHAR          * buffer,
               /* IN */ UINT16           buffer_len
           );

/**
 *  \brief To register a callback to handle fixed channel events and data.
 *
 *  \par Description:
 *  This API is used to register a callback which can handle incoming
 *  fixed channel data of maximum packet size specified and connect/disconnect
 *  event.
 *
 *  \param [in] cid
 *         Fixed Channel Identifier
 *  \param [in] mtu
 *         Maximum size of the packet callback function can handle
 *  \param [in] callback
 *         Pointer to callback function
 *
 *  \return API_RESULT
 *          - API_SUCCESS: If successful.
 *          - Error Codes: Error code describing cause of failure.
 *
 *  \note
 *  This function does not validate the specified MTU size.
 */
API_RESULT l2cap_register_fixed_channel_cb
           (
               /* IN */ UINT16                            cid,
               /* IN */ UINT16                            mtu,
               /* IN */ L2CAP_FIXED_CHANNEL_EVENT_HANDLER callback
           );

/**
 *  \brief To update the MTU for an already registered fixed channel.
 *
 *  \par Description:
 *  This API can be used to update MTU size of an already registered fixed
 *  channel.
 *
 *  \param [in] device_handle
 *         Device handle of the peer device.
 *  \param [in] cid
 *         Fixed Channel Identifier
 *  \param [in] mtu
 *         Maximum size of the packet can be received on the associated fixed
 *         channel
 *
 *  \return API_RESULT
 *          - API_SUCCESS: If successful.
 *          - Error Codes: Error code describing cause of failure.
 */
API_RESULT l2cap_update_fixed_channel_mtu
           (
               /* IN */ DEVICE_HANDLE * device_handle,
               /* IN */ UINT16          cid,
               /* IN */ UINT16          mtu
           );

#ifdef L2CAP_SUPPORT_CBFC_MODE
/**
 *  \brief To register a higher level protocol (PSM) which supports CBFC mode
 *  over L2CAP.
 *
 *  \par Description:
 *  This API registers a new upper layer protocol, or, PSM to L2CAP,
 *  along with the set of callbacks, for the L2CAP Credit Based Flow Control
 *  mode.
 *
 *  \param [in] psm
 *         This parameter must be a pointer to the L2CAP_PSM_CBFC variable
 *         containing the PSM value of the higher-level protocol and
 *         the set of callbacks for this protocol (PSM).
 *
 *  \return API_RESULT
 *          - API_SUCCESS: If successful.
 *          - Error Codes: Error code describing cause of failure.
 */
API_RESULT l2cap_cbfc_register_psm
           (
               /* IN */ L2CAP_PSM_CBFC   * psm
           );

/**
 *  \brief To de-register an Upper Layer LE_PSM from L2CAP.
 *
 *  \par Description:
 *  This API de-registers a upper layer protocol, or, LE_PSM from L2CAP,
 *  for the L2CAP Credit Based Flow Control mode.
 *
 *  \param [in] psm
 *         Already registered LE Protocol/Service Multiplexer for L2CAP
 *         Credit Based Flow Control mode.
 *
 *  \return API_RESULT
 *          - API_SUCCESS: If successful.
 *          - Error Codes: Error code describing cause of failure.
 *  \note
 *  It must be noted that this routine can succeed for a registered
 *  LE_PSM only if there exists no active L2CAP channels for it.
 */
API_RESULT l2cap_cbfc_unregister_psm
           (
               /* IN */ UINT16  psm
           );

/**
 *  'l2ca_' APIs are those called from the upper layer to send a relevant
 *  command from L2CAP to the peer. These are the interface functions
 *  L2CAP and the upper layer.
 */

/** L2CA Utility APIs */

/**
 *  \brief To establish L2CAP channel for LE Credit Based Flow Control mode.
 *
 *  \par Description:
 *  This L2CAP service interface initiates L2CAP channel establishment
 *  procedure for CBFC mode, to the specified remote Bluetooth device,
 *  for the specified PSM representing an upper layer protocol above L2CAP.
 *
 *  \param [in] handle
 *         Device handle of the peer device.
 *  \param [in] remote_psm
 *         Remote PSM, representing the upper layer protocol above L2CAP.
 *  \param [in] local_psm
 *         Local PSM, representing the upper layer protocol above L2CAP.
 *  \param [in] param
 *         This parameter must be a pointer to the L2CAP_CBFC_CONNECT_PARAM
 *         variable containing the connection parameters for the L2CAP channel.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2ca_cbfc_connect_req
           (
               /* IN */ DEVICE_HANDLE            * handle,
               /* IN */ UINT16                     remote_psm,
               /* IN */ UINT16                     local_psm,
               /* IN */ L2CAP_CBFC_CONNECT_PARAM * param
           );

/**
 *  \brief To respond to an incoming L2CAP connection request for
 *  LE Credit Based Flow Control mode.
 *
 *  \par Description:
 *  This L2CAP service interface enables an upper layer protocol to respond
 *  to L2CAP connection request for LE Credit Based Flow Control mode
 *  for the specified PSM from the specified remote Bluetooth device.
 *  It is mandatory that the upper layer PSM always responds back by calling
 *  this service interface upon receiving CBFC Connection Request.
 *  The channel is established (opened) only when the PSM concerned responds
 *  back with successful response. Otherwise, the channel establishment request
 *  from the peer will be rejected by L2CAP with appropriate result
 *  and status as received from the upper layer PSM.
 *
 *  \param [in] local_cid
 *         This parameter specifies the local L2CAP channel end-point for
 *         this new L2CAP channel. On receipt of L2CAP Connect Request command
 *         from the peer, local L2CAP will temporarily create a channel -
 *         this parameter identifies the new channel.
 *         If the upper layer PSM chooses to reject this connection,
 *         this temporary channel will be closed.
 *  \param [in] response
 *         This parameter specifies the response of the upper layer for
 *         the new L2CAP channel establishment request from the peer.
 *         It must be set to a value as specified in L2CAP Connect Result Codes.
 *  \param [in] param
 *         This parameter must be a pointer to the L2CAP_CBFC_CONNECT_PARAM
 *         variable containing the connection parameters for the L2CAP channel.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2ca_cbfc_connect_rsp
           (
               /* IN */ UINT16 local_cid,
               /* IN */ UINT16 response,
               /* IN */ L2CAP_CBFC_CONNECT_PARAM * param
           );
/**
 *  \brief To send LE Flow Control Credit packet.
 *
 *  \par Description:
 *  This L2CAP service interface enables an upper layer protocol to send
 *  LE Flow Control Credit packet to remote Bluetooth device, when it is
 *  capable of receiving additional LE-frames.
 *
 *  \param [in] local_cid
 *         This parameter specifies the local channel end-point for the L2CAP
 *         channel. For initiator of L2CAP channel establishment, this must be
 *         set to the value indicated by the L2CA_ConnectCNF callback.
 *         For responder, upper layer protocol obtains this value when L2CAP
 *         invokes L2CA_ConnectIND callback.
 *  \param [in] credit
 *         The credit value field represents number of credits the receiving
 *         device can increment, corresponding to the number of LE-frames
 *         that can be sent to the peer device sending the LE Flow Control
 *         Credit packet.
 *         The credit value field shall be a number between 1 and 65535.
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 */
API_RESULT l2ca_cbfc_send_flow_control_credit
           (
               /* IN */ UINT16 local_cid,
               /* IN */ UINT16 credit
           );

/**
 *  \brief To write on an L2CAP CBFC Channel.
 *
 *  \par Description:
 *  This API is used to send a packet on L2CAP CBFC channel.
 *
 *  \param [in] device_handle
 *         Device handle of the peer device.
 *  \param [in] cid
 *         This parameter specifies the local channel end-point for the L2CAP
 *         channel. For initiator of L2CAP channel establishment, this must be
 *         set to the value indicated by the L2CA_ConnectCNF callback.
 *         For responder, upper layer protocol obtains this value when L2CAP
 *         invokes L2CA_ConnectIND callback.
 *  \param [in] buffer
 *         Buffer containing packet to be sent
 *  \param [in] buffer_len
 *         Packet length
 *
 *  \return API_SUCCESS or an error code indicating reason for failure
 *
 *  \note
 *
 *  LE Information frame (LE-Frame) packet format:
 *
 *    16    16    16       n
 *  +-----+-----+-----+---...---+
 *  |  L  |  C  |  S  |    I    |
 *  +-----+-----+-----+---...---+
 *  LSB                       MSB
 *  <-   BLH  ->
 *
 *  BLH: Basic L2CAP Header (4 Octets).
 *  L: Length (2 Octets) and C: Channel ID (2 Octets) of Basic L2CAP Header.
 *  S: L2CAP SDU Length (2 Octets) present in first LE Frame of the SDU.
 *  I: Information Payload ('n' Octets)
 *     For first LE-frame of the SDU, 'n' = L - 2
 *     For subsequent LE-frames, 'n' = L
 *
 *  There are two ways 'buffer' and 'buffer_len' can be passed to this interface,
 *  and that dependts on if 'L2CAP_ALLOC_FOR_CBFC_SDU' feature flag is defined
 *  or not.
 *
 *  If 'L2CAP_ALLOC_FOR_CBFC_SDU' feature flag is defined:
 *  - 'buffer' shall only contain Information Payload.
 *  - 'buffer_len' shall contain the length of the Information Payload.
 *  - L2CAP will allocate memory for complete LE-frame, including associated
 *    header and copy the application provided Information Payload in 'buffer'.
 *    Application can free (if allocated) or reuse the 'buffer' once this API
 *    returns.
 *  - If the 'buffer' is allocated, application can free the memory
 *    on return of this API.
 *
 *  If 'L2CAP_ALLOC_FOR_CBFC_SDU' feature flag is not defined:
 *  - 'buffer' shall have additional 6 Octets of memory for L2CAP header,
 *    and have L2CAP SDU Length field and Information Payload filled.
 *    L2CAP will costruct the remaining Basic L2CAP Header fields,
 *    in the 'buffer' provided by the caller.
 *  - 'buffer_len' shall contain the length 'L2CAP SDU Length' field
 *    in addition to the length of Information Payload (i.e., all the octets
 *    filled by the caller).
 *  - L2CAP will assume the caller has provided an allocate memory for
 *    the complete LE-frame. L2CAP will frame the Basic L2CAP Header in the
 *    provied buffer. L2CAP will free the application 'buffer' once the
 *    transmission of SDU is complete.
 *  - If the 'buffer' is allocated, application shall not free the memory
 *    on return of this API. Application shall free the buffer in the handling
 *    of corresponding Tx complete callback l2ca_data_write_cb().
 *
 *  In general, for memory constaint devices, 'L2CAP_ALLOC_FOR_CBFC_SDU'
 *  should not be defined.
 *  This is to avoid allocation of buffer for L2CAP SDU in the L2CAP implementation
 *  and also to avoid copying the data from application buffer to L2CAP buffer
 *  (as much as possible).
 *  In some cases where the SDU has to be segmented to smaller PDUs, based on the MTU
 *  and MPS value, L2CAP needs to frame the PDUs from the SDU provided by the application.
 */
API_RESULT l2ca_channel_data_write
            (
                /* IN */ DEVICE_HANDLE * device_handle,
                /* IN */ UINT16          cid,
                /* IN */ UCHAR         * buffer,
                /* IN */ UINT16          buffer_len
            );
#endif /* L2CAP_SUPPORT_CBFC_MODE */


/**
 * \}
 */
/**
 * \}
 */

/* ---------------------------------------------- Platform Definitions */


/* ---------------------------------------------- Feature Definitions */


/* ---------------------------------------------- Test Hooks */

#ifdef __cplusplus
};
#endif

#endif /* _H_L2CAP_ */

