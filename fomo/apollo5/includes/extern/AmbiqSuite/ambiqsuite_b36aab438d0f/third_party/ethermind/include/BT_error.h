
/**
 *  \file BT_error.h
 *
 *  \brief This file lists all the Error Codes returned by BlueLitE Stack API.
 */

/*
 *  Copyright (C) 2014. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_ERROR_
#define _H_BT_ERROR_

/**
 * \defgroup BLE_ERROR_CODES BlueLitE Stack & Profile Error Codes
 * \{
 *  This section contains all the error codes defined for BlueLitE stack
 *  and profiles.
 *  <p>
 *  \anchor error_code_overview
 *  <b>Theory:</b>
 *  <p>
 *    Every API under BlueLitE Protocol Suite returns \ref API_RESULT,
 *    which is a 2 Byte Unsigned Short variable. The higher order byte
 *    signifies the Module from which the Error has been generated, and
 *    the lower order byte encodes the actual reason of Error.
 *    <p>
 *    Each module under BlueLitE Stack is given unique
 *    Error ID (the higher order byte). Also, for each module the Error
 *    Code (the lower order byte) signifies an unique error situation.
 *    <p>
 *    For BlueLitE Protocol Modules (eg, HCI, RFCOMM etc.), Error IDs are
 *    assigned from the range 0x10 to 0x4F. For Profiles, the range
 *    is from 0x50 to 0x7F.
 *    <p>
 *    The definition of \ref API_SUCCESS is 0x0000 - which is the 'Success'
 *    return value for an API returning \ref API_RESULT. All other values for
 *    should be treated as Errors.
 *    <p>
 *    The definition of \ref API_FAILURE is 0xFFFF - which stands for
 *    "Unknown Error Situation".
 *    <p>
 *  <b>Note:</b>
 *    <p>
 *    The applications using native/profile BlueLitE API should NOT do
 *    any check on the basis of \ref API_FAILURE - rather, the correct way to
 *    detect an error situation is by doing a negative check on \ref
 *    API_SUCCESS.
 *    <p>
 *    For example,
 *    \code if ( BT_hci_inquiry(x, y, z) == API_FAILURE ) \endcode
 *    ... Wrong !
 *    <p>
 *    <p>
 *    \code if ( BT_hci_inquiry(x, y, z) != API_SUCCESS ) \endcode
 *    ... Correct !
 */

/**
 * \defgroup ble_error_codes_defines Defines
 * \{
 */

/* Definition of API_RESULT */

/**
 * \addtogroup bt_defines BlueLite Defines
 * @{ */
/**
 * \addtogroup feature_macros Feature Macros
 *
 * @{ */
 /**
 * \addtogroup Host_Stack_Macros
 *
 * @{ */
 /**
 * \addtogroup Misc
 *
 * @{ */

#ifndef API_RESULT_DEFINED
typedef UINT16    API_RESULT;
/** Multiple definition protection guard for API_RESULT */
#define API_RESULT_DEFINED
#endif /* API_RESULT_DEFINED */

/** @} end: Misc */
/** @} end: Host_Stack_Macros */
/** @} end: feature_macros */
/** @} end: bt_defines */

/* Definitions of API_SUCCESS & API_FAILURE */
#ifdef API_SUCCESS
#undef API_SUCCESS
#endif /* API_SUCCESS */
#define API_SUCCESS             0x0000

#ifdef API_FAILURE
#undef API_FAILURE
#endif /* API_FAILURE */
#define API_FAILURE             0xFFFF

/** \} */

/* ====================== EtherMind Module Error IDs ====================== */

/**
 * \defgroup btle_error_codes_groups Error Grouping
 * \{
 */

/**
 * \defgroup btle_error_codes_groups_std Specification Error Codes (0x00 - 0x0F)
 * Error IDs for Bluetooth Specification Defined Error Codes (0x00 - 0x0F).
 * \{
 */

/** Error Codes for Host Controller - \ref btle_error_codes_module_hc */
#define HC_ERR_ID                               0x0000
/** \cond ignore */
#define ATT_SPEC_ERR_ID                         0x0300
#define SM_LE_ERR_ID                            0x0400
/** \endcond */

/** \} */

/**
 * \defgroup btle_error_codes_groups_core Core Modules (0x10 - 0x1F)
 * Error IDs for BlueLitE Core Modules (0x10 - 0x1F).
 * \{
 */

/** Error Codes for BT Common - \ref btle_error_codes_module_btcommon */
#define BT_COMMON_ERR_ID                        0x1000
/** Error Codes for Write Task - \ref btle_error_codes_module_wt */
#define WT_ERR_ID                               0x1100
/** Error Codes for Timer - \ref btle_error_codes_module_timer */
#define TIMER_ERR_ID                            0x1200
/** Error Codes for State Machine - \ref btle_error_codes_module_utils */
#define FSM_ERR_ID                              0x1300
/** Error Codes for Device Queue - \ref btle_error_codes_module_utils */
#define DQ_ERR_ID                               0x1400
/** Error Codes for RACP - \ref btle_error_codes_module_utils */
#define RACP_ERR_ID                             0x1500

/** \} */

/**
 * \defgroup btle_error_codes_groups_protocols Protocols (0x20 - 0x3F)
 * Error IDs for BlueLitE Protocol Modules (0x20 - 0x3F).
 * \{
 */

