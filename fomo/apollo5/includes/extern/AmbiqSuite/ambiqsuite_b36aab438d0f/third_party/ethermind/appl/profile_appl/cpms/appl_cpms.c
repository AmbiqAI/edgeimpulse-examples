
/**
 *  \file appl_cpms.c
 *
 *  This file contains the Sample Profile application..
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef CPMS

/* --------------------------------------------- External Global Variables */
extern UCHAR appl_hvc_flag;
extern APPL_GAP_ADV_DATA appl_gap_adv_data[APPL_GAP_MAX_ADV_DATA_OPTIONS];
/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
UCHAR cpm_msrmt_data_default[APPL_CPM_MSRMNT_LENGTH];

UCHAR cpm_msrmt_set_cumu_data[4];

UCHAR cpm_broadcast_data[APPL_CPM_BROADCAST_DATA_LENGTH] =
{
    /* Flags */
    0x02, 0x01, 0x04,

    /* Advertising Interval */
    0x03, 0x1A, 0x02, 0x00,

    /* Service Data */
    0x07, 0x16, 0x18, 0x18, 0x00, 0x00, 0x32, 0x00,
};

static UCHAR cpm_msrmt_data[APPL_CPM_MSRMNT_LENGTH] =
{
    /* Flag */
    0xFF, 0x1F,

    /* Instantaneous Power */
    0x32, 0x00,

    /* Pedal Power Balance Present */
    0x00,

    /* Accumulated Torque */
    0x00, 0x00,

    /* Wheel Revolution Data */
    /* Cumulative Wheel Revolutions */
    0x01, 0x00, 0x00, 0x00,

    /* Last Wheel Event Time */
    0x01, 0x00,

    /* Crank Revolution Data */
    /* Cumulative Crank Revolutions */
    0x00, 0x00,

    /* Last Crank Event Time */
    0x00, 0x00,

    /* Extreme Force Magnitudes */
    /* Maximum Force Magnitude */
    0x00, 0x00,

    /*  Minimum Force Magnitude */
    0x00, 0x00,

    /* Extreme Angles */
    0x12, 0x34, 0x56,

    /* Top Dead Spot Angle */
    0x00, 0x00,

    /* Bottom Dead Spot Angle */
    0x00, 0x00,

    /* Accumulated Energy */
    0x00, 0x00
};

static UCHAR cpm_vector_data[APPL_CPM_VECTOR_DATA_LENGTH] =
{
    /* Flag */
    0x1F,

    /* Crank Revolution Data */
    /* Cumulative Crank Revolutions */
    0x01, 0x00,

    /* Last Crank Event Time */
    0x01, 0x00,

    /* First Crank Measurement Angle  */
    0x00, 0x00,

    /* Instantaneous Force Magnitude Array */
    0x00, 0x00,

    /* Instantaneous Torque Magnitude Array */
    0x00, 0x01
};

static UCHAR cpm_supported_sensor_location[APPL_CPM_SUPPORTED_SENSOR_LOC_COUNT] =
{
    0x04, 0x05, 0x06
};

static GATT_DB_HANDLE  appl_batt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BATTERY_INST,
    GATT_CHAR_BATTERY_LVL_INST
};
static GATT_DB_HANDLE  appl_cpm_msrmt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_CPM_INST,
    GATT_CHAR_CPM_MSRMT_INST
};
static GATT_DB_HANDLE  appl_cpm_vector_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_CPM_INST,
    GATT_CHAR_CPM_VECTOR_INST
};
static GATT_DB_HANDLE  appl_cpm_cp_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_CPM_INST,
    GATT_CHAR_CPM_CNTRL_POINT_INST
};

static UCHAR              appl_batt_update_count;
static UCHAR              appl_batt_percent;
static ATT_ATTR_HANDLE    appl_cpm_msrmt_hndl;
static ATT_ATTR_HANDLE    appl_cpm_vector_hndl;
static ATT_ATTR_HANDLE    appl_cpm_cp_hndl;
static UCHAR              appl_cpm_msrmt_cnfgd;
static UCHAR              appl_cpm_vector_cnfgd;
static UCHAR              appl_cpm_cp_cnfgd;
static BT_timer_handle    appl_cpm_msrmt_timer_hndl;
static BT_timer_handle    appl_cpm_vector_timer_hndl;
static BT_timer_handle    cntrl_pnt_timer_handle;
static UCHAR              appl_cpm_count;
static UCHAR              appl_cpm_count_comp_val;
static UCHAR              appl_cpm_msrmt_obs_data_len;

