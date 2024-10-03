
/**
 *  \file appl_gls.c
 *
 *  This file contains the Glucose Sensor application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef GLS

/* --------------------------------------------- Global Definitions */
/* GL Response Codes */
#define APPL_GL_RACP_SUCCESS                 0x01
#define APPL_GL_RACP_OP_CODE_NOT_SUPPORTED   0x02
#define APPL_GL_RACP_INVALID_OPERATOR        0x03
#define APPL_GL_RACP_OPRTR_NOT_SUPPORTED     0x04
#define APPL_GL_RACP_INVALID_OPERAND         0x05
#define APPL_GL_RACP_NO_RECORDS_FOUND        0x06
#define APPL_GL_RACP_ABRT_UNSUCCESSFUL       0x07
#define APPL_GL_RACP_PROC_INCOMPLETE         0x08
#define APPL_GL_RACP_OPERAND_NOT_SUPPORTED   0x09

/* Application RACP States */
#define APPL_RACP_INIT_STATE                 0x00
#define APPL_RACP_TRANSFR_ONGOING_STATE      0x01
#define APPL_RACP_CLR_STATE                  0x02

/* Glucose Meter Defined RACP Operations */
#define APPL_GL_RACP_STRD_RECORD_REQ_OPCODE       0x01
#define APPL_GL_RACP_CLR_RECORD_REQ_OPCODE        0x02
#define APPL_GL_RACP_ABRT_RPRTING_REQ_OPCODE      0x03
#define APPL_GL_RACP_STRD_RECORD_CNT_REQ_OPCODE   0x04
#define APPL_GL_RACP_STRD_RECORD_CNT_RSP_OPCODE   0x05
#define APPL_GL_RACP_RSP_OPCODE                   0x06

/* Glucose Meter Defined RACP Operators */
#define APPL_GL_RACP_NULL_OPRTR                   0x00
#define APPL_GL_RACP_ALL_RCRD_OPRTR               0x01
#define APPL_GL_RACP_LSS_THN_EQL_TO_OPRTR         0x02
#define APPL_GL_RACP_GRTR_HNN_EQL_TO_OPRTR        0x03
#define APPL_GL_RACP_WINTHIN_RNG_OPRTR            0x04
#define APPL_GL_RACP_FRST_RCRD_OPRTR              0x05
#define APPL_GL_RACP_LST_RCRD_OPRTR               0x06

/* Glucose Meter Defined RACP Filters */
#define APPL_GL_RACP_SEQ_NO_FILTER                0x01
#define APPL_GL_RACP_USR_FACING_TIME_FILTER       0x02

#define APPL_GLS_ADV_DATA_LEN                     21

#define APPL_RECORD_TRNSFER_INTERVAL               3
#define APPL_UPDATE_RECORD_INTERVAL                15
#define APPL_GLUCOSE_MEASUREMENT_LENGTH            18
#define APPL_GLUCOSE_MEASUREMENT_CONTEXT_LENGTH    17
#define APPL_GLUCOSE_MEASUREMENT_COUNT             9
#define APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT     2
#define APPL_STORED_RECORD_COUNT                   3
#define APPL_CONTEXT_INFORMATION_FLAG_MASK         0x10

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

DECL_STATIC DECL_CONST RACP_MODULE_T gls_racp =
{
    appl_gls_report_stored_records,

    appl_gls_report_stored_number_of_records,

    appl_gls_delete_stored_record,

    appl_gls_abort_operation
};

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
const char gls_menu[] = " \n\
========= GLS : M A I N   M E N U ========= \n\
 \n\
    0. Exit \n\
    1. Refresh \n\
\n\
    2. Set Glucose Concentration Units to kg/L \n\
    3. Set Glucose Concentration Units to mol/L \n\
    4. Set Medication Value Units to kilograms \n\
    5. Set Medication Value Units to liters \n\
    6. Generate 1 Record \n\
    7. Generate 3 Record \n\
    8. Update time off to +1 hrs \n\
    9. Update time off to -2 hrs \n\
   10. Delete All Records \n\
   11. Generate 4 Record \n\
   12. Enable or disable Glucose Measurement Context in last record\n\
\n\
    Your Option ?\n";
#else /* BT_SW_TXP */
static const char main_options[] = "\r\n\
======== GLS : M A I N   M E N U ========\r\n\
0. Exit\r\n\
1. Refresh\r\n\
2. Power ON Dev\r\n\
3. Discon\r\n\
4. Set Gls contn Unts-kg/L\r\n\
5. Set Gls contn Unts-mol/L\r\n\
6. Set Medn Val Unts-kgs\r\n\
7. Set Medn Val Unts-lts\r\n\
8. Gen 1 Rec\r\n\
9. Gen 3 Rec\r\n\
10. Updt time off:+1 hr\r\n\
11. Updt time off:-2 hr\r\n\
12. Del All Recs\r\n\
13. Gen 4 Rec\r\n\
14. Enable Gls Msrmt Cntxt\r\n\
15. Disable Gls Msrmt Cntxt\r\n\
 Your Option ?\r\n";
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

static BT_timer_handle timer_handle;

/**
 *  Glusoce Measurement Observed Value:
 *  1st Octet - Flag
 *  2nd & 3rd Octet  - Sequence Number
 *  4th - 10th Octet - TimeStamp (In YYYY-MM-DD:HR-MIN-SEC)
 */
static UCHAR gls_msrmt_obs_data
             [APPL_GLUCOSE_MEASUREMENT_LENGTH * APPL_GLUCOSE_MEASUREMENT_COUNT] =
{
    /* Record 1 */
    0x00,
    /* Flag */
    0x0B,
    /* sequence number */
    0x00, 0x00,
    /* TimeStamp (In YYYY-MM-DD:HR-MIN-SEC) */
    0xDC, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00,
    /* Time Offset Field */
    0x00, 0x00,
    /* Glucose Concentration Field and Type-Sample Location Field */
    0x16, 0xD1, 0x11,
    /* Sensor Status Annunciation Field */
    0x00, 0x00,

    /* Record 2 */
    0x00, 0x1F, 0x01, 0x00, 0xDC, 0x07, 0x06, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 3 */
    0x00, 0x0F, 0x02, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 4 */
    0x00, 0x0F, 0x03, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0x3C, 0x00,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 5 */
    0x00, 0x0F, 0x07, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0x3D, 0x00,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 6 */
    0x00, 0x0F, 0x08, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0x3E, 0x00,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 7 */
    0x00, 0x0F, 0x09, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0xC4, 0xFF,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 8 */
    0x00, 0x0F, 0x0A, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0xC3, 0xFF,
    0x37, 0xC0, 0x11, 0x00, 0x00,

    /* Record 9 */
    0x00, 0x0F, 0x0B, 0x00, 0xDC, 0x07, 0x06, 0x09, 0x00, 0x00, 0x00, 0xC2, 0xFF,
    0x37, 0xC0, 0x11, 0x00, 0x00
};