/** Error Codes for HCI - \ref btle_error_codes_module_hci */
#define HCI_ERR_ID                              0x2000
/** \cond ignore */
#define SM_ERR_ID                               0x2100
/** \endcond */
/** Error Codes for L2CAP - \ref btle_error_codes_module_l2cap */
#define L2CAP_ERR_ID                            0x2300
/**
 * Error Codes for Security Manager Protocol -
 * \ref btle_error_codes_module_smp
 */
#define SMP_ERR_ID                              0x2E00
/** Error Codes for Attribute Protocol - \ref btle_error_codes_module_att */
#define ATT_ERR_ID                              0x2F00

/** \} */

/**
 * \defgroup btle_error_codes_groups_profiles Profiles (0x50 - 0x7F)
 * Error IDs for BlueLitE Profile Modules (0x50 - 0x7F).
 * \{
 */

/** Error Codes for GATT DB - \ref btle_error_codes_module_gatt_db */
#define GATT_DB_ERR_ID                          0xC400
/** \cond ignore */
/** Error Codes for GAP - \ref btle_error_codes_module_gap */
#define GAP_ERR_ID                              0x5000
/** \endcond */

/** \} */

/**
 * \defgroup btle_error_codes_groups_utility Utility Modules (0xA0 - 0xAF)
 * Error IDs for EtherMind Utility/Tools Modules (0xA0 - 0xAF).
 * \{
 */

/** \cond ignore */
#define BT_FOPS_ERR_ID                          0xA000
/** \endcond */
/** Error Codes for Persistant Storage - \ref btle_error_codes_module_utils */
#define STORAGE_ERR_ID                          0xA100

/** \} */

/** \} */

/* ================== EtherMind Common Reason Error Codes ================= */

/**
 * \defgroup btle_error_codes_module_common Common
 * \{
 */

#define MUTEX_INIT_FAILED                       0x0001
#define COND_INIT_FAILED                        0x0002
#define MUTEX_LOCK_FAILED                       0x0003
#define MUTEX_UNLOCK_FAILED                     0x0004
#define MEMORY_ALLOCATION_FAILED                0x0005

/** \} */

/* ===================== EtherMind Module Error Codes ===================== */

/* ======================================= Section 'Host Controller (LMP)' */
/**
 * \defgroup btle_error_codes_module_hc Host Controller (LMP)
 * \{
 */

#define HC_UNKNOWN_HCI_COMMAND                  (0x0001 | HC_ERR_ID)
#define HC_NO_CONNECTION                        (0x0002 | HC_ERR_ID)
#define HC_HARDWARE_FAILURE                     (0x0003 | HC_ERR_ID)
#define HC_PAGE_TIMEOUT                         (0x0004 | HC_ERR_ID)
#define HC_AUTHENTICATION_FAILURE               (0x0005 | HC_ERR_ID)
#define HC_KEY_MISSING                          (0x0006 | HC_ERR_ID)
#define HC_MEMORY_FULL                          (0x0007 | HC_ERR_ID)
#define HC_CONNECTION_TIMEOUT                   (0x0008 | HC_ERR_ID)
#define HC_MAX_NUMBER_OF_CONNECTIONS            (0x0009 | HC_ERR_ID)
#define HC_MAX_NUMBER_OF_SCO_CONNECTIONS        (0x000A | HC_ERR_ID)
#define HC_ACL_CONNECTION_ALREADY_EXISTS        (0x000B | HC_ERR_ID)
#define HC_COMMAND_DISALLOWED                   (0x000C | HC_ERR_ID)
#define HC_HOST_REJECTED_LIMITED_RESOURCES      (0x000D | HC_ERR_ID)
#define HC_HOST_REJECTED_SECURITY_REASONS       (0x000E | HC_ERR_ID)
#define HC_HOST_REJECTED_PERSONAL_DEVICE        (0x000F | HC_ERR_ID)
#define HC_HOST_TIMEOUT                         (0x0010 | HC_ERR_ID)
#define HC_UNSUPPORTED_FEATURE_OR_PARAMETER     (0x0011 | HC_ERR_ID)
#define HC_INVALID_HCI_COMMAND_PARAMETERS       (0x0012 | HC_ERR_ID)
#define HC_OTHER_END_TERMINATED_USER            (0x0013 | HC_ERR_ID)
#define HC_OTHER_END_TERMINATED_LOW_RESOURCES   (0x0014 | HC_ERR_ID)
#define HC_OTHER_END_TERMINATED_POWER_OFF       (0x0015 | HC_ERR_ID)
#define HC_CONNECTION_TERMINATED_BY_LOCAL_HOST  (0x0016 | HC_ERR_ID)
#define HC_REPEATED_ATTEMPTS                    (0x0017 | HC_ERR_ID)
#define HC_PAIRING_NOT_ALLOWED                  (0x0018 | HC_ERR_ID)
#define HC_UNKNOWN_LMP_PDU                      (0x0019 | HC_ERR_ID)
#define HC_UNSUPPORTED_REMOTE_FEATURE           (0x001A | HC_ERR_ID)
#define HC_SCO_OFFSET_REJECTED                  (0x001B | HC_ERR_ID)
#define HC_SCO_INTERVAL_REJECTED                (0x001C | HC_ERR_ID)
#define HC_SCO_AIR_MODE_REJECTED                (0x001D | HC_ERR_ID)
#define HC_INVALID_LMP_PARAMETERS               (0x001E | HC_ERR_ID)
#define HC_UNSPECIFIED_ERROR                    (0x001F | HC_ERR_ID)
#define HC_UNSUPPORTED_LMP_PARAMETER_VALUE      (0x0020 | HC_ERR_ID)
#define HC_ROLE_CHANGE_NOT_ALLOWED              (0x0021 | HC_ERR_ID)
#define HC_LMP_RESPONSE_TIMEOUT                 (0x0022 | HC_ERR_ID)
#define HC_LMP_ERROR_TRANSACTION_COLLISION      (0x0023 | HC_ERR_ID)
#define HC_LMP_PDU_NOT_ALLOWED                  (0x0024 | HC_ERR_ID)
#define HC_ENCRYPTION_MODE_NOT_ACCEPTABLE       (0x0025 | HC_ERR_ID)
#define HC_UNIT_KEY_USED                        (0x0026 | HC_ERR_ID)
#define HC_QOS_IS_NOT_SUPPORTED                 (0x0027 | HC_ERR_ID)
#define HC_INSTANT_PASSED                       (0x0028 | HC_ERR_ID)
#define HC_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED  (0x0029 | HC_ERR_ID)
#define HC_DIFFERENT_TRANSACTION_COLLISION      (0x002A | HC_ERR_ID)
#define HC_QOS_UNACCEPTABLE_PARAMETER           (0x002C | HC_ERR_ID)
#define HC_QOS_REJECTED                         (0x002D | HC_ERR_ID)
#define HC_CHANNEL_CLASSIFICATION_NOT_SUPPORTED (0x002E | HC_ERR_ID)
#define HC_INSUFFICIENT_SECURITY                (0x002F | HC_ERR_ID)
#define HC_PARAMETER_OUT_OF_MANDATORY_RANGE     (0x0030 | HC_ERR_ID)
#define HC_ROLE_SWITCH_PENDING                  (0x0032 | HC_ERR_ID)
#define HC_RESERVED_SLOT_VIOLATION              (0x0034 | HC_ERR_ID)
#define HC_ROLE_SWITCH_FAILED                   (0x0035 | HC_ERR_ID)
#define HC_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE  (0x0036 | HC_ERR_ID)
#define HC_SECURE_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST \
                                                (0x0037 | HC_ERR_ID)