static UINT16             crank_len;
static UINT16             chain_len;
static UINT16             chain_weight;
static UINT16             span_len;
static UCHAR              sampling_rate;
static UCHAR              start_off_comp;
static UINT16             company_id = 0x006A;
static UCHAR              no_of_manufr_data = 0x0a;
static UCHAR              data[5];
static UCHAR              cps_reverse;
UCHAR                     cmd_status;
UCHAR                     calib_date[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
UCHAR                     incorrect_calbn_position;

#ifndef BT_SW_TXP
static const char main_options[] = " \r\n\
======== CPMS : M A I N   M E N U ======== \r\n\
    0.  Exit \r\n\
    1.  Refresh \r\n\
 \r\n\
   10.  Set incorrect position for offset compensation \r\n\
 \r\n\
    Your Option ? \0";
#else /* BT_SW_TXP */
static const char main_options[] = "\r\n\
======== SPS : M A I N   M E N U ========\r\n\
 0. Exit\r\n\
 1. Refresh\r\n\
 2. Power ON Device\r\n\
 3. Disconnect\r\n\
 \r\n\
 4. Set incorrect position for offset compensation\r\n\
 5. Reset correct position for offset compensation\r\n\
 \r\n\
 Your Option ?\r\n";
#endif /* BT_SW_TXP */

/* --------------------------------------------- Functions */

void appl_cpms_init(void)
{
    appl_batt_update_count = 0;
    appl_batt_percent = 100;
    appl_cpm_count_comp_val = 0;

    appl_cpms_server_reinitialize ();

    /* Copy default cycling power measurement data */
    BT_mem_copy
    (
        cpm_msrmt_data_default,
        cpm_msrmt_data,
        APPL_CPM_MSRMNT_LENGTH
    );

    incorrect_calbn_position = BT_FALSE;

}

void appl_cpms_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    API_RESULT        retval;
    UINT16            cli_cnfg;

    cli_cnfg = 0;


    /** Register L2CAP */
    retval = BT_l2cap_register_le_event_cb(appl_cpms_l2cap_callback);

    appl_batt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_cpm_msrmt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_cpm_msrmt_db_handle,
        &appl_cpm_msrmt_hndl
    );
    BT_gatt_db_get_char_cli_cnfg(&appl_cpm_msrmt_db_handle,&value);

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_cpm_msrmt_db_handle,cli_cnfg);
    }

    appl_cpm_vector_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_cpm_vector_db_handle,
        &appl_cpm_vector_hndl
    );

    appl_cpm_cp_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_cpm_cp_db_handle,
        &appl_cpm_cp_hndl
    );
}

void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    APPL_HANDLE    appl_handle;
    API_RESULT     retval;

    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&handle,&ekey_size);

    /* Verify if security requirements are available with the link */
    retval = appl_smp_assert_security
             (
                 &handle.device_id,
                 security,
                 ekey_size
             );

    /* Security requirements satisfied? */
    if (API_SUCCESS != retval)
    {
        /* No. Return */
        return;
    }

    /* Security requirements satisfied, go ahead with data transfer */
    appl_get_handle_from_device_handle(handle.device_id,&appl_handle);

    if (GATT_CLI_CNFG_NOTIFICATION == config)
    {
        if (GATT_CHAR_CPM_MSRMT_INST == handle.char_id)
        {
            /* Start Transfer Timer to Send measurement every second */
            if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_msrmt_timer_hndl)
            {
                BT_stop_timer (appl_cpm_msrmt_timer_hndl);
                appl_cpm_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
            }

            appl_cpm_msrmt_cnfgd = BT_TRUE;

            retval = BT_start_timer
                     (
                         &appl_cpm_msrmt_timer_hndl,
                         APPL_CPM_MSRMT_INTERVAL,
                         appl_timer_expiry_handler,
                         &appl_handle,
                         sizeof (appl_handle)
                     );
            APPL_TRC (
            "[APPL]:[%02X] Started Timer with result 0x%04X, timer handle %p\n",
            appl_handle,retval,appl_cpm_msrmt_timer_hndl);
        }
        else if (GATT_CHAR_CPM_VECTOR_INST == handle.char_id)
        {
            /* Send L2CAP connection parameter update request */
            retval = BT_l2cap_le_connection_param_update_request
                     (
                         &handle.device_id,
                         0x000A,
                         0x000A,
                         0x0000,
                         0x03BB
                     );
        }
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        /* Turn Off Transfer Timer if On */
        if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_msrmt_timer_hndl)
        {
            BT_stop_timer (appl_cpm_msrmt_timer_hndl);
            appl_cpm_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
        }
        appl_cpm_msrmt_cnfgd = BT_FALSE;
    }
    else
    {
        /* Incorrect Configuration */
    }
}

