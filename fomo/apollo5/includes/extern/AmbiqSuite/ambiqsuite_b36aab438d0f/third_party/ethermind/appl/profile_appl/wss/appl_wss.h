
/**
 *  \file appl_wss.h
 *
 *  Sample Profile Application Header.
 */

/*
 *  Copyright (C) 2012. MindTree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_WSS_
#define _H_APPL_WSS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Global Definitions */
/**
 * This flag is defined if BCS is supported by the Weight Scale.
 */
#define BCS_SUPPORTED_BY_WSS

/* --------------------------------------------- Structures/Data Types */
typedef struct cntrl_point_param
{
    UCHAR          rsp_code;

    UCHAR          req_code;

    APPL_HANDLE    appl_handle;

}APPL_USER_CNTRL_POINT_RSP_PARAM;

typedef struct
{
    UCHAR   data[20];
    UINT16   length;
}APPL_BCS_CONTINUES_OBS_DATA;


typedef struct
{
    UINT16        consent_code;
    DEVICE_HANDLE dev_id;
    UCHAR         appl_consent_retry;
    UCHAR         valid_consent_code_sent;
} APPL_UDS_CP;

/* --------------------------------------------- Macros */
#define APPL_WS_MSRMT_LENGTH                     15
#define APPL_WS_MSRMT_INTERVAL                   5
#define APPL_BC_MSRMT_LENGTH                     30
#define APPL_BC_MSRMT_INTERVAL                   5
#define APPL_DCIC_INTERVAL                       5
#define APPL_DCIC_LENGTH                         4
#define APPL_BC_FAETURE                          4
#define APPL_MAX_BC_MEASUREMENT_DATA             34
#define APPL_USER_IDX_OFFSET_IN_BCS_MSRNT        11
#define APPL_USER_IDX_OFFSET_IN_WSS_MSRNT        10

/* User Control Point Opcodes */
#define APPL_REGISTER_NEW_USER_REQ               0x01
#define APPL_CONSENT_REQ                         0x02
#define APPL_DELETE_USER_DATA_REQ                0x03
#define APPL_RESPONSE_CODE                       0x20

#define APPL_REGISTER_NEW_USER_REQ_LEN           3
#define APPL_CONSENT_REQ_LEN                     4
#define APPL_DELETE_USER_DATA_REQ_LEN            1

/* User Control Point Response Value */
#define APPL_UCP_SUCCESS                         0x01
#define APPL_UCP_OPCODE_NOT_SUPPORTED            0x02
#define APPL_UCP_INVALID_PARAMETER               0x03
#define APPL_UCP_OPERATION_FAILED                0x04
#define APPL_UCP_USER_NOT_AUTHORIZED             0x05

#define APPL_INVALID_USER_INDEX                  0xFF

/* Maximum nuber of attempts for Consent */
#define APPL_MAX_CONSENT_RETRY                   3

/* Maximum number of New Users */
#define APPL_MAX_NEW_USER_SUPPORT                2

/* Application Error code */
#define APPL_USER_DATA_ACCESS_NOT_PERMITTED      0x80

/* Consent Code Range */
#define APPL_MIN_CONSENT_CODE_VALUE              0x0000
#define APPL_MAX_CONSENT_CODE_VALUE              0x270F

#define APPL_VALIDATE_CONSENT_CODE(val)\
        (((APPL_MAX_CONSENT_CODE_VALUE >= (val)) &&\
        (APPL_MIN_CONSENT_CODE_VALUE <= (val)))? BT_TRUE : BT_FALSE)

#define APPL_UDS_CP_STRUCT_INIT(idx)\
        appl_uds_cp[idx].consent_code            = 0xFFFF; \
        appl_uds_cp[idx].dev_id                  = 0xFF; \
        appl_uds_cp[idx].appl_consent_retry      = 0; \
        appl_uds_cp[idx].valid_consent_code_sent = 0xFF;

#define APPL_SEND_BCS_HNDL_VAL  BT_att_send_hndl_val_ind

/* --------------------------------------------- Functions */
void appl_wss_init(void);
void appl_ws_timer_expiry_handler (void *data, UINT16 datalen);
void appl_db_timer_expiry_handler (void *data, UINT16 datalen);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_wss_connect(APPL_HANDLE  * appl_handle);
void appl_send_wss_measurement (APPL_HANDLE  * handle);
void appl_wss_server_reinitialize (void);
void appl_send_bcs_measurement (APPL_HANDLE * handle);
void appl_bcs_timer_expiry_handler (void *data_param, UINT16 datalen);
void appl_send_bcs_remaining_data(ATT_HANDLE_VALUE_PAIR * hndl_val_params);
API_RESULT appl_user_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           );
API_RESULT appl_validate_user_data_access
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           );
void appl_profile_menu_handler(void);
API_RESULT appl_check_consent_code
           (
                UINT16 consent_code,
                UCHAR user_index
           );
API_RESULT appl_get_newly_regstd_user_index
           (
                /* IN */  UINT16  consent_code_of_user,
                /* OUT */ UCHAR   * user_idx,
                /* IN */  DEVICE_HANDLE dev_handle
           );
void appl_uds_cp_structure_init();

/* Profile handling */
#define APPL_PROFILE_INIT appl_wss_init
#define APPL_PROFILE_CONNECT(x) appl_wss_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_wss_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_wss_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_wss_measurment_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#ifndef USE_BT_PL_TIMER
#define APPL_USE_IDLE_TIMER
#endif /* USE_BT_PL_TIMER */
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_WSS_ */