static UCHAR gls_msrmt_seq_id_index[APPL_GLUCOSE_MEASUREMENT_COUNT] =
{
    2,

    20,

    38,

    56,

    74,

    92,

    110,

    128,

    146
};

/* Glusoce Measurement Context Observed Value */
 static UCHAR gls_msrmt_cntxt_obs_data
             [APPL_GLUCOSE_MEASUREMENT_CONTEXT_LENGTH * APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT] =
{
    /* Record 1 */

    /* Flag */
    0xFF,
    /* sequence number */
    0x01, 0x00,
    /* Extended Flags  */
    0x00,
    /* Carbohydrate ID */
    0x01,
    /* Carbohydrate - units of kilograms */
    0xDA, 0xD2,
    /* Meal */
    0x01,
    /* Tester + Health */
    0x51,
    /* Exercise Duration */
    0xFF, 0x00,
    /* Exercise Intensity */
    0x89,
    /* Medication ID */
    0x01,
    /**
     * Medication - units of kilograms(if bit4 is set, bit 5 is not set)
     * Medication - units of liters(if bit4 is set, bit 5 is set)
     *
     * The intended value to be sent is
     * 96mg = (96 * 10^(-3))Kg
     * In SFLOAT,
     * exponent = -3 = 0xD(2's Complement of -3: with 4bits Value as Exp)
     * mantissa = 96 = 0x060(in Hexadecimal with 24bits Value of Mantissa)
     * Combined Value = 0xD060
     * i.e. 0x60, 0xD0 in Byte Stream (LSB to MSB)
     */
    0x60, 0xD0,

    /* HbA1c */
    0x07, 0x00,

    /* Record 2 */
    0xDF, 0x02, 0x00, 0x00, 0x01, 0xDA, 0xD2, 0x01, 0x51, 0xFF, 0x00, 0x89,
    0x01, 0x60, 0xA0, 0x07, 0x00,
};

static UCHAR gls_cntxt_msrmt_seq_id_index[APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT] =
{
    1,

    18
};

static UCHAR appl_gl_feature_value[] = { 0xFF, 0x03 };

static GATT_DB_HANDLE  appl_gl_msrmt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_GLUCOSE_METER_INST,
    GATT_CHAR_GL_MSRMNT_INST
};

static GATT_DB_HANDLE  appl_gl_cntxt_msrmt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_GLUCOSE_METER_INST,
    GATT_CHAR_GL_CNTXT_INST
};

static GATT_DB_HANDLE  appl_gl_racp_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_GLUCOSE_METER_INST,
    GATT_CHAR_GL_RACP_INST
};

static ATT_ATTR_HANDLE appl_gl_msrmt_hndl;
static ATT_ATTR_HANDLE appl_gl_cntxt_msrmt_hndl;
static ATT_ATTR_HANDLE appl_gl_racp_hndl;
static ATT_ATTR_HANDLE appl_gl_feature_hndl;
static UCHAR  appl_gl_operating_state;
static UCHAR  appl_record_count;
static UCHAR  appl_stored_record_count;
static UINT16 appl_seg_index;
static UCHAR  msrmnt_to_send;
UCHAR  last_record_update;
UCHAR gls_racp_id;
APPL_HANDLE appl_hndl;
UCHAR count;

/* --------------------------------------------- Functions */

void appl_gls_init(void)
{
    gls_racp_id = RACP_MAX_MODULES;

    appl_gls_server_reinitialize();
}

void gls_racp_init(void)
{
    API_RESULT retval;

    if (RACP_MAX_MODULES == gls_racp_id)
    {
        retval = BT_racp_register_module(&gls_racp, &gls_racp_id);

        if (API_SUCCESS != retval)
        {
            APPL_ERR("[APPL]: RACP Registration Failed!\n");
        }
    }
    else
    {
        APPL_ERR("[APPL]: RACP Re-Registration Attempted!\n");
    }
}

void appl_gls_connect(APPL_HANDLE  * appl_handle)
{
    API_RESULT retval;

    /* Calling RACP Registration for GLS from here */
    /* This is done as profile init is invoked ahead of BT_bluetooth_on in SingleMode */
    gls_racp_init();

#ifndef APPL_GLS_SINGLE_BOND_SUPPORT
    appl_set_target_address_in_adv_data(APPL_GLS_ADV_DATA_LEN);
#endif /* APPL_GLS_SINGLE_BOND_SUPPORT */

    appl_gl_msrmt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    appl_hndl = (*appl_handle);

    /* Get Glucose Measurement Characteristics Handle */
    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_gl_msrmt_db_handle,
                  &appl_gl_msrmt_hndl
             );

    appl_gl_cntxt_msrmt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    /* Get Glucose Context Measurement Characteristics Handle */
    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_gl_cntxt_msrmt_db_handle,
                  &appl_gl_cntxt_msrmt_hndl
             );

    appl_gl_racp_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    /* Get Glucose RCAP Characteristics Handle */
    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_gl_racp_db_handle,
                  &appl_gl_racp_hndl
             );

#ifdef APPL_GLS_SINGLE_BOND_SUPPORT
    appl_gl_msrmt_db_handle.char_id = GATT_CHAR_GL_FEATURES_INST;

    value.val = appl_gl_feature_value;
    value.len = sizeof(appl_gl_feature_value);

    /* Set Multi Bond bit in Glucose feature to zero */
    retval = BT_gatt_db_set_char_val
             (
                &appl_gl_msrmt_db_handle,
                &value
             );
#endif /* APPL_GLS_SINGLE_BOND_SUPPORT */

}

