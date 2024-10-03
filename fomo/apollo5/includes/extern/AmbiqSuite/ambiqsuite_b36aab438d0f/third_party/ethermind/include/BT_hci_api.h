
/**
 *  \file BT_hci_api.h
 *
 *  \brief This Header File contains the API Function declarations of the APIs
 *  exported by the Mindtree Bluetooth Stack for the HCI Layer.
 */

/*
 *  Copyright (C) 2014. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_HCI_API_
#define _H_BT_HCI_API_

/**
 * \defgroup hci_module HCI (Host Controller Interface)
 * \{
 */

/* =================================================== Header File Inclusion */
#include "BT_common.h"

/**
 * \defgroup hci_defines Defines
 * \{
 */

/**
 * \defgroup hci_constants Constants
 * \{
 */

/* =================================================== Constants/Definitions */
/** HCI Device Role - Master/Slave */
/** Device Role Master */
#define BT_DEVICE_ROLE_MASTER                            0x00
/** Device Role Slave */
#define BT_DEVICE_ROLE_SLAVE                             0x01

/** Invalid HCI Connection Handle */
#define HCI_INVALID_CONNECTION_HANDLE                    0xFFFF

/** Maximum Advertising Data Length */
#define HCI_MAX_ADVERTISING_DATA_LENGTH                  0x1F

/** Advertising Enable/Disable */
/** Advertising disabled */
#define HCI_ADVERTISING_DISABLE                          0x00
/** Advertising enabled */
#define HCI_ADVERTISING_ENABLE                           0x01

/** Advertising Type */
/** Connectable undirected advertising */
#define HCI_ADV_IND                                      0x00
/** Connectable directed advertising */
#define HCI_ADV_DIRECT_IND                               0x01
/** Scannable undirected advertising */
#define HCI_ADV_SCAN_IND                                 0x02
/** Non connectable undirected advertising */
#define HCI_NONCONN_IND                                  0x03

/** Scan Respond Event Type */
#define HCI_SCAN_RSP                                     0x04

/** Advertising Filter Policy */
/** Allow Scan Request from Any, Allow Connect Request from Any */
#define HCI_ADV_FLTR_SCAN_REQ_ANY_CON_REQ_ANY            0x00
/** Allow Scan Request from White List Only, Allow Connect Request from Any */
#define HCI_ADV_FLTR_SCAN_REQ_WH_LIST_CON_REQ_ANY        0x01
/** Allow Scan Request from Any, Allow Connect Request from White List Only */
#define HCI_ADV_FLTR_SCAN_REQ_ANY_CON_REQ_WH_LIST        0x02
/**
 * Allow Scan Request from White List Only, Allow Connect Request
 * from White List Only.
 */
#define HCI_ADV_FLTR_SCAN_REQ_WH_LIST_CON_REQ_WH_LIST    0x03

/** LE Scan Enable/Disable */
/** Scanning disabled */
#define HCI_LE_SCAN_DISABLE                              0x00
/** Scanning enabled */
#define HCI_LE_SCAN_ENABLE                               0x01

/** LE Scan Filter Duplicates Enable/Disable */
/** Duplicate filtering disabled */
#define HCI_LE_SCAN_DUP_FLTR_DISABLE                     0x00
/** Duplicate filtering enabled */
#define HCI_LE_SCAN_DUP_FLTR_ENABLE                      0x01

/** LE Scan Type */
/** Passive Scanning. No SCAN_REQ packets shall be sent */
#define HCI_LE_SCAN_TYPE_PASSIVE                         0x00
/** Active scanning. SCAN_REQ packets may be sent */
#define HCI_LE_SCAN_TYPE_ACTIVE                          0x01

/** LE Scanning Filter Policy */
/**
 * Accept all advertisement packets.
 * Directed advertising packets which are not addressed for this device
 * shall be ignored.
 */
#define HCI_LE_SCAN_FLTR_ACCEPT_ALL                      0x00

/**
 * Ignore advertisement packets from devices not in the White List Only.
 * Directed advertising packets which are not addressed for this device
 * shall be ignored.
 */
#define HCI_LE_SCAN_FLTR_USE_WH_LIST                     0x01

/** LE Initiator Filter Policy */
/**
 * White list is not used to determine which advertiser to connect to.
 * Peer_Address_Type and Peer_Address shall be used.
 */
#define HCI_INIT_FLTR_DONOT_USE_WH_LIST                  0x00
/**
 * White list is used to determine which advertiser to connect to.
 * Peer_Address_Type and Peer_Address shall be ignored.
 */
#define HCI_INIT_FLTR_USE_WH_LIST                        0x01

/** LE Transmitter Test Packet Payload */
/** Pseudo-Random bit sequence 9 */
#define HCI_LE_TX_TEST_PKT_TYPE_00                       0x00
/** Pattern of alternating bits '11110000' */
#define HCI_LE_TX_TEST_PKT_TYPE_01                       0x01
/** Pattern of alternating bits '10101010' */
#define HCI_LE_TX_TEST_PKT_TYPE_02                       0x02
/** Pseudo-Random bit sequence 15 */
#define HCI_LE_TX_TEST_PKT_TYPE_03                       0x03
/** Pattern of All '1' bits */
#define HCI_LE_TX_TEST_PKT_TYPE_04                       0x04
/** Pattern of All '0' bits */
#define HCI_LE_TX_TEST_PKT_TYPE_05                       0x05
/** Pattern of alternating bits '00001111' */
#define HCI_LE_TX_TEST_PKT_TYPE_06                       0x06
/** Pattern of alternating bits '0101' */
#define HCI_LE_TX_TEST_PKT_TYPE_07                       0x07

/** Transmit Power Level Type */
/** Read Current Transmit Power Level */
#define HCI_READ_CUR_TX_POWER_LEVEL                      0x00
/** Read Maximum Transmit Power Level */
#define HCI_READ_MAX_TX_POWER_LEVEL                      0x01

/** Connection Parameter Range */
/** Connection Interval Minimum - 0x0006 to 0x0C80 */
#define HCI_LE_MIN_CONN_INTRVL_MIN_RANGE                 0x0006
#define HCI_LE_MIN_CONN_INTRVL_MAX_RANGE                 0x0C80

/** Connection Interval Maximum - 0x0006 to 0x0C80 */
#define HCI_LE_MAX_CONN_INTRVL_MIN_RANGE                 0x0006
#define HCI_LE_MAX_CONN_INTRVL_MAX_RANGE                 0x0C80

/** Connection Latency - 0x0000 to 0x01F4 */
#define HCI_LE_CONN_LATENCY_MIN_RANGE                    0x0000
#define HCI_LE_CONN_LATENCY_MAX_RANGE                    0x01F4

/** Supervision Timeout - 0x000A to 0x0C80 */
#define HCI_LE_SUPERVISION_TO_MIN_RANGE                  0x000A
#define HCI_LE_SUPERVISION_TO_MAX_RANGE                  0x0C80

/**
 *  HCI Packet Types - exchanged between
 *  the HCI & HCI-Transport Layers.
 */
#define HCI_COMMAND_PACKET                               0x01
#define HCI_ACL_DATA_PACKET                              0x02
#define HCI_SCO_DATA_PACKET                              0x03
#define HCI_EVENT_PACKET                                 0x04

/**
 * \defgroup hci_command_opcodes Command Opcodes
 * \{
 */

/* +++ HCI Command OpCodes ++++++++++++++++++++++++++++++++++++++++ */
/*
 * HCI Commands defined for BLE Single Mode, with corresponding feature flags
 *
 * Note:
 * ## Commands primarily required for Master(M) and Slave(S) are marked
 * ## Remaining commands are applicable for both roles
 *
 * -- Reset Command
 *    <HCI_RESET_SUPPORT>
 *
 * -- Read Local Version Information Command
 *    <HCI_READ_LOCAL_VERSION_INFORMATION_SUPPORT>
 *
 * -- Read Local Supported Features Command
 *    <HCI_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT>
 *
 * -- Read BD_ADDR Command
 *    <HCI_READ_BD_ADDR_SUPPORT>
 *
 * -- Set Event Mask Command
 *    <HCI_SET_EVENT_MASK_SUPPORT>
 *
 * -- Disconnect Command
 *    <HCI_DISCONNECT_SUPPORT>
 *
 * -- Read Remote Version Information Command
 *    <HCI_READ_REMOTE_VERSION_INFORMATION_SUPPORT>
 *
 * -- Read Transmit Power Level Command
 *    <HCI_READ_TRANSMIT_POWER_LEVEL_SUPPORT>
 *
 * -- Read RSSI Command
 *    <HCI_READ_RSSI_SUPPORT>
 *
 * -- Read Buffer Size Command
 *    <HCI_READ_BUFFER_SIZE_SUPPORT>
 *
 * -- Host Buffer Size Command
 *    <HCI_HOST_BUFFER_SIZE_SUPPORT>
 *
 * -- Set Controller To Host Flow Control Command
 *    <HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL_SUPPORT>
 *
 * -- Host Number Of Completed Packets Command
 * -- Read Local Supported Commands Command
 *    <HCI_READ_LOCAL_SUPPORTED_COMMANDS_SUPPORT>
 *
 * -- LE Read Buffer Size Command
 *    <HCI_LE_READ_BUFFER_SIZE_SUPPORT>
 *
 * -- LE Read Local Supported Features Command
 *    <HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT>
 *
 * -- LE Read Supported States Command
 *    <HCI_LE_READ_SUPPORTED_STATES_SUPPORT>
 *
 * -- LE Add Device To White List Command
 *    <HCI_LE_ADD_DEVICE_TO_WHITE_LIST_SUPPORT>
 *
 * -- LE Clear White List Command
 *    <HCI_LE_CLEAR_WHITE_LIST_SUPPORT>
 *
 * -- LE Read White List Size Command
 *    <HCI_LE_READ_WHITE_LIST_SIZE_SUPPORT>
 *
 * -- LE Remove Device From White List Command
 *    <HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_SUPPORT>
 *
 * -- LE Set Event Mask Command
 *    <HCI_LE_SET_EVENT_MASK_SUPPORT>
 *
 * -- LE Test End Command
 *    <HCI_LE_TEST_END_SUPPORT>
 *
 * -- LE Set Advertise Enable Command (S)
 *    <HCI_LE_SET_ADVERTISING_ENABLE_SUPPORT>
 *
 * -- LE Set Advertising Data Command (S)
 *    <HCI_LE_SET_ADVERTISING_DATA_SUPPORT>
 *
 * -- LE Set Advertising Parameters Command (S)
 *    <HCI_LE_SET_ADVERTISING_PARAMETERS_SUPPORT>
 *
 * -- LE Set Random Address Command
 *    <HCI_LE_SET_RANDOM_ADDRESS_SUPPORT>
 *
 * -- LE Read Advertising Channel TX Power Command
 *    <HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_SUPPORT>
 *
 * -- LE Transmitter Test Command
 *    <HCI_LE_TRANSMITTER_TEST_COMMAND_SUPPORT>
 *
 * -- LE Set Scan Enable Command (M)
 *    <HCI_LE_SET_SCAN_ENABLE_SUPPORT>
 *
 * -- LE Set Scan Parameters Command (M)
 *    <HCI_LE_SET_SCAN_PARAMETERS_SUPPORT>
 *
 * -- LE Receiver Test Command
 *    <HCI_LE_RECEIVER_TEST_COMMAND_SUPPORT>
 *
 * -- LE Set Scan Response Data Command (S)
 *    <HCI_LE_SET_SCAN_RESPONSE_DATA_SUPPORT>
 *
 * -- LE Create Connection Cancel Command (M)
 *    <HCI_LE_CREATE_CONNECTION_CANCEL_SUPPORT>
 *
 * -- LE Create Connection Command (M)
 *    <HCI_LE_CREATE_CONNECTION_SUPPORT>
 *
 * -- LE Read Remote Used Features Command
 *    <HCI_LE_READ_REMOTE_USED_FEATURES_SUPPORT>
 *
 * -- LE Connection Update Command (M)
 *    <HCI_LE_CONNECTION_UPDATE_SUPPORT>
 *
 * -- LE Set Host Channel Classification Command (M)
 *    <HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_SUPPORT>
 *
 * -- LE Read Channel Map Command
 *    <HCI_LE_READ_CHANNEL_MAP_SUPPORT>
 *
 * -- LE Encrypt Command
 *    <HCI_LE_ENCRYPT_SUPPORT>
 *
 * -- LE Long Term Key Requested Reply Command
 *    <HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_SUPPORT>
 *
 * -- LE Long Term Key Requested Negative Reply Command
 *    <HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_SUPPORT>
 *
 * -- LE Rand Command
 *    <HCI_LE_RAND_SUPPORT>
 *
 * -- LE Start Encryption Command (M)
 *    <HCI_LE_START_ENCRYPTION_SUPPORT>
 */

/* No Operation Opcode */
#define HCI_NO_OPERATION_OPCODE                                 0x0000

/* Link Control Commands - OGF : 0x01 */
#define HCI_DISCONNECT_OPCODE                                   0x0406
#define HCI_READ_REMOTE_VERSION_INFORMATION_OPCODE              0x041D

/* Link Policy Commands - OGF : 0x02 */

/* Host Controller & BaseBand Commands - OGF : 0x03 */
#define HCI_SET_EVENT_MASK_OPCODE                               0x0C01
#define HCI_RESET_OPCODE                                        0x0C03
#define HCI_READ_TRANSMIT_POWER_LEVEL_OPCODE                    0x0C2D
#define HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL_OPCODE     0x0C31
#define HCI_HOST_BUFFER_SIZE_OPCODE                             0x0C33
#define HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_OPCODE             0x0C35
#define HCI_SET_EVENT_MASK_PAGE_2_OPCODE                        0x0C63

#define HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_OPCODE           0x0C7B
#define HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_OPCODE          0x0C7C

/* Informational Parameters - OGF : 0x04 */
#define HCI_READ_LOCAL_VERSION_INFORMATION_OPCODE               0x1001
#define HCI_READ_LOCAL_SUPPORTED_COMMANDS_OPCODE                0x1002
#define HCI_READ_LOCAL_SUPPORTED_FEATURES_OPCODE                0x1003
#define HCI_READ_BUFFER_SIZE_OPCODE                             0x1005
#define HCI_READ_BD_ADDR_OPCODE                                 0x1009

/* Status Parameters - OGF : 0x05 */
#define HCI_READ_RSSI_OPCODE                                    0x1405

/* Testing Commands - OGF : 0x06 */

/* Bluetooth Low Energy Commands */
#define HCI_LE_SET_EVENT_MASK_OPCODE                            0x2001
#define HCI_LE_READ_BUFFER_SIZE_OPCODE                          0x2002
#define HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_OPCODE             0x2003
#define HCI_LE_SET_RANDOM_ADDRESS_OPCODE                        0x2005
#define HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE                0x2006
#define HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_OPCODE         0x2007
#define HCI_LE_SET_ADVERTISING_DATA_OPCODE                      0x2008
#define HCI_LE_SET_SCAN_RESPONSE_DATA_OPCODE                    0x2009
#define HCI_LE_SET_ADVERTISING_ENABLE_OPCODE                    0x200A
#define HCI_LE_SET_SCAN_PARAMETERS_OPCODE                       0x200B
#define HCI_LE_SET_SCAN_ENABLE_OPCODE                           0x200C
#define HCI_LE_CREATE_CONNECTION_OPCODE                         0x200D
#define HCI_LE_CREATE_CONNECTION_CANCEL_OPCODE                  0x200E
#define HCI_LE_READ_WHITE_LIST_SIZE_OPCODE                      0x200F
#define HCI_LE_CLEAR_WHITE_LIST_OPCODE                          0x2010
#define HCI_LE_ADD_DEVICE_TO_WHITE_LIST_OPCODE                  0x2011
#define HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_OPCODE             0x2012
#define HCI_LE_CONNECTION_UPDATE_OPCODE                         0x2013
#define HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_OPCODE           0x2014
#define HCI_LE_READ_CHANNEL_MAP_OPCODE                          0x2015
#define HCI_LE_READ_REMOTE_USED_FEATURES_OPCODE                 0x2016
#define HCI_LE_ENCRYPT_OPCODE                                   0x2017
#define HCI_LE_RAND_OPCODE                                      0x2018
#define HCI_LE_START_ENCRYPTION_OPCODE                          0x2019
#define HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_OPCODE             0x201A
#define HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_OPCODE    0x201B
#define HCI_LE_READ_SUPPORTED_STATES_OPCODE                     0x201C
#define HCI_LE_RECEIVER_TEST_COMMAND_OPCODE                     0x201D
#define HCI_LE_TRANSMITTER_TEST_COMMAND_OPCODE                  0x201E
#define HCI_LE_TEST_END_OPCODE                                  0x201F
#define HCI_LE_REMOTE_CONN_PARAM_REQ_RPLY_OPCODE                0x2020
#define HCI_LE_REMOTE_CONN_PARAM_REQ_NEG_RPLY_OPCODE            0x2021

/* BT 4.2 Specification */
#define HCI_LE_SET_DATA_LENGTH_OPCODE                           0x2022
#define HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LEN_OPCODE           0x2023
#define HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LEN_OPCODE          0x2024
#define HCI_LE_READ_LOCAL_P_256_PUBLIC_KEY_OPCODE               0x2025
#define HCI_LE_GENERATE_DHKEY_OPCODE                            0x2026
#define HCI_LE_ADD_DEV_TO_RESOLVING_LIST_OPCODE                 0x2027
#define HCI_LE_REMOVE_DEV_FROM_RESOLVING_LIST_OPCODE            0x2028
#define HCI_LE_CLR_RESOLVING_LIST_OPCODE                        0x2029
#define HCI_LE_READ_RESOLVING_LIST_SIZE_OPCODE                  0x202A
#define HCI_LE_READ_PEER_RESOLVABLE_ADDR_OPCODE                 0x202B
#define HCI_LE_READ_LOCAL_RESOLVABLE_ADDR_OPCODE                0x202C
#define HCI_LE_SET_ADDR_RESOLUTION_ENABLE_OPCODE                0x202D
#define HCI_LE_SET_RESOLVABLE_PRIVATE_ADDR_TIMEOUT_OPCODE       0x202E
#define HCI_LE_READ_MAXIMUM_DATA_LENGTH_OPCODE                  0x202F

/* BT 5.0 Specification */
#define HCI_LE_READ_PHY_OPCODE                                  0x2030
#define HCI_LE_SET_DEFAULT_PHY_OPCODE                           0x2031
#define HCI_LE_SET_PHY_OPCODE                                   0x2032
#define HCI_LE_ENHANCED_RECEIVER_TEST_OPCODE                    0x2033
#define HCI_LE_ENHANCED_TRANSMITTER_TEST_OPCODE                 0x2034

/* HCI LE Advertising Extension support related defines */
#define HCI_LE_SET_ADVERTISING_SET_RANDOM_ADDRESS_OPCODE        0x2035
#define HCI_LE_SET_EXTENDED_ADV_PARAMS_OPCODE                   0x2036
#define HCI_LE_SET_EXTENDED_ADVERTISING_DATA_OPCODE             0x2037
#define HCI_LE_SET_EXTENDED_SCAN_RESPONSE_DATA_OPCODE           0x2038
#define HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_OPCODE             0x2039
#define HCI_LE_READ_MAX_ADV_DATA_LENGTH_OPCODE                  0x203A
#define HCI_LE_READ_NUM_SUPPORTED_ADV_SETS_OPCODE               0x203B
#define HCI_LE_REMOVE_ADVERTISING_SET_OPCODE                    0x203C
#define HCI_LE_CLEAR_ADVERTISING_SETS_OPCODE                    0x203D
#define HCI_LE_SET_PERIODIC_ADV_PARAMS_OPCODE                   0x203E
#define HCI_LE_SET_PERIODIC_ADVERTISING_DATA_OPCODE             0x203F
#define HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE_OPCODE           0x2040
#define HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_OPCODE              0x2041
#define HCI_LE_SET_EXTENDED_SCAN_ENABLE_OPCODE                  0x2042
#define HCI_LE_EXTENDED_CREATE_CONNECTION_OPCODE                0x2043
#define HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_OPCODE          0x2044
#define HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_CANCEL_OPCODE   0x2045
#define HCI_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC_OPCODE       0x2046
#define HCI_LE_ADD_DEV_TO_PERIODIC_ADVSER_LIST_OPCODE           0x2047
#define HCI_LE_RM_DEV_FRM_PERIODIC_ADVSER_LIST_OPCODE           0x2048
#define HCI_LE_CLEAR_PERIODIC_ADVERTISER_LIST_OPCODE            0x2049
#define HCI_LE_READ_PERIODIC_ADVERTISER_LIST_SIZE_OPCODE        0x204A

#define HCI_LE_READ_TRANSMIT_POWER_OPCODE                       0x204B
#define HCI_LE_READ_RF_PATH_COMPENSATION_OPCODE                 0x204C
#define HCI_LE_WRITE_RF_PATH_COMPENSATION_OPCODE                0x204D
#define HCI_LE_SET_PRIVACY_MODE_OPCODE                          0x204E