#define HC_HOST_BUSY_PAIRING                    (0x0038 | HC_ERR_ID)
#define HC_CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND \
                                                (0x0039 | HC_ERR_ID)
#define HC_CONTROLLER_BUSY                      (0x003A | HC_ERR_ID)
#define HC_UNACCEPTEBALE_CONNECTION_INTERVAL    (0x003B | HC_ERR_ID)
#define HC_ADVERTISING_TIMEOUT                  (0x003C | HC_ERR_ID)
#define HC_CONNECTION_TREMINATED_DUE_TO_MIC_FAILURE \
                                                (0x003D | HC_ERR_ID)
#define HC_CONNECTION_FAILED_TO_BE_ESTABLISHED  (0x003E | HC_ERR_ID)
#define HC_MAC_CONNECTION_FAILED                (0x003F | HC_ERR_ID)

/** \} */

/* ======================================= Section 'BT Common' */
/**
 * \defgroup btle_error_codes_module_btcommon BT Common
 * \{
 */

#define BLUETOOTH_ON_ALREADY_INITIALIZED        (0x0011 | BT_COMMON_ERR_ID)
#define BLUETOOTH_ON_NOT_COMPLETED              (0x0012 | BT_COMMON_ERR_ID)
#define BLUETOOTH_INVALID_PARAMETER_VALUE       (0x0013 | BT_COMMON_ERR_ID)

/** \} */

/* ======================================= Section 'Write Task' */

/**
 * \defgroup btle_error_codes_module_wt Write Task
 * \{
 */

#define WT_MUTEX_INIT_FAILED                    \
        (MUTEX_INIT_FAILED | WT_ERR_ID)
#define WT_COND_INIT_FAILED                     \
        (COND_INIT_FAILED | WT_ERR_ID)
#define WT_MUTEX_LOCK_FAILED                    \
        (MUTEX_LOCK_FAILED | WT_ERR_ID)
#define WT_MUTEX_UNLOCK_FAILED                  \
        (MUTEX_UNLOCK_FAILED | WT_ERR_ID)
#define WT_MEMORY_ALLOCATION_FAILED             \
        (MEMORY_ALLOCATION_FAILED | WT_ERR_ID)

#define WT_NOT_RUNNING                          (0x0011 | WT_ERR_ID)
#define WT_QUEUE_FULL                           (0x0012 | WT_ERR_ID)

/** \} */

/* ======================================= Section 'Timer' */
/**
 * \defgroup btle_error_codes_module_timer Timer
 * \{
 */

#define TIMER_MUTEX_INIT_FAILED                 \
        (MUTEX_INIT_FAILED | TIMER_ERR_ID)
#define TIMER_COND_INIT_FAILED                  \
        (COND_INIT_FAILED | TIMER_ERR_ID)
#define TIMER_MUTEX_LOCK_FAILED                 \
        (MUTEX_LOCK_FAILED | TIMER_ERR_ID)
#define TIMER_MUTEX_UNLOCK_FAILED               \
        (MUTEX_UNLOCK_FAILED | TIMER_ERR_ID)
