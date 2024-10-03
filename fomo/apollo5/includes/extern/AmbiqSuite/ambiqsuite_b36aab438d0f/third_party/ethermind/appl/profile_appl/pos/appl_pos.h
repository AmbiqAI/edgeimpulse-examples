
/**
 *  \file appl_pos.h
 *
 *  Pulse Oximeter Server Application Header.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_POS_
#define _H_APPL_POS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Global Definitions */
typedef struct appl_pos_racp_req_param
{
    UCHAR       req_op_code;

    UCHAR       oprtr;

    UCHAR       operand[20];

}APPL_RACP_REQ_PARAM;

/* Structure to be passed to Timer Handler */
typedef struct appl_pos_racp_timer_param
{
    APPL_HANDLE          handle;

    UCHAR                rsp_code_val;

    APPL_RACP_REQ_PARAM  appl_req_param;

}APPL_TIMER_PARAM;

/* --------------------------------------------- Functions */
void appl_pos_init(void);
void appl_spot_chk_msrmt_timer_expiry_handler (void *data, UINT16 datalen);
void appl_continus_msrmt_timer_expiry_handler (void *data, UINT16 datalen);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_pos_connect(APPL_HANDLE  * appl_handle);

void appl_send_pulse_ox_measurement (APPL_HANDLE  * handle);
void appl_pos_server_reinitialize (void);
void appl_racp_manage_transfer (APPL_TIMER_PARAM * appl_timer_param);
void appl_racp_timer_expiry_handler (void *data, UINT16 datalen);
void appl_racp_send_ind_timer_expiry_handler (void *data, UINT16 datalen);
API_RESULT appl_pos_continus_msrmt_hndlr();
void pos_racp_init(void);
API_RESULT appl_pos_racp_operation_hndlr
           (
                APPL_TIMER_PARAM * appl_timer_param
           );
API_RESULT appl_pos_racp_rprt_strd_record
           (
                APPL_TIMER_PARAM  *appl_timer_param
           );
API_RESULT appl_pos_racp_clear_strd_records
           (
                APPL_TIMER_PARAM  *appl_timer_param
           );
API_RESULT appl_pos_racp_send_response
           (
                APPL_TIMER_PARAM * appl_timer_param
           );
API_RESULT appl_pos_racp_write_handler
           (
               ATT_VALUE    * value
           );
void appl_timer_expiry_handler
     (
         void    * data,
         UINT16    datalen
     );
void appl_pos_send_rsp_handler
     (
         RACP_REQ_INFO   * pos_racp_req_info,
         UINT16            len,
         UINT16            result
     );
API_RESULT appl_pos_access_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );
API_RESULT appl_pos_report_stored_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );
API_RESULT appl_pos_report_stored_number_of_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );
API_RESULT appl_pos_delete_stored_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );
API_RESULT appl_pos_abort_operation
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );

/* Profile handling */
#define APPL_PROFILE_INIT appl_pos_init
#define APPL_PROFILE_CONNECT(x) appl_pos_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_pulse_ox_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_pos_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_pulse_ox_msrmt_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_POS_ */