/* Vendor Specific Parameters - OGF : 0x3F */

/** \} */

/** \} */

/** \} */

/**
 * \defgroup hci_events Events
 * \{
 */

/* +++ HCI Event Codes ++++++++++++++++++++++++++++++++++++++++++++ */

#define HCI_DISCONNECTION_COMPLETE_EVENT                        0x05

#define HCI_ENCRYPTION_CHANGE_EVENT                             0x08

#define HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT      0x0C

#define HCI_COMMAND_COMPLETE_EVENT                              0x0E
#define HCI_COMMAND_STATUS_EVENT                                0x0F
#define HCI_HARDWARE_ERROR_EVENT                                0x10

#define HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT                   0x13

#define HCI_DATA_BUFFER_OVERFLOW_EVENT                          0x1A

#define HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT               0x30

#define HCI_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_EVENT         0x57

/**
 *  LE Meta Event is used to encapsulate all LE Controller specific events.
 *  Subevnt code the first octect of the event parameters.
 *  The possible subevnt codes are:
 *   - \ref HCI_LE_CONNECTION_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_ADVERTISING_REPORT_SUBEVENT
 *   - \ref HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT
 *   - \ref HCI_LE_REMOTE_CONN_PARAM_REQ_SUBEVENT
 *   - \ref HCI_LE_DATA_LENGTH_CHANGE_SUBEVENT
 *   - \ref HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_GENERATE_DHKEY_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_DIRECT_ADVERTISING_REPORT_SUBEVENT
 *   - \ref HCI_LE_PHY_UPDATE_COMPLETE_SUBEVENT
 *   - \ref HCI_LE_EXTENDED_ADVERTISING_REPORT_SUBEVENT
 *   - \ref HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_SUBEVENT
 *   - \ref HCI_LE_PERIODIC_ADVERTISING_REPORT_SUBEVENT
 *   - \ref HCI_LE_PERIODIC_ADVERTISING_SYNC_LOST_SUBEVENT
 *   - \ref HCI_LE_SCAN_TIMEOUT_SUBEVENT
 *   - \ref HCI_LE_ADVERTISING_SET_TERMINATED_SUBEVENT
 *   - \ref HCI_LE_SCAN_REQUEST_RECEIVED_SUBEVENT
 *   - \ref HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT
 */
#define HCI_LE_META_EVENT                                       0x3E

/* Bluetooth Low Energy Subevents */
/**
 *  The LE Connection Complete subevent indicates to both of the devices
 *  forming the connection that a new connection has been created.
 */
#define HCI_LE_CONNECTION_COMPLETE_SUBEVENT                     0x01

/**
 *  The LE Advertising Report subevent indicates that a Bluetooth device or
 *  multiple Bluetooth devices have responded to an active scan or received
 *  some information during a passive scan.
 */
#define HCI_LE_ADVERTISING_REPORT_SUBEVENT                      0x02

/**
 *  The LE Connection Update Complete subevent is used to indicate that the
 *  Controller process to update the connection has completed.
 */
#define HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT              0x03

/**
 *  The LE Read Remote Used Features Complete subevent is used to indicate the
 *  completion of the process of the Controller obtaining the used features
 *  of the remote Bluetooth device.
 */
#define HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT      0x04

/**
 *  The LE Long Term Key Requested subevent indicates that the master device is
 *  attempting to encrypt or re-encrypt the link and is requesting the Long Term
 *  key.
 */
#define HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT                 0x05

/**
 *  The LE Remote connection Paramter Request subevent indicates that the master
 *  device or the slave device has received a change in connection paramters
 *  request from its peer.
 */
#define HCI_LE_REMOTE_CONN_PARAM_REQ_SUBEVENT                   0x06

/**
 *  The LE Data Length Change subevent notifies the Host of a change to either the
 *  maximum Payload length or the maximum transmission time of Data Channel
 *  PDUs in either direction. The values reported are the maximum that will
 *  actually be used on the connection following the change.
 */
#define HCI_LE_DATA_LENGTH_CHANGE_SUBEVENT                      0x07

/**
 *  The LE Read Local P-256 Public Key Complete subevent is generated when
 *  local P-256 key generation is complete.
 */
#define HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_SUBEVENT     0x08

/**
 *  The LE Generate DHKey Complete subevent indicates that LE Diffie Hellman
 *  key generation has been completed by the Controller.
 */
#define HCI_LE_GENERATE_DHKEY_COMPLETE_SUBEVENT                 0x09

/**
 *  The LE Enhanced Connection Complete subevent indicates to both of the Hosts
 *  forming the connection that a new connection has been created.
 */
#define HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT            0x0A

/**
 *  The LE Direct Advertising Report subevent indicates that directed advertisements
 *  have been received where the advertiser is using a resolvable private address
 *  and the Scan_Filter_Policy is equal to 0x02 or 0x03.
 */
#define HCI_LE_DIRECT_ADVERTISING_REPORT_SUBEVENT               0x0B

/**
 *  The LE PHY Update Complete subevent indicates that the Controller has changed the
 *  transmitter PHY or receiver PHY in use.
 */
#define HCI_LE_PHY_UPDATE_COMPLETE_SUBEVENT                     0x0C

/**
 *  The LE Extended Scan Ended subevent indicates that scanning has finished.
 */
#define HCI_LE_EXTENDED_ADVERTISING_REPORT_SUBEVENT             0x0D

/**
 *  The LE Periodic Advertising Sync Established Event indicates that the
 *  Controller has received the first periodic advertising packet from an
 *  advertiser after the LE_Periodic_Advertising_Create_Sync Command has
 *  been sent to the Controller.
 */
#define HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_SUBEVENT   0x0E

/**
 *  The LE Periodic Advertising Data Receive subevent provides the Host with
 *  the data received from a Periodic Advertising packet.
 */
#define HCI_LE_PERIODIC_ADVERTISING_REPORT_SUBEVENT             0x0F

/**
 *  The LE Periodic Advertising Receive Ended subevent indicates when the
 *  Controller has lost synchronization with the Periodic Advertiser
 */
#define HCI_LE_PERIODIC_ADVERTISING_SYNC_LOST_SUBEVENT          0x10

/**
 *  The LE Extended Advertising Ended subevent indicates that advertising in
 *  a given advertising set has finished
 */
#define HCI_LE_SCAN_TIMEOUT_SUBEVENT                            0x11

/**
 *  The LE Extended Advertising Ended subevent indicates that advertising in
 *  a given advertising set has finished
 */
#define HCI_LE_ADVERTISING_SET_TERMINATED_SUBEVENT              0x12

/**
 *  The LE Scan Request Received subevent indicates that a SCAN_REQ PDU or an
 *  AUX_SCAN_REQ PDU has been received by the advertiser.
 */
#define HCI_LE_SCAN_REQUEST_RECEIVED_SUBEVENT                   0x13

/**
 *  The LE Channel Selection Algorithm subevent indicates which channel selection
 *  algorithm is used on a data channel connection.
 */
#define HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT             0x14

/* \} */

#ifdef HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_SUPPORT
/* HCI Extended Scan Parameters related Mask Values */
#define HCI_LE_SET_EXT_SCAN_PARAM_1M_PHY_MASK                   0x01
#define HCI_LE_SET_EXT_SCAN_PARAM_2M_PHY_MASK                   0x02
#define HCI_LE_SET_EXT_SCAN_PARAM_CODED_PHY_MASK                0x04
#endif /* HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_SUPPORT */
/**
 * \defgroup hci_utility_macros Utility Macros
 * \{
 */

/* =================================================== Macros */
#define hci_pack_1_byte_param BT_PACK_LE_1_BYTE_VAL

#define hci_pack_2_byte_param BT_PACK_LE_2_BYTE_VAL

#define hci_pack_3_byte_param BT_PACK_LE_3_BYTE_VAL

#define hci_pack_4_byte_param BT_PACK_LE_4_BYTE_VAL

#define hci_unpack_1_byte_param BT_UNPACK_LE_1_BYTE

#define hci_unpack_2_byte_param BT_UNPACK_LE_2_BYTE

#define hci_unpack_3_byte_param BT_UNPACK_LE_3_BYTE

#define hci_unpack_4_byte_param BT_UNPACK_LE_4_BYTE

/** \} */

/**
 * \addtogroup hci_defines Defines
 * \{
 */

/**
 * \defgroup hci_structures Structures
 * \{
 */

/* =================================================== Structures/Data Types */
/* HCI Command/API Opcode */
typedef struct
{
    UCHAR  ogf;
    UINT16 ocf;

} HCI_OPCODE;

/* HCI Connection List */
typedef struct
{
    /* ACL Connection Handle */
    UINT16 acl_handle;

    /* BD_ADDR of the Remote Device */
    BT_DEVICE_ADDR  bd_addr;

    /* Current Mode of the ACL Link */
    UCHAR  current_mode;

} HCI_CONNECTION_LIST;

/** \} */

/** \} */

/**
 * \defgroup hci_callack Application Callback
 * \{
 */

#ifdef HCI_TX_RUN_TIME_SELECTION
typedef API_RESULT (* HCI_TRANSPORT_CB)
                   (
                       UCHAR    packet_type,
                       UCHAR *  packet_data,
                       UINT16   packet_len,
                       UCHAR    flag
                   ) DECL_REENTRANT;

API_RESULT BT_hci_register_tx_cb (/* IN */ HCI_TRANSPORT_CB callback_ptr);
#endif /* HCI_TX_RUN_TIME_SELECTION */

API_RESULT BT_hci_register_event_indication_callback
           (
               API_RESULT (* callback_ptr)
                          (
                              UINT8    event_code,
                              UINT8 *  event_data,
                              UINT8    event_datalen
                          )
           );

/** \} */

/**
 * \defgroup hci_api API Definitions
 * \{
 */

/* ==========================================================  API Functions */