void appl_gls_server_reinitialize (void)
{
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    appl_gl_operating_state = APPL_RACP_INIT_STATE;
    appl_record_count = 0;
    appl_stored_record_count = APPL_STORED_RECORD_COUNT;
    appl_seg_index = APPL_STORED_RECORD_COUNT;
    msrmnt_to_send = BT_FALSE;
    last_record_update = 0;
    count = 0;
}

API_RESULT appl_gl_racp_validate_operands
           (
                /* IN */ UCHAR    oprtr,
                /* IN */ UCHAR  * operand,
                /* IN */ UCHAR    len
           )
{
    API_RESULT    retval;
    UINT16 min_seg_id;
    UINT16 max_seg_id;

    retval = API_SUCCESS;

    if ((oprtr != APPL_GL_RACP_ALL_RCRD_OPRTR) ||
        (oprtr != APPL_GL_RACP_FRST_RCRD_OPRTR) ||
        (oprtr != APPL_GL_RACP_LST_RCRD_OPRTR))
    {
        switch (oprtr)
        {
        case APPL_GL_RACP_LSS_THN_EQL_TO_OPRTR:
        case APPL_GL_RACP_GRTR_HNN_EQL_TO_OPRTR:
            /** checking length of 3 is 1 byte of filter type and 2 bytes of seq no
             *  checking length of 8 is 1 byte of filter type and 7 bytes of user facing time
             */
            if ((APPL_GL_RACP_SEQ_NO_FILTER == operand[0] && 3 == len) ||
                (APPL_GL_RACP_USR_FACING_TIME_FILTER == operand[0] && 8 == len))
            {
                retval = API_SUCCESS;
            }
            else
            {
                retval = APPL_GL_RACP_OPERAND_NOT_SUPPORTED | APPL_ERR_ID;
            }

            break;
        case APPL_GL_RACP_WINTHIN_RNG_OPRTR:
            if (APPL_GL_RACP_SEQ_NO_FILTER == operand[0] && 5 == len)
            {
                BT_UNPACK_LE_2_BYTE
                (
                    &min_seg_id,
                    operand + 1
                );

                BT_UNPACK_LE_2_BYTE
                (
                    &max_seg_id,
                    operand + 3
                );

                if (min_seg_id > max_seg_id)
                {
                    retval = APPL_GL_RACP_INVALID_OPERAND | APPL_ERR_ID;
                }
                else
                {
                    retval = API_SUCCESS;
                }
            }
            else
            {
                retval = APPL_GL_RACP_OPERAND_NOT_SUPPORTED | APPL_ERR_ID;
            }

            break;
        default:
            break;
        }
    }

    return retval;
}


#if 0
API_RESULT appl_gl_racp_write_handler
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    UCHAR               operation;
    UCHAR               oprtr;
    API_RESULT          retval;
    APPL_TIMER_PARAM    appl_timer_param;

    retval = ATT_INVALID_ATTRIBUTE_LEN | APPL_ERR_ID;

    if (APPL_RACP_INIT_STATE == appl_gl_operating_state ||
        APPL_GL_RACP_ABRT_RPRTING_REQ_OPCODE == value->val[0])
    {
        appl_get_handle_from_device_handle (handle->device_id,&appl_timer_param.handle);

        /** Validate RACP Operation */
        if (1 < value->len)
        {
            operation = value->val[0];
            oprtr = value->val[1];

            appl_timer_param.appl_req_param.req_op_code = operation;
            appl_timer_param.appl_req_param.oprtr       = oprtr;
            BT_mem_copy
            (
                &appl_timer_param.appl_req_param.operand[0],
                &value->val[2],
                (value->len - 2)
            );

            if (BT_FALSE != APPL_RACP_OPCODE_IN_REQ_RANGE(operation))
            {
                retval = appl_gl_racp_validate_operations
                         (
                              oprtr,
                              value->val+2,
                              value->len-2
                          );
                if (API_SUCCESS == retval)
                {
                    /** Minimum length length is 2 */
                    switch (operation)
                    {
                    case APPL_GL_RACP_STRD_RECORD_REQ_OPCODE:
                    case APPL_GL_RACP_CLR_RECORD_REQ_OPCODE:
                    case APPL_GL_RACP_STRD_RECORD_CNT_REQ_OPCODE:
                        appl_timer_param.rsp_code_val = APPL_GL_RACP_SUCCESS;
                        break;
                    case APPL_GL_RACP_ABRT_RPRTING_REQ_OPCODE:
                        if (APPL_RACP_TRANSFR_ONGOING_STATE == appl_gl_operating_state
                            || APPL_RACP_CLR_STATE == appl_gl_operating_state)
                        {
                            /* Stop Ongoing transfer */
                            if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
                            {
                                retval = BT_stop_timer (timer_handle);
                                APPL_TRC (
                                "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                                retval,timer_handle);
                                timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                            }
                        }
                        appl_gl_operating_state = APPL_RACP_INIT_STATE;
                        appl_timer_param.rsp_code_val = APPL_GL_RACP_SUCCESS;
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    appl_timer_param.rsp_code_val = (UCHAR) retval;
                    retval = API_SUCCESS;
                }
            }
            else
            {
                appl_timer_param.rsp_code_val = APPL_GL_RACP_OP_CODE_NOT_SUPPORTED;
                retval = API_SUCCESS;
            }
        }
        appl_manage_trasnfer(&appl_timer_param);
    }
    else
    {
        retval = ATT_APPL_PROCEDURE_ALREADY_IN_PROGRESS | APPL_ERR_ID;
    }
    return retval;
}
#endif /* 0 */



void appl_manage_trasnfer (/* IN */ APPL_TIMER_PARAM * appl_timer_param)
{
    API_RESULT     retval;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_start_timer
             (
                 &timer_handle,
                 APPL_RECORD_TRNSFER_INTERVAL,
                 appl_timer_expiry_handler,
                 appl_timer_param,
                 sizeof(APPL_TIMER_PARAM)
             );
    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,timer_handle);
}

void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    API_RESULT         retval;
    APPL_TIMER_PARAM * appl_timer_param;

    appl_timer_param = (APPL_TIMER_PARAM *) data;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_start_timer
             (
                 &timer_handle,
                 APPL_RECORD_TRNSFER_INTERVAL,
                 appl_timer_expiry_handler,
                 appl_timer_param,
                 sizeof (APPL_TIMER_PARAM)
             );

    appl_gl_racp_operation_hndlr(appl_timer_param);

    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,timer_handle);
}

