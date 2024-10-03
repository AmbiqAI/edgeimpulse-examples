
/**
 *  \file appl_cpms.h
 *
 *  Sample Profile Application Header.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_CPMS_
#define _H_APPL_CPMS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Global Definitions */
typedef struct cntrl_point_param
{
    APPL_HANDLE    appl_handle;

    ATT_VALUE      rsp_param;

}APPL_CPM_CNTRL_POINT_RSP_PARAM;

#define APPL_NUM_OF_CPM_MSRMT_MASK_FIELD        9
#define APPL_CPM_MSRMNT_LENGTH                  30
#define APPL_CPM_MSRMT_INTERVAL                 5
#define APPL_CPM_VECTOR_INTERVAL                5
#define APPL_CPM_VECTOR_DATA_LENGTH             11

/* CPM Control Point Opcodes */
#define APPL_CPM_SET_CUMMULATIVE_VALUE_OPCODE 0x01
#define APPL_CPM_UPDATE_SENSOR_LOCATION_OPCODE 0x02
#define APPL_CPM_REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE 0x03
#define APPL_CPM_SET_CRANK_LENGTH_OPCODE     0x04
#define APPL_CPM_REQ_CRANK_LENGTH_OPCODE     0x05
#define APPL_CPM_SET_CHAIN_LENGTH_OPCODE     0x06
#define APPL_CPM_REQ_CHAIN_LENGTH_OPCODE     0x07
#define APPL_CPM_SET_CHAIN_WEIGHT_OPCODE     0x08
#define APPL_CPM_REQ_CHAIN_WEIGHT_OPCODE     0x09
#define APPL_CPM_SET_SPAN_LENGTH_OPCODE      0x0A
#define APPL_CPM_REQ_SPAN_LENGTH_OPCODE      0x0B
#define APPL_CPM_STRT_OFFSET_COMPNSTN_OPCODE 0x0C
#define APPL_MASK_CPM_CHAR_CONTENT_OPCODE    0x0D
#define APPL_CPM_REQ_SAMPLING_RATE_OPCODE    0x0E
#define APPL_CPM_REQ_FACTRY_CALIB_DATE_OPCODE 0x0F
#define APPL_CPM_STRT_ENHCED_OFFSET_COMPNSTN_OPCODE 0X10
#define APPL_CPM_RESPONSE_CODE               0x20


#define APPL_CPM_CUMMULATIVE_VALUE_LENGTH    4

/* Supported Sensor Location Count */
#define APPL_CPM_SUPPORTED_SENSOR_LOC_COUNT  3

/* CPM Control Point Response Values */
#define APPL_CPM_SUCCESS                     0x01
#define APPL_CPM_OPCODE_NOT_SUPPORTED        0x02
#define APPL_CPM_INVALID_PARAMETER           0x03
#define APPL_CPM_OPERATION_FAILED            0x04

/* CPM Control point Response parameter */
#define APPL_CPM_INCORRECT_CALBN_POSITION   0x01

/* Valid Crank Length Param */
#define APPL_MAX_CRANK_LENGTH                30
#define APPL_MIN_CRANK_LENGTH                10
#define APPL_VALID_CRANK_LENGTH_PARAM_LEN    2

/* Valid Chain Length Param */
#define APPL_MAX_CHAIN_LENGTH                50
#define APPL_MIN_CHAIN_LENGTH                10
#define APPL_VALID_CHAIN_LENGTH_PARAM_LEN    2

/* Valid Chain Weight Param */
#define APPL_MAX_CHAIN_WEIGHT                50
#define APPL_MIN_CHAIN_WEIGHT                10
#define APPL_VALID_CHAIN_WEIGHT_PARAM_LEN    2

/* Valid Span Length Param */
#define APPL_MAX_SPAN_LENGTH                20
#define APPL_MIN_SPAN_LENGTH                10
#define APPL_VALID_SPAN_LENGTH_PARAM_LEN    2

/* Vector Control Values */
#define APPL_START_CPM_VECTOR_TRANSFER      0x01
#define APPL_STOP_CPM_VECTOR_TRANSFER       0x00

#define APPL_CPM_INAPPROPRIATE_CONNECTION_PARAMETERS  0x80
#define APPL_CPM_BROADCAST_DATA_LENGTH                15