#ifdef __cplusplus
extern "C"{
#endif

/* Internal Functions (Not APIs) ------------------------------------------- */
/* \cond ignore_this Not to be documented - not used by application */
API_RESULT hci_write_command_UCHAR
           (
               UINT16    opcode,
               UCHAR     param
           );

API_RESULT hci_write_command_UINT16
           (
               UINT16    opcode,
               UINT16    param
           );

API_RESULT hci_write_command_UCHAR_array
           (
               UINT16    opcode,
               UCHAR   * param,
               UINT16    length
           );

API_RESULT hci_common_api_handler_no_params
           (
               UINT16    opcode
           );

#ifdef BT_4_2
API_RESULT hci_write_command_BD_ADDR_with_TYPE
           (
               UINT16    opcode,
               UCHAR     addr_type,
               UCHAR   * bd_addr
           );
#endif /* BT_4_2 */

API_RESULT hci_write_command_connection_handle
           (
               UINT16    opcode,
               UINT16    connection_handle
           );

API_RESULT hci_write_command_adv_scan_response_data
           (
               UINT16   opcode,
               UCHAR    data_len,
               UCHAR *  data_param
           );
/** \endcond */
/* ------------------------------------------------------------------------- */


/* HCI External APIs ------------------------------------------------------- */

#ifndef HCI_LITE
API_RESULT BT_hci_decode_opcode
           (
               UINT16        opcode,
               HCI_OPCODE *  hci_opcode
           );
#endif /* HCI_LITE */

API_RESULT BT_hci_get_local_bd_addr
           (
               UCHAR *  bd_addr
           );

API_RESULT BT_hci_get_connection_details
           (
               HCI_CONNECTION_LIST *   hci_conn_list,
               UCHAR                   num_allocated,
               UCHAR *                 num_present
           );

#ifdef BT_LE
API_RESULT BT_hci_own_conn_bd_address
           (
               /* IN */  BT_DEVICE_ADDR *  remote_bd_addr,
               /* OUT */ BT_DEVICE_ADDR *  bd_addr
           );
#endif /* BT_LE */

#ifdef HCI_HAVE_INIT_COMMAND_MASK
API_RESULT BT_hci_set_init_command_mask
           (
               UINT32    command_mask
           );
#endif /* HCI_HAVE_INIT_COMMAND_MASK */

#ifdef BT_SUPPORT_CONTROLLER_INIT_SKIP
API_RESULT BT_hci_set_controller_init (/* IN */ UCHAR state);
#endif /* BT_SUPPORT_CONTROLLER_INIT_SKIP */

API_RESULT BT_hci_get_connection_handle
           (
               BT_DEVICE_ADDR * bd_addr,
               UINT16 *  handle
           );

API_RESULT BT_hci_get_connection_type
           (
               UCHAR *   bd_addr,
               UCHAR     bd_addr_type,
               UCHAR *   conn_type
           );

#ifdef HCI_SUPPORT_GET_DEVICE_ROLE
/**
 *  \brief To get role of the local device.
 *
 *  \par Description:
 *       This API can be used to get the local device role
 *       (Master or Slave) in a HCI LE connection.
 *
 *       If the local device has a HCI LE connection,
 *       role will be one of the following values
 *       - BT_DEVICE_ROLE_MASTER
 *       - BT_DEVICE_ROLE_SLAVE
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_get_device_role
           (
               BT_DEVICE_ADDR * bd_addr,
               UCHAR          * role
           );
#endif /* HCI_SUPPORT_GET_DEVICE_ROLE */

#define BT_hci_get_acl_connection_handle(bd_addr, handle) \
        BT_hci_get_connection_handle((bd_addr), \
        BT_BD_PUBLIC_ADDRESS_TYPE, (handle));
/**
 *  \brief To get LE connection handle for specified Bluetooth Address and
 *         Address type.
 *
 *  \par Description:
 *      This API retrives the LE connection handle, if one exist, for a
 *      specified remote Bluetooth Address and Address type as given by bd_addr
 *      and bd_addr_type paramters respectively.
 *
 *  \param [in] bd_addr
 *         The Remote Bluetooth Device Address for which the Connection
 *         Handle to retrieved.
 *  \param [out] handle
 *         Pointer to a caller allocated UINT16 type variable onto which the
 *         Connection Handle will be stored.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  \ref BLE_ERROR_CODES.
 */
#define BT_hci_get_le_connection_handle(bd_addr, handle)\
        BT_hci_get_connection_handle   \
        ((bd_addr), (handle));

/* ------------------------------------------------------------------------- */

/* HCI Link Control APIs --------------------------------------------------- */

#ifdef HCI_READ_REMOTE_VERSION_INFORMATION_SUPPORT
/**
 *  \brief To retrieve version information of a remote Bluetooth device.
 *
 *  \par Description:
 *       This API is used to obtain the values for the version information
 *       of a remote Bluetooth device, identified by the connection_handle.
 *       The connection_handle must be a connection handle for an ACL or LE
 *       connection.
 *
 *       When the Controller receives the Read_Remote_Version_Information
 *       command, the Controller shall send the Command Status event to
 *       the Host. When the Link Manager or Link Layer has completed the
 *       sequence to determine the remote version information, the local
 *       Controller shall send a Read Remote Ver-sion Information Complete
 *       event to the Host. The Read Remote Version Information Complete event
 *       contains the status of this command, and parameters describing
 *       the version and subversion of the LMP or Link Layer used by
 *       the remote device.
 *
 *  \param [in] connection_handle
 *         Connection Handle of the ACL Connection with
 *         the remote Bluetooth device.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_remote_version_information(connection_handle) \
        hci_write_command_connection_handle \
        ( \
               HCI_READ_REMOTE_VERSION_INFORMATION_OPCODE, \
               (connection_handle) \
        )
#endif /* HCI_READ_REMOTE_VERSION_INFORMATION_SUPPORT */

#ifdef HCI_DISCONNECT_SUPPORT
/**
 *  \brief To disconnect an ACL/SCO/eSCO/BLE link.
 *
 *  \par Description:
 *       This API initiates termination of an existing ACL, SCO, eSCO or BLE
 *       connection with a remote Bluetooth device, as identified by
 *       the supplied Connection Handle.
 *       The Reason command parameter indicates the reason for ending
 *       the connection. The remote Controller will receive the Reason command
 *       parameter in the Disconnection Complete event. All synchronous
 *       connections on a physical link should be disconnected before the ACL
 *       connection on the same physical connection is disconnected.
 *
 *       When the Controller receives the Disconnect command, it shall send
 *       the Command Status event to the Host. The Disconnection Complete event
 *       will occur at each Host when the termination of the connection has
 *       completed, and indicates that this command has been completed.
 *
 *  \param [in] connection_handle
 *         Connection Handle for the existing ACL, SCO, eSCO or BLE connection
 *         to be disconnected.
 *
 *  \param [in] reason
 *         This specifies the reason for disconnection.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_disconnect
           (
               UINT16  connection_handle,
               UCHAR   reason
           );
#endif /* HCI_DISCONNECT_SUPPORT */

/* ------------------------------------------------------------------------- */

/* HCI Link Policy APIs ---------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* HCI Host Controller & Baseband APIs ------------------------------------- */

#ifdef HCI_SET_EVENT_MASK_SUPPORT
/**
 *  \brief To set the HCI Event Filter at the local Bluetooth device.
 *
 *  \par Description:
 *       This API is used to control which events are generated by the HCI for
 *       the Host. If the bit in the Event_Mask is set to a one, then the
 *       event associated with that bit will be enabled. For an LE Controller,
 *       the "LE Meta Event" bit in the Event_Mask shall enable or disable all
 *       LE events in the LE Meta Event.
 *       The Host has to deal with each event that occurs. The event mask
 *       allows the Host to control how much it is interrupted.
 *
 *  \param [in] event_mask
 *         It is an 8 octet value. Refer to the Bluetooth HCI specification
 *         for the definition of the masks for each event.
 *         For LE Meta-Event, mask 0x2000000000000000 shall be set.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_set_event_mask(event_mask) \
        hci_write_command_UCHAR_array \
        (HCI_SET_EVENT_MASK_OPCODE, (event_mask), 8)
#endif /* HCI_SET_EVENT_MASK_SUPPORT */

#ifdef HCI_SET_EVENT_MASK_PAGE_2_SUPPORT
/**
 *  \brief To set the HCI Event Filter at the local Bluetooth device.
 *
 *  \par Description:
 *       This API is used to control which events are generated by the HCI for
 *       the Host. If the bit in the Event_Mask is set to a one, then the
 *       event associated with that bit will be enabled. For an LE Controller,
 *       the "LE Meta Event" bit in the Event_Mask shall enable or disable all
 *       LE events in the LE Meta Event.
 *       The Host has to deal with each event that occurs. The event mask
 *       allows the Host to control how much it is interrupted.
 *
 *  \param [in] event_mask
 *         It is an 8 octet value. Refer to the Bluetooth HCI specification
 *         for the definition of the masks for each event.
 *         For LE Meta-Event, mask 0x2000000000000000 shall be set.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_set_event_mask_page_2(event_mask) \
        hci_write_command_UCHAR_array \
        (HCI_SET_EVENT_MASK_PAGE_2_OPCODE, (event_mask), 8)
#endif /* HCI_SET_EVENT_MASK_PAGE_2_SUPPORT */

#ifdef HCI_RESET_SUPPORT
/**
 *  \brief To reset the local Bluetooth controller.
 *
 *  \par Description:
 *       The Reset command will reset the Controller and the Link Manager
 *       on the BR/EDR Controller, the PAL on an AMP Controller,
 *       or the Link Layer on an LE Controller.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_reset() \
        hci_common_api_handler_no_params (HCI_RESET_OPCODE)
#endif /* HCI_RESET_SUPPORT */

#ifdef HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL_SUPPORT
/**
 *  \brief To set the Flow Control setting in the direction from the Bluetooth
 *         device to the Host.
 *
 *  \par Description:
 *       This API can be used to turn flow control on or off for ACL or
 *       SCO/eSCO data sent in the direction from the Bluetooth device to
 *       the Host Stack.
 *       If flow control is turned off, the Host will not send the Host Number
 *       of Completed Packets HCI command. The Bluetooth device will ignore
 *       that command if the Host sends it and flow control is off.
 *       If flow control is turned on for HCI ACL Data Packets and off for HCI
 *       SCO Data Packets, Host Number of Completed Packets commands sent by the
 *       Host should only contain Connection Handles for ACL connections.
 *       If flow control is turned off for HCI ACL Data Packets and on for HCI
 *       SCO Data Packets, Host Number of Completed Packets commands sent by the
 *       Host should only contain Connection Handles for SCO connections.
 *       If flow control is turned on for HCI ACL Data Packets and HCI SCO Data
 *       Packets, the Host will send Host Number Of Completed Packets commands
 *       both for ACL connections and SCO connections.
 *       This Flow Control Enable setting must only be attempted if no
 *       connections to other remote Bluetooth devices exist.
 *
 *  \param [in] flow_enable
 *         Value of the Flow Control setting to be set in the Bluetooth device.
 *         Value 0x00: Flow Control OFF for both ACL and SCO packets.
 *         Value 0x01: Flow Control ON for ACL, OFF for SCO packets.
 *         Value 0x02: Flow Control OFF for ACL, ON for SCO packets.
 *         Value 0x03: Flow Control ON for both ACL and SCO packets.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_set_host_controller_to_host_flow_control
           (
               UCHAR  flow_enable
           );
#endif /* HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL_SUPPORT */


#ifdef HCI_HOST_CONTROLLER_FLOW_ON
#ifdef HCI_HOST_BUFFER_SIZE_SUPPORT
#define BT_hci_host_buffer_size() \
        hci_common_api_handler_no_params (HCI_HOST_BUFFER_SIZE_OPCODE)
#endif /* HCI_HOST_BUFFER_SIZE_SUPPORT */
#endif /* HCI_HOST_CONTROLLER_FLOW_ON */

#ifdef HCI_READ_TRANSMIT_POWER_LEVEL_SUPPORT
/**
 *  \brief To read the value of Transmit Power Level.
 *
 *  \par Description:
 *       This command reads the values for the Transmit_Power_Level parameter
 *       for the specified Connection_Handle. The Connection_Handle shall be
 *       a Connection_Handle for an ACL connection.
 *
 *  \param [in] handle
 *         Specifies which Connection_Handle's Transmit Power Level setting
 *         to read.
 *         Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
 *
 *  \param [in] power_type
 *         This parameter specifies whether the current or maximum power level
 *         to read.
 *         The valid values are:
 *         Value 0x00: Read current Transmit Power Level.
 *         Value 0x01: Read maximum Transmit Power Level.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_read_transmit_power_level
           (
               UINT16   handle,
               UCHAR    power_type
           );
#endif /* HCI_READ_TRANSMIT_POWER_LEVEL_SUPPORT */

/* ------------------------------------------------------------------------- */

/* HCI Host Controller Informational APIs ---------------------------------- */
#ifdef HCI_READ_LOCAL_VERSION_INFORMATION_SUPPORT
/**
 *  \brief To read Version information of the local Bluetooth device.
 *
 *  \par Description:
 *       This API reads the values for the version information for the local
 *       Controller.
 *       The HCI Version information defines the version information of
 *       the HCI layer. The LMP/PAL Version information defines the version
 *       of the LMP or PAL. The Manufacturer_Name information indicates
 *       the manufacturer of the local device.
 *       The HCI Revision and LMP/PAL Subversion are implementation dependent.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_local_version_information() \
        hci_common_api_handler_no_params \
        (HCI_READ_LOCAL_VERSION_INFORMATION_OPCODE)
#endif /* HCI_READ_LOCAL_VERSION_INFORMATION_SUPPORT */

#ifdef HCI_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT
/**
 *  \brief To read list of LMP features of the local Bluetooth device.
 *
 *  \par Description:
 *       This API reads the list of features supported by the Link Manager
 *       Protocol of the local Bluetooth device.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_local_supported_features() \
        hci_common_api_handler_no_params \
        (HCI_READ_LOCAL_SUPPORTED_FEATURES_OPCODE)
#endif /* HCI_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT */

#ifdef HCI_READ_BD_ADDR_SUPPORT
/**
 *  \brief To read the Bluetooth Device Address from local Bluetooth device.
 *
 *  \par Description:
 *       This API reads the Bluetooth Device Address (BD_ADDR) of the local
 *       Bluetooth device.
 *       On a BR/EDR Controller, this command reads the Bluetooth Controller
 *       address (BD_ADDR).
 *       On an LE Controller, this command shall read the Public Device
 *       Address. If this Controller does not have a Public Device Address,
 *       the value 0x000000000000 shall be returned.
 *       On a BR/EDR/LE Controller, the public address shall be the same as
 *       the BD_ADDR.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_bd_addr() \
        hci_common_api_handler_no_params (HCI_READ_BD_ADDR_OPCODE)
#endif /* HCI_READ_BD_ADDR_SUPPORT */

#ifdef HCI_READ_BUFFER_SIZE_SUPPORT
/**
 *  \brief To read the size and number of buffers the local Bluetooth device
 *         maintains for outgoing ACL, Synchronous (SCO/eSCO) and BLE data.
 *
 *  \par Description:
 *       This API reads reads the size and number of buffers the local
 *       Bluetooth device maintains for outgoing ACL, Synchronous (SCO/eSCO)
 *       and BLE data.
 *       The Read_Buffer_Size command is used toread the maximum size of
 *       the data portion of HCI ACL and synchronous Data Packets sent from
 *       the Host to the Controller. The Host will segment the data to be
 *       transmitted from the Host to the Controller according to these sizes,
 *       so that the HCI Data Packets will con-tain data with up to these
 *       sizes. The Read_Buffer_Size command also returns the total number of
 *       HCI ACL and synchronous Data Packets that can be stored in the data
 *       buffers of the Controller. The Read_Buffer_Size command must be
 *       issued by the Host before it sends any data to the Controller.
 *       For a device supporting BR/EDR and LE, if the LE_Read_Buffer_Size
 *       command returned zero for the number of buffers, then buffers
 *       returned by Read_Buffer_Size are shared between BR/EDR and LE.
 *       On an Primary Controller that supportsLE only, the Read_Buffer_Size
 *       command shall not be supported (the LE_Read_Buffer_Size command is
 *       to be used in this case).
 *       The HC_ACL_Data_Packet_Length return parameter will be used to
 *       determine the size of the L2CAP segments contained in ACL Data
 *       Packets, which are transferred from the Host to the Controller to be
 *       broken up into baseband pack-ets by the Link Manager.
 *       The HC_Synchronous_Data_Packet_Length return parameter is used to
 *       determine the maximum size of HCI synchronous Data Packets.
 *       Both the Host and the Controller must support command and event
 *       packets, where the data portion (excluding header) contained in the
 *       packets is 255 octets in size. The HC_Total_Num_ACL_Data_Packets
 *       return parameter contains the total number of HCI ACL Data Packets
 *       that can be stored in the data buffers of the Controller. The Host
 *       will determine how the buffers are to be divided between different
 *       Connection Handles. The HC_Total_Num_Synchronous_Data_Packets return
 *       parameter gives the sameinformation but for HCI synchronous Data
 *       Packets.
 *       Note: The HC_ACL_Data_Packet_Length and
 *       HC_Synchronous_Data_Packet_Length return parameters do not include
 *       the length of the HCI Data Packet header.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_buffer_size() \
        hci_common_api_handler_no_params (HCI_READ_BUFFER_SIZE_OPCODE)
#endif /* HCI_READ_BUFFER_SIZE_SUPPORT */

/* ------------------------------------------------------------------------- */

/* HCI Status Parameters APIs ---------------------------------------------- */

#ifdef HCI_READ_RSSI_SUPPORT
/**
 *  \brief To retrieve the RSSI value for an ACL or BLE connection.
 *
 *  \par Description:
 *       This API reads the value of Received Signal Strength Indication (RSSI)
 *       value from the controller.
 *       For a BR/EDR Controller, a Connection_Handle is used as the Handle
 *       command parameter and return parameter. The RSSI parameter returns
 *       the difference between the measured Received Signal Strength
 *       Indication (RSSI) and the limits of the Golden Receive Power Range
 *       for a Connection Handle to another BR/EDR Controller.
 *       The Connection_Handle must be a Connection_Handle for an ACL
 *       connection. Any positive RSSI value returned by the Controller
 *       indicates how many dB the RSSI is above the upper limit, any
 *       negative value indicates how many dB the RSSI is below the lower
 *       limit. The value zero indicates that the RSSI is inside the Golden
 *       Receive Power Range.
 *       Note: How accurate the dB values will be depends on the Bluetooth
 *       hardware. The only requirements for the hardware are that the BR/EDR
 *       Controller is able to tell whether the RSSI is inside, above or below
 *       the Golden Device Power Range.
 *       The RSSI measurement compares the received signal power with two
 *       threshold levels, which define the Golden Receive Power Range.
 *       The lower threshold level corresponds to a received power between
 *       -56 dBm and 6 dB above the actual sensitivity of the receiver.
 *       The upper threshold level is 20 dB above the lower threshold level
 *       to an accuracy of +/- 6 dB.
 *       For an AMP Controller, a Physical_Link_Handle is used for the Handle
 *       command parameter and return parameter. The meaning of the RSSI metric
 *       is AMP type specific and defined in the AMP PALs.
 *       For an LE transport, a Connection_Handle is used as the Handle command
 *       parameter and return parameter. The meaning of the RSSI metric is an
 *       absolute receiver signal strength value in dBm to +/- 6 dBm accuracy.
 *       If the RSSI cannot be read, the RSSI metric shall be set to 127.
 *
 *  \param [in] connection_handle
 *         The Connection Handle to be used to identify the connection
 *         for reading the RSSI.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_rssi(connection_handle) \
        hci_write_command_connection_handle \
        ( \
               HCI_READ_RSSI_OPCODE, \
               (connection_handle) \
        )
#endif /* HCI_READ_RSSI_SUPPORT */

/* ------------------------------------------------------------------------- */

/* HCI Bluetooth Low Energy API's ----------------------------------------------- */

#ifdef BT_LE
#ifdef HCI_LE_SET_EVENT_MASK_SUPPORT
/**
 *  \brief To set the LE event mask.
 *
 *  \par Description:
 *    The BT_hci_le_set_event_mask API is used to control which LE events are
 *    generated by the HCI for the Host. If the bit in the LE_Event_Mask is set
 *    to a one, then the event associated with that bit will be enabled.
 *    The Host has to deal with each event that is generated by an LE
 *    Controller. The event mask allows the Host to control which events will
 *    interrupt it.
 *    For LE events to be generated, the LE Meta Event bit in the Event_Mask
 *    shall also be set. If that bit is not set, then LE events not shall be
 *    generated, regardless of how the LE_Event_Mask is set.
 *
 *  \param [in] p1 LE Event Mask (8 Octets)
 *          Bit   LE Event Types
 *         ----   --------------
 *         - 0  : LE Connection Complete Event
 *         - 1  : LE Advertising Report Event
 *         - 2  : LE Connection Update Complete Event
 *         - 3  : LE Read Remote Features Complete Event
 *         - 4  : LE Long Term Key Request Event
 *         - 5  : LE Remote Connection Parameter Request Event
 *         - 6  : LE Data Length Change Event
 *         - 7  : LE Read Local P-256 Public Key Complete Event
 *         - 8  : LE Generate DHKey Complete Event
 *         - 9  : LE Enhanced Connection Complete Event
 *         - 10 : LE Directed Advertising Report Event
 *         - 11 : LE PHY Update Complete Event
 *         - 12 : LE Extended Advertising Report Event
 *         - 13 : LE Periodic Advertising Sync Established Event
 *         - 14 : LE Periodic Advertising Report Event
 *         - 15 : LE Periodic Advertising Sync Lost Event
 *         - 16 : LE Extended Scan Timeout Event
 *         - 17 : LE Extended Advertising Set Terminated Event
 *         - 18 : LE Scan Request Received Event
 *         - 19 : LE Channel Selection Algorithm Event
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_set_event_mask(p1)  \
        hci_write_command_UCHAR_array \
        (HCI_LE_SET_EVENT_MASK_OPCODE, (p1), 8)
#endif /* HCI_LE_SET_EVENT_MASK_SUPPORT */

#ifdef HCI_LE_READ_BUFFER_SIZE_SUPPORT
/**
 *  \brief To read the size and number of buffers the local Bluetooth
 *         controller maintains for outgoing LE data.
 *
 *  \par Description:
 *       This API is usedto read the maximum size of the data portion of HCI
 *       LE ACL Data Packets sent from the Host to the Controller.
 *       The Host will segment the data transmitted to the Controller
 *       according to these values, so that the HCIData Packets will contain
 *       datawith up to this size. The LE_Read_Buffer_Size command also returns
 *       the total number of HCI LE ACL Data Packets that can be stored in the
 *       data buffers of the Controller. The LE_Read_Buffer_Size command must
 *       be issued by the Host before it sends any data to an LE Controller.
 *       If the Controller returns a length value of zero, the Host shall use
 *       the Read_Buffer_Size command to determine the size of the data
 *       buffers (shared between BR/EDR and LE transports).
 *       Note: Both the Read_Buffer_Size and LE_Read_Buffer_Size commands may
 *       return buffer length parameter valuesthat are non-zero. This allows
 *       a Controller to offer differentsized buffers for BR/EDRdata packets
 *       and LE data packets.
 *       The HC_LE_ACL_Data_Packet_Length return parameter shall be used to
 *       determine the size of the L2CAP PDU segments contained in ACL Data
 *       Packets, which are transferred from the Host to the Controller to be
 *       broken up into packets by the Link Layer. Both the Host and the
 *       Controller shall support command and event packets, where the data
 *       portion (excluding header) contained in the packets is 255 octets in
 *       size. The HC_Total_Num_LE_ACL_Data_Packets return parameter contains
 *       the total number of HCI ACL Data Packets that can be stored in the
 *       data buffers of the Controller. The Host determines how the buffers
 *       are to be divided between different Connection Handles.
 *       Note: The HC_LE_ACL_Data_Packet_Length return parameter does not
 *       include the length of the HCI Data Packet header.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_buffer_size()  \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_BUFFER_SIZE_OPCODE)
#endif /* HCI_LE_READ_BUFFER_SIZE_SUPPORT */

#ifdef HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT
/**
 *  \brief To read supported LE features by local Bluetooth Controller.
 *
 *  \par Description:
 *       This API can be used to request the list of the supported LE features.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_local_supported_features()  \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_OPCODE)
#endif /* HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT */

#ifdef HCI_LE_SET_RANDOM_ADDRESS_SUPPORT
/**
 *  \brief To set the LE Random Device Address of local Bluetooth Device.
 *
 *  \par Description:
 *       This API can be used to set the LE Random Device Address in the
 *       Controller.
 *       If this command is used to change the address, the new random address
 *       shall take effect for advertising no later than the next successful
 *       LE Set Advertising Enable Command, for scanning no later than the next
 *       successful LE Set Scan Enable Command or LE Set Extended Scan Enable
 *       Command, and for initiating no later than the next successful
 *       LE Create Connection Command or LE Extended Create Connection Command.
 *
 *  \param [in] rand_addr
 *         LE Random Device Address
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note If Extended Advertising is in use, this command only affects
 *  the address used for scanning and initiating. The addresses used for
 *  advertising are set by the LE_Set_Advertising_Set_Random_Address command.
 *
 *  If the Host issues this command when scanning or legacy advertising is
 *  enabled, the Controller shall return the error code Command Disallowed
 *  (0x0C).
 */
#define BT_hci_le_set_random_address(rand_addr)  \
        hci_write_command_UCHAR_array \
        (HCI_LE_SET_RANDOM_ADDRESS_OPCODE, (rand_addr), BT_BD_ADDR_SIZE)
#endif /* HCI_LE_SET_RANDOM_ADDRESS_SUPPORT */

#ifdef HCI_LE_SET_ADVERTISING_PARAMETERS_SUPPORT
/**
 *  \brief To set advertising parameters.
 *
 *  \par Description:
 *       This API can be used to set the advertising parameters.
 *
 *       The Advertising_Interval_Min shall be less than or equal to the
 *       Advertising_Interval_Max. The Advertising_Interval_Min and
 *       Advertising_Interval_Max should not be the same value to enable the
 *       Controller to determine the best advertising interval given other
 *       activities.
 *
 *       For high duty cycle directed advertising, i.e. when Advertising_Type
 *       is 0x01 (ADV_DIRECT_IND, high duty cycle), the Advertising_Interval_Min
 *       and Advertising_Interval_Max parameters are not used and shall be ignored.
 *
 *       The Advertising_Type is used to determine the packet type that is used
 *       for advertising when advertising is enabled.
 *
 *       Own_Address_Type parameter indicates the type of address being used
 *       in the advertising packets.
 *
 *       If Own_Address_Type equals 0x02 or 0x03, the Peer_Address parameter
 *       contains the peer's Identity Address and the Peer_Address_Type
 *       parameter contains the Peer's Identity Type (i.e. 0x00 or 0x01).
 *       These parameters are used to locate the corresponding local IRK
 *       in the resolving list; this IRK is used to generate the own address
 *       used in the advertisement.
 *
 *       If directed advertising is performed, i.e. when Advertising_Type is
 *       set to 0x01 (ADV_DIRECT_IND, high duty cycle) or 0x04 (ADV_DIRECT_IND,
 *       low duty cycle mode), then the Peer_Address_Type and Peer_Address
 *       shall be valid.
 *
 *       If Own_Address_Type equals 0x02 or 0x03, the Controller generates
 *       the peer's Resolvable Private Address using the peer's IRK
 *       corresponding to the peer's Identity Address contained in
 *       the Peer_Address parameter and peer's Identity Address Type
 *       (i.e. 0x00 or 0x01) contained in the Peer_Address_Type parameter.
 *
 *       The Advertising_Channel_Map is a bit field that indicates the advertising
 *       channels that shall be used when transmitting advertising packets.
 *       At least one channel bit shall be set in the Advertising_Channel_Map parameter.
 *
 *       The Advertising_Filter_Policy parameter shall be ignored when
 *       directed advertising is enabled.
 *
 *       The Host shall not issue this command when advertising is enabled
 *       in the Controller; if it is the Command Disallowed error code shall be used.
 *
 *       If the advertising interval range provided by the Host
 *       (Advertising_Interval_Min, Advertising_Interval_Max) is outside
 *       the advertising interval range supported by the Controller,
 *       then the Controller shall return the Unsupported Feature or
 *       Parameter Value (0x11) error code.
 *
 *  \param [in] advertising_interval_min
 *         Minimum advertising interval for undirected and low duty cycle
 *         direct advertising.
 *
 *  \param [in] advertising_interval_max
 *         Maximum advertising interval for undirected and low duty cycle
 *         directed advertising.
 *
 *  \param [in] advertising_type
 *         One of the following types
 *         0x00: Connectable undirected advertising (ADV_IND) (default)
 *         0x01: Connectable high duty cycle directed advertising
 *               (ADV_DIRECT_IND, high duty cycle)
 *         0x02: Scannable undirected advertising (ADV_SCAN_IND)
 *         0x03: Non connectable undirected advertising (ADV_NONCONN_IND)
 *         0x04: Connectable low duty cycle directed advertising
 *               (ADV_DIRECT_IND, low duty cycle)
 *
 *  \param [in] own_addr_type
 *         One of the following types
 *         0x00: Public Device Address (default)
 *         0x01: Random Device Address
 *         0x02: Controller generates Resolvable Private Address based on
 *               the local IRK from the resolving list. If the resolving list
 *               contains no matching entry, use the public address.
 *         0x03: Controller generates Resolvable Private Address based on
 *               the local IRK from the resolving list. If the resolving list
 *               contains no matching entry, use the random address
 *               from LE_Set_Random_Address.
 *
 *  \param [in] peer_addr_type
 *         Address Type of Remote Device
 *         0x00 - Public Device Address (default) or Public Identity Address.
 *         0x01 - Random Device Address or Random (static) Identity Address.
 *
 *  \param [in] peer_addr
 *         Bluetooth Device Address of Remote Device.
 *         Public Device Address, Random Device Address,
 *         Public Identity Address, or Random (static) Identity Address
 *         of the device to be connected.
 *
 *  \param [in] advertising_channel_map
 *         Channle map selection, using bitmask.
 *         xxxxxxx1b: Enable channel 37 use
 *         xxxxxx1xb: Enable channel 38 use
 *         xxxxx1xxb: Enable channel 39 use
 *         00000111b: Default (all channels enabled)
 *
 *  \param [in] advertising_filter_policy
 *         One of the following values
 *         0x00: Process scan and connection requests from all devices
 *               (i.e., the White List is not in use) (default).
 *         0x01: Process connection requests from all devices and only scan
 *               requests from devices that are in the White List.
 *         0x02: Process scan requests from all devices and only connection
 *               requests from devices that are in the White List.
 *         0x03: Process scan and connection requests only from devices
 *               in the White List.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_advertising_parameters
           (
               UINT16   advertising_interval_min,
               UINT16   advertising_interval_max,
               UCHAR    advertising_type,
               UCHAR    own_addr_type,
               UCHAR    peer_addr_type,
               UCHAR  * peer_addr,
               UCHAR    advertising_channel_map,
               UCHAR    advertising_filter_policy
           );
#endif /* HCI_LE_SET_ADVERTISING_PARAMETERS_SUPPORT */

#ifdef HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_SUPPORT
/**
 *  \brief To read the transmit power level used for LE advertising channel
 *         packets.
 *
 *  \par Description:
 *       This API can be used to read the transmit power level used for LE
 *       advertising channel packets.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_advertising_channel_tx_power()  \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_OPCODE)
#endif /* HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_SUPPORT */

#ifdef HCI_LE_SET_ADVERTISING_DATA_SUPPORT
/**
 *  \brief To set advertising data used in advertsing packets that have a data
 *  field.
 *
 *  \par Description:
 *       This API is used to set the data used in advertsing packets
 *       that have a data field.
 *
 *       If advertising is currently enabled, the Controller shall use the new
 *       data in subsequent advertising events. If an advertising event is in
 *       progress when this command is issued, the Controller may use the old
 *       or new data for that event.
 *       If advertising is currently disabled, the data shall be kept by
 *       the Controller and used once advertising is enabled.
 *
 *  \param [in] advertising_data_len
 *         The number of significant octets in the Advertising_Data.
 *
 *  \param [in] advertising_data
 *         Advertising data.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_set_advertising_data(advertising_data_len, advertising_data)\
        hci_write_command_adv_scan_response_data \
        ( \
            HCI_LE_SET_ADVERTISING_DATA_OPCODE, \
            (advertising_data_len), \
            (advertising_data) \
        )
#endif /* HCI_LE_SET_ADVERTISING_DATA_SUPPORT */

#ifdef HCI_LE_SET_SCAN_RESPONSE_DATA_SUPPORT
/**
 *  \brief To provide data used in Scanning Packets that have a data field.
 *
 *  \par Description:
 *       This API can be used to set the data used in Scanning Packets that
 *       have a data field.
 *
 *       If advertising is currently enabled, the Controller shall use the new
 *       data in subsequent advertising events. If an advertising event is in
 *       progress when this command is issued, the Controller may use the old
 *       or new data for that event.  If advertising is currently disabled,
 *       the data shall be kept by the Controller and used once advertising
 *       is enabled.
 *
 *  \param [in] scan_data_len
 *         The number of significant octets in the Scan_Response_Data
 *
 *  \param [in] scan_data
 *         Scan data used in scan packets
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_set_scan_response_data(scan_data_len, scan_data) \
        hci_write_command_adv_scan_response_data \
        ( \
            HCI_LE_SET_SCAN_RESPONSE_DATA_OPCODE, \
            (scan_data_len), \
            (scan_data) \
        )
#endif /* HCI_LE_SET_SCAN_RESPONSE_DATA_SUPPORT */

#ifdef HCI_LE_SET_ADVERTISING_ENABLE_SUPPORT
/**
 *  \brief To request the local Bluetooth Device to start or stop advertising.
 *
 *  \par Description:
 *       This API can be used to request the Controller to start or stop
 *       advertising. The Controller manages the timing of advertisements
 *       as per the advertising parameters given in the
 *       \ref BT_hci_le_set_advertising_parameters command.
 *
 *       The Controller shall continue advertising until the Host issues an
 *       LE_Set_Advertising_Enable command with Advertising_Enable set to 0x00
 *       (Advertising is disabled) or until a connection is created or until
 *       the Advertising is timed out due to high duty cycle Directed Advertising.
 *       In these cases, advertising is then disabled.
 *       If the advertising parameters' Own_Address_Type parameter is set to 0x01
 *       and the random address for the device has not been initialized,
 *       the Controller shall return the error code Invalid HCI Command Parameters (0x12).
 *       If the advertising parameters' Own_Address_Type parameter is set to 0x03,
 *       the controller's resolving list did not contain a matching entry, and the random
 *       address for the device has not been initialized, the Controller shall return the
 *       error code Invalid HCI Command Parameters (0x12).
 *
 *  \param [in] advertising_enable
 *       Enable/Disable Advertising
 *       0x00: Disable Advertising (Default)
 *       0x01: Enable Advertising
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note Enabling advertising when it is already enabled can cause the random address
 *  to change. Disabling advertising when it is already disabled has no effect.
 */
#define BT_hci_le_set_advertising_enable(advertising_enable)  \
        hci_write_command_UCHAR \
        (HCI_LE_SET_ADVERTISING_ENABLE_OPCODE, (advertising_enable))
#endif /* HCI_LE_SET_ADVERTISING_ENABLE_SUPPORT */

#ifdef HCI_LE_SET_SCAN_PARAMETERS_SUPPORT
/**
 *  \brief To set scan parameters.
 *
 *  \par Description:
 *       This API can be used to set the scan parameters.
 *
 *       The LE_Scan_Type parameter controls the type of scan to perform.
 *
 *       The LE_Scan_Interval and LE_Scan_Window parameters are recommendations
 *       from the Host on how long (LE_Scan_Window) and how frequently
 *       (LE_Scan_Interval) the Controller should scan. The LE_Scan_Window
 *       parameter shall always be set to a value smaller or equal to the value
 *       set for the LE_Scan_Interval parameter. If they are set to the same
 *       value scanning should be run continuously.
 *
 *       The Own_Address_Type parameter determines the address being used
 *       in the scan request packets.
 *
 *       The Host shall not issue this command when scanning is enabled in the
 *       Controller; if it is the Command Disallowed error code shall be used.
 *
 *  \param [in] le_scan_type
 *         One of the following scan types.
 *         0x00: Passive Scanning. No scanning PDUs shall be sent (default)
 *         0x01: Active scanning. Scanning PDUs may be sent.
 *
 *  \param [in] le_scan_interval
 *         The time interval from when the Controller started its last
 *         LE scan until it begins the subsequent LE scan.
 *
 *  \param [in] le_scan_window
 *         The duration of the LE scan. LE_Scan_Window shall be less than
 *         or equal to LE_Scan_Interval
 *
 *  \param [in] own_addr_type
 *         One of the following types
 *         0x00: Public Device Address (default)
 *         0x01: Random Device Address
 *         0x02: Controller generates Resolvable Private Address based on
 *               the local IRK from the resolving list. If the resolving list
 *               contains no matching entry, use the public address.
 *         0x03: Controller generates Resolvable Private Address based on
 *               the local IRK from the resolving list. If the resolving list
 *               contains no matching entry, use the random address
 *               from LE_Set_Random_Address.
 *
 *  \param [in] scan_filter_policy
 *         One of the following values
 *         0x00: Accept all advertising packets except directed advertising
 *         packets not addressed to this device (default).
 *
 *         0x01: Accept only advertising packets from devices where
 *         the advertiser's address is in the White List. Directed advertising
 *         packets which are not addressed to this device shall be ignored.
 *
 *         0x02: Accept all advertising packets except directed advertising
 *         packets where the initiator's identity address does not address
 *         this device.
 *         Note: Directed advertising packets where the initiator's address is
 *         a resolvable private address that cannot be resolved are also accepted.
 *
 *         0x03: Accept all advertising packets except:
 *               - advertising packets where the advertiser's identity address
 *                 is not in the White List; and
 *               - directed advertising packets where the initiator's identity
 *                 address does not address this device
 *         Note: Directed advertising packets where the initiator's address is a
 *         resolvable private address that cannot be resolved are also accepted.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_scan_parameters
           (
               UCHAR    le_scan_type,
               UINT16   le_scan_interval,
               UINT16   le_scan_window,
               UCHAR    own_addr_type,
               UCHAR    scan_filter_policy
           );
#endif /* HCI_LE_SET_SCAN_PARAMETERS_SUPPORT */

#ifdef HCI_LE_SET_SCAN_ENABLE_SUPPORT
/**
 *  \brief To request the local Bluetooth Device to start or stop scanning.
 *
 *  \par Description:
 *       This API can be used to request the Controller to start or stop
 *       scanning. Scanning is used to discover advertising devices nearby.
 *
 *       The Filter_Duplicates parameter controls whether the Link Layer should
 *       filter duplicate advertising reports (Filtering_Enabled) to the Host,
 *       or if the Link Layer should generate advertising reports for each packet
 *       received (Filtering_Disabled).
 *
 *       If the scanning parameters' Own_Address_Type parameter is set to
 *       0x01 or 0x03 and the random address for the device has not been
 *       initialized, the Controller shall return the error code
 *       Invalid HCI Command Parameters (0x12).
 *
 *       If the LE_Scan_Enable parameter is set to 0x01 and scanning is already
 *       enabled, any change to the Filter_Duplicates setting shall take effect.
 *       Note: Disabling scanning when it is disabled has no effect.
 *
 *  \param [in] le_scan_enable
 *       Enable/Disable Scanning
 *       0x00: Disable Scanning
 *       0x01: Enable Scanning
 *
 *  \param [in] filter_dups
 *       Enable/Disable Duplicate Filtering
 *       0x00: Disable Duplicate Filtering
 *       0x01: Enable Duplicate Filtering
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_scan_enable
           (
               UCHAR    le_scan_enable,
               UCHAR    filter_dups
           );
#endif /* HCI_LE_SET_SCAN_ENABLE_SUPPORT */

#ifdef HCI_LE_CREATE_CONNECTION_SUPPORT
/**
 *  \brief To create LE Connection with remote device.
 *
 *  \par Description:
 *       This API is used to create a Link Layer connection to a connectable
 *       advertiser.
 *
 *       The LE_Scan_Interval and LE_Scan_Window parameters are recommendations
 *       from the Host on how long (LE_Scan_Window) and how frequently
 *       (LE_Scan_Interval) the Controller should scan. The LE_Scan_Window
 *       parameter shall be set to a value smaller or equal to the value set
 *       for the LE_Scan_Interval parameter.  If both are set to the same value,
 *       scanning should run continuously.
 *
 *       The Initiator_Filter_Policy is used to determine whether the White List
 *       is used. If the White List is not used, the Peer_Address_Type and
 *       the Peer_Address parameters specify the address type and address of
 *       the advertising device to connect to.
 *
 *       Peer_Address_Type parameter indicates the type of address used in
 *       the connectable advertisement sent by the peer. The Host shall not set
 *       Peer_Address_Type to either 0x02 or 0x03 if both the Host and
 *       the Controller support the LE Set Privacy Mode command. If a Controller
 *       that supports the LE Set Privacy Mode command receives the LE Create
 *       Connection command with Peer_Address_Type set to either 0x02 or 0x03,
 *       it may use either device privacy mode or network privacy mode for that
 *       peer device.
 *
 *       Peer_Address parameter indicates the Peer's Public Device Address,
 *       Random (static) Device Address, Non-Resolvable Private Address or
 *       Resolvable Private Address depending on the Peer_Address_Type parameter.
 *
 *       Own_Address_Type parameter indicates the type of address being used
 *       in the connection request packets.
 *
 *       The Conn_Interval_Min and Conn_Interval_Max parameters define the minimum
 *       and maximum allowed connection interval. The Conn_Interval_Min parameter
 *       shall not be greater than the Conn_Interval_Max parameter.
 *
 *       The Conn_Latency parameter defines the maximum allowed connection latency.
 *
 *       The Supervision_Timeout parameter defines the link supervision timeout
 *       for the connection. The Supervision_Timeout in milliseconds shall be
 *       larger than (1 + Conn_Latency) * Conn_Interval_Max * 2,
 *       where Conn_Interval_Max is given in milliseconds.
 *
 *       The Minimum_CE_Length and Maximum_CE_Length parameters are informative
 *       parameters providing the Controller with the expected minimum and
 *       maximum length of the connection events. The Minimum_CE_Length parameter
 *       shall be less than or equal to the Maximum_CE_Length parameter.
 *
 *       If the Host issues this command when another LE_Create_Connection
 *       command is pending in the Controller, the Controller shall return
 *       the error code Command Disallowed (0x0C).
 *
 *       If the Own_Address_Type parameter is set to 0x01 and the random address
 *       for the device has not been initialized, the Controller shall return
 *       the error code Invalid HCI Command Parameters (0x12).
 *
 *       If the Own_Address_Type parameter is set to 0x03, the
 *       Initiator_Filter_Policy parameter is set to 0x00, the controller's
 *       resolving list did not contain a matching entry, and the random address
 *       for the device has not been initialized, the Controller shall return
 *       the error code Invalid HCI Command Parameters (0x12).
 *
 *       If the Own_Address_Type parameter is set to 0x03, the
 *       Initiator_Filter_Policy parameter is set to 0x01, and the random address
 *       for the device has not been initialized, the Controller shall return
 *       the error code Invalid HCI Command Parameters (0x12).
 *
 *  \param [in] le_scan_interval
 *         The time interval from when the Controller started its last
 *         LE scan until it begins the subsequent LE scan.
 *
 *  \param [in] le_scan_window
 *         The duration of the LE scan. LE_Scan_Window shall be less than
 *         or equal to LE_Scan_Interval.
 *
 *  \param [in] initiator_filter_policy
 *         0x00: White list is not used to determine which advertiser
 *               to connect to.
 *               Peer_Address_Type and Peer_Address shall be used.
 *         0x01: White list is used to determine which advertiser
 *               to connect to.
 *               Peer_Address_Type and Peer_Address shall be ignored.
 *
 *  \param [in] peer_address_type
 *         One of the following types
 *         0x00: Public Device Address
 *
 *         0x01: Random Device Address
 *
 *         0x02: Public Identity Address (Corresponds to peer's Resolvable
 *         Private Address). This value shall only be used by the Host if
 *         either the Host or the Controller does not support
 *         the LE Set Privacy Mode command.
 *
 *         0x03: Random (static) Identity Address (Corresponds to peer's
 *         Resolvable Private Address). This value shall only be used by
 *         a Host if either the Host or the Controller does not support
 *         the LE Set Privacy Mode command.
 *
 *  \param [in] peer_address
 *         Public Device Address, Random Device Address, Public Identity
 *         Address, or Random (static) Identity Address of the device to be
 *         connected.
 *
 *  \param [in] own_address_type
 *         One of the following types
 *         0x00: Public Device Address
 *         0x01: Random Device Address
 *         0x02: Controller generates Resolvable Private Address based on
 *               the local IRK from the resolving list. If the resolving list
 *               contains no matching entry, use the public address.
 *         0x03: Controller generates Resolvable Private Address based on
 *               the local IRK from the resolving list. If the resolving list
 *               contains no matching entry, use the random address
 *               from LE_Set_Random_Address.
 *
 *  \param [in] conn_interval_min
 *         Minimum value for the connection event interval.
 *         This shall be less than or equal to Conn_Interval_Max.
 *
 *  \param [in] conn_interval_max
 *         Maximum value for the connection event interval.
 *         This shall be greater than or equal to Conn_Interval_Min.
 *
 *  \param [in] conn_latency
 *         Slave latency for the connection in number of connection events.
 *
 *  \param [in] supervision_timeout
 *         Supervision Timeout for the LE Link.
 *
 *  \param [in] minimum_ce_length
 *         Information parameter about the minimum length of connection
 *         needed for this LE connection.
 *
 *  \param [in] maximum_ce_length
 *         Information parameter about the maximum length of connection
 *         needed for this LE connection.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_create_connection
           (
               UINT16   le_scan_interval,
               UINT16   le_scan_window,
               UCHAR    initiator_filter_policy,
               UCHAR    peer_address_type,
               UCHAR *  peer_address,
               UCHAR    own_address_type,
               UINT16   conn_interval_min,
               UINT16   conn_interval_max,
               UINT16   conn_latency,
               UINT16   supervision_timeout,
               UINT16   maximum_ce_length,
               UINT16   minimum_ce_length
           );
#endif /* HCI_LE_CREATE_CONNECTION_SUPPORT */

#ifdef HCI_LE_CREATE_CONNECTION_CANCEL_SUPPORT
/**
 *  \brief To cancel the Create Connection procedure.
 *
 *  \par Description:
 *       This API is used to cancel the \ref BT_hci_le_create_connection or
 *       \ref BT_hci_le_extended_create_connection commands.
 *       This command shall only be issued after the \ref BT_hci_le_create_connection
 *       or \ref BT_hci_le_extended_create_connection commands have been issued,
 *       a Command Status event has been received for the LE Create Connection
 *       or LE_Extended_Create_Connection commands, and before the LE Connection
 *       Complete or LE Enhanced Connection Complete events.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_create_connection_cancel()  \
        hci_common_api_handler_no_params \
        (HCI_LE_CREATE_CONNECTION_CANCEL_OPCODE)
#endif /* HCI_LE_CREATE_CONNECTION_CANCEL_SUPPORT */

#ifdef HCI_LE_READ_WHITE_LIST_SIZE_SUPPORT
/**
 *  \brief To read the total number of white list entries that can be stored
 *         in the Controller.
 *
 *  \par Description:
 *       This API can be used to read the total number of white list entries
 *       that can be stored in the Controller.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note: The number of entries that can be stored is not fixed and
 *  the Controller can change it at any time (e.g. because the memory used to
 *  store the White List can also be used for other purposes).
 */
#define BT_hci_le_read_white_list_size()  \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_WHITE_LIST_SIZE_OPCODE)
#endif /* HCI_LE_READ_WHITE_LIST_SIZE_SUPPORT */

#ifdef HCI_LE_CLEAR_WHITE_LIST_SUPPORT
/**
 *  \brief To clear the white list stored in the Controller.
 *
 *  \par Description:
 *       This API can be used to clear the white list stored in the Controller.
 *       This command can be used at any time except when:
 *       - The advertising filter policy uses the white list and advertising is
 *          enabled.
 *       - The scanning filter policy uses white list and scanning is enabled.
 *       - The initiator filter policy uses the white list and a response to
 *         \ref BT_hci_le_create_connection API is outstanding.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_clear_white_list()  \
        hci_common_api_handler_no_params \
        (HCI_LE_CLEAR_WHITE_LIST_OPCODE)
#endif /* HCI_LE_CLEAR_WHITE_LIST_SUPPORT */

#if ((defined HCI_LE_ADD_DEVICE_TO_WHITE_LIST_SUPPORT) || \
     (defined HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_SUPPORT))