API_RESULT appl_gl_racp_operation_hndlr(APPL_TIMER_PARAM * appl_timer_param)
{
    API_RESULT             retval;

    if (0 != appl_stored_record_count)
    {
        /* Report Stored Records - ALL */
        if (APPL_GL_RACP_STRD_RECORD_REQ_OPCODE == appl_timer_param->appl_req_param.req_op_code &&
            APPL_GL_RACP_SUCCESS == appl_timer_param->rsp_code_val)
        {
            /* Update Operating State */
            appl_gl_operating_state = APPL_RACP_TRANSFR_ONGOING_STATE;
            retval = appl_gl_racp_rprt_strd_record (appl_timer_param);
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send RACP Records,"
                " reason 0x%04X", retval);
            }
        }
        else if (APPL_GL_RACP_CLR_RECORD_REQ_OPCODE == appl_timer_param->appl_req_param.req_op_code &&
            APPL_GL_RACP_SUCCESS == appl_timer_param->rsp_code_val)
        {
            /* Update Operating State */
            appl_gl_operating_state = APPL_RACP_CLR_STATE;
            retval = appl_gl_racp_clear_strd_records (appl_timer_param);
        }
        else
        {
            /* Update Operating State */
            appl_gl_operating_state = APPL_RACP_TRANSFR_ONGOING_STATE;
            retval = appl_gl_racp_send_response (appl_timer_param);
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
                " reason 0x%04X", retval);
            }
        }
    }
    else
    {
        if (APPL_GL_RACP_STRD_RECORD_CNT_REQ_OPCODE !=
            appl_timer_param->appl_req_param.req_op_code)
        {
            appl_timer_param->rsp_code_val = APPL_GL_RACP_NO_RECORDS_FOUND;
        }
        retval = appl_gl_racp_send_response (appl_timer_param);
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
            " reason 0x%04X", retval);
        }
    }
    return retval;
}

API_RESULT appl_gl_racp_clear_strd_records(APPL_TIMER_PARAM  *appl_timer_param)
{
    API_RESULT retval;
    UINT16 min_seg_id;
    UINT16 max_seg_id;
    UINT16 seg_id;
    UINT16 index;

    if (APPL_GL_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        for (index = 0; index <= appl_stored_record_count; index++)
        {
            gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] - 2] = 0x01;
        }
        appl_stored_record_count = 0;
    }
    else if (APPL_GL_RACP_WINTHIN_RNG_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        BT_UNPACK_LE_2_BYTE
        (
            &min_seg_id,
            appl_timer_param->appl_req_param.operand + 1
        );

        BT_UNPACK_LE_2_BYTE
        (
            &max_seg_id,
            appl_timer_param->appl_req_param.operand + 3
        );

        for (index = 0; index < appl_stored_record_count; index++)
        {
            BT_UNPACK_LE_2_BYTE
            (
                &seg_id,
                &gls_msrmt_obs_data[gls_msrmt_seq_id_index[index]]
            );

            if (seg_id >= min_seg_id && seg_id <= max_seg_id)
            {
                gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] - 2] = 0x01;
                /* appl_stored_record_count --; */
            }
        }
    }
    else
    {
        appl_timer_param->rsp_code_val = APPL_GL_RACP_INVALID_OPERATOR;
    }
    retval = appl_gl_racp_send_response (appl_timer_param);
    if (API_SUCCESS != retval)
    {
        APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
        " reason 0x%04X", retval);
    }
    return API_SUCCESS;
}