#define TIMER_MEMORY_ALLOCATION_FAILED          \
        (MEMORY_ALLOCATION_FAILED | TIMER_ERR_ID)

#define TIMER_HANDLE_IS_NULL                    (0x0011 | TIMER_ERR_ID)
#define TIMER_CALLBACK_IS_NULL                  (0x0012 | TIMER_ERR_ID)
#define TIMER_QUEUE_EMPTY                       (0x0013 | TIMER_ERR_ID)
#define TIMER_QUEUE_FULL                        (0x0014 | TIMER_ERR_ID)
#define TIMER_ENTITY_SEARCH_FAILED              (0x0015 | TIMER_ERR_ID)
#define TIMER_NULL_PARAMETER_NOT_ALLOWED        (0x0016 | TIMER_ERR_ID)
#define TIMER_TIMEOUT_ZERO_NOT_ALLOWED          (0x0017 | TIMER_ERR_ID)

/** \} */

/* ======================================= Section 'RACP' */
/**
* \defgroup btle_error_codes_module_utils RACP
* \{
*/

#define RACP_MUTEX_INIT_FAILED                    \
        (MUTEX_INIT_FAILED | RACP_ERR_ID)
#define RACP_COND_INIT_FAILED                     \
        (COND_INIT_FAILED | RACP_ERR_ID)
#define RACP_MUTEX_LOCK_FAILED                    \
        (MUTEX_LOCK_FAILED | RACP_ERR_ID)
#define RACP_MUTEX_UNLOCK_FAILED                  \
        (MUTEX_UNLOCK_FAILED | RACP_ERR_ID)
#define RACP_MEMORY_ALLOCATION_FAILED             \
        (MEMORY_ALLOCATION_FAILED | RACP_ERR_ID)
/** \} */

/* ======================================= Section 'HCI' */
/**
 * \defgroup btle_error_codes_module_hci HCI
 * \{
 */

#define HCI_MUTEX_INIT_FAILED                    \
        (MUTEX_INIT_FAILED | HCI_ERR_ID)
#define HCI_COND_INIT_FAILED                     \
        (COND_INIT_FAILED | HCI_ERR_ID)
#define HCI_MUTEX_LOCK_FAILED                    \
        (MUTEX_LOCK_FAILED | HCI_ERR_ID)
#define HCI_MUTEX_UNLOCK_FAILED                  \
        (MUTEX_UNLOCK_FAILED | HCI_ERR_ID)
#define HCI_MEMORY_ALLOCATION_FAILED             \
        (MEMORY_ALLOCATION_FAILED | HCI_ERR_ID)

#define HCI_INVALID_PARAMETER_VALUE             (0x0011 | HCI_ERR_ID)
#define HCI_PARAMETER_OUTSIDE_RANGE             (0x0012 | HCI_ERR_ID)
#define HCI_NULL_PARAMETER_NOT_ALLOWED          (0x0013 | HCI_ERR_ID)
#define HCI_CREATE_CONTEXT_FAILED               (0x0015 | HCI_ERR_ID)
#define HCI_SEND_COMMAND_FAILED                 (0x0016 | HCI_ERR_ID)
#define HCI_DEVICE_ASSERTION_FAILED             (0x0017 | HCI_ERR_ID)
#define HCI_STATE_NOT_CONNECTED                 (0x0018 | HCI_ERR_ID)
#define HCI_STATE_NOT_DISCONNECTED              (0x0019 | HCI_ERR_ID)
#define HCI_STATE_ALREADY_CONNECTED             (0x001A | HCI_ERR_ID)
#define HCI_STATE_ALREADY_DISCONNECTED          (0x001B | HCI_ERR_ID)
#define HCI_INVALID_COMMAND_LENGTH              (0x001C | HCI_ERR_ID)
#define HCI_HOST_CONTROLLER_RESET_FAILED        (0x001D | HCI_ERR_ID)
#define HCI_DEVICE_ATTR_ADDITION_FAILED         (0x001E | HCI_ERR_ID)
#define HCI_CREATE_CONNECTION_ENTITY_FAILED     (0x001F | HCI_ERR_ID)
#define HCI_CONNECTION_ENTITY_ASSERT_FAILED     (0x0020 | HCI_ERR_ID)
#define HCI_TX_BUFFER_FULL                      (0x0021 | HCI_ERR_ID)
#define HCI_ACL_LINK_IN_USE_BY_OTHER_SERVICE    (0x0022 | HCI_ERR_ID)
#define HCI_COMMAND_QUEUE_FULL                  (0x0023 | HCI_ERR_ID)
#define HCI_INVALID_COMMAND_PACKET              (0x0024 | HCI_ERR_ID)
#define HCI_NO_ACTIVE_SCO_CONNECTION            (0x0025 | HCI_ERR_ID)

#define HCI_API_NOT_SUPPORTED                   (0x00FF | HCI_ERR_ID)

/** \} */


/* ======================================= Section 'L2CAP' */
/**
 * \defgroup btle_error_codes_module_l2cap L2CAP
 * \{
 */

#define L2CAP_MUTEX_INIT_FAILED                 \
        (MUTEX_INIT_FAILED | L2CAP_ERR_ID)
#define L2CAP_COND_INIT_FAILED                  \
        (COND_INIT_FAILED | L2CAP_ERR_ID)