/**
 * \cond ignore_this Ignore this fundtion while generating doxygen document
 */
API_RESULT BT_hci_le_add_remove_device_to_white_list
           (
               UINT16   opcode,
               UCHAR    addr_type,
               UCHAR  * addr
           );
/**
 * \endcond
 */
#endif /* HCI_LE_ADD_DEVICE_TO_WHITE_LIST_SUPPORT || HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_SUPPORT */

#ifdef HCI_LE_ADD_DEVICE_TO_WHITE_LIST_SUPPORT
/**
 *  \brief To add a single device to the white list stored in the Controller.
 *
 *  \par Description:
 *       This API can be used to add a single device to the white list
 *       stored in the Controller.
 *       This command can be used at any time except when:
 *       - The advertising filter policy uses the white list and advertising is
 *          enabled.
 *       - The scanning filter policy uses the white list and
 *         scanning is enabled.
 *       - The initiator filter policy uses the white list and a response to
 *         \ref BT_hci_le_create_connection API is outstanding.
 *
 *       When a Controller cannot add a device to the White List because there
 *       is no space available, it shall return the error code Memory Capacity
 *       Exceeded (0x07).
 *
 *       Address is ignored when Address_Type is set to 0xFF.
 *
 *  \param [in] addr_type
 *         Bluetooth Device Address Type of the device address to be added.
 *         0x00: Public Device Address
 *         0x01: Random Device Address
 *         0xFF: Devices sending anonymous advertisements
 *
 *  \param [in] addr
 *         Public Device Address or Random Device Address of the device
 *         to be added to the White List.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_add_device_to_white_list(addr_type, addr)\
        BT_hci_le_add_remove_device_to_white_list\
        (HCI_LE_ADD_DEVICE_TO_WHITE_LIST_OPCODE, (addr_type), (addr))
#endif /* HCI_LE_ADD_DEVICE_TO_WHITE_LIST_SUPPORT */

#ifdef HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_SUPPORT
/**
 *  \brief To remove a single device from the white list stored in the Controller.
 *
 *  \par Description:
 *       This API can be used to remove a single device from the white list
 *       stored in the Controller.
 *       This command can be used at any time except when:
 *       - The advertising filter policy uses the white list and advertising is
 *         enabled.
 *       - The scanning filter policy uses the white list and scanning is
 *         enabled.
 *       - The initiator filter policy uses the white list and a response to
 *         \ref BT_hci_le_create_connection API is outstanding.
 *
 *       Address is ignored when Address_Type is set to 0xFF.
 *
 *  \param [in] addr_type
 *         Bluetooth Device Address Type of the device address to be removed.
 *         0x00: Public Device Address
 *         0x01: Random Device Address
 *         0xFF: Devices sending anonymous advertisements
 *
 *  \param [in] addr
 *         Public Device Address or Random Device Address of the device
 *         to be removed from the White List.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_remove_device_from_white_list(addr_type, addr)\
        BT_hci_le_add_remove_device_to_white_list\
        (HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_OPCODE, (addr_type), (addr))
#endif /* HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_SUPPORT */