API_RESULT appl_gl_racp_rprt_strd_record(APPL_TIMER_PARAM  *appl_timer_param)
{
    ATT_HANDLE_VALUE_PAIR  appl_gl_msrmt_param;
    ATT_HANDLE_VALUE_PAIR  appl_gl_cntxt_msrmt_param;
    API_RESULT retval;
    UINT16     seg_index;
    UINT16     oprnd;
    UCHAR      contxt_flag;
    UINT16     index;

    seg_index = 0;
    oprnd = 0;
    retval = API_SUCCESS;
    contxt_flag = 0;
    appl_gl_msrmt_param.handle = ATT_INVALID_ATTR_HANDLE_VAL;

    /* For All Record Operator */
    if (APPL_GL_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        if (0x01 != gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count] - 2])
        {
            appl_gl_msrmt_param.handle = appl_gl_msrmt_hndl;
            appl_gl_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_LENGTH;

            if (1 == last_record_update)
            {
                appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
                    [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 2];
            }
            else
            {
                appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
                [APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count];
            }

            contxt_flag = appl_check_context_info_flag(appl_record_count);
        }
        appl_record_count++;
    }
    else if (APPL_GL_RACP_LSS_THN_EQL_TO_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        if (APPL_GL_RACP_SEQ_NO_FILTER == appl_timer_param->appl_req_param.operand[0])
        {
            BT_UNPACK_LE_2_BYTE
            (
                &oprnd,
                &(appl_timer_param->appl_req_param.operand[1])
            );
        }
        else
        {
            for (index = 0; index < APPL_STORED_RECORD_COUNT; index++)
            {
                if (gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] + 5] ==
                    appl_timer_param->appl_req_param.operand[4])
                {
                    BT_UNPACK_LE_2_BYTE
                    (
                        &oprnd,
                        &gls_msrmt_obs_data[gls_msrmt_seq_id_index[index]]
                    );
                    break;
                }
            }
        }
        while (appl_record_count < APPL_STORED_RECORD_COUNT)
        {
            BT_UNPACK_LE_2_BYTE
            (
                &seg_index,
                &(gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count]])
            );

            if ((seg_index <= oprnd) &&
                (0x01 != gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count] - 2]))
            {
                msrmnt_to_send = BT_TRUE;
                appl_timer_param->rsp_code_val = APPL_GL_RACP_SUCCESS;

                appl_gl_msrmt_param.handle = appl_gl_msrmt_hndl;
                appl_gl_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_LENGTH;
                appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
                    [APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count];
                contxt_flag = appl_check_context_info_flag(appl_record_count);
                appl_record_count++;
                break;
            }

            if (!(msrmnt_to_send))
            {
                appl_timer_param->rsp_code_val = APPL_GL_RACP_NO_RECORDS_FOUND;
            }
            appl_record_count++;
        }
    }
    /* For Greater than or equal to Operator (Seq Id) */
    else if (APPL_GL_RACP_GRTR_HNN_EQL_TO_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        if (APPL_GL_RACP_SEQ_NO_FILTER == appl_timer_param->appl_req_param.operand[0])
        {
            BT_UNPACK_LE_2_BYTE
            (
                &oprnd,
                &(appl_timer_param->appl_req_param.operand[1])
            );
        }
        else
        {
            for (index = 0; index < APPL_STORED_RECORD_COUNT; index++)
            {
                if (gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] + 5] ==
                    appl_timer_param->appl_req_param.operand[4])
                {
                    BT_UNPACK_LE_2_BYTE
                    (
                        &oprnd,
                        &gls_msrmt_obs_data[gls_msrmt_seq_id_index[index]]
                    );
                    break;
                }
            }
        }
        while (appl_record_count < APPL_STORED_RECORD_COUNT)
        {
            BT_UNPACK_LE_2_BYTE
            (
                &seg_index,
                &(gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count]])
            );

            if ((seg_index >= oprnd) &&
                (0x01 != gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count] - 2]))
            {
                msrmnt_to_send = BT_TRUE;
                appl_timer_param->rsp_code_val = APPL_GL_RACP_SUCCESS;

                appl_gl_msrmt_param.handle = appl_gl_msrmt_hndl;
                appl_gl_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_LENGTH;
                appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
                    [APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count];
                contxt_flag = appl_check_context_info_flag(appl_record_count);
                appl_record_count++;
                break;
            }

            if (!(msrmnt_to_send))
            {
                appl_timer_param->rsp_code_val = APPL_GL_RACP_NO_RECORDS_FOUND;
            }
            appl_record_count++;
        }
    }
    else if (APPL_GL_RACP_WINTHIN_RNG_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        if (APPL_GL_RACP_SEQ_NO_FILTER == appl_timer_param->appl_req_param.operand[0])
        {
            BT_UNPACK_LE_2_BYTE
            (
                &oprnd,
                &(appl_timer_param->appl_req_param.operand[1])
            );

            BT_UNPACK_LE_2_BYTE
            (
                &index,
                &(appl_timer_param->appl_req_param.operand[3])
            );

            while (appl_record_count < appl_stored_record_count)
            {
                BT_UNPACK_LE_2_BYTE
                (
                    &seg_index,
                    &(gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count]])
                );

                if ((seg_index >= oprnd && seg_index <= index) &&
                    (0x01 != gls_msrmt_obs_data[gls_msrmt_seq_id_index[appl_record_count] - 2]))
                {
                    msrmnt_to_send = BT_TRUE;
                    appl_timer_param->rsp_code_val = APPL_GL_RACP_SUCCESS;

                    appl_gl_msrmt_param.handle = appl_gl_msrmt_hndl;
                    appl_gl_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_LENGTH;
                    appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
                        [APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count];
                    contxt_flag = appl_check_context_info_flag(appl_record_count);
                    appl_record_count++;
                    break;
                }

                if (!(msrmnt_to_send))
                {
                    appl_timer_param->rsp_code_val = APPL_GL_RACP_NO_RECORDS_FOUND;
                }
                appl_record_count++;
            }

        }
    }
    else if (APPL_GL_RACP_FRST_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        appl_gl_msrmt_param.handle = appl_gl_msrmt_hndl;
        appl_gl_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_LENGTH;
        appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
            [APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count];
        contxt_flag = appl_check_context_info_flag(appl_record_count);
        appl_record_count += appl_stored_record_count;
    }
    else if (APPL_GL_RACP_LST_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        appl_record_count = (appl_stored_record_count - 1);
        appl_gl_msrmt_param.handle = appl_gl_msrmt_hndl;
        appl_gl_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_LENGTH;
        appl_gl_msrmt_param.value.val = &gls_msrmt_obs_data
            [APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count];
        contxt_flag = appl_check_context_info_flag(appl_record_count);
        appl_record_count ++;
    }
    else
    {
        appl_timer_param->rsp_code_val = APPL_GL_RACP_INVALID_OPERATOR;
        appl_record_count = appl_stored_record_count;
    }

    if ((APPL_GL_RACP_SUCCESS == appl_timer_param->rsp_code_val) &&
        (ATT_INVALID_ATTR_HANDLE_VAL != appl_gl_msrmt_param.handle))
    {
        appl_gl_msrmt_param.value.len -= 1;
        appl_gl_msrmt_param.value.val += 1;

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(appl_timer_param->handle),
                     &appl_gl_msrmt_param
                 );
        if (API_SUCCESS != retval)
        {
            appl_timer_param->rsp_code_val = APPL_GL_RACP_PROC_INCOMPLETE;
            retval = appl_gl_racp_send_response (appl_timer_param);
            APPL_TRC ("[** ERR **]: Failed to send Glucose measurement, "
            "reason 0x%04X", retval);
        }
        if (BT_FALSE != contxt_flag)
        {
            contxt_flag = 0;
            appl_gl_cntxt_msrmt_param.handle = appl_gl_cntxt_msrmt_hndl;
            appl_gl_cntxt_msrmt_param.value.len = APPL_GLUCOSE_MEASUREMENT_CONTEXT_LENGTH;

            if (1 == last_record_update)
            {
                appl_gl_cntxt_msrmt_param.value.val = &gls_msrmt_cntxt_obs_data
                [(gls_cntxt_msrmt_seq_id_index\
                [APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT - 1]) - 1];
            }
            else
            {
                appl_gl_cntxt_msrmt_param.value.val = &gls_msrmt_cntxt_obs_data
                [APPL_GLUCOSE_MEASUREMENT_CONTEXT_LENGTH * (appl_record_count - 2)];
            }

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(appl_timer_param->handle),
                         &appl_gl_cntxt_msrmt_param
                     );
            if (API_SUCCESS != retval)
            {
                appl_timer_param->rsp_code_val = APPL_GL_RACP_PROC_INCOMPLETE;
                retval = appl_gl_racp_send_response (appl_timer_param);
                APPL_TRC ("[** ERR **]: Failed to send Glucose Context Measurement,"
                " reason 0x%04X", retval);
            }
        }
    }

    if (appl_stored_record_count == appl_record_count)
    {
        retval = appl_gl_racp_send_response (appl_timer_param);
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
            " reason 0x%04X", retval);
        }
    }
    return retval;
}