/* Utility Macros */
#define APPL_CPM_SUPPORTED_OPCODE(op)\
    (((APPL_CPM_STRT_ENHCED_OFFSET_COMPNSTN_OPCODE >= (op)) && \
        (APPL_CPM_SET_CUMMULATIVE_VALUE_OPCODE <= (op)))? BT_TRUE : BT_FALSE)

#define APPL_CPM_VALIDATE_AND_SET_CRANK_LEN_PARAM(src,dest,len,rsp)\
        if (((APPL_MAX_CRANK_LENGTH >= (UINT16)(*(src))) &&\
            (APPL_MIN_CRANK_LENGTH <= (UINT16)(*(src)))) &&\
            (APPL_VALID_CRANK_LENGTH_PARAM_LEN == (len)))\
        {\
            BT_PACK_LE_2_BYTE(&(dest),(src))\
            (rsp) = APPL_CPM_SUCCESS;\
        }\
        else\
        {\
            (rsp) = APPL_CPM_INVALID_PARAMETER;\
        }

#define APPL_CPM_VALIDATE_AND_SET_CHAIN_LEN_PARAM(src,dest,len,rsp)\
        if (((APPL_MAX_CHAIN_LENGTH >= (UINT16)(*(src))) &&\
            (APPL_MIN_CHAIN_LENGTH <= (UINT16)(*(src)))) &&\
            (APPL_VALID_CHAIN_LENGTH_PARAM_LEN == (len)))\
        {\
            BT_PACK_LE_2_BYTE(&(dest),(src))\
            (rsp) = APPL_CPM_SUCCESS;\
        }\
        else\
        {\
            (rsp) = APPL_CPM_INVALID_PARAMETER;\
        }

#define APPL_CPM_VALIDATE_AND_SET_CHAIN_WEIGHT_PARAM(src,dest,len,rsp)\
        if (((APPL_MAX_CHAIN_WEIGHT >= (UINT16)(*(src))) &&\
            (APPL_MIN_CHAIN_WEIGHT <= (UINT16)(*(src)))) &&\
            (APPL_VALID_CHAIN_WEIGHT_PARAM_LEN == (len)))\
        {\
            BT_PACK_LE_2_BYTE(&(dest),(src))\
            (rsp) = APPL_CPM_SUCCESS;\
        }\
        else\
        {\
            (rsp) = APPL_CPM_INVALID_PARAMETER;\
        }

#define APPL_CPM_VALIDATE_AND_SET_SPAN_LEN_PARAM(src,dest,len,rsp)\
        if (((APPL_MAX_SPAN_LENGTH >= (UINT16)(*(src))) &&\
            (APPL_MIN_SPAN_LENGTH <= (UINT16)(*(src)))) &&\
            (APPL_VALID_SPAN_LENGTH_PARAM_LEN == (len)))\
        {\
            BT_PACK_LE_2_BYTE(&(dest),(src))\
            (rsp) = APPL_CPM_SUCCESS;\
        }\
        else\
        {\
            (rsp) = APPL_CPM_INVALID_PARAMETER;\
        }
/* --------------------------------------------- Functions */
void appl_cpms_init(void);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_cpm_vector_timer_expiry_handler (void *data, UINT16 datalen);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_cpms_connect(APPL_HANDLE  * appl_handle);
void appl_send_cpms_measurement (APPL_HANDLE  * handle);
void appl_cpms_server_reinitialize (void);
void appl_cpm_cntrl_point_cnfg_handler (GATT_DB_HANDLE  *handle,UINT16  config);
void appl_cpm_manage_brodcast(UINT16 config);
void appl_cpms_l2cap_callback
     (
         DEVICE_HANDLE * handle,
         UCHAR           event_type,
         UCHAR         * event_data,
         UINT16          event_datalen
     );
void appl_cpms_profile_handler(UCHAR event_status);
void appl_manage_cpm_vector_msrmnt (DEVICE_HANDLE * handle, UINT16 result);

API_RESULT appl_cpm_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           );
void appl_cpms_menu_handler(void);

/* Profile handling */
#define APPL_PROFILE_INIT appl_cpms_init
#define APPL_PROFILE_CONNECT(x) appl_cpms_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_cpms_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_cpms_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_cpms_measurment_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#ifndef USE_BT_PL_TIMER
#define APPL_USE_IDLE_TIMER
#endif /* USE_BT_PL_TIMER */
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_CPMS_ */