#ifdef HCI_LE_CONNECTION_UPDATE_SUPPORT
/**
 *  \brief To change LE Connection Parameters with remote device.
 *
 *  \par Description:
 *       This API is used to change the Link Layer connection parameters of
 *       a connection. This command may be issued on both the master and slave.
 *
 *       The Conn_Interval_Min and Conn_Interval_Max parameters are used to
 *       define the minimum and maximum allowed connection interval.
 *       The Conn_Interval_Min parameter shall not be greater than
 *       the Conn_Interval_Max parameter.
 *
 *       The Conn_Latency parameter shall define the maximum allowed connection
 *       latency.
 *
 *       The Supervision_Timeout parameter shall define the link supervision
 *       timeout for the LE link. The Supervision_Timeout in milliseconds
 *       shall be larger than (1 + Conn_Latency) * Conn_Interval_Max * 2,
 *       where Conn_Interval_Max is given in milliseconds.
 *
 *       The Minimum_CE_Length and Maximum_CE_Length are information parameters
 *       providing the Controller with a hint about the expected minimum and
 *       maximum length of the connection events. The Minimum_CE_Length shall
 *       be less than or equal to the Maximum_CE_Length.
 *
 *       The actual parameter values selected by the Link Layer may be
 *       different from the parameter values provided by the Host through
 *       this command.
 *
 *  \param [in] connection_handle
 *         Connection handle to identify the connection.
 *
 *  \param [in] conn_interval_min
 *         Minimum value for the connection interval.
 *         This shall be less than or equal to Conn_Interval_Max.
 *
 *  \param [in] conn_interval_max
 *         Maximum value for the connection interval.
 *         This shall be greater than or equal to Conn_Interval_Min.
 *
 *  \param [in] conn_latency
 *         Slave latency for the connection in number of connection events.
 *
 *  \param [in] supervision_timeout
 *         Supervision Timeout for the LE Link.
 *
 *  \param [in] minimum_ce_length
 *         Information parameter about the minimum length of connection event
 *         needed for this LE connection. How this value is used is outside the
 *         scope of this specification.
 *
 *  \param [in] maximum_ce_length
 *         Information parameter about the maximum length of connection event
 *         needed for this LE connection. How this value is used is outside the
 *         scope of this specification.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_connection_update
           (
               UINT16   connection_handle,
               UINT16   conn_interval_min,
               UINT16   conn_interval_max,
               UINT16   conn_latency,
               UINT16   supervision_timeout,
               UINT16   minimum_ce_length,
               UINT16   maximum_ce_length
           );
#endif /* HCI_LE_CONNECTION_UPDATE_SUPPORT */

#ifdef HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_SUPPORT
/**
 *  \brief To specify a channel classification for data channels based on its
 *         \"local information\".
 *
 *  \par Description:
 *       This API is used to specify a channel classification for data
 *       channels based on its \"local information\". This classification
 *       persists until overwritten with a subsequent
 *       \ref BT_hci_le_set_host_channel_classification API is initiated or
 *       until the Controller is reset.
 *
 *       If this command is used, the Host should send it within 10 seconds
 *       of knowing that the channel classification has changed. The interval
 *       between two successive commands sent shall be at least one second.
 *
 *       This command shall only be used when the local device supports
 *       the Master role.
 *
 *  \param [in] channel_map
 *         This parameter contains 5 octet byte stream conatining, 37 1-bit
 *         fields. The nth such field (in the range 0 to 36) contains the
 *         value for the link laye channel index n.
 *         - Channel n is bad: 0
 *         - Channel n is unknown: 1
 *
 *         The most significant bits are reserved and shall be set to 0
 *         for future use.
 *         At least one channel shall be marked as unknown.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_set_host_channel_classification(channel_map)  \
        hci_write_command_UCHAR_array \
        (HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_OPCODE, (channel_map), 5)
#endif /* HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_SUPPORT */

#ifdef HCI_LE_READ_CHANNEL_MAP_SUPPORT
/**
 *  \brief To read current channel map for specified connection handle.
 *
 *  \par Description:
 *       This API can be used to read current channel map for the specified
 *       connection handle.
 *
 *       The returned value indicates the state of the Channel_Map specified
 *       by the last transmitted or received Channel_Map (in a CONNECT_IND or
 *       LL_CHANNEL_MAP_IND message) for the specified Connection_Handle,
 *       regardless of whether the Master has received an acknowledgement.
 *
 *  \param [in] conn_handle
 *         Connection Handle to be used to identify remote device.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_channel_map(conn_handle)  \
        hci_write_command_connection_handle \
        (HCI_LE_READ_CHANNEL_MAP_OPCODE, (conn_handle))
#endif /* HCI_LE_READ_CHANNEL_MAP_SUPPORT */

#ifdef HCI_LE_READ_REMOTE_USED_FEATURES_SUPPORT
/**
 *  \brief To read a list of the LE features from the remote device.
 *
 *  \par Description:
 *       This API can be used to request for the list of LE features from
 *       remote device which is associated with specfied connection handle.
 *
 *       This command may be issued on both the master and slave.
 *
 *       Note: If a connection already exists between the two devices and
 *       the features have already been fetched on that connection,
 *       the Controller may use a cached copy of the features.
 *
 *  \param [in] conn_handle
 *         Connection Handle to be used to identify remote device.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_remote_used_features(conn_handle)  \
        hci_write_command_connection_handle \
        (HCI_LE_READ_REMOTE_USED_FEATURES_OPCODE, (conn_handle))
#endif /* HCI_LE_READ_REMOTE_USED_FEATURES_SUPPORT */

#ifdef HCI_LE_ENCRYPT_SUPPORT
/**
 *  \brief To perform AES Encryption.
 *
 *  \par Description:
 *       The LE_Encrypt command is used to request the Controller to encrypt
 *       the Plaintext_Data in the command using the Key given in the command
 *       and returns the Encrypted_Data to the Host. The AES-128 bit block
 *       cypher is defined in NIST Publication FIPS-197.

 *  \param [in] key
 *         128 bit key for the encryption of the data given in the command.
 *         The most significant octet of the key corresponds to key[0] using
 *         the notation specified in FIPS 197.
 *
 *  \param [in] plain_text
 *         128 bit data block that is requested to be encrypted.
 *         The most significant octet of the PlainText_Data corresponds to
 *         in[0] using the notation specified in FIPS 197.
 *
 *  \param [out] encrypt_cmd_counter
 *         Encryption command counter is a out parameter which species the
 *         encryption counter. This parameter is used by inter stack
 *         to keep track each encryption command initiated and response
 *         received for each of them.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_encrypt
           (
               UCHAR *  key,
               UCHAR *  plain_text,
               UCHAR *  encrypt_cmd_counter
           );
#endif /* HCI_LE_ENCRYPT_SUPPORT */

#ifdef HCI_LE_RAND_SUPPORT
/**
 *  \brief To request for 8 octects of random data to be generated.
 *
 *  \par Description:
 *       This API can be used to request the Controller to generate 8 octects
 *       of random data to be sent to the Host.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_rand()  \
        hci_common_api_handler_no_params \
        (HCI_LE_RAND_OPCODE)
#endif /* HCI_LE_RAND_SUPPORT */

#ifdef HCI_LE_START_ENCRYPTION_SUPPORT
/**
 *  \brief To request for encryption or re-encryption of a BLE connection.
 *
 *  \par Description:
 *       This API is used to authenticate the given encryption key associated
 *       with the remote device specified by the connection handle, and once
 *       authenticated will encrypt the connection.
 *
 *       If the connection is already encrypted then the Controller shall
 *       pause connection encryption before attempting to authenticate the
 *       given encryption key, and then re-encrypt the connection. While
 *       encryption is paused no user data shall be transmitted.
 *
 *       On an authentication failure, the connection shall be automatically
 *       disconnected by the Link Layer. If this command succeeds, then the
 *       connection shall be encrypted.
 *       This command shall only be used when the local device's role is Master.
 *
 *  \param [in] conn_handle
 *         Connection handle to identify the connection
 *
 *  \param [in] random_number
 *         64-bit Random Number
 *
 *  \param [in] encrypted_diversifier
 *         16-bit encryption diversifier (ediv)
 *
 *  \param [in] long_term_key
 *         128-bit long term key (LTK)
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_start_encryption
           (
               UINT16   conn_handle,
               UCHAR  * random_number,
               UINT16   encrypted_diversifier,
               UCHAR  * long_term_key
           );
#endif /* HCI_LE_START_ENCRYPTION_SUPPORT */

#ifdef HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_SUPPORT
/**
 *  \brief To respond to LE Long Term Key Request.
 *
 *  \par Description:
 *       The LE_Long_Term_Key_Request Reply command is used to reply to an LE
 *       Long Term Key Request event from the Controller, and specifies the
 *       Long_Term_Key parameter that shall be used for this Connection_Handle.
 *
 *  \param [in] connection_handle
 *         Connection handle to identify the connection.
 *
 *  \param [in] long_term_key
 *         128-bit long term key (LTK) for the given connection.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_long_term_key_requested_reply
           (
               UINT16   connection_handle,
               UCHAR  * long_term_key
           );
#endif /* HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_SUPPORT */

#ifdef HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_SUPPORT
/**
 *  \brief To reply with a negative response to LE Long Term Key request.
 *
 *  \par Description:
 *       This API can be used to reply to an LE Long Term Key Request event
 *       from the Controller if the Host cannot provide a Long Term Key for
 *       this Connection_Handle.
 *
 *  \param [in] conn_handle
 *         Connection handle to identify the connection.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_long_term_key_requested_negative_reply(conn_handle)  \
        hci_write_command_connection_handle \
        (HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_OPCODE, (conn_handle))
#endif /* HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_SUPPORT */

#ifdef HCI_LE_READ_SUPPORTED_STATES_SUPPORT
/**
 *  \brief To read local Bluetooth Controller supported states and state
 *         combinations.
 *
 *  \par Description:
 *       This API can be used to read states and state combinations that local
 *       link layer supports.
 *
 *       LE_States is an 8-octet bit field. If a bit is set to 1 then this
 *       state or state combination is supported by the Controller.
 *       Multiple bits in LE_States may be set to 1 to indicate support for
 *       multiple state and state combinations.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_supported_states()  \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_SUPPORTED_STATES_OPCODE)
#endif /* HCI_LE_READ_SUPPORTED_STATES_SUPPORT */

#ifdef HCI_LE_RECEIVER_TEST_COMMAND_SUPPORT
/**
 *  \brief To start a test where the DUT receives test reference packets at a
 *  fixed interval.
 *
 *  \par Description:
 *       This API can be used to  start a test where the DUT receives test
 *       reference packets at a fixed interval. The tester generates the
 *       test reference packets.
 *
 *  \param [in] rx_channel
 *         rx_channel : N = (F - 2402) / 2
 *         Range: 0x00 - 0x27.
 *         Frequency Range : 2402 MHz to 2480 MHz
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_receiver_test_command(rx_channel)  \
        hci_write_command_UCHAR \
        (HCI_LE_RECEIVER_TEST_COMMAND_OPCODE, (rx_channel))
#endif /* HCI_LE_RECEIVER_TEST_COMMAND_SUPPORT */

#ifdef HCI_LE_TRANSMITTER_TEST_COMMAND_SUPPORT
/**
 *  \brief To start a test where the DUT generates test reference
 *         packets at a fixed interval.
 *
 *  \par Description:
 *       This API can be used to start a test where the DUT generates
 *       test reference packets at a fixed interval. The Controller
 *       shall transmit at maximum power.
 *
 *       An LE Controller supporting the LE_Transmitter_Test command shall
 *       support Packet_Payload values 0x00, 0x01 and 0x02. An LE Controller
 *       may support other values of Packet_Payload.
 *
 *  \param [in] tx_channel
 *         Trasmit Channel<br>
 *         tx_channel = (F - 2402) / 2<br>
 *         Value Range: 0x00 to 0x27.<br>
 *         Frequency Range : 2402 MHz to 2480 MHz
 *  \param [in] test_data_length
 *         Length in bytes of payload data in each packet<br>
 *         Range: 0x00 to 0xFF
 *  \param [in] packet_payload
 *         Packet payload
 *         - 0x00: PRBS9 sequence '11111111100000111101...' (in transmission order)
 *         - 0x01: Repeated '11110000' (in transmission order) sequence
 *         - 0x02: Repeated '10101010' (in transmission order) sequence
 *         - 0x03: PRBS15 sequence
 *         - 0x04: Repeated '11111111' (in transmission order) sequence
 *         - 0x05: Repeated '00000000' (in transmission order) sequence
 *         - 0x06: Repeated '00001111' (in transmission order) sequence
 *         - 0x07: Repeated '01010101' (in transmission order) sequence
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_transmitter_test_command
           (
               UCHAR    tx_channel,
               UCHAR    test_data_length,
               UCHAR    packet_payload
           );
#endif /* HCI_LE_TRANSMITTER_TEST_COMMAND_SUPPORT */

#ifdef HCI_LE_TEST_END_SUPPORT
/**
 *  \brief To stop any test which is in progress.
 *
 *  \par Description:
 *       This API can be used to stop any test which is in progress.
 *       The Number_Of_Packets for a transmitter test shall be reported as
 *       0x0000. The Number_Of_Packets is an unsigned number and contains
 *       the number of received packets.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_test_end()  \
        hci_common_api_handler_no_params \
        (HCI_LE_TEST_END_OPCODE)
#endif /* HCI_LE_TEST_END_SUPPORT */
#endif /* BT_LE */

#ifdef HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT
/**
 *  \brief To read authenticated payload timeout for a connection.
 *
 *  \par Description:
 *       This command reads the Authenticated_Payload_Timeout parameter in the
 *       Primary Controller on the specified Connection_Handle.
 *       When the Connection_Handle is associated with an ACL connection,
 *       the The Link Manager will use this parameter to trigger when to use
 *       the LMP ping sequence. When the Connection_Handle is associated with
 *       an LE connection, the Link Layer will use this parameter to trigger
 *       when to use the LE ping sequence.
 *
 *  \param [in] connection_handle
 *         Connection Handle of the ACL Connection with
 *         the remote Bluetooth device.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_authenticated_payload_timeout(connection_handle) \
        hci_write_command_connection_handle \
        ( \
               HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_OPCODE, \
               (connection_handle) \
        )
#endif /* HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT */

#ifdef HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT
/**
 *  \brief To write authenticated payload timeout for a connection.
 *
 *  \par Description:
 *       This command writes the Authenticated_Payload_Timeout parameter in
 *       the Primary Controller for the specified Connection_Handle.
 *       When the Connection_Handle identifies a BR/EDR ACL connection and
 *       the connection is in sniff mode, the Authenticated_Payload_Timeout
 *       shall be equal to or greater than Tsniff. If the connection is in sniff
 *       subrating mode, the Authenticated_Payload_Timeout shall be equal to or
 *       greater than (max subrate)xTsniff. If the connection is put into hold
 *       mode, the Authenticated_Payload_Timeout shall be equal to or greater
 *       than the holdTO value. If the connection is in Park state or if the
 *       Connection_Handle corresponds to a synchronous connection, this command
 *       shall be rejected with the error code Command Disallowed (0x0C).
 *
 *       When the Connection_Handle identifies an LE connection,
 *       the Authenticated_Payload_Timeout shall be equal to or greater than
 *       connInterval * (1 + connSlaveLatency) if the device is master and equal
 *       to or greater than connInterval if the device is slave.
 *
 *       The master of an LE connection may change the connInterval resulting it
 *       to be larger than the Authentication_Payload_Timeout used by LE slave.
 *       In this case, the slave Host may receive one or more Authentication
 *       Payload Timeout Expired events without sending LL_PING_REQ
 *       to the master.
 *
 *  \param [in] connection_handle
 *         Connection Handle of the ACL Connection with
 *         the remote Bluetooth device.
 *
 *  \param [in] authenticated_payload_timeout
 *         Authenticated Payload Timeout.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_write_authenticated_payload_timeout
           (
               /* IN */ UINT16 connection_handle,
               /* IN */ UINT16 authenticated_payload_timeout
           );
#endif /* HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT */

/* ------------------------------------------------------------------------- */

/* HCI Testing Commands APIs ----------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* New APIs from Bluetooth Spec 1.2 ---------------------------------------- */

#define BT_hci_set_controller_to_host_flow_control \
        BT_hci_set_host_controller_to_host_flow_control

#ifdef HCI_READ_LOCAL_SUPPORTED_COMMANDS_SUPPORT
/**
 *  \brief To read list of supported HCI Commands from the local Bluetooth
 *         device.
 *
 *  \par Description:
 *       This API reads the list of supported HCI commands from the Link
 *       Manager Protocol of the local Bluetooth device.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_read_local_supported_commands() \
        hci_common_api_handler_no_params \
        (HCI_READ_LOCAL_SUPPORTED_COMMANDS_OPCODE)
#endif /* HCI_READ_LOCAL_SUPPORTED_COMMANDS_SUPPORT */

#ifdef HCI_LL_TOPOLOGY_CONN_UPDATE_SUPPORT
/**
 *  \brief To accept change connection parameter request
 *
 *  \par Description:
 *       This API can be used by both the master Host and the slave Host
 *       to reply to the HCI LE Remote Connection Parameter Request event.
 *       This indicates that the Host has accepted the remote device's
 *       request to change connection parameters.
 *
 *       The Interval_Min and Interval_Max parameters define the minimum and
 *       maximum allowed connection interval. The Interval_Min parameter shall
 *       not be greater than the Interval_Max parameter.
 *
 *       The Latency parameter shall define the maximum allowed slave latency
 *       for the connection in number of connection events.
 *
 *       The Timeout parameter shall define the link supervision timeout for
 *       the LE link.  The Timeout in milliseconds shall be larger than
 *       (1 + Latency) * Interval_Max * 2, where Interval_Max is given in
 *       milliseconds.
 *
 *       The Minimum_CE_Length and Maximum_CE_Length are information parameters
 *       providing the Controller with a hint about the expected minimum and
 *       maximum length of the connection events. The Minimum_CE_Length shall
 *       be less than or equal to the Maximum_CE_Length.
 *
 *       The actual parameter values selected by the Link Layer may be different
 *       from the parameter values provided by the Host through this command.
 *
 *  \param [in] connection_handle
 *         Connection Handle for the existing BLE connection.
 *
 *  \param [in] conn_interval_min
 *         Minimum value for the connection interval.
 *         This shall be less than or equal to Conn_Interval_Max.
 *
 *  \param [in] conn_interval_max
 *         Maximum value for the connection interval.
 *         This shall be greater than or equal to Conn_Interval_Min.
 *
 *  \param [in] conn_latency
 *         Maximum allowed slave latency for the connection specified as the
 *         number of connection events.
 *
 *  \param [in] supervision_timeout
 *         Supervision Timeout for the connection.
 *
 *  \param [in] minimum_ce_length
 *         Information parameter about the minimum length of connection
 *         needed for this LE connection.
 *
 *  \param [in] maximum_ce_length
 *         Information parameter about the maximum length of connection
 *         needed for this LE connection.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_le_remote_conn_param_req_reply
           (
               UINT16   connection_handle,
               UINT16   conn_interval_min,
               UINT16   conn_interval_max,
               UINT16   conn_latency,
               UINT16   supervision_timeout,
               UINT16   minimum_ce_length,
               UINT16   maximum_ce_length
           );

/**
 *  \brief To reject change connection parameter request
 *
 *  \par Description:
 *       This API can be used by both the master Host and the slave Host
 *       to reply to the HCI LE Remote Connection Parameter Request event.
 *       This indicates that the Host has rejected the remote device's request
 *       to change connection parameters.
 *       The reason for the rejection is given in the Reason parameter.
 *
 *       Instead of issuing this command, the Host should try to provide alternative
 *       connection parameters to the Link Layer via the HCI LE Remote Connection
 *       Parameter Request Reply command
 *
 *  \param [in] connection_handle
 *         Connection Handle for the existing BLE connection.
 *
 *  \param [in] reason
 *         This specifies the reason for rejecting connection parameter
 *         update request.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_le_remote_conn_param_req_negtive_reply
           (
               UINT16   connection_handle,
               UCHAR    reason
           );
#endif /* HCI_LL_TOPOLOGY_CONN_UPDATE_SUPPORT */

/* New APIs from Bluetooth Spec 4.2 ---------------------------------------- */

/* Data Length Extension Related */
#ifdef HCI_LE_SET_DATA_LENGTH_SUPPORT
/**
 *  \brief To suggest maximum transmission packet size and maximum packet transmission time
 *  to be used for a given connection.
 *
 *  \par Description:
 *       This API is used to suggest maximum transmission packet size and maximum packet
 *       transmission time (connMaxTxOctets and connMaxTxTime) to be used for a given connection.
 *       The Controller may use smaller or larger values based on local information.
 *
 *  \param [in] connection_handle
 *         Connection Handle for the existing BLE connection.
 *
 *  \param [in] tx_octets
 *         This specifies the preferred maximum number of payload octets that the local Controller
 *         should include in a single Link Layer packet for this connection.
 *
 *  \param [in] tx_time
 *         This specifies the preferred maximum number of microseconds that the local Controller
 *         should use to transmit a single Link Layer packet for this connection.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_le_set_data_length
           (
                UINT16   connection_handle,
                UINT16   tx_octets,
                UINT16   tx_time
           );
#endif /* HCI_LE_SET_DATA_LENGTH_SUPPORT */

#ifdef HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LEN_SUPPORT
/**
 *  \brief To read the Host's suggested values for the Controller's maximum
 *  transmitted number of payload octets and maximum packet transmission time
 *  to be used for new connections.
 *
 *  \par Description:
 *       This API is used to read the Host's suggested values
 *       (SuggestedMaxTxOctets and SuggestedMaxTxTime) for the Controller's
 *       maximum transmitted number of payload octets and maximum packet
 *       transmission time to be used for new connections.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_le_read_suggested_default_data_length() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LEN_OPCODE)
#endif /* HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LEN_SUPPORT */