UCHAR appl_check_context_info_flag(UCHAR appl_record_count)
{
    if (APPL_CONTEXT_INFORMATION_FLAG_MASK == \
        ((gls_msrmt_obs_data[(APPL_GLUCOSE_MEASUREMENT_LENGTH * appl_record_count) + 1]) & APPL_CONTEXT_INFORMATION_FLAG_MASK))
    {
        return BT_TRUE;
    }

    return BT_FALSE;
}

API_RESULT appl_gl_racp_send_response (APPL_TIMER_PARAM * appl_timer_param)
{
    API_RESULT             retval;
    ATT_HANDLE_VALUE_PAIR  hndl_val_param;
    UCHAR                  val[4];
    UINT16                 count;

    /* Initialize record count */
    appl_record_count = 0;

    if (APPL_GL_RACP_STRD_RECORD_CNT_REQ_OPCODE == appl_timer_param->appl_req_param.req_op_code
        && APPL_GL_RACP_SUCCESS == appl_timer_param->rsp_code_val)
    {
        val[0] = APPL_GL_RACP_STRD_RECORD_CNT_RSP_OPCODE;
        val[1] = APPL_GL_RACP_NULL_OPRTR;

        if (APPL_GL_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
        {
            count = appl_stored_record_count;
        }
        else if (APPL_GL_RACP_GRTR_HNN_EQL_TO_OPRTR == appl_timer_param->appl_req_param.oprtr)
        {
            count = appl_get_stored_records_count(appl_timer_param, APPL_GL_RACP_GRTR_HNN_EQL_TO_OPRTR);
        }
        else if (APPL_GL_RACP_LSS_THN_EQL_TO_OPRTR == appl_timer_param->appl_req_param.oprtr)
        {
            count = appl_get_stored_records_count(appl_timer_param,APPL_GL_RACP_LSS_THN_EQL_TO_OPRTR);
        }
        else
        {
            /* Need to update for LAST and FIRST Record!! */
            count = appl_get_stored_records_count(appl_timer_param, 0);
        }
        val[2] = (UCHAR)(count);
        val[3] = (UCHAR)(count >> 8);
    }
    else
    {
        val[0] = APPL_GL_RACP_RSP_OPCODE;
        val[1] = APPL_GL_RACP_NULL_OPRTR;
        val[2] = appl_timer_param->appl_req_param.req_op_code;
        val[3] = appl_timer_param->rsp_code_val;
    }
    hndl_val_param.handle = appl_gl_racp_hndl;
    hndl_val_param.value.len = 4;
    hndl_val_param.value.val = &val[0];

    retval = BT_att_send_hndl_val_ind
             (
                 &APPL_GET_ATT_INSTANCE(appl_timer_param->handle),
                 &hndl_val_param
             );

    /* Intialize the operating state */
    appl_gl_operating_state = APPL_RACP_INIT_STATE;
    msrmnt_to_send = BT_FALSE;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    return retval;
}

UINT16 appl_get_stored_records_count(APPL_TIMER_PARAM * appl_timer_param, UCHAR type)
{
    UINT16  seg_index;
    UINT16  oprnd;
    UINT16  index;
    UINT16  count;

    count = 0;

    BT_UNPACK_LE_2_BYTE
    (
        &oprnd,
        &(appl_timer_param->appl_req_param.operand[1])
    );

    for (index = 0; index < APPL_STORED_RECORD_COUNT; index++)
    {
        BT_UNPACK_LE_2_BYTE
        (
            &seg_index,
            &(gls_msrmt_obs_data[gls_msrmt_seq_id_index[index]])
        );

        /*For Greater than or equal Operator (Seg ID) */
        if (seg_index >= oprnd && APPL_GL_RACP_GRTR_HNN_EQL_TO_OPRTR == type)
        {
            count++;
        }
        else if (seg_index >= oprnd && APPL_GL_RACP_LSS_THN_EQL_TO_OPRTR == type)
        {
            count++;
        }
    }
    return count;
}

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
void appl_profile_menu_handler (void)
{
    int choice;
    UINT16 index;
    int record_no;
    int total_records;
    APPL_GLS_USER_FACING_TIME appl_user_facing_time;

    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_device_init_req,
        NULL
    );

    while (1)
    {
        printf("\n");
        printf("%s", gls_menu);
        scanf("%d", &choice);

        switch(choice)
        {
        case 0: /* exit */
            return;

        case 1: /* Refresh */
            break;

        case 2:

            /* Updating the last record Flag field of glucose measurement */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] = 0x0B;

             /** Updating the last record Glucose Concentration Units of
              * glucose measurement to kg/L
              */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) + 12] = 0xB0;

            last_record_update = 1;

            break;

        case 3:
            /* Updating the last record Flag field of glucose measurement */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] = 0x0F;

            /** Updating the last record Glucose Concentration Units of
             * glucose measurement to liters
             */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) + 12] = 0xD0;

            last_record_update = 1;

            break;

        case 4:
            /* Updating the last record Flag field of glucose measurement context */
            gls_msrmt_cntxt_obs_data\
                [(gls_cntxt_msrmt_seq_id_index\
                [APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT - 1]) - 1] = 0xDF;

            last_record_update = 1;

            break;

        case 5:
            /* Updating the last record Flag field of glucose measurement context */
            gls_msrmt_cntxt_obs_data\
                [(gls_cntxt_msrmt_seq_id_index\
                [APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT - 1]) - 1] = 0xFF;

            last_record_update = 1;
            break;

        case 6:
            appl_stored_record_count = 1;

            for (index = 0; index <= APPL_STORED_RECORD_COUNT; index++)
            {
                gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] - 2] = 0x00;
                BT_PACK_LE_2_BYTE
                (
                    &gls_msrmt_obs_data[gls_msrmt_seq_id_index[index]],
                    &appl_seg_index
                );

                appl_seg_index++;

                if (index < (APPL_STORED_RECORD_COUNT -1))
                {
                    BT_PACK_LE_2_BYTE
                    (
                        &(gls_msrmt_cntxt_obs_data[gls_cntxt_msrmt_seq_id_index[index]]),
                        &appl_seg_index
                    );
                }
            }
            break;

        case 7:
            appl_stored_record_count += count;
            appl_activate_gls_records();
            count = APPL_STORED_RECORD_COUNT;
            break;

        case 8:
            /**
             * update the user facing time by +1 hour for 4th
             * glucose measurement record
             */
            appl_user_facing_time.hours = 1;
            appl_user_facing_time.minutes = 0;
            record_no = 4;
            total_records = 3;

            appl_gls_update_time_offset
            (
                &appl_user_facing_time,
                record_no,
                total_records
            );
            break;

        case 9:
            /**
             * update the user facing time by -2 hour for 7th
             * glucose measurement record
             */
            appl_user_facing_time.hours = -2;
            appl_user_facing_time.minutes = 0;
            total_records = 1;
            record_no = 7;

            appl_gls_update_time_offset
            (
                &appl_user_facing_time,
                record_no,
                total_records
            );
            break;

        case 10:
            appl_stored_record_count = 0;
            break;

        case 11:
            appl_stored_record_count = 4;
            appl_activate_gls_records();
            break;

        case 12:
            printf ("1 (Enable), 0 (Disable)\n");
            scanf ("%d", &choice);
            if (choice)
            {
                gls_msrmt_obs_data\
                    [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] |= 0x10;
            }
            else
            {
                gls_msrmt_obs_data\
                    [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] &= 0x0F;
            }
            break;

        default:
            break;
        }
    }
}
#else /* BT_SW_TXP */