#define L2CAP_MUTEX_LOCK_FAILED                 \
        (MUTEX_LOCK_FAILED | L2CAP_ERR_ID)
#define L2CAP_MUTEX_UNLOCK_FAILED               \
        (MUTEX_UNLOCK_FAILED | L2CAP_ERR_ID)
#define L2CAP_MEMORY_ALLOCATION_FAILED          \
        (MEMORY_ALLOCATION_FAILED | L2CAP_ERR_ID)

#define L2CAP_INVALID_LMP_STATE                 (0x0011 | L2CAP_ERR_ID)
#define L2CAP_API_INVALID_PARAMS                (0x0012 | L2CAP_ERR_ID)
#define L2CAP_BUFFER_ALLOCATION_FAILED          (0x0013 | L2CAP_ERR_ID)
#define L2CAP_STATE_INCORRECT_FOR_OPERATION     (0x0014 | L2CAP_ERR_ID)
#define L2CAP_INVALID_ECHO_DATA_LENGTH          (0x0015 | L2CAP_ERR_ID)
#define L2CAP_NULL_VALUE_NOT_ACCEPTABLE         (0x0016 | L2CAP_ERR_ID)
#define L2CAP_INSUFFICIENT_BUFFER_SPACE         (0x0017 | L2CAP_ERR_ID)
#define L2CAP_COMMAND_TIMEOUT                   (0x0018 | L2CAP_ERR_ID)

#define L2CAP_PSM_WRONG_ENCODING                (0x0020 | L2CAP_ERR_ID)
#define L2CAP_PSM_HAS_ACTIVE_CID                (0x0021 | L2CAP_ERR_ID)
#define L2CAP_PSM_NOT_REGISTERED                (0x0022 | L2CAP_ERR_ID)
#define L2CAP_PSM_ALREADY_REGISTERED            (0x0024 | L2CAP_ERR_ID)
#define L2CAP_PSM_ENTITY_ALLOC_FAILED           (0x0025 | L2CAP_ERR_ID)
#define L2CAP_RTX_TIMER_START_FAILED            (0x0026 | L2CAP_ERR_ID)
#define L2CAP_MTU_CHECK_FAILED                  (0x0027 | L2CAP_ERR_ID)
#define L2CAP_UNACCEPTABLE_MTU_VALUE            (0x0028 | L2CAP_ERR_ID)
#define L2CAP_MTU_REQUESTED_LT_MIN_MTU          (0x0029 | L2CAP_ERR_ID)
#define L2CAP_CONNECTION_ENTITY_NOT_FOUND       (0x002A | L2CAP_ERR_ID)
#define L2CAP_CONNECTION_ENTITY_ALLOC_FAILED    (0x002B | L2CAP_ERR_ID)
#define L2CAP_SIGNALLING_ENTITY_NOT_FOUND       (0x002C | L2CAP_ERR_ID)
#define L2CAP_SIGNALLING_ENTITY_EXISTS          (0x002D | L2CAP_ERR_ID)
#define L2CAP_COMMAND_ENTITY_NOT_FOUND          (0x002E | L2CAP_ERR_ID)
#define L2CAP_COMMAND_ENTITY_ALLOC_FAILED       (0x002F | L2CAP_ERR_ID)
#define L2CAP_INVALID_CONFIG_TIMEOUT            (0x0030 | L2CAP_ERR_ID)
#define L2CAP_LOCAL_CONFIG_IN_PROGRESS          (0x0031 | L2CAP_ERR_ID)
#define L2CAP_GROUP_CONNECTION_ENTITY_NOT_FOUND (0x0032 | L2CAP_ERR_ID)
#define L2CAP_GROUP_FOR_PSM_EXISTS              (0x0033 | L2CAP_ERR_ID)
#define L2CAP_GROUP_MEMBER_EXISTS               (0x0034 | L2CAP_ERR_ID)
#define L2CAP_GROUP_HAS_NO_DEVICE               (0x0035 | L2CAP_ERR_ID)
#define L2CAP_FLOW_SPEC_BLOCK_ALLOC_FAILED      (0x0036 | L2CAP_ERR_ID)
#define L2CAP_LP_DISCONNECTED                   (0x0037 | L2CAP_ERR_ID)
#define L2CAP_CID_DISCONNECTED                  (0x0038 | L2CAP_ERR_ID)

#define L2CAP_FEC_OPTIONS_NULL                  (0x0040 | L2CAP_ERR_ID)
#define L2CAP_UNSUPPORTED_PEER_FEC_MODE         (0x0041 | L2CAP_ERR_ID)
#define L2CAP_FEC_ENTITY_ALLOCATION_FAILED      (0x0042 | L2CAP_ERR_ID)
#define L2CAP_FEC_TX_WINDOW_FULL                (0x0043 | L2CAP_ERR_ID)
#define L2CAP_FEC_DATA_WRITE_FAILURE            (0x0044 | L2CAP_ERR_ID)
#define L2CAP_PSM_WRONG_FEC_SUPPORT             (0x0045 | L2CAP_ERR_ID)
#define L2CAP_INCONSISTENT_FEC_MODE             (0x0046 | L2CAP_ERR_ID)
#define L2CAP_INCORRECT_SDU_LENGTH              (0x0047 | L2CAP_ERR_ID)
#define L2CAP_FEC_I_FRAME_MAX_TRANSMIT          (0x0048 | L2CAP_ERR_ID)
#define L2CAP_FEC_INVALID_REQ_SEQ               (0x0049 | L2CAP_ERR_ID)
#define L2CAP_FEC_UNACCEPTABLE_MPS_VALUE        (0x0050 | L2CAP_ERR_ID)
#define L2CAP_FEC_UNACCEPTABLE_TX_WINDOW_VALUE  (0x0051 | L2CAP_ERR_ID)
#define L2CAP_UNACCEPTABLE_FCS_VALUE            (0x0052 | L2CAP_ERR_ID)
#define L2CAP_FEC_OPTIONS_NOT_RENEGOTIATABLE    (0x0053 | L2CAP_ERR_ID)