#ifdef HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LEN_SUPPORT
/**
 *  \brief To specify preferred values for the Controller's maximum transmission
 *  number of payload octets and maximum packet transmission time to be used for
 *  new connections.
 *
 *  \par Description:
 *       This API is used to specify preferred values for the Controller's
 *       maximum transmission number of payload octets and maximum packet
 *       transmission time to be used for new connections. The Controller may
 *       use smaller or larger values for connInitialMaxTxOctets and
 *       connInitialMaxTxTime based on local information.
 *
 *  \param [in] suggested_max_tx_octets
 *         This specifies the Host's suggested value for the Controller's maximum transmitted
 *         number of payload octets to be used for new connections.
 *
 *  \param [in] suggested_max_tx_time
 *         This specifies the Host's suggested value for the Controller's maximum packet
 *         transmission time to be used for new connections.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_le_write_suggested_default_data_length
           (
               UINT16   suggested_max_tx_octets,
               UINT16   suggested_max_tx_time
           );
#endif /* HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LEN_SUPPORT */

/* LE Secure Connections Related */
#ifdef HCI_LE_READ_LOCAL_P_256_PUBLIC_KEY_SUPPORT
/**
 *  \brief To return the local P-256 public key from the Controller.
 *
 *  \par Description:
 *       This API is used to return the local P-256 public key from
 *       the Controller.  The Controller shall generate a new P-256
 *       public/private key pair upon receipt of this command.
 *
 *       The keys returned via this command shall not be used when Secure
 *       Connections is used over the BR/EDR transport.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_le_read_local_p256_public_key() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_LOCAL_P_256_PUBLIC_KEY_OPCODE)
#endif /* HCI_LE_READ_LOCAL_P_256_PUBLIC_KEY_SUPPORT */

#ifdef HCI_LE_GENERATE_DHKEY_SUPPORT
/**
 *  \brief To initiate generation of a Diffie-Hellman key in the Controller
 *  for use over the LE transport.
 *
 *  \par Description:
 *       This API is used to initiate generation of a Diffie-Hellman key in
 *       the Controller for use over the LE transport. This command takes
 *       the remote P-256 public key as input.  The Diffie-Hellman key
 *       generation uses the private key generated by
 *       LE_Read_Local_P256_Public_Key command.
 *
 *       The Diffie-Hellman key returned via this command shall not be
 *       generated using any keys used for Secure Connections over
 *       the BR/EDR transport.
 *
 *  \param [in] remote_public_key
 *         The remote P-256 public key:
 *             X, Y format
 *             Octets 31-0: X co-ordinate
 *             Octets 63-32: Y co-ordinate
 *             Little Endian Format
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_le_generate_dhkey(remote_public_key) \
        hci_write_command_UCHAR_array \
        (HCI_LE_GENERATE_DHKEY_OPCODE, (remote_public_key), 64)
#endif /* HCI_LE_GENERATE_DHKEY_SUPPORT */

/* Privacy 1.2 related HCI Commands */
#ifdef HCI_LE_ADD_DEV_TO_RESOLVING_LIST_SUPPORT
/**
 *  \brief To add one device to the list of address translations used
 *   to resolve Resolvable Private Addresses in the controller.
 *
 *  \par Description:
 *       This API can be used to add one device to the list of address
 *       translations used to resolve Resolvable Private Addresses in the
 *       Controller.
 *
 *       This command cannot be used when address translation is enabled
 *       in the Controller and:
 *       - Advertising is enabled
 *       - Scanning is enabled
 *       - Create connection command is outstanding
 *
 *       This command can be used at any time when address translation is
 *       disabled in the Controller.
 *
 *       The added device shall be set to Network Privacy mode.
 *
 *       When a Controller cannot add a device to the list because there
 *       is no space available, it shall return the error code Memory Capacity
 *       Exceeded (0x07).
 *
 *  \param [in] peer_id_addr_type
 *         - Public identity Address: \ref BT_BD_PUBLIC_ADDRESS_TYPE (0x00)
 *         - Random (static) identity Address: \ref BT_BD_RANDOM_ADDRESS_TYPE (0x01)
 *
 *  \param [in] peer_id_addr
 *         Public or Random (static) Identity address of the peer device
 *
 *  \param [in] peer_irk
 *         IRK of the peer device
 *
 *  \param [in] local_irk
 *         IRK of the local device
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_add_device_to_resolving_list
           (
               UCHAR    peer_id_addr_type,
               UCHAR *  peer_id_addr,
               UCHAR *  peer_irk,
               UCHAR *  local_irk
           );
#endif /* HCI_LE_ADD_DEV_TO_RESOLVING_LIST_SUPPORT */

#ifdef HCI_LE_REMOVE_DEV_FROM_RESOLVING_LIST_SUPPORT
/**
 *  \brief To remove one device from the list of address translations used
 *   to resolve Resolvable Private Addresses in the controller.
 *
 *  \par Description:
 *       This API can be used to remove one device from the list of address
 *       translations used to resolve Resolvable Private Addresses in the controller.
 *
 *       This command cannot be used when address translation is enabled in the
 *       Controller and:
 *       - Advertising is enabled
 *       - Scanning is enabled
 *       - Create connection command is outstanding
 *
 *       This command can be used at any time when address translation is
 *       disabled in the Controller.
 *
 *       When a Controller cannot remove a device from the resolving list
 *       because it is not found, it shall return the error code
 *       Unknown Connection Identifier (0x02).
 *
 *  \param [in] peer_id_addr_type
 *         - Public identity Address: \ref BT_BD_PUBLIC_ADDRESS_TYPE (0x00)
 *         - Random (static) identity Address: \ref BT_BD_RANDOM_ADDRESS_TYPE (0x01)
 *
 *  \param [in] peer_id_addr
 *         Public or Random (static) Identity Address of the peer device to be removed.
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_remove_device_from_resolving_list(peer_id_addr_type, peer_id_addr)\
        hci_write_command_BD_ADDR_with_TYPE \
        (HCI_LE_REMOVE_DEV_FROM_RESOLVING_LIST_OPCODE , (peer_id_addr_type), (peer_id_addr))
#endif /* HCI_LE_REMOVE_DEV_FROM_RESOLVING_LIST_SUPPORT */

#ifdef HCI_LE_CLR_RESOLVING_LIST_SUPPORT
/**
 *  \brief To remove all devices from the list of address translations
 *   used to resolve Resolvable Private Addresses in the Controller.
 *
 *  \par Description:
 *       This API is used to remove all devices from the list of address
 *       translations used to resolve Resolvable Private Addresses
 *       in the Controller.
 *
 *       This command cannot be used when address translation is enabled
 *       in the Controller and:
 *       - Advertising is enabled
 *       - Scanning is enabled
 *       - Create connection command is outstanding
 *
 *       This command can be used at any time when address translation is
 *       disabled in the Controller.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
#define BT_hci_le_clear_resolving_list() \
        hci_common_api_handler_no_params \
        (HCI_LE_CLR_RESOLVING_LIST_OPCODE)
#endif /* HCI_LE_CLR_RESOLVING_LIST_SUPPORT */

#ifdef HCI_LE_READ_RESOLVING_LIST_SIZE_SUPPORT
/**
 *  \brief To read the total number of address translation
 *  entries in the resolving list that can be stored in the Controller.
 *
 *  \par Description:
 *       This API is used to read the total number of address translation
 *       entries in the resolving list that can be stored in the Controller.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 *
 *  \note:
 *  The number of entries that can be stored is not fixed and the Controller
 *  can change it at any time (e.g. because the memory used to store the list
 *  can also be used for other purposes).
 */
#define BT_hci_le_read_resolving_list_size() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_RESOLVING_LIST_SIZE_OPCODE)
#endif /* HCI_LE_READ_RESOLVING_LIST_SIZE_SUPPORT */


#ifdef HCI_LE_READ_PEER_RESOLVABLE_ADDR_SUPPORT
/**
 *  \brief To get the current peer Resolvable Private Address being
 *  used for the corresponding peer Public or Random (static) Identity Address.
 *
 *  \par Description:
 *       This API is used to get the current peer Resolvable Private Address
 *       being used for the corresponding peer Public or Random (static)
 *       Identity Address. The peer's resolvable address being used may change
 *       after the command is called.
 *
 *       This command can be used at any time.
 *
 *       When a Controller cannot find a Resolvable Private Address associated
 *       with the Peer Identity Address, it shall return the error code Unknown
 *       Connection Identifier (0x02).
 *
 *  \param [in] peer_id_addr_type
 *         - Public identity Address: \ref BT_BD_PUBLIC_ADDRESS_TYPE (0x00)
 *         - Random (static) identity Address: \ref BT_BD_RANDOM_ADDRESS_TYPE (0x01)
 *
 *  \param [in] peer_id_addr
 *         Public or Random (static) Identity address of the peer device
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_peer_resolvable_address(peer_id_addr_type, peer_id_addr)\
        hci_write_command_BD_ADDR_with_TYPE \
        (HCI_LE_READ_PEER_RESOLVABLE_ADDR_OPCODE, (peer_id_addr_type), (peer_id_addr))
#endif /* HCI_LE_READ_PEER_RESOLVABLE_ADDR_SUPPORT */


#ifdef HCI_LE_READ_LOCAL_RESOLVABLE_ADDR_SUPPORT
/**
 *  \brief To get the current local Resolvable Private Address being used
 *  for the corresponding peer Identity Address.
 *
 *  \par Description:
 *       This API is used to get the current local Resolvable Private Address
 *       being used for the corresponding peer Identity Address. The local's
 *       resolvable address being used may change after the command is called.
 *
 *       This command can be used at any time.
 *
 *       When a Controller cannot find a Resolvable Private Address associated
 *       with the Peer Identity Address, it shall return the error code Unknown
 *       Connection Identifier (0x02).
 *
 *  \param [in] peer_id_addr_type
 *         - Public identity Address: \ref BT_BD_PUBLIC_ADDRESS_TYPE (0x00)
 *         - Random (static) identity Address: \ref BT_BD_RANDOM_ADDRESS_TYPE (0x01)
 *
 *  \param [in] peer_id_addr
 *         Public or Random (static) Identity address of the peer device
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_local_resolvable_address(peer_id_addr_type, peer_id_addr)\
        hci_write_command_BD_ADDR_with_TYPE \
        (HCI_LE_READ_LOCAL_RESOLVABLE_ADDR_OPCODE, (peer_id_addr_type), (peer_id_addr))
#endif /* HCI_LE_READ_LOCAL_RESOLVABLE_ADDR_SUPPORT */


#ifdef HCI_LE_SET_ADDR_RESOLUTION_ENABLE_SUPPORT
/**
 *  \brief To enable resolution of Resolvable Private Addresses
 *  in the Controller.
 *
 *  \par Description:
 *       This API can be used to enable resolution of Resolvable Private
 *       Addresses in the Controller.
 *
 *       This can be used at any time except when:
 *       - Advertising is enabled
 *       - Scanning is enabled
 *       - Create connection command is outstanding
 *
 *  \param [in] addr_resolution_enable
 *         Enable/Disable Resolution of of Resolvable Private Addresses
 *         0x00: Address Resolution in controller disabled (default)
 *         0x01: Address Resolution in controller enabled
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note:
 *  Enabling address resolution when it is already enabled, or disabling it
 *  when it is already disabled, has no effect.
 */
#define BT_hci_le_set_address_resolution_enable(addr_resolution_enable)  \
       hci_write_command_UCHAR \
       (HCI_LE_SET_ADDR_RESOLUTION_ENABLE_OPCODE, (addr_resolution_enable))
#endif /* HCI_LE_SET_ADDR_RESOLUTION_ENABLE_SUPPORT */


#ifdef HCI_LE_SET_RESOLVABLE_PRIVATE_ADDR_TIMEOUT_SUPPORT

/**
 *  \brief To set the length of time the controller uses a Resolvable Private Address
 *  before a new resolvable private address is generated and starts being used
 *
 *  \par Description:
 *       This API is used to set the length of time the controller uses a Resolvable Private Address
 *       before a new resolvable private address is generated and starts being used.
 *
 *       This timeout applies to all addresses generated by the controller.
 *
 *  \param [in] rpa_timeout
 *         rpa_timeout measured in seconds
 *         Range for N: 0x0001 - 0xA1B8 (1 sec - approximately 11.5 hours)
 *         Default: N = 0x0384 (900 secs or 15 minutes)
 *
 *  \return API_RESULT
 *  API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_set_resolvable_private_address_timeout(rpa_timeout)\
        hci_write_command_UINT16 \
        (HCI_LE_SET_RESOLVABLE_PRIVATE_ADDR_TIMEOUT_OPCODE , (rpa_timeout))
#endif /* HCI_LE_SET_RESOLVABLE_PRIVATE_ADDR_TIMEOUT_SUPPORT */


#ifdef HCI_LE_READ_MAXIMUM_DATA_LEN_SUPPORT

/**
 *  \brief To read the Controller's maximum supported payload octets and packet duration times
 *  for transmission and reception
 *
 *  \par Description:
 *       This API is used to read the Controller's maximum supported payload octets and
 *       packet duration times for transmission and reception (supportedMaxTxOctets and
 *       supportedMaxTxTime, supportedMaxRxOctets, and supportedMaxRxTime)
 *
 *  \return API_RESULT
 *  API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_maximum_data_length() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_MAXIMUM_DATA_LENGTH_OPCODE)
#endif /* HCI_LE_READ_MAXIMUM_DATA_LEN_SUPPORT */


/* New APIs from Bluetooth Spec 5.0 ---------------------------------------- */
#ifdef HCI_LE_READ_PHY_SUPPORT
/**
 *  \brief To read the current transmitter PHY and receiver PHY.
 *
 *  \par Description:
 *       This API is used to read the current transmitter PHY and receiver PHY
 *       on the connection identified by the Connection_Handle.
 *
 *  \param [in] connection_handle
 *         Connection Handle
 *         Range:0x0000-0x0EFF
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_read_phy(connection_handle) \
        hci_write_command_connection_handle \
        ( \
            HCI_LE_READ_PHY_OPCODE, \
            (connection_handle) \
        )
#endif /* HCI_LE_READ_PHY_SUPPORT */

#ifdef HCI_LE_SET_DEFAULT_PHY_SUPPORT
/**
 *  \brief To specify preferred values for the transmitter PHY and receiver PHY.
 *
 *  \par Description:
 *       This API is used to specify the preferred values for the transmitter
 *       PHY and receiver PHY to be used for all subsequent connections over
 *       the LE transport.
 *
 *  \param [in] all_phys
 *         The ALL_PHYS parameter is a bit field that allows the Host to specify,
 *         for each direction, whether it has no preference among the PHYs that
 *         the Controller supports in a given direction or whether it will
 *         specify particular PHYs in the TX_PHYS or RX_PHYS parameter.
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0               The Host has no preference among the transmitter PHYs
 *                         supported by the Controller
 *
 *         1               The Host has no preference among the receiver PHYs
 *                         supported by the Controller
 *
 *  \param [in] tx_phys
 *         The TX_PHYS parameter is a bit field that indicates the transmitter PHYs that the Host
 *         prefers the Controller to use. If the ALL_PHYS parameter specifies that the Host has no
 *         preference, the TX_PHYS parameter is ignored; otherwise at least one bit shall be set
 *         to 1.
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0               The Host prefers to use the LE 1M transmitter PHY (possibly among others)
 *         1               The Host prefers to use the LE 2M transmitter PHY (possibly among others)
 *         2               The Host prefers to use the LE Coded transmitter PHY (possibly among others)
 *
 *  \param [in] rx_phys
 *         The RX_PHYS parameter is a bit field that indicates the receiver PHYs that the Host
 *         prefers the Controller to use. If the ALL_PHYS parameter specifies that the Host has no
 *         preference, the RX_PHYS parameter is ignored; otherwise at least one bit shall be set
 *         to 1.
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0               The Host prefers to use the LE 1M transmitter PHY (possibly among others)
 *         1               The Host prefers to use the LE 2M transmitter PHY (possibly among others)
 *         2               The Host prefers to use the LE Coded transmitter PHY (possibly among others)
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_default_phy
           (
               UCHAR    all_phys,
               UCHAR    tx_phys,
               UCHAR    rx_phys
           );
#endif /* HCI_LE_SET_DEFAULT_PHY_SUPPORT */

#ifdef HCI_LE_SET_PHY_SUPPORT
/**
 *  \brief To set the PHY preferences for a connection.
 *
 *  \par Description:
 *       This API is  used to set the PHY preferences for the connection
 *       identified by the Connection_Handle. The Controller might not be able
 *       to make the change (e.g. because the peer does not support
 *       the requested PHY) or may decide that the current PHY is preferable.
 *
 *       If, for at least one direction, the Host has specified a preference
 *       and the current PHY is not one of those preferred, the Controller
 *       shall request a change.  Otherwise the Controller may, but need not,
 *       request a change.
 *
 *       The PHY preferences provided by the LE Set PHY command override those
 *       provided via the LE Set Default PHY command or any preferences
 *       previously set using the LE Set PHY command on the same connection.
 *
 *       The PHY_options parameter is a bit field that allows the Host to
 *       specify options for PHYs. The default value for a new connection shall
 *       be all zero bits. The Controller may override any preferred coding for
 *       transmitting on the LE Coded PHY.
 *
 *       The Host may specify a preferred coding even if it prefers not to use
 *       the LE Coded transmitter PHY since the Controller may override the PHY
 *       preference.
 *
 *  \param [in] connection_handle
 *         Connection Handle
 *         Range:0x0000-0x0EFF
 *
 *  \param [in] all_phys
 *         The ALL_PHYS parameter is a bit field that allows the Host to specify,
 *         for each direction, whether it has no preference among the PHYs that the Controller
 *         supports in a given direction or whether it will specify particular PHYs in
 *         the TX_PHYS or RX_PHYS parameter.
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0               The Host has no preference among the transmitter PHYs
 *                         supported by the Controller
 *
 *         1               The Host has no preference among the receiver PHYs
 *                         supported by the Controller
 *
 *  \param [in] tx_phys
 *         The TX_PHYS parameter is a bit field that indicates the transmitter PHYs that the Host
 *         prefers the Controller to use. If the ALL_PHYS parameter specifies that the Host has no
 *         preference, the TX_PHYS parameter is ignored; otherwise at least one bit shall be set
 *         to 1.
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0               The Host prefers to use the LE 1M transmitter PHY (possibly among others)
 *         1               The Host prefers to use the LE 2M transmitter PHY (possibly among others)
 *         2               The Host prefers to use the LE Coded transmitter PHY (possibly among others)
 *
 *  \param [in] rx_phys
 *         The RX_PHYS parameter is a bit field that indicates the receiver PHYs that the Host
 *         prefers the Controller to use. If the ALL_PHYS parameter specifies that the Host has no
 *         preference, the RX_PHYS parameter is ignored; otherwise at least one bit shall be set
 *         to 1.
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0               The Host prefers to use the LE 1M transmitter PHY (possibly among others)
 *         1               The Host prefers to use the LE 2M transmitter PHY (possibly among others)
 *         2               The Host prefers to use the LE Coded transmitter PHY (possibly among others)
 *
 *  \param [in] phy_options
 *
 *         Bit number      Meaning
 *         ----------      -------
 *         0 - 1           0 = the Host has no preferred coding when transmitting on the LE Coded PHY
 *                         1 = the Host prefers that S=2 coding be used when transmitting on the LE Coded PHY
 *                         2 = the Host prefers that S=8 coding be used when transmitting on the LE Coded PHY
 *                         3 = Reserved for future use
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_phy
           (
                UINT16   connection_handle,
                UCHAR    all_phys,
                UCHAR    tx_phys,
                UCHAR    rx_phys,
                UINT16   phy_options
           );
#endif /* HCI_LE_SET_PHY_SUPPORT */

#ifdef HCI_LE_ENHANCED_RECEIVER_TEST_SUPPORT
/**
 *  \brief To start a test where the DUT receives test reference packets at a
 *  fixed interval.
 *
 *  \par Description:
 *       This API can be used to  start a test where the DUT receives test
 *       reference packets at a fixed interval. The tester generates the
 *       test reference packets.
 *
 *  \param [in] rx_channel Receive channel.
 *                         rx_channel : N = (F - 2402) / 2
 *                         Range: 0x00 - 0x27.
 *                         Frequency Range : 2402 MHz to 2480 MHz
 *
 *  \param [in] phy PHY to test
 *                  0x01: Receiver set to receive data at 1 M PHY
 *                  0x02: Receiver set to receive data at 2 M PHY
 *                  0x03: Receiver set to use the LE Coded PHY
 *
 *  \param [in] modulation_index Modulation Index type used for the test
 *                               0x00: Assume transmitter will have a standard modulation index
 *                               0x01: Assume transmitter will have a stable modulation index
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_enhanced_receiver_test_command
           (
               UCHAR rx_channel,
               UCHAR phy,
               UCHAR modulation_index
           );
#endif /* HCI_LE_ENHANCED_RECEIVER_TEST_SUPPORT */