void console_app_read (UCHAR * choice, UCHAR * value, UINT16 length)
{
    UINT16 index;
    UCHAR rec_no;
    UCHAR no_of_records;
    APPL_GLS_USER_FACING_TIME appl_user_facing_time;

    /* Check the choice */
    switch (*choice)
    {
        case 0:
            return;

        case 1:
            CONSOLE_OUT("%s", main_options);
            break;

        case 2:
            appl_power_on_device();
            break;

        case 3:
            CONSOLE_OUT("Initiated Disconnection\n");
            /* Disconnect connected device */
            *choice = 1;
            appl_disconnect_device();
        break;

        case 4:
            /* Updating the last record Flag field of glucose measurement */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] = 0x0B;

             /** Updating the last record Glucose Concentration Units of
              * glucose measurement to kg/L
              */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) + 12] = 0xB0;

            last_record_update = 1;
            *choice = 1;
            break;
        case 5:
            /* Updating the last record Flag field of glucose measurement */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] = 0x0F;

            /** Updating the last record Glucose Concentration Units of
             * glucose measurement to liters
             */
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) + 12] = 0xD0;

            last_record_update = 1;
            *choice = 1;
            break;
        case 6:
            /* Updating the last record Flag field of glucose measurement context */
            gls_msrmt_cntxt_obs_data\
                [(gls_cntxt_msrmt_seq_id_index\
                [APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT - 1]) - 1] = 0xDF;

            last_record_update = 1;
            *choice = 1;
            break;
        case 7:
            /* Updating the last record Flag field of glucose measurement context */
            gls_msrmt_cntxt_obs_data\
                [(gls_cntxt_msrmt_seq_id_index\
                [APPL_GLUCOSE_MEASUREMENT_CONTEXT_COUNT - 1]) - 1] = 0xFF;

            last_record_update = 1;
            *choice = 1;
            break;

        case 8:
            appl_stored_record_count = 1;

            for (index = 0; index <= APPL_STORED_RECORD_COUNT; index++)
            {
                gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] - 2] = 0x00;
                BT_PACK_LE_2_BYTE
                (
                    &gls_msrmt_obs_data[gls_msrmt_seq_id_index[index]],
                    &appl_seg_index
                );

                appl_seg_index++;

                if (index < (APPL_STORED_RECORD_COUNT -1))
                {
                    BT_PACK_LE_2_BYTE
                    (
                        &(gls_msrmt_cntxt_obs_data[gls_cntxt_msrmt_seq_id_index[index]]),
                        &appl_seg_index
                    );
                }
            }
            *choice = 1;
            break;
        case 9:
            appl_stored_record_count += count;
            appl_activate_gls_records();
            count = APPL_STORED_RECORD_COUNT;
            *choice = 1;
            break;
        case 10:
            /**
             * update the user facing time by +1 hour for 4th
             * glucose measurement record
             */
            appl_user_facing_time.hours = 1;
            appl_user_facing_time.minutes = 0;
            rec_no = 4;
            no_of_records = 3;

            appl_gls_update_time_offset
            (
                &appl_user_facing_time,
                rec_no,
                no_of_records
            );
            *choice = 1;
            break;
        case 11:
             /**
             * update the user facing time by -2 hour for 7th
             * glucose measurement record
             */
            appl_user_facing_time.hours = -2;
            appl_user_facing_time.minutes = 0;
            no_of_records = 1;
            rec_no = 7;

            appl_gls_update_time_offset
            (
                &appl_user_facing_time,
                rec_no,
                no_of_records
            );
            *choice = 1;
            break;
        case 12:
            appl_stored_record_count = 0;
            *choice = 1;
            break;
        case 13:
            appl_stored_record_count = 4;
            appl_activate_gls_records();
            *choice = 1;
            break;

        case 14:
            CONSOLE_OUT("\r\nGls Msrmt Context in Last Record Enabled..\r\n");
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] |= 0x10;
            *choice = 1;
            break;

        case 15:
            CONSOLE_OUT("\r\nGls Msrmt Context in Last Record Disabled..\r\n");
            gls_msrmt_obs_data\
                [(gls_msrmt_seq_id_index[appl_stored_record_count - 1]) - 1] &= 0x0F;
            *choice = 1;
            break;

        default:
            CONSOLE_OUT("\r\nInvalid Option\n");
            *choice = 1;
            break;
    }
}

const char * appl_get_profile_menu (void)
{
    return main_options;
}
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