#define L2CAP_NO_LMP_CONNECTION                 (0x0054 | L2CAP_ERR_ID)
#define L2CAP_API_NOT_ALLOWED                   (0x0055 | L2CAP_ERR_ID)

#define L2CAP_CHANNEL_NOT_FOUND                 (0x0070 | L2CAP_ERR_ID)
#define L2CAP_NOT_ENOUGH_CREDITS                (0x0071 | L2CAP_ERR_ID)
#define L2CAP_CHANNEL_INVALID_STATE             (0x0072 | L2CAP_ERR_ID)
#define L2CAP_CREDIT_OVERFLOW                   (0x0073 | L2CAP_ERR_ID)
#define L2CAP_UNACCEPTABLE_CREDIT_VALUE         (0x0074 | L2CAP_ERR_ID)
#define L2CAP_INVALID_CHANNEL_PARAM             (0x0081 | L2CAP_ERR_ID)
#define L2CAP_CBFC_PSM_NOT_IN_RANGE             (0x0082 | L2CAP_ERR_ID)
#define L2CAP_TX_QUEUE_NOT_EMPTY                (0x0083 | L2CAP_ERR_ID)
#define L2CAP_CBFC_TX_ALREADY_IN_PROGRESS       (0x0084 | L2CAP_ERR_ID)
#define L2CAP_API_NOT_SUPPORTED                 (0x00F0 | L2CAP_ERR_ID)

/** \} */

/* ======================================= Section 'SMP' */

/**
 * \defgroup btle_error_codes_module_smp SMP
 * \{
 */

/* SMP Specification defiend Error Codes */
#define SMP_LE_PASSKEY_ENTRY_FAILED              (0x01 | SM_LE_ERR_ID)
#define SMP_LE_OOB_DATA_NOT_AVAILABLE            (0x02 | SM_LE_ERR_ID)
#define SMP_LE_AUTHENTICATION_REQ_NOT_MET        (0x03 | SM_LE_ERR_ID)
#define SMP_LE_CONFIRM_VALUE_NOT_MATCH           (0x04 | SM_LE_ERR_ID)
#define SMP_LE_PAIRING_NOT_SUPPORTED             (0x05 | SM_LE_ERR_ID)
#define SMP_LE_INSUFFICIENT_ENCRYPTION_KEY_SIZE  (0x06 | SM_LE_ERR_ID)
#define SMP_LE_COMMAND_NOT_SUPPORTED             (0x07 | SM_LE_ERR_ID)
#define SMP_LE_UNSPECIFIED_REASON                (0x08 | SM_LE_ERR_ID)
#define SMP_LE_REPEATED_ATTEMTS                  (0x09 | SM_LE_ERR_ID)
#define SMP_LE_INVALID_PARAMETERS                (0x0A | SM_LE_ERR_ID)
#ifdef SMP_LESC
#define SMP_LE_DHKEY_CHECK_FAILED                (0x0B | SM_LE_ERR_ID)
#define SMP_LE_NUMERIC_COMPARISON_FAILED         (0x0C | SM_LE_ERR_ID)
#define SMP_LE_BREDR_PAIRING_IN_PROGRESS         (0x0D | SM_LE_ERR_ID)
#define SMP_LE_CT_KEY_GEN_NOT_ALLOWED            (0x0E | SM_LE_ERR_ID)
#endif /* SMP_LESC */

/* SMP Internal error code */
#define SMP_MUTEX_INIT_FAILED                   \
        (MUTEX_INIT_FAILED | SMP_ERR_ID)
#define SMP_COND_INIT_FAILED                    \
        (COND_INIT_FAILED | SMP_ERR_ID)
#define SMP_MUTEX_LOCK_FAILED                   \
        (MUTEX_LOCK_FAILED | SMP_ERR_ID)
#define SMP_MUTEX_UNLOCK_FAILED                 \
        (MUTEX_UNLOCK_FAILED | SMP_ERR_ID)
#define SMP_MEMORY_ALLOCATION_FAILED            \
        (MEMORY_ALLOCATION_FAILED | SMP_ERR_ID)

#define SMP_INVALID_PARAMETERS                         (0x0011 | SMP_ERR_ID)
#define SMP_NO_DEVICE_ENTITY                           (0x0012 | SMP_ERR_ID)
#define SMP_CONTEXT_TABLE_FULL                         (0x0013 | SMP_ERR_ID)
#define SMP_CONTEXT_NOT_FOUND                          (0x0014 | SMP_ERR_ID)
#define SMP_AUTHENTICATION_TIMEOUT                     (0x0015 | SMP_ERR_ID)
#define SMP_INCORRECT_STATE                            (0x0016 | SMP_ERR_ID)
#define SMP_OPERATION_IN_PROGRESS                      (0x0017 | SMP_ERR_ID)
#define SMP_LINK_DISCONNECTED                          (0x0018 | SMP_ERR_ID)
#define SMP_PROCEDURE_FAILED                           (0x0019 | SMP_ERR_ID)
#define SMP_DEVICE_ENTRY_PURGE_FAILED                  (0x001A | SMP_ERR_ID)