#ifdef HCI_LE_ENHANCED_TRANSMITTER_TEST_SUPPORT
/**
 *  \brief To start a test where the DUT generates test reference
 *         packets at a fixed interval.
 *
 *  \par Description:
 *       This API can be used to start a test where the DUT generates
 *       test reference packets at a fixed interval. The Controller
 *       shall transmit at maximum power.
 *       An LE Controller supporting the LE_Transmitter_Test command shall
 *       support Packet_Payload values 0x00, 0x01 and 0x02. An LE Controller
 *       supporting the LE Coded PHY shall also support Packet_Payload value
 *       0x04. An LE Controller may support other values of Packet_Payload.
 *
 *  \param [in] tx_channel
 *         Trasmit Channel<br>
 *         tx_channel = (F - 2402) / 2
 *         Value Range: 0x00 to 0x27.
 *         Frequency Range : 2402 MHz to 2480 MHz
 *
 *  \param [in] test_data_length
 *         Length in bytes of payload data in each packet
 *
 *  \param [in] packet_payload
 *         Packet payload
 *         - 0x00: PRBS9 sequence '11111111100000111101...' (in transmission order)
 *         - 0x01: Repeated '11110000' (in transmission order) sequence
 *         - 0x02: Repeated '10101010' (in transmission order) sequence
 *         - 0x03: PRBS15 sequence
 *         - 0x04: Repeated '11111111' (in transmission order) sequence
 *         - 0x05: Repeated '00000000' (in transmission order) sequence
 *         - 0x06: Repeated '00001111' (in transmission order) sequence
 *         - 0x07: Repeated '01010101' (in transmission order) sequence
 *
 *  \param [in] phy PHY to test
 *                  0x01: Transmitter set to use the LE 1M PHY
 *                  0x02: Transmitter set to use the LE 2M PHY
 *                  0x03: Transmitter set to use the LE Coded PHY with S=8 data coding
 *                  0x04: Transmitter set to use the LE Coded PHY with S=2 data coding
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_enhanced_transmitter_test_command
           (
               UCHAR    tx_channel,
               UCHAR    test_data_length,
               UCHAR    packet_payload,
               UCHAR    phy
           );
#endif /* HCI_LE_ENHANCED_TRANSMITTER_TEST_SUPPORT */

/* HCI Advertising Extension Commands APIs ---------------------------------- */
#ifdef HCI_LE_SET_ADVERTISING_SET_RANDOM_ADDRESS_SUPPORT
/**
 *  \brief To set the Random device address for the advertising set
 *
 *  \par Description:
 *       This API is used by the Host to set the random device address specified
 *       by the Random_Address parameter. This address is used in the Controller
 *       for the advertiser's address contained in the advertising PDUs for the
 *       advertising set specified by the Advertising_Handle parameter.
 *
 *       If the Host issues this command while an advertising set using
 *       connectable advertising is enabled, the Controller shall return the
 *       error code Command Disallowed (0x0C). The Host may issue this command
 *       at any other time.
 *
 *       If this command is used to change the address, the new random address
 *       shall take effect for advertising no later than the next successful LE
 *       Extended Set Advertising Enable Command and for periodic advertising no
 *       later than the next successful LE Periodic Advertising Enable Command.
 *
 *  \param [in] advertising_handle
 *         0x00 - 0xEF Used to identify an advertising set
 *
 *  \param [in] random_address
 *         Random Device Address
 *
 *  \return
 *       API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_adv_set_random_address
           (
               UCHAR   advertising_handle,
               UCHAR * random_address
           );
#endif /* HCI_LE_SET_ADVERTISING_RANDOM_ADDRESS_SUPPORT */

#ifdef HCI_LE_SET_EXTENDED_ADV_PARAMS_SUPPORT
/**
 *  \brief To set extended advertising parameters
 *
 *  \par Description:
 *       This API is used to is used by the Host to set extended advertising parameters.
 *
 *  \param [in] adv_handle
 *         0x00 - 0xEF: Used to identify the advertising set whose parameters are being configured
 *
 *  \param [in] adv_event_properties
 *         0 Connectable advertising
 *         1 Scannable advertising
 *         2 Directed advertising
 *         3 High Duty Cycle Directed Connectable advertising (<= 3.75 ms Advertising Interval)
 *         4 Use legacy advertising PDUs
 *         5 Omit advertiser's address from all PDUs ("anonymous advertising")
 *         6 Include TxPower in the extended header of the advertising PDU
 *
 *  \param [in] primary_advertising_interval_min
 *         Minimum advertising interval for undirected and low duty cycle directed advertising.
 *         Range: 0x000020 to 0xFFFFFF
 *         Time = N * 0.625 msec
 *         Time Range: 20 ms to 10,485.759375 s
 *
 *  \param [in] primary_advertising_interval_max
 *         Maximum advertising interval for undirected and low duty cycle directed advertising.
 *         Range: 0x000020 to 0xFFFFFF
 *         Time = N * 0.625 msec
 *         Time Range: 20 ms to 10,485.759375 s
 *
 *  \param [in] primary_advertising_channel_map
 *         0 - Channel 37 shall be used
 *         1 - Channel 38 shall be used
 *         2 - Channel 39 shall be used
 *
 *  \param [in] own_address_type
 *         0x00    Public Device Address
 *         0x01    Random Device Address
 *         0x02    Controller generates Resolvable Private Address based on the local
 *                 IRK from resolving list. If resolving list contains no matching entry,
 *                 use public address.
 *         0x03    Controller generates Resolvable Private Address based on the local IRK
 *                 from resolving list. If resolving list contains no matching entry, use
 *                 random address from LE_Set_Random_Address.
 *
 *  \param [in] peer_address_type
 *         0x00    Public Device Address or Public Identity Address
 *         0x01    Random Device Address or Random (static) Identity Address
 *
 *  \param [in] peer_address
 *         Public Device Address, Random Device Address, Public Identity Address,
 *         or Random (static) Identity Address of the device to be connected.
 *
 *  \param [in] advertising_filter_policy
 *         0x00    Process scan and connection requests from all devices
 *                 (i.e., the White List is not in use).
 *         0x01    Process connection requests from all devices and only scan requests
 *                 from devices that are in the White List.
 *         0x02    Process scan requests from all devices and only connection requests
 *                 from devices that are in the White List.
 *         0x03    Process scan and connection requests only from devices in the
 *                 White List.
 *
 *  \param [in] advertising_tx_power
 *         Size: 1 Octet (signed integer)
 *         Range: -127 <= N <= +126
 *         Units: dBm
 *         127: Host has no preference
 *
 *  \param [in] primary_advertising_phy
 *         0x01    Primary advertisement PHY is LE 1M
 *         0x03    Primary advertisement PHY is LE Coded
 *
 *  \param [in] secondary_adv_max_skip
 *         0x00 AUX_ADV_IND shall be sent prior to the next advertising event
 *         0x01-0xFF Maximum advertising events the Controller can skip before sending the
 *         AUX_ADV_IND packets on the secondary advertising channel
 *
 *  \param [in] secondary_advertising_phy
 *         0x01    Secondary advertisement PHY is LE 1M
 *         0x02    Secondary advertisement PHY is LE 2M
 *         0x03    Secondary advertisement PHY is LE Coded
 *
 *  \param [in] advertising_set_id
 *         0x00 - 0x0F Value of the Advertising SID subfield in the ADI field of the PDU
 *
 *  \param [in] scan_req_nfy_enable
 *         0x00  Scan request notifications disabled
 *         0x01  Scan request notifications enabled
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_extended_advertising_parameters
           (
               UCHAR   adv_handle,
               UINT16  adv_event_properties,
               UINT32  primary_advertising_interval_min,
               UINT32  primary_advertising_interval_max,
               UCHAR   primary_advertising_channel_map,
               UCHAR   own_address_type,
               UCHAR   peer_address_type,
               UCHAR * peer_address,
               UCHAR   advertising_filter_policy,
               INT8    advertising_tx_power,
               UCHAR   primary_advertising_phy,
               UCHAR   secondary_adv_max_skip,
               UCHAR   secondary_advertising_phy,
               UCHAR   advertising_set_id,
               UCHAR   scan_req_nfy_enable
           );
#endif /* HCI_LE_SET_EXTENDED_ADV_PARAMS_SUPPORT */

#ifdef HCI_LE_SET_EXTENDED_ADVERTISING_DATA_SUPPORT
/**
 *  \brief Used to set the data used in advertising packets
 *
 *  \par Description:
 *       This API is used to set the data used in advertising PDUs that
 *       have a data field. This command may be issued at any time after
 *       an advertising set identified by the Advertising_Handle parameter
 *       has been created using the LE Set Extended Advertising Parameters
 *       Command (see Section 7.8.53), regardless of whether advertising
 *       in that set is enabled or disabled.
 *
 *  \param [in] advertising_handle
 *         Used to identify an advertising set
 *
 *  \param [in] operation
 *         0x00  - Intermediate fragment of fragmented extended advertising data
 *         0x01  - First fragment of fragmented extended advertising data
 *         0x02  - Last fragment of fragmented extended advertising data
 *         0x03  - Complete extended advertising data
 *         0x04  - Unchanged data (just update the Advertising DID)
 *         All other values Reserved for future use

 *  \param [in] frag_pref
 *         0x00 - The Controller may fragment all Host advertising data
 *         0x01 - The Controller should not fragment or should minimize
 *                fragmentation of Host advertising data
 *         All other values Reserved for future use

 *  \param [in] advertising_data_length
 *         0-251 - The number of octets in the Advertising Data parameter
 *
 *  \param [in] advertising_data
 *         Advertising data
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_extended_advertising_data
           (
               /* IN */ UCHAR   advertising_handle,
               /* IN */ UCHAR   operation,
               /* IN */ UCHAR   frag_pref,
               /* IN */ UCHAR   advertising_data_length,
               /* IN */ UCHAR * advertising_data
           );
#endif /* HCI_LE_SET_EXTENDED_ADVERTISING_DATA_SUPPORT */

#ifdef HCI_LE_SET_EXTENDED_SCAN_RESPONSE_DATA_SUPPORT
/**
 *  \brief Used to provide data used in Scanning Packets
 *
 *  \par Description:
 *       This API is used to provide scan response data used in scanning
 *       response PDUs. This command may be issued at any time after the
 *       advertising set identified by the Advertising_Handle parameter
 *       has been created using the LE Set Extended Advertising Parameters
 *       Command regardless of whether advertising in that set is enabled
 *       or disabled.
 *
 *  \param [in] advertising_handle
 *         Used to identify an advertising set
 *
 *  \param [in] operation
 *         0x00   Intermediate fragment of fragmented scan response data
 *         0x01   First fragment of fragmented scan response data
 *         0x02   Last fragment of fragmented scan response data
 *         0x03   Complete scan response data
 *
 *  \param [in] fragment_pref
 *         0x00   The Controller may fragment all scan response data
 *         0x01   The Controller should not fragment or should minimize
 *                fragmentation of scan response data
 *
 *  \param [in] scan_response_data_length
 *         0 - 251 The number of octets in the Scan_Response Data parameter
 *
 *  \param [in] scan_response_data
 *         Scan response data
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_extended_scan_response_data
           (
               UCHAR   advertising_handle,
               UCHAR   operation,
               UCHAR   fragment_pref,
               UCHAR   scan_response_data_length,
               UCHAR * scan_response_data
           );
#endif /* HCI_LE_SET_EXTENDED_SCAN_RESPONSE_DATA_SUPPORT */

#ifdef HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_SUPPORT
/**
 *  \brief Used to request the Controller to enable or disable one or
 *  more advertising sets
 *
 *  \par Description:
 *       This API is used to request the Controller to enable or disable
 *       one or more advertising sets using the advertising sets identified
 *       by the Advertising_Handle[i] parameter. The Controller manages the
 *       timing of advertisements in accordance with the advertising
 *       parameters given in the LE_Set_Extended_Advertising_Parameters
 *       command. The Number_of_Sets parameter is the number of advertising
 *       sets contained in the parameter arrays.
 *       If Enable and Number_of_Sets are both set to 0x00, then all
 *       advertising sets are disabled.
 *
 *  \param [in] enable
 *         0x00 - Advertising is disabled
 *         0x01 - Advertising is enabled
 *
 *  \param [in] number_of_sets
 *         0x00   Disable all advertising sets
 *         0x01 - 0x3F Number of advertising sets to enable or disable
 *
 *  \param [in] advertising_handle
 *         0x00 - 0xEF Used to identify an advertising set
 *
 *  \param [in] duration
 *         0x0000 No advertising duration. Advertising to continue until
 *                the Host disables it.
 *         N = 0xXXXX Advertising duration
 *         Range: 0x0001 - 0xFFFF
 *         Time = N * 10 ms
 *         Time Range: 10 ms to 655,350 ms
 *
 *  \param [in] max_extd_adv_evts
 *         Maximum number of extended advertising events the Controller shall
 *         attempt to send prior to terminating the extended advertising
 *         0x00    No maximum number of advertising events.
 *         N=0xXX  Maximum number of extended advertising events the Controller
 *                 shall attempt to send prior to terminating the extended advertising
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_extended_advertising_enable
           (
               UCHAR    enable,
               UCHAR    number_of_sets,
               UCHAR  * advertising_handle,
               UINT16 * duration,
               UCHAR  * max_extd_adv_evts
           );
#endif /* HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_SUPPORT */

#ifdef HCI_LE_READ_MAX_ADV_DATA_LENGTH_SUPPORT
/**
 *  \brief To read the maximum length of advertising data supported by
 *  the advertising Controller
 *
 *  \par Description:
 *       This API is used to used to read the maximum length of data supported
 *       by the Controller for use as advertisement data or scan response data
 *       in an advertising event or as periodic advertisement data.
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_maximum_advertising_data_length() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_MAX_ADV_DATA_LENGTH_OPCODE)
#endif /* HCI_LE_READ_MAX_ADV_DATA_LENGTH_SUPPORT */

#ifdef HCI_LE_READ_NUM_SUPPORTED_ADV_SETS_SUPPORT
/**
 *  \brief To read the maximum number of advertising sets.
 *
 *  \par Description:
 *       This API is used to read the maximum number of advertising sets supported
 *       by the advertising Controller at the same time.
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note
 *  The number of advertising sets that can be supported is not fixed
 *  and the Controller can change it at any time because the memory
 *  used to store advertising sets can also be used for other purposes.
 */
#define BT_hci_le_read_num_supported_advertising_sets() \
        hci_common_api_handler_no_params \
       (HCI_LE_READ_NUM_SUPPORTED_ADV_SETS_OPCODE)
#endif /* HCI_LE_READ_NUM_SUPPORTED_ADV_SETS_SUPPORT */

#ifdef HCI_LE_REMOVE_ADVERTISING_SET_SUPPORT
/**
 *  \brief To remove an advertising set from the Controller.
 *
 *  \par Description:
 *       This API is used to remove an advertising set from the Controller.
 *
 *       If the advertising set corresponding to the Advertising_Handle parameter
 *       does not exist, then the Controller shall return the error code Unknown
 *       Advertising Identifier (0x42). If advertising on the advertising set
 *       is enabled, then the Controller shall return the error code Command
 *       Disallowed (0x0C).
 *
 *  \param [in] advertising_handle
 *       0x00 - 0xEF   Used to identify an advertising set
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_remove_advertising_set(advertising_handle) \
        hci_write_command_UCHAR \
        (HCI_LE_REMOVE_ADVERTISING_SET_OPCODE, (advertising_handle))
#endif /* HCI_LE_REMOVE_ADVERTISING_SET_SUPPORT */

#ifdef HCI_LE_CLEAR_ADVERTISING_SETS_SUPPORT
/**
 *  \brief To remove all existing advertising sets from the Controller
 *
 *  \par Description:
 *       This API is used to remove all existing advertising sets from the
 *       Controller.
 *
 *       If the advertising set corresponding to the Advertising_Handle
 *       parameter does not exist, then the Controller shall return
 *       the error code Unknown Advertising Identifier (0x42).
 *       If advertising on the advertising set is enabled, then the
 *       Controller shall return the error code Command Disallowed (0x0C).
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note
 *  All advertising sets are cleared on HCI reset.
 */
#define BT_hci_le_clear_advertising_sets() \
        hci_common_api_handler_no_params \
        (HCI_LE_CLEAR_ADVERTISING_SETS_OPCODE)
#endif /* HCI_LE_CLEAR_ADVERTISING_SETS_SUPPORT */

#ifdef HCI_LE_SET_PERIODIC_ADV_PARAMS_SUPPORT
/**
 *  \brief To set the parameters for periodic advertising
 *
 *  \par Description:
 *       This API is used to set the parameters for periodic advertising.
 *
 *       The Advertising_Handle parameter identifies the advertising set whose
 *       periodic advertising parameters are being configured. If the
 *       corresponding advertising set does not already exist, then the
 *       Controller shall return the error code Unknown Advertising Identifier
 *       (0x42).
 *
 *  \param [in] periodic_adv_handle
 *         0x00 - 0xEF  Used to identify a periodic advertisement
 *
 *  \param [in] periodic_advertising_interval_min
 *         Minimum advertising interval for periodic advertising.
 *         Range: 0x0006 to 0xFFFF
 *         Time = N * 1.25 ms
 *         Time Range: 7.5 ms to 81.91875 s
 *
 *  \param [in] periodic_advertising_interval_max
 *         Maximum advertising interval for periodic advertising.
 *         Range: 0x0006 to 0xFFFF
 *         Time = N * 1.25 ms
 *         Time Range: 7.5 ms to 81.91875 s
 *
 *  \param [in] periodic_adv_prty
 *         Bit Number       Parameter Description
 *         ----------       ---------------------
 *         6                Include TxPower in the advertising PDU
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_periodic_advertising_parameters
           (
               UCHAR   periodic_adv_handle,
               UINT16  periodic_advertising_interval_min,
               UINT16  periodic_advertising_interval_max,
               UINT16  periodic_adv_prty
           );
#endif /* HCI_LE_SET_PERIODIC_ADV_PARAMS_SUPPORT */

#ifdef HCI_LE_SET_PERIODIC_ADVERTISING_DATA_SUPPORT
/**
 *  \brief To set the data used in periodic advertising PDUs.
 *
 *  \par Description:
 *       This API is used to set the data used in periodic advertising PDUs.
 *       This command may be issued at any time after the advertising set
 *       identified by the Advertising_Handle parameter has been configured
 *       for periodic advertising using the
 *       LE_Set_Periodic_Advertising_Parameters Command, regardless of
 *       whether advertising in that set is enabled or disabled. If the
 *       advertising set has not been configured for periodic advertising,
 *       then the Controller shall return the error code Command Disallowed
 *       (0x0C).
 *
 *  \param [in] advertising_handle
 *         0x00 - 0xEF Used to identify an advertising set
 *
 *  \param [in] operation
 *         0x00   Intermediate fragment of fragmented scan response data
 *         0x01   First fragment of fragmented scan response data
 *         0x02   Last fragment of fragmented scan response data
 *         0x03   Complete scan response data
 *
 *  \param [in] advertising_data_length
 *         0 - 252 The number of octets in the Advertising Data parameter
 *
 *  \param [in] advertising_data
 *         Periodic advertising data
 *
 *  \return
 *       API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_periodic_advertising_data
           (
               UCHAR   advertising_handle,
               UCHAR   operation,
               UCHAR   advertising_data_length,
               UCHAR * advertising_data
           );
#endif /* HCI_LE_SET_PERIODIC_ADVERTISING_DATA_SUPPORT */

#ifdef HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE_SUPPORT
/**
 *  \brief To enable or disable periodic advertising
 *
 *  \par Description:
 *       This api is used to request the Controller to enable or disable
 *       the periodic advertising for the advertising set specified by
 *       the Advertising_Handle parameter (ordinary advertising is not
 *       affected).
 *
 *  \param [in] enable
 *         0x00    Periodic advertising is disabled (default)
 *         0x01    Periodic advertising is enabled
 *
 *  \param [in] advertising_handle
 *         0x00 - 0xEF Used to identify an advertising set
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_periodic_advertising_enable
           (
               UCHAR   enable,
               UCHAR   advertising_handle
           );
#endif /* HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE_SUPPORT */

