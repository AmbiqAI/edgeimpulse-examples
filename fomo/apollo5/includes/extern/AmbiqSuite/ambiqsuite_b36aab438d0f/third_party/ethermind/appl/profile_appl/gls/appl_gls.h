
/**
 *  \file appl_gls.h
 *
 *  Application Header File for Glucose Sensor.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_GLS_
#define _H_APPL_GLS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl_gap.h"

/* --------------------------------------------- Global Definitions */
typedef struct appl_gl_racp_req_param
{
    UCHAR       req_op_code;

    UCHAR       oprtr;

    UCHAR       operand[20];
}APPL_RACP_REQ_PARAM;

/* Structure to be passed to Timer Handler */
typedef struct appl_gl_racp_timer_param
{
    APPL_HANDLE          handle;

    UCHAR                rsp_code_val;

    APPL_RACP_REQ_PARAM  appl_req_param;

}APPL_TIMER_PARAM;

typedef struct appl_gls_user_facing_time
{
    INT16 hours;

    INT16 minutes;

} APPL_GLS_USER_FACING_TIME;

/* --------------------------------------------- Functions */

void appl_gls_init (void);
void appl_gls_connect(APPL_HANDLE  * appl_handle);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_gls_server_reinitialize (void);
API_RESULT appl_gl_racp_write_handler
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           );
void appl_manage_trasnfer (APPL_TIMER_PARAM  * appl_param);
API_RESULT appl_gl_racp_operation_hndlr (APPL_TIMER_PARAM    * appl_param);
API_RESULT appl_gl_racp_send_response (APPL_TIMER_PARAM    * appl_param);
API_RESULT appl_gl_racp_rprt_strd_record (APPL_TIMER_PARAM  *appl_timer_param);
UINT16 appl_get_stored_records_count(APPL_TIMER_PARAM * appl_timer_param, UCHAR type);
API_RESULT appl_gl_racp_clear_strd_records(APPL_TIMER_PARAM  *appl_timer_param);
UCHAR appl_check_context_info_flag(UCHAR appl_record_count);
void appl_profile_menu_handler (void);
void appl_activate_gls_records (void);
void gls_racp_init(void);

API_RESULT appl_gls_racp_write_handler
           (
                ATT_VALUE         * value
           );
void appl_racp_timer_expiry_handler(void *data, UINT16 datalen);
API_RESULT appl_gl_racp_validate_operands
            (
                /* IN */ UCHAR    oprtr,
                /* IN */ UCHAR  * operand,
                /* IN */ UCHAR    len
            );
void appl_send_rsp_handler
     (
         RACP_REQ_INFO   * gls_racp_req_info,
         UINT16            len,
         UINT16            result
     );

API_RESULT appl_gls_report_stored_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );

API_RESULT appl_gls_report_stored_number_of_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );

API_RESULT appl_gls_delete_stored_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );

API_RESULT appl_gls_abort_operation
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );

API_RESULT appl_gls_access_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           );
INT16 appl_gls_convert_from_hrs_to_minutes
      (
          APPL_GLS_USER_FACING_TIME * appl_user_facing_time,
          INT16                      prev_record_time_offset
      );

void appl_gls_update_time_offset
     (
         APPL_GLS_USER_FACING_TIME * appl_user_facing_time,
         UCHAR record_no,
         UCHAR total_records
     );

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_gls_init()
#define APPL_PROFILE_CONNECT(x) appl_gls_connect(x)
#define APPL_SEND_MEASUREMENT(...)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_gls_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_glucose_meter_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_GLS_ */