#define SMP_DISPLAY_PASSKEY_UPDATED                    (0x00A0 | SMP_ERR_ID)
#define SMP_REMOTE_SIDE_PIN_KEY_MISSING                (0x00A1 | SMP_ERR_ID)
/** \} */

/* ======================================= Section 'ATT' */
/**
 * \defgroup btle_error_codes_module_att ATT
 * \{
 */

#define ATT_MUTEX_INIT_FAILED                   \
        (MUTEX_INIT_FAILED | ATT_ERR_ID)
#define ATT_COND_INIT_FAILED                    \
        (COND_INIT_FAILED | ATT_ERR_ID)
#define ATT_MUTEX_LOCK_FAILED                   \
        (MUTEX_LOCK_FAILED | ATT_ERR_ID)
#define ATT_MUTEX_UNLOCK_FAILED                 \
        (MUTEX_UNLOCK_FAILED | ATT_ERR_ID)
#define ATT_ERR_MEM_ALLOCATION_FAILED            \
        (MEMORY_ALLOCATION_FAILED | ATT_ERR_ID)

/* ATT Common Error Codes */
#define ATT_NULL_PARAMETER_NOT_ALLOWED                 (0x0011 | ATT_ERR_ID)
#define ATT_UNKNOWN_PDU_TYPE                           (0x0012 | ATT_ERR_ID)
#define ATT_INVALID_INSTANCE                           (0x0013 | ATT_ERR_ID)
#define ATT_INVALID_OPERATION                          (0x0014 | ATT_ERR_ID)
#define ATT_INVLAID_DB_HANDLE                          (0x0015 | ATT_ERR_ID)
#define ATT_INVALID_PARAMETER                          (0x0016 | ATT_ERR_ID)
#define ATT_RESPONSE_TIMED_OUT                         (0x0017 | ATT_ERR_ID)
#define ATT_UNEXPECTED_PDU                             (0x0018 | ATT_ERR_ID)
#define ATT_NO_FREE_INSTANCE                           (0x0019 | ATT_ERR_ID)
#define ATT_INVALID_PARAM_LEN                          (0x0020 | ATT_ERR_ID)

/** \} */

/**
 * \defgroup btle_error_codes_module_gatt_db GATT DB
 * \{
 */

#define GATT_DB_MUTEX_INIT_FAILED                               \
        (MUTEX_INIT_FAILED | GATT_DB_ERR_ID)
#define GATT_DB_COND_INIT_FAILED                                \
        (COND_INIT_FAILED | GATT_DB_ERR_ID)
#define GATT_DB_MUTEX_LOCK_FAILED                               \
        (MUTEX_LOCK_FAILED | GATT_DB_ERR_ID)
#define GATT_DB_MUTEX_UNLOCK_FAILED                             \
        (MUTEX_UNLOCK_FAILED | GATT_DB_ERR_ID)
#define GATT_DB_MEMORY_ALLOCATION_FAILED                        \
        (MEMORY_ALLOCATION_FAILED | GATT_DB_ERR_ID)

#define GATT_DB_INVALID_OFFSET                   (0x0007 | GATT_DB_ERR_ID)
#define GATT_DB_NULL_PARAMETER_NOT_ALLOWED       (0x0011 | GATT_DB_ERR_ID)
#define GATT_DB_UNSUPPORTED_GROUP_TYPE           (0x0012 | GATT_DB_ERR_ID)
#define GATT_DB_INSUFFICIENT_BUFFER_LEN          (0x0013 | GATT_DB_ERR_ID)
#define GATT_DB_MORE_MATCHING_RESULT_FOUND       (0x0014 | GATT_DB_ERR_ID)
#define GATT_DB_NO_MATCHING_RESULT               (0x0015 | GATT_DB_ERR_ID)
#define GATT_DB_HANDLE_NOT_FOUND                 (0x0016 | GATT_DB_ERR_ID)
#define GATT_DB_HANDLE_NOT_IN_RANGE              (0x0017 | GATT_DB_ERR_ID)
#define GATT_DB_HANDLE_IN_GROUP_RANGE            (0x0018 | GATT_DB_ERR_ID)
#define GATT_DB_INVALID_OPERATION                (0x0019 | GATT_DB_ERR_ID)
#define GATT_DB_UUID_NOT_IN_BT_SPACE             (0x0020 | GATT_DB_ERR_ID)
#define GATT_DB_INVALID_ATTR_HANDLE              (0x0021 | GATT_DB_ERR_ID)
#define GATT_DB_INSUFFICIENT_SECURITY            (0x0022 | GATT_DB_ERR_ID)
#define GATT_DB_INSUFFICIENT_ENC_KEY_SIZE        (0x0023 | GATT_DB_ERR_ID)
#define GATT_DB_INVALID_INSTANCE                 (0x0024 | GATT_DB_ERR_ID)
#define GATT_DB_INCORRECT_UUID_FRMT              (0x0025 | GATT_DB_ERR_ID)
#define GATT_DB_UUID_FRMT_UNSUPPORTED            (0x0026 | GATT_DB_ERR_ID)
#define GATT_DB_TYPE_MISMATCH                    (0x0027 | GATT_DB_ERR_ID)
#define GATT_DB_INSUFFICIENT_ENCRYPTION          (0x0028 | GATT_DB_ERR_ID)
#define GATT_DB_NON_EDITABLE_ATTR_VALUE          (0x0029 | GATT_DB_ERR_ID)
#define GATT_DB_INSUFFICIENT_AUTHORIZATION       (0x002A | GATT_DB_ERR_ID)
#define GATT_DB_INVALID_TRANSPORT_ACCESS         (0x002B | GATT_DB_ERR_ID)
/* Not an error code. More of a status code */
#define GATT_DB_DELAYED_RESPONSE                 (0x0060 | GATT_DB_ERR_ID)