#ifdef HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_SUPPORT
/**
 *  \brief To set the extended scan parameters
 *
 *  \par Description:
 *       This API is used to set the extended scan parameters to be used
 *       on the primary advertising channels.
 *
 *  \param [in] own_address_type
 *         0x00    Public Device Address
 *         0x01    Random Device Address
 *         0x02    Controller generates Resolvable Private Address based on the local
 *                 IRK from resolving list. If resolving list contains no matching entry,
 *                 use public address.
 *         0x03    Controller generates Resolvable Private Address based on the local IRK
 *                 from resolving list. If resolving list contains no matching entry, use
 *                 random address from LE_Set_Random_Address.
 *
 *  \param [in] scanning_filter_policy
 *         0x00    Accept all
 *                 - advertisement packets except directed advertising packets not
 *                   addressed to this device
 *         0x01    Accept only
 *                 - advertisement packets from devices where the advertiser's address
 *                   is in the White list.
 *                 - Directed advertising packets which are not addressed for this device
 *                   shall be ignored.
 *         0x02    Accept all
 *                 - advertising packets except directed advertising packets where
 *                   the initiator's identity address does not address this device.
 *                 - Note: directed advertising packets where the initiator's address
 *                   is a resolvable private address that cannot be resolved are also accepted.
 *         0x03    Accept all advertising packets except:
 *                 - advertising packets where the advertiser's identity address
 *                   is not in the White List; and
 *                 - directed advertising packets where the initiator's identity
 *                   address does not address this device
 *                 - Note: directed advertising packets where the initiator's
 *                   address is a resolvable private address that cannot be
 *                   resolved are also accepted
 *
 *  \param [in] scanning_phy
 *         Scanning PHYs
 *         Bit number     Parameter Description
 *         ----------     ---------------------
 *         0              Scan advertisements on the LE 1M PHY
 *         2              Scan advertisements on the LE Coded PHY
 *
 *  \param [in] scan_type
 *         0x00    Passive Scanning. No SCAN_REQ PDUs shall be sent.
 *         0x01    Active Scanning. SCAN_REQ PDUs may be sent.
 *
 *  \param [in] scan_interval
 *         This is defined as the time interval from when the Controller started
 *         its last LE scan until it begins the subsequent scan on the primary
 *         advertising channel.
 *         Range: 0x0004 to 0x4000
 *         Time = N * 0.625 msec
 *         Time Range: 2.5 ms to 40.959375 seconds
 *
 *  \param [in] scan_window
 *         The duration of the scan on the primary advertising channel.
 *         LE_Scan_Window shall be less than or equal to LE_Scan_Interval
 *         Range: 0x0004 to 0x4000
 *         Time = N * 0.625 msec
 *         Time Range: 2.5 ms to 40.959375 seconds
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_extended_scan_parameters
           (
               UCHAR    own_address_type,
               UCHAR    scanning_filter_policy,
               UCHAR    scanning_phy,
               UCHAR  * scan_type,
               UINT16 * scan_interval,
               UINT16 * scan_window
           );
#endif /* HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_SUPPORT */

#ifdef HCI_LE_SET_EXTENDED_SCAN_ENABLE_SUPPORT
/**
 *  \brief Used to enable or disable scanning
 *
 *  \par Description:
 *       This API is used to enable or disable scanning.  The Enable parameter
 *       determines whether scanning is enabled or disabled. If it is disabled,
 *       the remaining parameters are ignored.
 *
 *  \param [in] enable
 *         0x00    Scanning disabled
 *         0x01    Scanning enabled
 *
 *  \param [in] filter_duplicates
 *         0x00    Duplicate filtering disabled
 *         0x01    Duplicate filtering enabled
 *         0x02    Duplicate filtering enabled, reset for each scan period
 *
 *  \param [in] duration
 *         0x0000 Scan continuously until explicitly disable
 *         0xXXXX Scan duration.
 *                Range: 0x0001 - 0xFFFF
 *                Time = N * 10 msec
 *                Time Range: 10 ms to 655.35 s
 *
 *  \param [in] period
 *         0x0000 Periodic scanning disabled
 *         N = 0xXXXX Time interval from when the Controller started its last Scan_Duration
 *                    until it begins the subsequent Scan_Duration.
 *         Range: 0x0001 - 0xFFFF
 *         Time = N * 1.28 sec
 *         Time Range: 1.28 s to 83,884.8 s
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_extended_scan_enable
           (
               UCHAR   enable,
               UCHAR   filter_duplicates,
               UINT16  duration,
               UINT16  period
           );
#endif /* HCI_LE_SET_EXTENDED_SCAN_ENABLE_SUPPORT */

#ifdef HCI_LE_EXTENDED_CREATE_CONNECTION_SUPPORT
/**
 *  \brief To create a Link Layer connection to a connectable advertiser.
 *
 *  \par Description:
 *      This api is used to create a Link Layer connection to a connectable
 *      advertiser. LE_Extended_Create_Connection command can be used
 *      in place of LE_Create_Connection command.
 *
 *  \param [in] initiator_filter_policy
 *         0x00  White list is not used to determine which advertiser to connect to.
 *               Peer_Address_Type and Peer_Address shall be used.
 *         0x01  White list is used to determine which advertiser to connect to.
 *               Peer_Address_Type and Peer_Address shall be ignored.
 *
 *  \param [in] own_address_type
 *         0x00 Public Device Address
 *         0x01 Random Device Address
 *         0x02 Controller generates the Resolvable Private Address based on the local
 *              IRK from the resolving list. If the resolving list contains no matching
 *              entry, then use the public address.
 *         0x03 Controller generates the Resolvable Private Address based on the local
 *              IRK from the resolving list. If the resolving list contains no matching
 *              entry, then use the random address from the most recent successful
 *              LE_Set_Random_Address Command.
 *
 *  \param [in] peer_address_type
 *         0x00 Public Device Address or Public Identity Address
 *         0x01 Random Device Address or Random (static) Identity Address
 *
 *  \param [in] peer_address
 *          Public Device Address, Random Device Address, Public Identity Address,
 *          or Random (static) Identity Address of the device to be connected.
 *
 *  \param [in] initiating_phys
 *         Bit number    Parameter Description
 *         ----------    ---------------------
 *         0             Scan connectable advertisements on the LE 1M PHY.
 *                       Connection parameters for the LE 1M PHY are provided.
 *
 *         1             Connection parameters for the LE 2M PHY are provided.
 *
 *         2             Scan connectable advertisements on the LE Coded PHY.
 *                       Connection parameters for the LE Coded PHY are provided.
 *
 *  \param [in] scan_interval
 *         Time interval from when the Controller started its last scan until
 *         it begins the subsequent scan on the primary advertising channel
 *         Range: 0x0004 to 0xFFFF
 *         Time = N * 0.625 ms
 *         Time Range: 2.5 ms to 40.959375 s
 *
 *  \param [in] scan_window
 *         Duration of the scan on the primary advertising channel.
 *         Range: 0x0004 to 0xFFFF
 *         Time = N * 0.625 ms
 *         Time Range: 2.5 ms to 40.959375 s
 *
 *  \param [in] conn_interval_min
 *         Minimum value for the connection interval. This shall be less
 *         than or equal to Conn_Interval_Max.
 *         Range: 0x0006 to 0x0C80
 *         Time = N * 1.25 msec
 *         Time Range: 7.5 msec to 4 s
 *
 *  \param [in] conn_interval_max
 *         Maximum value for the connection interval. This shall be greater
 *         than or equal to Conn_Interval_Min.
 *         Range: 0x0006 to 0x0C80
 *         Time = N * 1.25 msec
 *         Time Range: 7.5 msec to 4 s
 *
 *  \param [in] conn_latency
 *         Slave latency for the connection in number of connection events.
 *         Range: 0x0000 to 0x01F3
 *
 *  \param [in] supervision_timeout
 *         Supervision timeout for the LE Link.
 *         Range: 0x000A to 0x0C80
 *         Time = N * 10 msec
 *         Time Range: 100 msec to 32 s
 *
 *  \param [in] minimum_ce_length
 *         Informative parameter recommending the minimum length of connection
 *         event needed for this LE connection.
 *         Range: 0x0000 - 0xFFFF
 *         Time = N * 0.625 msec
 *
 *  \param [in] maximum_ce_length
 *         Informative parameter recommending the maximum length of connection
 *         event needed for this LE connection.
 *         Range: 0x0000 - 0xFFFF
 *         Time = N * 0.625 msec
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_extended_create_connection
           (
                UCHAR    initiator_filter_policy,
                UCHAR    own_address_type,
                UCHAR    peer_address_type,
                UCHAR  * peer_address,
                UCHAR    initiating_phys,
                UINT16 * scan_interval,
                UINT16 * scan_window,
                UINT16 * conn_interval_min,
                UINT16 * conn_interval_max,
                UINT16 * conn_latency,
                UINT16 * supervision_timeout,
                UINT16 * minimum_ce_length,
                UINT16 * maximum_ce_length
           );
#endif /* HCI_LE_EXTENDED_CREATE_CONNECTION_SUPPORT */

#ifdef HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_SUPPORT
/**
 *  \brief To control the reception of Periodic Advertising packets
 *
 *  \par Description:
 *       This API is used to synchronize with periodic advertising from
 *       an advertiser and begin receiving periodic advertising packets.
 *
 *       This command may be issued whether or not scanning is enabled and
 *       scanning may be enabled and disabled (see the LE Set Extended Scan Enable
 *       Command) while this command is pending. However, synchronization can only
 *       occur when scanning is enabled. While scanning is disabled, no attempt to
 *       synchronize will take place.
 *
 *  \param [in] filter_policy
 *         0x00    Use the Advertising_SID, Advertising_Address_Type, and
 *                 Advertising_Address parameters to determine which advertiser
 *                 to listen to.
 *         0x01    Use the Periodic Advertiser List to determine which advertiser
 *                 to listen to.
 *
 *  \param [in] advertising_sid
 *         0x00 - 0x0F  Advertising SID subfield in the ADI field used to
 *                      identify the Periodic Advertising
 *
 *  \param [in] adv_address_type
 *         0x00 Public Device Address
 *         0x01 Random Device Address
 *
 *  \param [in] adv_address
 *         Public Device Address, Random Device Address, Public Identity Address,
 *         or Random (static) Identity Address of the advertiser.
 *
 *  \param [in] skip
 *         The number of periodic advertising packets that can be skipped
 *         after a successful receive.
 *         Range: 0x0000 to 0x1F3
 *
 *  \param [in] sync_timeout
 *         Supervision timeout for the periodic advertising.
 *         Range: 0x000A to 0x4000
 *         Time = N*10 msec
 *         Time Range: 100 ms to 163.84 seconds
 *
 *  \param [in] unused
 *         This value must be used by the Host.
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_periodic_adv_create_sync
           (
               UCHAR   filter_policy,
               UCHAR   advertising_sid,
               UCHAR   adv_address_type,
               UCHAR * adv_address,
               UINT16  skip,
               UINT16  sync_timeout,
               UCHAR   unused
           );
#endif /* HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_SUPPORT */

#ifdef HCI_LE_PERIODIC_ADV_CREATE_SYNC_CANCEL_SUPPORT
/**
 *  \brief To cancel the LE_Periodic_Advertising_Create_Sync command
 *
 *  \par Description:
 *      This api is used to cancel the LE_Periodic_Advertising_Create_Sync
 *      command while it is pending.
 *
 *      If the Host issues this command while no
 *      LE_Periodic_Advertising_Create_Sync command is pending, the
 *      Controller shall return the error code Command Disallowed (0x0C).
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_periodic_advertising_create_sync_cancel() \
        hci_common_api_handler_no_params \
       (HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_CANCEL_OPCODE)
#endif /* HCI_LE_PERIODIC_ADV_CREATE_SYNC_CANCEL_SUPPORT */

#ifdef HCI_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC_SUPPORT
/**
 *  \brief To stop reception of periodic advertising
 *
 *  \par Description:
 *      This api is used to stop reception of the periodic advertising
 *      identified by the Sync_Handle parameter.
 *
 *  \param [in] sync_handle
 *         Sync_Handle to be used to identify the periodic advertiser.
 *         Range: 0x0000-0x0EFF
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_periodic_adv_terminate_sync
           (
                UINT16  sync_handle
           );
#endif /* HCI_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC_SUPPORT */

#ifdef HCI_LE_ADD_DEV_TO_PERIODIC_ADVSER_LIST_SUPPORT
/**
 *  \brief To add a single device to the Periodic Advertiser list
 *
 *  \par Description:
 *       This API is used to add a single device to the Periodic Advertiser
 *       list stored in the Controller. Any additions to the Periodic
 *       Advertiser list take effect immediately. If the device is already
 *       on the list, the Controller shall return the error code Invalid HCI
 *       Command Parameters (0x12).
 *
 *       If the Host issues this command when an
 *       LE_Periodic_Advertising_Create_Sync command is pending, the Controller
 *       shall return the error code Command Disallowed (0x0C).
 *
 *       When a Controller cannot add a device to the Periodic Advertiser list
 *       because the list is full, the Controller shall return the error code
 *       Memory Capacity Exceeded (0x07).
 *
 *  \param [in] advertiser_address_type
 *         0x00    Public Device Address or Public Identity Address
 *         0x01    Random Device Address or Random (static) Identity Address
 *
 *  \param [in] advertiser_address
 *         Public Device Address, Random Device Address, Public Identity Address,
 *         or Random (static) Identity Address of the advertiser.
 *
 *  \param [in] advertising_sid
 *         0x00-0x0F Advertising SID subfield in the ADI field used to identify
 *         the Periodic Advertising
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_add_device_to_periodic_advertiser_list
           (
               UCHAR   advertiser_address_type,
               UCHAR * advertiser_address,
               UCHAR   advertising_sid
           );
#endif /* HCI_LE_ADD_DEV_TO_PERIOD_ADVSER_LIST_SUPPORT */

#ifdef HCI_LE_RM_DEV_FROM_PERIODIC_ADVSER_LIST_SUPPORT
/**
 *  \brief To remove a single device to the Periodic Advertiser list
 *
 *  \par Description:
 *       This API is used to remove one device from the list of Periodic
 *       Advertisers stored in the Controller. Removals from the Periodic
 *       Advertisers List take effect immediately.
 *
 *  \param [in] advertiser_address_type
 *         0x00    Public Device Address or Public Identity Address
 *         0x01    Random Device Address or Random (static) Identity Address
 *
 *  \param [in] advertiser_address
 *         Public Device Address, Random Device Address, Public Identity Address,
 *         or Random (static) Identity Address of the advertiser.
 *
 *  \param [in] advertising_sid
 *         0x00-0x0F Advertising SID subfield in the ADI field used to identify
 *         the Periodic Advertising
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_remove_device_from_periodic_advertiser_list
           (
               UCHAR   advertiser_address_type,
               UCHAR * advertiser_address,
               UCHAR   advertising_sid
           );
#endif /* HCI_LE_RM_DEV_FROM_PERIODIC_ADVSER_LIST_SUPPORT */

#ifdef HCI_LE_CLEAR_PERIODIC_ADVSER_LIST_SUPPORT
/**
 *  \brief To remove all devices from the list of Periodic Advertisers
 *  in the Controller.
 *
 *  \par Description:
 *       This API is used to remove all devices from the list of Periodic
 *       Advertisers in the Controller.
 *
 *       If this command is used when an LE_Periodic_Advertising_Create_Sync
 *       command is pending, the Controller shall return the error code Command
 *       Disallowed (0x0C).
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_clear_periodic_advertiser_list() \
        hci_common_api_handler_no_params \
        (HCI_LE_CLEAR_PERIODIC_ADVERTISER_LIST_OPCODE)
#endif /* HCI_LE_CLEAR_PERIODIC_ADVSER_LIST_SUPPORT */

#ifdef HCI_LE_READ_PERIODIC_ADVSER_LIST_SIZE_SUPPORT
/**
 *  \brief To read the total number of Periodic Advertiser list entries that can be
 *  stored in the Controller
 *
 *  \par Description:
 *       This API is used to read the total number of Periodic Advertiser list entries
 *       that can be stored in the Controller
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 *
 *  \note
 *  The number of entries that can be stored is not fixed and the Controller
 *  can change it at any time (e.g., because the memory used to store the list
 *  can also be used for other purposes).
 */
#define BT_hci_le_read_periodic_advertiser_list_size() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_PERIODIC_ADVERTISER_LIST_SIZE_OPCODE)
#endif /* HCI_LE_READ_PERIODIC_ADVSER_LIST_SIZE_SUPPORT */

#ifdef HCI_LE_READ_TRANSMIT_POWER_SUPPORT
/**
 *  \brief To read the minimum and maximum transmit powers.
 *
 *  \par Description:
 *       This API is used read the minimum and maximum transmit powers supported by
 *       the Controller
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_transmit_power() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_TRANSMIT_POWER_OPCODE)
#endif /* HCI_LE_READ_TRANSMIT_POWER_SUPPORT */

#ifdef HCI_LE_READ_RF_PATH_COMPENSATION_SUPPORT
/**
 *  \brief To read the RF Path Compensation Value.
 *
 *  \par Description:
 *       This API is used to read the RF Path Compensation Values parameter used
 *       in the Tx Power Level and RSSI calculation.
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
#define BT_hci_le_read_rf_path_compensation() \
        hci_common_api_handler_no_params \
        (HCI_LE_READ_RF_PATH_COMPENSATION_OPCODE)
#endif /* HCI_LE_READ_RF_PATH_COMPENSATION_SUPPORT */

#ifdef HCI_LE_WRITE_RF_PATH_COMPENSATION_SUPPORT
/**
 *  \brief To indicate the RF path gain or loss
 *
 *  \par Description:
 *       This API is used to indicate the RF path gain or loss between
 *       the RF transceiver and the antenna contributed by intermediate
 *       components. A positive value means a net RF path gain and a
 *       negative value means a net RF path loss. The RF Tx Path Compensation
 *       Value parameter shall be used by the Controller to calculate radiative
 *       Tx Power Level used in the TxPower field in the Extended Header using
 *       the following equation:
 *
 *       Radiative Tx Power Level = Tx Power Level at RF transceiver output +
 *                                  RF Tx Path Compensation Value
 *
 *       For example, if the Tx Power Level is +4 (dBm) at RF transceiver output
 *       and the RF Path Compensation Value is -1.5 (dB), the radiative Tx Power
 *       Level is +4+(-1.5) = 2.5 (dBm).
 *
 *       The RF Rx Path Compensation Value parameter shall be used by the
 *       Controller to calculate the RSSI value reported to the Host.
 *
 *  \param [in] rf_tx_path_comp_value
 *         RF tx Path compensation Value.
 *         Range: -128.0 dB (0xFB00) = N = 128.0 dB (0x0500)
 *         Units: 0.1dB
 *
  * \param [in] rf_rx_path_comp_value
 *         RF rx Path compensation Value.
 *         Range: -128.0 dB (0xFB00) = N = 128.0 dB (0x0500)
 *         Units: 0.1dB
 *
 *  \return
 *      API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_write_rf_path_compensation
           (
               UINT16   rf_tx_path_comp_value,
               UINT16   rf_rx_path_comp_value
           );
#endif /* HCI_LE_WRITE_RF_PATH_COMPENSATION_SUPPORT */

#ifdef HCI_LE_SET_PRIVACY_MODE_SUPPORT
/**
 *  \brief To set the privacy mode.
 *
 *  \par Description:
 *       This API is used to allow the Host to specify the privacy mode
 *       to be used for a given entry on the resolving list.
 *
 *  \param [in] peer_identity_addr_type
 *         Address type of the peer Bluetooth device identity address
 *         - Public Device or Public Identity Address:
 *           \ref BT_BD_PUBLIC_ADDRESS_TYPE (0x00)
 *         - Random Device or Random (static) Identity Address:
 *           \ref BT_BD_RANDOM_ADDRESS_TYPE (0x01)
 *
 *  \param [in] peer_identity_addr
 *         Public Identity Address or Random (static) Identity Address
 *         of the advertiser
 *
 *  \param [in] privacy_mode
 *         - 0x00: Use Network Privacy Mode for this peer device (default)
 *         - 0x01: Use Device Privacy Mode for this peer device
 *
 *  \return API_RESULT
 *     API_SUCCESS or one of the error codes as defined in \ref BLE_ERROR_CODES.
 */
API_RESULT BT_hci_le_set_privacy_mode
           (
                UCHAR    peer_identity_addr_type,
                UCHAR *  peer_identity_addr,
                UCHAR    privacy_mode
           );
#endif /* HCI_LE_SET_PRIVACY_MODE_SUPPORT */


/* ------------------------------------------------------------------------- */
/* New APIs from Bluetooth Spec 2.1+EDR ------------------------------------ */
/* ------------------------------------------------------------------------- */

/* HCI Vendor Specific Commands APIs --------------------------------------- */

/**
 *  \brief To send a vendor specific HCI command to the local Bluetooth device.
 *
 *  \par Description:
 *       This API can be used to send vendor specific HCI commands to the local
 *       Bluetooth device. The HCI Opcode and parameters are specific to
 *       the vendor specific command. Refer to the documentation of
 *       the Bluetooth device to find out the details of available vendor
 *       specific commands.
 *
 *  \param [in] ocf
 *         Opcode Command Field for the vendor specific Opcode.
 *
 *  \param [in] params
 *         Parameters to be framed for the vendor specific HCI command.
 *
 *  \param [in] params_length
 *         Total length of the parameters passed.
 *
 *  \return
 *      API_RESULT: API_SUCCESS or one of the error codes as defined in
 *                  BT_error.h or Host Controller Error Codes section.
 */
API_RESULT BT_hci_vendor_specific_command
           (
               UINT16   ocf,
               UCHAR *  params,
               UCHAR    params_length
           );

/** \} */

/* ------------------------------------------------------------------------- */


#ifdef __cplusplus
};
#endif

/* \} */

#endif /* _H_BT_HCI_API_ */