void appl_cpm_vector_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_vector_timer_hndl)
    {
        BT_stop_timer (appl_cpm_vector_timer_hndl);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", appl_cpm_vector_timer_hndl);
        appl_cpm_vector_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &appl_cpm_vector_timer_hndl,
        APPL_CPM_MSRMT_INTERVAL,
        appl_cpm_vector_timer_expiry_handler,
        &fsm_param.handle,
        sizeof(fsm_param.handle)
    );

    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_measurement_ind,
        &fsm_param
    );
}
void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_VALUE          value;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_msrmt_timer_hndl)
    {
        BT_stop_timer (appl_cpm_msrmt_timer_hndl);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", appl_cpm_msrmt_timer_hndl);
        appl_cpm_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &appl_cpm_msrmt_timer_hndl,
        APPL_CPM_MSRMT_INTERVAL,
        appl_timer_expiry_handler,
        &fsm_param.handle,
        sizeof(fsm_param.handle)
    );

    APPL_TRC (
    "[APPL]: Started Timer %p\n",appl_cpm_msrmt_timer_hndl);

    appl_batt_update_count ++;
    value.len = 1;
    value.val = &appl_batt_percent;

    /* Battery is decreased every 5 minutes (5 * 6) times 10 seconds */
    if (30 == appl_batt_update_count)
    {
        appl_batt_percent--;
        APPL_TRC (
        "[APPL]: Battery Percentage 0x%04X\n",appl_batt_percent);

        appl_batt_update_count = 0;

        BT_gatt_db_set_char_val (&appl_batt_db_handle,&value);
        if (0 == appl_batt_percent)
        {
            /* Resetting battery percentage to 100 */
            appl_batt_percent = 100;
        }
    }

    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
         APPL_FSM_ID,
         ev_appl_measurement_ind,
         &fsm_param
    );
}