void appl_gls_update_time_offset
     (
         APPL_GLS_USER_FACING_TIME * appl_user_facing_time,
         UCHAR record_no,
         UCHAR total_records
     )
{
    INT16 total_mins;
    INT16 prev_rec_timeoffset;
    unsigned int i;

    if(record_no < 2)
    {
        APPL_ERR("\n[ERR] Requested to access non existing previous record\n");
        return;
    }

    /* Get the time offset of the previous record */
    BT_UNPACK_LE_2_BYTE
    (
        &prev_rec_timeoffset,
        &gls_msrmt_obs_data[gls_msrmt_seq_id_index[record_no - 2] + 9]
    );

    total_mins = appl_gls_convert_from_hrs_to_minutes
                 (
                     appl_user_facing_time,
                     prev_rec_timeoffset
                 );
    /** Total Minues is equal to
     *  new user facing time + previous records time offset
     */
    CONSOLE_OUT("\n%d hour + %d minutes = %d minutes\n\n", appl_user_facing_time->hours,
    appl_user_facing_time->minutes, total_mins);

    for (i = 0; i < total_records; i++)
    {
        BT_PACK_LE_2_BYTE
        (
            &gls_msrmt_obs_data[gls_msrmt_seq_id_index[record_no - 1] + 9],
            (UINT16 *)&total_mins
        );
        record_no = record_no + 1;
        total_mins += 1;
    }
}

INT16 appl_gls_convert_from_hrs_to_minutes
      (
          APPL_GLS_USER_FACING_TIME * appl_user_facing_time,
          INT16                       prev_record_time_offset
      )
{
    INT16 minute;
    INT16 total_minutes;

    minute = appl_user_facing_time->hours * 60;

    /**
     *  Total minutes can be taken by adding or subtracting the new user facing time
     *  from the previous record time offset
     */
    total_minutes = minute + appl_user_facing_time->minutes + prev_record_time_offset;

    return total_minutes;
}

void appl_activate_gls_records (void)
{
    UINT16 index;

    for (index = 0; index < appl_stored_record_count; index++)
    {
        gls_msrmt_obs_data[gls_msrmt_seq_id_index[index] - 2] = 0x00;
    }
}



API_RESULT appl_gls_racp_write_handler
           (
                ATT_VALUE         * value
           )
{
    API_RESULT          retval;
    API_RESULT          retv;
    RACP_REQ_INFO       gls_racp_req_info;

    retval = API_SUCCESS;

    if (APPL_RACP_INIT_STATE == appl_gl_operating_state ||
        APPL_GL_RACP_ABRT_RPRTING_REQ_OPCODE == value->val[0])
    {
        if (1 < value->len)
        {
            retv = BT_racp_req_handler
                   (
                       gls_racp_id,
                       value->val,
                       value->len,
                       &gls_racp_req_info
                   );
            if (API_SUCCESS != retv)
            {
                /*
                 * Translating the RACP related error to
                 * GLS related specific error.
                 */
                if ((RACP_ERR_ID | RACP_OPERAND_NOT_SUPPORTED) == retv)
                {
                    retv = (RACP_ERR_ID | RACP_INVALID_OPERAND);
                }

                appl_send_rsp_handler(&gls_racp_req_info, value->len, retv);
            }
        }
        else
        {
            retval = ATT_INVALID_ATTRIBUTE_LEN | APPL_ERR_ID;
        }
    }
    else
    {
        retval = ATT_APPL_PROCEDURE_ALREADY_IN_PROGRESS | APPL_ERR_ID;
    }

    return retval;
}

void appl_send_rsp_handler
    (
        RACP_REQ_INFO   * gls_racp_req_info,
        UINT16            len,
        UINT16            result
    )
{
    API_RESULT retval;
    APPL_TIMER_PARAM    appl_timer_param;

    appl_timer_param.handle = appl_hndl;
    appl_timer_param.rsp_code_val = (UCHAR)result;
    appl_timer_param.appl_req_param.req_op_code = gls_racp_req_info->racp_opcode;
    appl_timer_param.appl_req_param.oprtr = gls_racp_req_info->racp_operator;

    BT_mem_copy
    (
        appl_timer_param.appl_req_param.operand,
        gls_racp_req_info->racp_operand,
        len
    );

    retval = BT_start_timer
             (
                 &timer_handle,
                 APPL_RECORD_TRNSFER_INTERVAL,
                 appl_racp_timer_expiry_handler,
                 &appl_timer_param,
                 sizeof(APPL_TIMER_PARAM)
             );
}

void appl_racp_timer_expiry_handler(void *data, UINT16 datalen)
{
    API_RESULT         retval;
    APPL_TIMER_PARAM * appl_timer_param;

    appl_timer_param = (APPL_TIMER_PARAM *)data;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer(timer_handle);
        APPL_TRC(
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval, timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = appl_gl_racp_send_response(appl_timer_param);
    APPL_TRC("[** ERR **]: Failed to send Glucose Measurement,"
    " reason 0x%04X", retval);
}

API_RESULT appl_gls_access_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;
    APPL_TIMER_PARAM appl_timer_param;

    retval = API_SUCCESS;

    retval = appl_gl_racp_validate_operands
             (
                 req_info->racp_operator,
                 req_info->racp_operand,
                 length - 2
             );

    if (API_SUCCESS != retval)
    {
        appl_send_rsp_handler(req_info, length, retval);
        return retval;
    }

    appl_timer_param.handle = appl_hndl;
    appl_timer_param.rsp_code_val = APPL_GL_RACP_SUCCESS;
    appl_timer_param.appl_req_param.req_op_code = req_info->racp_opcode;
    appl_timer_param.appl_req_param.oprtr = req_info->racp_operator;

    BT_mem_copy
    (
        appl_timer_param.appl_req_param.operand,
        req_info->racp_operand,
        length
    );

    appl_manage_trasnfer(&appl_timer_param);

    return retval;
}

API_RESULT appl_gls_report_stored_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    retval = appl_gls_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
        "[APPL]: Accessing the records failed with result 0x%04X,",retval);
    }
    return retval;
}

API_RESULT appl_gls_report_stored_number_of_records
            (
                RACP_REQ_INFO * req_info,
                UINT16          length
            )
{
    API_RESULT retval;

    retval = appl_gls_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
        "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }
    return retval;
}

API_RESULT appl_gls_delete_stored_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    retval = appl_gls_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
        "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }
    return retval;
}

API_RESULT appl_gls_abort_operation
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    if (APPL_RACP_TRANSFR_ONGOING_STATE == appl_gl_operating_state
        || APPL_RACP_CLR_STATE == appl_gl_operating_state)
    {
        /* Stop Ongoing transfer */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            retval = BT_stop_timer(timer_handle);
            APPL_TRC(
                "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                retval, timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
    }
    appl_gl_operating_state = APPL_RACP_INIT_STATE;

    retval = appl_gls_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
        "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }

    return retval;
}

#endif /* GLS */