/** \} */

/**
 * \defgroup btle_error_codes_module_utils Utilities
 * \{
 */

#define FSM_UNKNOWN_EVENT_TYPE                          (0x0011|FSM_ERR_ID)

#define DQ_MUTEX_INIT_FAILED                    \
        (MUTEX_INIT_FAILED | DQ_ERR_ID)
#define DQ_COND_INIT_FAILED                     \
        (COND_INIT_FAILED | DQ_ERR_ID)
#define DQ_MUTEX_LOCK_FAILED                    \
        (MUTEX_LOCK_FAILED | DQ_ERR_ID)
#define DQ_MUTEX_UNLOCK_FAILED                  \
        (MUTEX_UNLOCK_FAILED | DQ_ERR_ID)
#define DQ_MEMORY_ALLOCATION_FAILED             \
        (MEMORY_ALLOCATION_FAILED | DQ_ERR_ID)

#define DQ_NULL_PARAMETER_NOT_ALLOWED                   (0x0011 | DQ_ERR_ID)
#define DQ_DEVICE_ALREADY_ENQUEUED                      (0x0012 | DQ_ERR_ID)
#define DQ_QUEUE_FULL                                   (0x0013 | DQ_ERR_ID)
#define DQ_DEVICE_NOT_FOUND                             (0x0014 | DQ_ERR_ID)
#define DQ_INVALID_HANDLE                               (0x0015 | DQ_ERR_ID)
#define DQ_DEVICE_IN_RESOLUTION                         (0x0016 | DQ_ERR_ID)

/* ======================================= Section 'Storage' */
/* Storage Internal error code */
#define STORAGE_MUTEX_INIT_FAILED                   \
        (MUTEX_INIT_FAILED | STORAGE_ERR_ID)
#define STORAGE_COND_INIT_FAILED                    \
        (COND_INIT_FAILED | STORAGE_ERR_ID)
#define STORAGE_MUTEX_LOCK_FAILED                   \
        (MUTEX_LOCK_FAILED | STORAGE_ERR_ID)
#define STORAGE_MUTEX_UNLOCK_FAILED                 \
        (MUTEX_UNLOCK_FAILED | STORAGE_ERR_ID)
#define STORAGE_MEMORY_ALLOCATION_FAILED            \
        (MEMORY_ALLOCATION_FAILED | STORAGE_ERR_ID)

#define STORAGE_INVALID_PARAMETERS                      (0x0011 | STORAGE_ERR_ID)
#define STORAGE_OPERATION_FAILED                        (0x0012 | STORAGE_ERR_ID)
#define STORAGE_OPERATION_MASKED                        (0x0013 | STORAGE_ERR_ID)

/** \} */

/* ======================================= Section 'BT-FOPS' */
/**
 * \defgroup btle_error_codes_module_fops FOPS
 * \{
 */
#define BT_FOPS_MUTEX_INIT_FAILED               \
        (MUTEX_INIT_FAILED | BT_FOPS_ERR_ID)
#define BT_FOPS_COND_INIT_FAILED                \
        (COND_INIT_FAILED | BT_FOPS_ERR_ID)
#define BT_FOPS_MUTEX_LOCK_FAILED               \
        (MUTEX_LOCK_FAILED | BT_FOPS_ERR_ID)
#define BT_FOPS_MUTEX_UNLOCK_FAILED             \
        (MUTEX_UNLOCK_FAILED | BT_FOPS_ERR_ID)
#define BT_FOPS_MEMORY_ALLOCATION_FAILED        \
        (MEMORY_ALLOCATION_FAILED | BT_FOPS_ERR_ID)

#define BT_FOPS_ERR_GET_CURRECT_DIRECTORY       (0x0011 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_GET_FILE_ATTRIBUTES         (0x0012 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_SET_PATH_FORWARD            (0x0013 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_SET_PATH_BACKWARD           (0x0014 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_CREATE_FOLDER               (0x0015 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_OPEN                   (0x0016 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_WRITE                  (0x0017 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_READ                   (0x0018 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FOLDER_DELETE               (0x0019 | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_DELETE                 (0x001A | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_NOT_FOUND              (0x001B | BT_FOPS_ERR_ID)
#define BT_FOPS_INVALID_PARAMETER_VALUE         (0x001C | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_SEEK_FAILED            (0x001D | BT_FOPS_ERR_ID)
#define BT_FOPS_ERR_FILE_COPY                   (0x001E | BT_FOPS_ERR_ID)

/** \} */
/** \} */

#endif /* _H_BT_ERROR_ */