void appl_cpms_server_reinitialize (void)
{
    if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_msrmt_timer_hndl)
    {
        BT_stop_timer (appl_cpm_msrmt_timer_hndl);
        appl_cpm_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_vector_timer_hndl)
    {
        BT_stop_timer (appl_cpm_vector_timer_hndl);
        appl_cpm_vector_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    if (BT_TIMER_HANDLE_INIT_VAL != cntrl_pnt_timer_handle)
    {
        BT_stop_timer (cntrl_pnt_timer_handle);
        cntrl_pnt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    appl_cpm_msrmt_obs_data_len = APPL_CPM_MSRMNT_LENGTH;
    appl_cpm_count = 0;
    appl_cpm_count_comp_val = 0;
    appl_hvc_flag = BT_FALSE;

    /* Restore the cycling power measurement data if masked */
    if (0xFF != cpm_msrmt_data[0])
    {
        BT_mem_copy
        (
            cpm_msrmt_data,
            cpm_msrmt_data_default,
            APPL_CPM_MSRMNT_LENGTH
        );
    }
}

void appl_send_cpms_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    UINT16 appl_cpm_inst_force_magn_arry;

    if (BT_FALSE != appl_cpm_msrmt_cnfgd)
    {
        APPL_TRC (
        "[0x%02X]:Sending measurement On Handle 0x%04X\n",
        (*handle),appl_cpm_msrmt_hndl);
        hndl_val_param.handle = appl_cpm_msrmt_hndl;

        if (APPL_CPM_MSRMNT_LENGTH ==
            appl_cpm_msrmt_obs_data_len)
        {
            cpm_msrmt_data[0] &= 0x3F;
            cpm_msrmt_data[1] &= 0x00;
            hndl_val_param.value.val = cpm_msrmt_data;
            hndl_val_param.value.len = 17;

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                     );

            if (API_SUCCESS == retval)
            {
                cpm_msrmt_data[13] = 0x40;
                cpm_msrmt_data[14] = 0x1F;
                cpm_msrmt_data[15] = 0x32;
                cpm_msrmt_data[16] = 0x00;
                hndl_val_param.value.val = &cpm_msrmt_data[13];
                hndl_val_param.value.len = 17;

                retval = BT_att_send_hndl_val_ntf
                         (
                             &APPL_GET_ATT_INSTANCE(*handle),
                             &hndl_val_param
                         );

                if ((API_SUCCESS == retval) && (BT_TRUE != cps_reverse))
                {
                    BT_mem_copy
                    (
                        cpm_msrmt_data,
                        cpm_msrmt_data_default,
                        APPL_CPM_MSRMNT_LENGTH
                    );
                }
            }
        }
        else
        {
            hndl_val_param.handle =  appl_cpm_msrmt_hndl;
            hndl_val_param.value.val = cpm_msrmt_data;
            hndl_val_param.value.len = appl_cpm_msrmt_obs_data_len;

            retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );
        }
    }
    else if (BT_FALSE != appl_cpm_vector_cnfgd)
    {
        APPL_TRC (
        "[0x%02X]:Sending measurement On Handle 0x%04X\n",
        (*handle),appl_cpm_msrmt_hndl);

        /**
         * Send Cycling power vector by excluding Instantaneous Torque
         * Magnitude Array value
         */
        cpm_vector_data[0] = 0x07;
        hndl_val_param.handle =  appl_cpm_vector_hndl;
        hndl_val_param.value.val = cpm_vector_data;
        hndl_val_param.value.len = (APPL_CPM_VECTOR_DATA_LENGTH - 2);

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );

        /* Copy the Instantaneous Force Magnitude Array value to the local varibale */
        BT_UNPACK_LE_2_BYTE(&appl_cpm_inst_force_magn_arry, &cpm_vector_data[7]);

        if (API_SUCCESS == retval)
        {
            /**
             * Send Cycling power vector by excluding Instantaneous Force
             * Magnitude Array value
             */
            hndl_val_param.handle = appl_cpm_vector_hndl;
            cpm_vector_data[0] = 0x1B;
            cpm_vector_data[7] = cpm_vector_data[9];
            cpm_vector_data[8] = cpm_vector_data[10];
            hndl_val_param.value.val = cpm_vector_data;
            hndl_val_param.value.len = (APPL_CPM_VECTOR_DATA_LENGTH - 2);

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                     );

            /* Reset the Force Magnitude Array value */
            BT_PACK_LE_2_BYTE(&cpm_vector_data[7], &appl_cpm_inst_force_magn_arry);
        }
    }

    if (appl_cpm_count_comp_val == appl_cpm_count)
    {
        if(BT_TRUE == cps_reverse)
        {
            cpm_msrmt_data[0x07]--;
            if (0xFF == cpm_msrmt_data[0x07])
            {
                cps_reverse = BT_FALSE;
                BT_mem_copy
                (
                    &cpm_msrmt_data[7],
                    &cpm_msrmt_set_cumu_data[0],
                    4
                );
                cps_reverse = BT_TRUE;
            }
        }
        appl_cpm_count_comp_val++;
    }
    else if (2 == appl_cpm_count)
    {
        /* Simulate Cummulative Wheel and Crank Revolutions */
        if (0xFF == cpm_msrmt_data[0x04] && 0xFF == cpm_vector_data[0x01])
        {
            cpm_msrmt_data[0x05]++;
            cpm_vector_data[0x02]++;
        }
        cpm_msrmt_data[0x04]++;
        cpm_vector_data[0x01]++;

        /* Simulate Last Wheel and Crank Event Time */
        if (0x40 == cpm_msrmt_data[0x08])
        {
            cpm_msrmt_data[0x05] = 0;
            cpm_vector_data[0x03] = 0;
        }
        cpm_msrmt_data[0x08]++;
        cpm_vector_data[0x03]++;

        appl_cpm_count = 0;
    }
    appl_cpm_count++;
    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "[** ERR **]: Failed to send measurement, reason 0x%04X",
        retval);
    }
}

void appl_cpm_cntrl_point_timer_handle (void * data, UINT16 datalen)
{
    /* Send Indication */
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    hndl_val_param.handle = appl_cpm_cp_hndl;
    hndl_val_param.value.val = ((APPL_CPM_CNTRL_POINT_RSP_PARAM *)data)->rsp_param.val;
    hndl_val_param.value.len = ((APPL_CPM_CNTRL_POINT_RSP_PARAM *)data)->rsp_param.len;

    if (APPL_CPM_REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE ==
        hndl_val_param.value.val[1])
    {
        hndl_val_param.value.len = 3 +
            APPL_CPM_SUPPORTED_SENSOR_LOC_COUNT;
        BT_mem_copy
        (
            (hndl_val_param.value.val + 3),
            &cpm_supported_sensor_location,
            APPL_CPM_SUPPORTED_SENSOR_LOC_COUNT
        );
    }

    if (BT_TIMER_HANDLE_INIT_VAL != cntrl_pnt_timer_handle)
    {
        BT_stop_timer (cntrl_pnt_timer_handle);
        cntrl_pnt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_att_send_hndl_val_ind
             (
                 &APPL_GET_ATT_INSTANCE(((APPL_CPM_CNTRL_POINT_RSP_PARAM *)data)->appl_handle),
                 &hndl_val_param
              );

    if (API_FAILURE != retval)
    {
        appl_hvc_flag = BT_TRUE;
    }
}

void appl_cpm_cntrl_point_cnfg_handler (GATT_DB_HANDLE  *handle,UINT16 config)
{
    if (GATT_CLI_CNFG_INDICATION == config)
    {
        appl_cpm_cp_cnfgd = BT_TRUE;
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        appl_cpm_cp_cnfgd = BT_FALSE;
    }
}

API_RESULT appl_cpm_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           )
{
    API_RESULT                        retval;
    APPL_CPM_CNTRL_POINT_RSP_PARAM    param;
    UINT16     temp_data;

    retval = API_SUCCESS;
    param.rsp_param.val = &data[0];
    param.rsp_param.len = 3;

    appl_get_handle_from_device_handle
    (
        handle->device_id,
        &param.appl_handle
    );

    if (BT_FALSE == appl_cpm_cp_cnfgd)
    {
        retval = ATT_CCD_IMPROPERLY_CONFIGURED | APPL_ERR_ID;
    }
    else if (BT_TRUE != appl_hvc_flag)
    {
        data[0] = APPL_CPM_RESPONSE_CODE;
        data[1] = value->val[0];
        data[2] = APPL_CPM_SUCCESS;

        if (BT_TRUE == APPL_CPM_SUPPORTED_OPCODE(value->val[0]))
        {
            switch(value->val[0])
            {
            case APPL_CPM_SET_CUMMULATIVE_VALUE_OPCODE:
                if (value->len >= (APPL_CPM_CUMMULATIVE_VALUE_LENGTH + 1))
                {
                    BT_mem_copy
                    (
                        cpm_msrmt_data+7,
                        &(value->val[1]),
                        APPL_CPM_CUMMULATIVE_VALUE_LENGTH
                    );
                    appl_cpm_count = 0;
                    if (0x05 == value->val[1])
                    {
                        cps_reverse = BT_TRUE;
                    }
                    data[2] = APPL_CPM_SUCCESS;
                    BT_mem_copy
                    (
                        &cpm_msrmt_set_cumu_data[0],
                        &cpm_msrmt_data[7],
                        4
                    );
                }
                else
                {
                    data[2] = APPL_CPM_INVALID_PARAMETER;
                }
                break;
            case APPL_CPM_UPDATE_SENSOR_LOCATION_OPCODE:
                {
                    if (value->len == 2)
                    {
                        GATT_DB_HANDLE   db_handle;
                        ATT_VALUE        att_value;
                        UCHAR            count;
                        count = 0;
                        data[2] = APPL_CPM_INVALID_PARAMETER;

                        while (count < APPL_CPM_SUPPORTED_SENSOR_LOC_COUNT)
                        {
                            if (value->val[1] == cpm_supported_sensor_location[count])
                            {
                                att_value.len = 1;
                                att_value.val = value->val+1;

                                db_handle = (*handle);
                                db_handle.char_id = GATT_CHAR_SNSR_LOC_INST;
                                BT_gatt_db_set_char_val (&db_handle,&att_value);
                                data[2] = APPL_CPM_SUCCESS;
                                break;
                            }
                            count ++;
                        }
                    }
                    else
                    {
                        data[2] = APPL_CPM_INVALID_PARAMETER;
                    }
                }
                break;
            case APPL_CPM_REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE:
                if (value->len != 1)
                {
                    data[2] = APPL_CPM_INVALID_PARAMETER;
                }
                break;
            case APPL_CPM_SET_CRANK_LENGTH_OPCODE:
                APPL_CPM_VALIDATE_AND_SET_CRANK_LEN_PARAM
                (
                    &value->val[1],
                    crank_len,
                    (value->len - 1),
                    data[2]
                );
                break;
            case APPL_CPM_REQ_CRANK_LENGTH_OPCODE:
                BT_PACK_LE_2_BYTE(&data[3],&crank_len);
                param.rsp_param.len = 5;
                break;
            case APPL_CPM_SET_CHAIN_LENGTH_OPCODE:
                APPL_CPM_VALIDATE_AND_SET_CHAIN_LEN_PARAM
                (
                    &value->val[1],
                    chain_len,
                    (value->len - 1),
                    data[2]
                );
                break;
            case APPL_CPM_REQ_CHAIN_LENGTH_OPCODE:
                BT_PACK_LE_2_BYTE(&data[3],&chain_len);
                param.rsp_param.len = 5;
                break;
            case APPL_CPM_SET_CHAIN_WEIGHT_OPCODE:
                APPL_CPM_VALIDATE_AND_SET_CHAIN_WEIGHT_PARAM
                (
                    &value->val[1],
                    chain_weight,
                    (value->len - 1),
                    data[2]
                );
                break;
            case APPL_CPM_REQ_CHAIN_WEIGHT_OPCODE:
                BT_PACK_LE_2_BYTE(&data[3],&chain_weight);
                param.rsp_param.len = 5;
                break;
            case APPL_CPM_SET_SPAN_LENGTH_OPCODE:
                APPL_CPM_VALIDATE_AND_SET_SPAN_LEN_PARAM
                (
                    &value->val[1],
                    span_len,
                    (value->len - 1),
                    data[2]
                )
                break;
            case APPL_CPM_REQ_SPAN_LENGTH_OPCODE:
                BT_PACK_LE_2_BYTE(&data[3],&span_len);
                param.rsp_param.len = 5;
                break;

            case APPL_CPM_REQ_FACTRY_CALIB_DATE_OPCODE:
                BT_mem_copy(&data[3], &calib_date[0], 7);
                param.rsp_param.len = 10;
                break;

            case APPL_CPM_REQ_SAMPLING_RATE_OPCODE:
                BT_PACK_LE_1_BYTE(&data[3],&sampling_rate);
                param.rsp_param.len = 4;
                break;

            case APPL_CPM_STRT_OFFSET_COMPNSTN_OPCODE:
                BT_PACK_LE_2_BYTE(&data[3],&start_off_comp);
                param.rsp_param.len = 5;
                break;

            case APPL_MASK_CPM_CHAR_CONTENT_OPCODE:
                if (value->len == 3)
                {
                    if (value->val[2] != 0x01)
                    {
                        data[2] = APPL_CPM_INVALID_PARAMETER;
                    }
                    else
                    {
                        for (temp_data = 0;
                         temp_data < APPL_NUM_OF_CPM_MSRMT_MASK_FIELD;
                         temp_data++)
                        {
                            if (0x01 == (value->val[1] & 0x01) || (value->val[2] & 0x01))
                            {
                                switch (temp_data)
                                {
                                case 0:
                                    if (0x01 == (cpm_msrmt_data[0] & 0x01))
                                    {
                                        cpm_msrmt_data[0] &= 0xFC;
                                        appl_cpm_msrmt_obs_data_len -= 1;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[4],
                                            &cpm_msrmt_data[5],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;
                                case 1:
                                    if (0x04 == (cpm_msrmt_data[0] & 0x04))
                                    {
                                        cpm_msrmt_data[0] &= 0xF3;
                                        appl_cpm_msrmt_obs_data_len -= 2;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[5],
                                            &cpm_msrmt_data[7],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                case 2:
                                    if (0x10 == (cpm_msrmt_data[0] & 0x10))
                                    {
                                        cpm_msrmt_data[0] &= 0xEF;
                                        appl_cpm_msrmt_obs_data_len -= 6;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[7],
                                            &cpm_msrmt_data[13],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                case 3:
                                    if (0x20 == (cpm_msrmt_data[0] & 0x20))
                                    {
                                        cpm_msrmt_data[0] &= 0xDF;
                                        appl_cpm_msrmt_obs_data_len -= 4;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[13],
                                            &cpm_msrmt_data[17],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                case 4:
                                    if (0x40 == (cpm_msrmt_data[0] & 0x40))
                                    {
                                        cpm_msrmt_data[0] &= 0x3F;
                                        appl_cpm_msrmt_obs_data_len -= 4;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[17],
                                            &cpm_msrmt_data[21],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                 case 5:
                                    if (0x01 == (cpm_msrmt_data[1] & 0x01))
                                    {
                                        cpm_msrmt_data[1] &= 0x1E;
                                        appl_cpm_msrmt_obs_data_len -= 3;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[21],
                                            &cpm_msrmt_data[24],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                case 6:
                                    if (0x02 == (cpm_msrmt_data[1] & 0x02))
                                    {
                                        cpm_msrmt_data[1] &= 0x1D;
                                        appl_cpm_msrmt_obs_data_len -= 2;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[24],
                                            &cpm_msrmt_data[26],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                case 7:
                                    if (0x04 == (cpm_msrmt_data[1] & 0x04))
                                    {
                                        cpm_msrmt_data[1] &= 0x1B;
                                        appl_cpm_msrmt_obs_data_len -= 2;
                                        BT_mem_copy
                                        (
                                            &cpm_msrmt_data[26],
                                            &cpm_msrmt_data[28],
                                            appl_cpm_msrmt_obs_data_len
                                        );
                                    }
                                    break;

                                case 8:
                                    if (0x08 == (cpm_msrmt_data[1] & 0x08))
                                    {
                                        cpm_msrmt_data[1] &= 0x07;
                                        appl_cpm_msrmt_obs_data_len -= 2;
                                    }
                                    break;

                                default:
                                    break;
                                }
                            }
                            value->val[1] = value->val[1] >> 1;
                        }
                    }
                }
                else
                {
                    data[2] = APPL_CPM_INVALID_PARAMETER;
                }
                break;
            case APPL_CPM_STRT_ENHCED_OFFSET_COMPNSTN_OPCODE:
                if (incorrect_calbn_position == BT_TRUE)
                {
                    data[2] = APPL_CPM_OPERATION_FAILED;
                    data[3] = APPL_CPM_INCORRECT_CALBN_POSITION;
                    param.rsp_param.len = 4;
                }
                else
                {
                    BT_PACK_LE_2_BYTE(&data[3], &start_off_comp);
                    BT_PACK_LE_2_BYTE(&data[5], &company_id);
                    BT_PACK_LE_1_BYTE(&data[7], &no_of_manufr_data);
                    param.rsp_param.len = 8;
                }
                break;

            default:
                data[2] = APPL_CPM_OPCODE_NOT_SUPPORTED;
                break;
            }

        }
        else
        {
            data[2] = APPL_CPM_OPCODE_NOT_SUPPORTED;
        }

        BT_start_timer
        (
             &cntrl_pnt_timer_handle,
             1,
             appl_cpm_cntrl_point_timer_handle,
             &param,
             sizeof (param)
         );
    }
    else
    {
        retval = ATT_PROCEDURE_ALREADY_IN_PROGRESS | APPL_ERR_ID;
    }

    return retval;
}


#ifdef APPL_GAP_BROADCASTER_SUPPORT
void appl_cpm_manage_brodcast (UINT16 config)
{
    if (GATT_SER_CNFG_BROADCAST == config)
    {
        APPL_GAP_SET_ADV_TYPE(0x03);
        APPL_GAP_SET_ADV_PARAM_ID(0x01);

        BT_hci_le_set_advertising_data
        (
            APPL_CPM_BROADCAST_DATA_LENGTH,
            cpm_broadcast_data
        );
    }
    else
    {
        appl_gap_stop_advertising();
    }

}
#endif /* APPL_GAP_BROADCASTER_SUPPORT */

void appl_cpms_l2cap_callback
     (
         DEVICE_HANDLE * handle,
         UCHAR           event_type,
         UCHAR         * event_data,
         UINT16          event_datalen
     )
{
    UINT16 length;
    UINT16 result;
    UCHAR   * bd_addr;
    UCHAR     bd_addr_type;
    BT_DEVICE_ADDR peer_bd_addr;

    device_queue_get_remote_addr (handle,&peer_bd_addr);
    bd_addr = BT_BD_ADDR(&peer_bd_addr);
    bd_addr_type = BT_BD_ADDR_TYPE (&peer_bd_addr);

    if (L2CAP_CONNECTION_UPDATE_RESPONSE_EVENT == event_type)
    {
        appl_unpack_2_byte_param(length, &event_data[0]);
        appl_unpack_2_byte_param(result, &event_data[2]);

#ifdef APPL_MENU_OPS
        CONSOLE_OUT("L2CAP Connection Update Response. Result: 0x%04X\n",
            result);
#endif /* APPL_MENU_OPS */

        APPL_TRC (
        "Received : L2CAP_CONNECTION_UPDATE\n");
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n",bd_addr[0],
        bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        APPL_TRC (
        "\tbd_addr_type : %02X\n", bd_addr_type);
        APPL_TRC (
        "\tLength       : %04X\n", length);
        APPL_TRC (
        "\tResult       : %04X\n", result);

        appl_manage_cpm_vector_msrmnt(handle, result);
    }
}

void appl_manage_cpm_vector_msrmnt (DEVICE_HANDLE * handle, UINT16 result)
{
    API_RESULT             retval;
    APPL_HANDLE            appl_handle;
    ATT_ERROR_RSP_PARAM    err_param;
    ATT_VALUE              value;
    UINT16                 cli_cnfg;

    appl_get_handle_from_device_handle(*handle,&appl_handle);

    /* If success send write response and start timer for sending notifications */
    if (0 == result)
    {
        value.val = data;
        cli_cnfg = GATT_CLI_CNFG_NOTIFICATION;
        BT_PACK_LE_2_BYTE(value.val, &cli_cnfg);
        value.len = 2;

        /* Set CCCD for CPM Vector*/
        retval = gatt_db_access_val_by_db_handle
                 (
                     &appl_cpm_vector_db_handle,
                     &value,
                     GATT_CLIENT_CONFIG,
                     GATT_DB_UPDATE
                 );

        /* Start Transfer Timer to Send measurement every second */
        if (BT_TIMER_HANDLE_INIT_VAL != appl_cpm_msrmt_timer_hndl)
        {
            BT_stop_timer (appl_cpm_msrmt_timer_hndl);
            appl_cpm_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
        }

        appl_cpm_vector_cnfgd = BT_TRUE;

        retval = BT_start_timer
                 (
                     &appl_cpm_vector_timer_hndl,
                     APPL_CPM_VECTOR_INTERVAL,
                     appl_timer_expiry_handler,
                     &appl_handle,
                     sizeof (appl_handle)
                 );
        APPL_TRC (
        "[APPL]:[%02X] Started Timer with result 0x%04X, timer handle %p\n",
        appl_handle,retval,appl_cpm_vector_timer_hndl);

        /* Send Write Response */
        retval = BT_att_write_rsp
                 (
                     &APPL_GET_ATT_INSTANCE(appl_handle)
                 );
    }
    /* Else send error response with reason set to Application error code */
    else
    {
        err_param.error_code = APPL_CPM_INAPPROPRIATE_CONNECTION_PARAMETERS;
        /* CCCD handle = Value handle + 1 */
        err_param.handle = appl_cpm_vector_hndl + 1;
        err_param.op_code = ATT_WRITE_REQ;

        retval = BT_att_send_error_rsp
                 (
                     &APPL_GET_ATT_INSTANCE(appl_handle),
                     &err_param
                 );
    }

}

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
void appl_profile_menu_handler(void)
{
    int choice;

    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_device_init_req,
        NULL
    );

    CONSOLE_OUT("\n");
    CONSOLE_OUT("%s", main_options);
    CONSOLE_IN("%d", &choice);

    switch (choice)
    {
    case 0:
        CONSOLE_OUT("\nExiting BlueLitE Menu Based Application ... \n\n");
        exit(0);

    case 1:
        CONSOLE_OUT("\nRefreshing ...\r\n\n");
        break;

    case 10:
        CONSOLE_OUT("\nSetting CPM server in an incorrect position for"
                    "offset compensation... \n");
        incorrect_calbn_position = BT_TRUE;
        break;
    }
}

#else /* BT_SW_TXP */

void console_app_read(UCHAR * choice, UCHAR * value, UINT16 length)
{
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
        CONSOLE_OUT("Setting CPM server in an incorrect position for offset compensation... \n");
        incorrect_calbn_position = BT_TRUE;
        *choice = 1;
        break;
		
    case 5:
        CONSOLE_OUT("Re-setting CPM server to correct position for offset compensation... \n");
        incorrect_calbn_position = BT_FALSE;
        *choice = 1;
        break;		

    default:
        CONSOLE_OUT("\r\nInvalid Option\n");
        *choice = 1;
        break;
    }
}

const char * appl_get_profile_menu(void)
{
    return main_options;
}
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

#endif /* CPMS */

