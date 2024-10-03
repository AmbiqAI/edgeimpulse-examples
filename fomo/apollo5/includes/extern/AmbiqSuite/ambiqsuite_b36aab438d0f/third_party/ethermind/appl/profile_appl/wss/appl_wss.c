
/**
 *  \file appl_wss.c
 *
 *  This file contains the Weight Scale Profile application..
 */

/*
 *  Copyright (C) 2012. MindTree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef WSS
/* --------------------------------------------- External Global Variables */
extern UCHAR appl_hvc_flag;

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
static UCHAR ws_obs_data[APPL_WS_MSRMT_LENGTH] =
{
    /* Flag */
    0x0E,

    /* Weight - SI */
    0x64, 0x19,

    /* Time Stamp */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00,

    /* User ID */
    0xFF,

    /* BMI */
    0x00, 0x00,

    /* Height - SI */
    0x00, 0x00
};

static UCHAR bc_obs_data[APPL_BC_MSRMT_LENGTH] =
{
    /* Flag */
    0x1F, 0xFE,

    /* Body Fat Percentage */
    0x0F, 0x00,

    /* Time Stamp */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* User ID */
    0xFF,

    /* Basal Metabolism */
    0x0F, 0x00,

    /* Muscle Percentage */
    0x0F, 0x00,

    /* Muscle Mass - Kilograms */
    0x0F, 0x00,

    /* Fat Free Mass - Kilograms */
    0x0F, 0x00,

    /* Soft Lean Mass - Kilograms */
    0x0F, 0x00,

    /* Body Water Mass - Kilograms */
    0x0F, 0x00,

    /* Impedance */
    0x00, 0x00,

    /* Weight - Kilograms */
    0x64, 0x0F,

    /* Height - Meters */
    0x0F, 0x00
};

UCHAR appl_bcs_faeture_val[APPL_BC_FAETURE] =
{
    0xFF,

    0x21,

    0x01,

    0x00
};

static GATT_DB_HANDLE  appl_batt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BATTERY_INST,
    GATT_CHAR_BATTERY_LVL_INST
};

static GATT_DB_HANDLE appl_wss_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_WGT_SCALE_INST,
    GATT_CHAR_WGT_MSRMNT_INST
};

static GATT_DB_HANDLE appl_bcs_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BDY_CMP_INST,
    GATT_CHAR_BC_MSRMNT_INST
};

static GATT_DB_HANDLE appl_uds_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_USER_DATA_INST,
    GATT_CHAR_USER_CNTRL_PNT_INST
};

static GATT_DB_HANDLE appl_dcic_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_USER_DATA_INST,
    GATT_CHAR_DB_CHNG_INC_INST
};

static BT_timer_handle    appl_ws_msrmt_timer_hndl;
static BT_timer_handle    appl_bc_msrmt_timer_hndl;
static BT_timer_handle    appl_db_chng_timer_hndl;
static BT_timer_handle    appl_cntrl_pnt_timer_handle;
static UCHAR              appl_batt_update_count;
static UCHAR              appl_batt_percent;
static ATT_ATTR_HANDLE    appl_wm_hndl;
static ATT_ATTR_HANDLE    appl_bcm_hndl;
static ATT_ATTR_HANDLE    appl_bcs_feature_hndl;
static ATT_ATTR_HANDLE    appl_dbc_hndl;
static ATT_ATTR_HANDLE    appl_user_cntrl_pnt_hndl;
static UCHAR              appl_ws_msrmt_cnfgd;
static UCHAR              appl_bc_msrmt_cnfgd;
static UCHAR              appl_db_chng_cnfgd;
static UCHAR              appl_user_cntrl_pnt_cnfgd;
static UINT32             appl_db_chnge_incmnt;
APPL_HANDLE               appl_hndl;

UCHAR                     user_idx;
UCHAR                     appl_remaining_bcs_obs_data;

#ifndef BT_SW_TXP
static const char main_options[] = " \r\n\
======== WSS : M A I N   M E N U ======== \r\n\
    0.  Exit \r\n\
    1.  Refresh \r\n\
 \r\n\
   10. Send WSS Measurement \r\n\
   11. Send BCS Measurement \r\n\
   12. Send Data change Measurement \r\n\
 \r\n\
   Your Option ?\r\n";
#else /* BT_SW_TXP */
static const char main_options[] = "\r\n\
======== WSS : M A I N   M E N U ========\r\n\
 0. Exit\r\n\
 1. Refresh\r\n\
 2. Power ON Device\r\n\
 3. Disconnect\r\n\
 4. Send WSS Measurement\r\n\
 5. Send BCS Measurement\r\n\
 6. Send Data change Measurement\r\n\
 \r\n\
 Your Option ?\r\n";
#endif /* BT_SW_TXP */

APPL_UDS_CP appl_uds_cp[APPL_MAX_NEW_USER_SUPPORT];

/* --------------------------------------------- Functions */

void appl_wss_init(void)
{
    appl_batt_update_count = 0;
    appl_batt_percent = 100;

    appl_uds_cp_structure_init();

    appl_wss_server_reinitialize ();
}

void appl_uds_cp_structure_init()
{
    UINT32 count;

    count = 0;
    /* Initialize appl_uds_cp structure */
    while (count < APPL_MAX_NEW_USER_SUPPORT)
    {
        APPL_UDS_CP_STRUCT_INIT(count);
        count++;
    }
}

void appl_wss_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    UCHAR             val_arr[2];
    UINT16            cli_cnfg;
    API_RESULT        retval;
    cli_cnfg = 0;

    value.len = 2;
    value.val = &val_arr[0];

    appl_wss_db_handle.device_id  = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_bcs_db_handle.device_id  = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_uds_db_handle.device_id  = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_dcic_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    appl_hndl = (* appl_handle);

    /* Update User Control Point Handle */
    appl_uds_db_handle.char_id = GATT_CHAR_USER_CNTRL_PNT_INST;
    BT_gatt_db_get_char_val_hndl (&appl_uds_db_handle,&appl_user_cntrl_pnt_hndl);

    BT_gatt_db_get_char_val_hndl (&appl_dcic_db_handle,&appl_dbc_hndl);
    BT_gatt_db_get_char_val
    (
        &appl_dcic_db_handle,
        &value
    );

    BT_PACK_LE_4_BYTE(&appl_db_chnge_incmnt,&value.val);

    BT_gatt_db_get_char_val_hndl (&appl_wss_db_handle,&appl_wm_hndl);
    BT_gatt_db_get_char_cli_cnfg(&appl_wss_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,val_arr);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg,APPL_GET_DEVICE_HANDLE((*appl_handle)));

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_wss_db_handle,cli_cnfg);
    }

    appl_bcs_db_handle.char_id = GATT_CHAR_BC_MSRMNT_INST;
    BT_gatt_db_get_char_val_hndl (&appl_bcs_db_handle,&appl_bcm_hndl);
    BT_gatt_db_get_char_cli_cnfg(&appl_bcs_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,val_arr);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg,APPL_GET_DEVICE_HANDLE((*appl_handle)));

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_bcs_db_handle,cli_cnfg);
    }

/* This macro should not be enabled if testing only BCS service */
#ifdef BCS_SUPPORTED_BY_WSS
    /* If the Body Composition Service is supported by the Weight Scale,
     * the Height and Weight fields shall not be present in the Body
     * Composition Measurement characteristic
     */
    value.val = appl_bcs_faeture_val;
    value.len = 4;

    appl_bcs_db_handle.char_id = GATT_CHAR_BC_FEATURE_INST;

    BT_gatt_db_get_char_val_hndl (&appl_bcs_db_handle,&appl_bcs_feature_hndl);

    retval = BT_gatt_db_set_char_val(&appl_bcs_db_handle, &value);

#endif /* BCS_SUPPORTED_BY_WSS */
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
    retval= appl_get_handle_from_device_handle(handle.device_id,&appl_handle);

    if (API_SUCCESS != retval)
    {
        return;
    }

    if (GATT_CLI_CNFG_INDICATION == config)
    {
        if (GATT_CHAR_WGT_MSRMNT_INST == handle.char_id)
        {
            appl_ws_msrmt_cnfgd = BT_TRUE;
        }
        else if (GATT_CHAR_BC_MSRMNT_INST == handle.char_id)
        {
            appl_bc_msrmt_cnfgd = BT_TRUE;
        }
        else if (GATT_CHAR_USER_CNTRL_PNT_INST == handle.char_id)
        {
            appl_user_cntrl_pnt_cnfgd = BT_TRUE;
        }
        else
        {
            /* Invalid Characteristics Instance */
        }
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        if (GATT_CHAR_WGT_MSRMNT_INST == handle.char_id)
        {
            appl_ws_msrmt_cnfgd = BT_FALSE;
        }
        else if (GATT_CHAR_BC_MSRMNT_INST == handle.char_id)
        {
            appl_bc_msrmt_cnfgd = BT_FALSE;
        }
        else if (GATT_CHAR_DB_CHNG_INC_INST == handle.char_id)
        {
            appl_db_chng_cnfgd = BT_TRUE;
            appl_db_chnge_incmnt++;
        }
        else if (GATT_CHAR_USER_CNTRL_PNT_INST == handle.char_id)
        {
            appl_user_cntrl_pnt_cnfgd = BT_FALSE;
        }
        else
        {
            /* Invalid Characteristics Instance */
        }
    }
    else
    {
        /* Incorrect Configuration */
    }
}

void appl_ws_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_VALUE          value;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    if (BT_TIMER_HANDLE_INIT_VAL != appl_ws_msrmt_timer_hndl)
    {
        BT_stop_timer (appl_ws_msrmt_timer_hndl);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", appl_ws_msrmt_timer_hndl);
        appl_ws_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &appl_ws_msrmt_timer_hndl,
        APPL_WS_MSRMT_INTERVAL,
        appl_ws_timer_expiry_handler,
        &fsm_param.handle,
        sizeof(fsm_param.handle)
    );

    APPL_TRC (
    "[APPL]: Started Timer %p\n",appl_ws_msrmt_timer_hndl);

    appl_batt_update_count ++;
    value.len = 1;
    value.val = &appl_batt_percent;

    /* Battery is decreased every 5 minutes (5 * 6) times 10 seconds */
    if (appl_batt_update_count == 30)
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

void appl_db_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    if (BT_TIMER_HANDLE_INIT_VAL != appl_db_chng_timer_hndl)
    {
        BT_stop_timer (appl_db_chng_timer_hndl);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", appl_db_chng_timer_hndl);
        appl_db_chng_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &appl_bc_msrmt_timer_hndl,
        APPL_DCIC_INTERVAL,
        appl_db_timer_expiry_handler,
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

void appl_wss_server_reinitialize (void)
{
    if (BT_TIMER_HANDLE_INIT_VAL != appl_ws_msrmt_timer_hndl)
    {
        BT_stop_timer (appl_ws_msrmt_timer_hndl);
        appl_ws_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    if (BT_TIMER_HANDLE_INIT_VAL != appl_bc_msrmt_timer_hndl)
    {
        BT_stop_timer (appl_bc_msrmt_timer_hndl);
        appl_bc_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }
    if (BT_TIMER_HANDLE_INIT_VAL != appl_cntrl_pnt_timer_handle)
    {
        BT_stop_timer (appl_cntrl_pnt_timer_handle);
        appl_cntrl_pnt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    appl_hvc_flag = BT_FALSE;
}

void appl_send_wss_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    ATT_VALUE              value;

    if (BT_TRUE == appl_ws_msrmt_cnfgd)
    {
        APPL_TRC (
        "[0x%02X]:Sending Weight Measurement On Handle 0x%04X\n",
        (*handle),appl_wm_hndl);

        appl_uds_db_handle.char_id = GATT_CHAR_USER_INDEX_INST;

        retval = BT_gatt_db_get_char_val
                 (
                     &appl_uds_db_handle,
                     &value
                 );

    /* Set the user index field in WSS measurement with the user index
     * available in the UDS user index characteristic
     */
        ws_obs_data[APPL_USER_IDX_OFFSET_IN_WSS_MSRNT] = (*value.val);

        hndl_val_param.handle =  appl_wm_hndl;
        hndl_val_param.value.val = ws_obs_data;
        hndl_val_param.value.len = APPL_WS_MSRMT_LENGTH;

        retval = BT_att_send_hndl_val_ind
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );
    }
}

void appl_send_bcs_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    ATT_VALUE             value;

    if (BT_TRUE == appl_bc_msrmt_cnfgd)
    {
        APPL_TRC (
        "[0x%02X]:Sending Body Composition Measurement On Handle 0x%04X\n",
        (*handle),appl_bcm_hndl);

        appl_uds_db_handle.char_id = GATT_CHAR_USER_INDEX_INST;

        BT_gatt_db_get_char_val
        (
            &appl_uds_db_handle,
            &value
        );

        /* Assign the User Index val received from the User Index characteristic
         * to User Index field in BCS measurement characteristic
         */
        bc_obs_data[APPL_USER_IDX_OFFSET_IN_BCS_MSRNT] = (*value.val);

        hndl_val_param.handle = appl_bcm_hndl;

        appl_remaining_bcs_obs_data = APPL_MAX_BC_MEASUREMENT_DATA;

        bc_obs_data[0] = 0x7F;
        bc_obs_data[1] = 0x10;
        hndl_val_param.value.val = bc_obs_data;

        appl_send_bcs_remaining_data(&hndl_val_param);
        retval = APPL_SEND_BCS_HNDL_VAL
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );

        BT_start_timer
        (
            &appl_ws_msrmt_timer_hndl,
            (UINT16)APPL_BC_MSRMT_INTERVAL,
            appl_bcs_timer_expiry_handler,
            &handle,
            sizeof (handle)
        );
    }
}

void appl_bcs_timer_expiry_handler (void *data_param, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;

    API_RESULT retval;

    retval = API_SUCCESS;

    fsm_param.handle = (*((APPL_HANDLE *)data_param));

    if (BT_TIMER_HANDLE_INIT_VAL != appl_ws_msrmt_timer_hndl)
    {
        BT_stop_timer (appl_ws_msrmt_timer_hndl);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", appl_ws_msrmt_timer_hndl);
        appl_ws_msrmt_timer_hndl = BT_TIMER_HANDLE_INIT_VAL;
    }

    hndl_val_param.handle = appl_bcm_hndl;

    if (BT_TRUE != appl_bc_msrmt_cnfgd)
    {
        APPL_TRC (
        "[0x%02X]:Sending Body Composition Measurement On Handle 0x%04X\n",
        (appl_hndl),appl_bcm_hndl);

        return;
    }

    if (appl_remaining_bcs_obs_data != 0)
    {
        bc_obs_data[16] = 0x80;
        bc_obs_data[17] = 0x1F;
        hndl_val_param.value.val = &bc_obs_data[16];

        appl_send_bcs_remaining_data(&hndl_val_param);

        retval = APPL_SEND_BCS_HNDL_VAL
                 (
                     &APPL_GET_ATT_INSTANCE(appl_hndl),
                     &hndl_val_param
                  );
    }
}

void appl_send_bcs_remaining_data(ATT_HANDLE_VALUE_PAIR * hndl_val_params)
{
    if (appl_remaining_bcs_obs_data < (ATT_DEFAULT_MTU - 3))
    {
        hndl_val_params->value.len = appl_remaining_bcs_obs_data;
    }
    else
    {
        hndl_val_params->value.len = (ATT_DEFAULT_MTU - 3);
    }

    appl_remaining_bcs_obs_data = (appl_remaining_bcs_obs_data - (hndl_val_params->value.len));
}

void appl_send_dn_chng_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    if (BT_TRUE == appl_db_chng_cnfgd)
    {
        APPL_TRC (
        "[0x%02X]:Sending Database Change Increment On Handle 0x%04X\n",
        (*handle),appl_dbc_hndl);

        hndl_val_param.handle =  appl_dbc_hndl;
        hndl_val_param.value.val = (UCHAR *)appl_db_chnge_incmnt;
        hndl_val_param.value.len = APPL_DCIC_LENGTH;

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );
    }
}

void appl_cntrl_point_timer_expiry_handler (void * data, UINT16 datalen)
{
    /* Send Indication */
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 rsp_val[4];

    rsp_val[0] = APPL_RESPONSE_CODE;
    rsp_val[1] = ((APPL_USER_CNTRL_POINT_RSP_PARAM *)data)->req_code;
    rsp_val[2] = ((APPL_USER_CNTRL_POINT_RSP_PARAM *)data)->rsp_code;

    hndl_val_param.handle = appl_user_cntrl_pnt_hndl;
    hndl_val_param.value.val = &rsp_val[0];
    hndl_val_param.value.len = 3;

    if ((APPL_REGISTER_NEW_USER_REQ == rsp_val[1]) && (APPL_UCP_SUCCESS == rsp_val[2]))
    {
        rsp_val[3] = user_idx;
        hndl_val_param.value.len = 4;
    }

    if (BT_TIMER_HANDLE_INIT_VAL != appl_cntrl_pnt_timer_handle)
    {
        BT_stop_timer (appl_cntrl_pnt_timer_handle);
        appl_cntrl_pnt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_att_send_hndl_val_ind
             (
                 &APPL_GET_ATT_INSTANCE(((APPL_USER_CNTRL_POINT_RSP_PARAM *)data)->appl_handle),
                 &hndl_val_param
              );

    if (API_FAILURE != retval)
    {
        appl_hvc_flag = BT_TRUE;
    }
}

API_RESULT appl_user_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           )
{
    APPL_USER_CNTRL_POINT_RSP_PARAM    rsp_param;
    API_RESULT    retval;
    UINT16        consent_code;
    int           i;
    ATT_VALUE     user_val;


    appl_get_handle_from_device_handle
    (
        handle->device_id,
        &rsp_param.appl_handle
    );

    rsp_param.req_code = value->val[0];

    if (BT_FALSE == appl_user_cntrl_pnt_cnfgd)
    {
        return  ATT_CCD_IMPROPERLY_CONFIGURED | APPL_ERR_ID;
    }
    else if (BT_TRUE != appl_hvc_flag)
    {
        switch (value->val[0])
        {
            case APPL_REGISTER_NEW_USER_REQ:
                if (APPL_REGISTER_NEW_USER_REQ_LEN == value->len)
                {
                    /* Extract consent code */
                    BT_UNPACK_LE_2_BYTE
                    (
                        &consent_code,
                        value->val + 1
                    );

                    if (APPL_VALIDATE_CONSENT_CODE(consent_code))
                    {
                        rsp_param.rsp_code = APPL_UCP_SUCCESS;

                        /* Get the User index for newly registered user */
                        retval = appl_get_newly_regstd_user_index
                                 (
                                    consent_code,
                                    &user_idx,
                                    handle->device_id
                                 );

                        if (API_SUCCESS != retval)
                        {
                            if ((user_idx + 1) >= APPL_MAX_NEW_USER_SUPPORT)
                            {
                                rsp_param.rsp_code = APPL_UCP_OPERATION_FAILED;
                            }
                        }
                    }
                    else
                    {
                        rsp_param.rsp_code = APPL_UCP_INVALID_PARAMETER;
                    }
                }
                else
                {
                    rsp_param.rsp_code = APPL_UCP_INVALID_PARAMETER;
                }
                break;

            case APPL_CONSENT_REQ:
                if (APPL_CONSENT_REQ_LEN == value->len)
                {
                    /* Extract User Index */
                    BT_UNPACK_LE_1_BYTE
                    (
                        &user_idx,
                        value->val + 1
                    );

                    /* Extract consent code */
                    BT_UNPACK_LE_2_BYTE
                    (
                        &consent_code,
                        value->val + 2
                    );

                    /* Validate the user index here */
                    if (0xFF == user_idx)
                    {
                        rsp_param.rsp_code = APPL_UCP_INVALID_PARAMETER;
                    }
                    else
                    {
                        appl_uds_cp[user_idx].appl_consent_retry =
                        appl_uds_cp[user_idx].appl_consent_retry + 1;

                        if (APPL_MAX_CONSENT_RETRY >= appl_uds_cp[user_idx].appl_consent_retry)
                        {
                            retval = appl_check_consent_code(consent_code, user_idx);

                            if (API_SUCCESS == retval)
                            {
                                rsp_param.rsp_code = APPL_UCP_SUCCESS;

                                appl_uds_cp[user_idx].valid_consent_code_sent = 1;

                                /* If received consent code same as consent code
                                * in stored locally then update the user index sent
                                * in the consent opcode to the User Index characteristic
                                * in the data base.
                                */
                                appl_uds_db_handle.char_id = GATT_CHAR_USER_INDEX_INST;

                                user_val.val = &user_idx;
                                user_val.len = 1;

                                retval = BT_gatt_db_set_char_val
                                         (
                                            &appl_uds_db_handle,
                                            &user_val
                                         );
                            }
                            else
                            {
                                appl_uds_cp[user_idx].valid_consent_code_sent = 0;
                                rsp_param.rsp_code = APPL_UCP_USER_NOT_AUTHORIZED;
                            }
                        }
                        else
                        {
                            rsp_param.rsp_code = APPL_UCP_OPERATION_FAILED;
                        }
                    }
                }
                else
                {
                    rsp_param.rsp_code = APPL_UCP_INVALID_PARAMETER;
                }
                break;

            case APPL_DELETE_USER_DATA_REQ:
                if (APPL_DELETE_USER_DATA_REQ_LEN == value->len)
                {
                    for (i = 0; i < APPL_MAX_NEW_USER_SUPPORT; i++)
                    {
                        if (0xFF != appl_uds_cp[i].dev_id)
                        {
                            if (handle->device_id == appl_uds_cp[i].dev_id)
                            {
                                if (1 == appl_uds_cp[i].valid_consent_code_sent)
                                {
                                    /* Reset the consent code and User index */
                                    APPL_UDS_CP_STRUCT_INIT(i);

                                    rsp_param.rsp_code = APPL_UCP_SUCCESS;

                                   /** When the requested device id is matched with one of the
                                    * device id in the appl_uds_cp structure then break the loop
                                    * no need to check with other device ids in the structure
                                    */
                                    break;
                                }
                                else
                                {
                                    rsp_param.rsp_code = APPL_UCP_USER_NOT_AUTHORIZED;
                                }
                            }
                        }
                        else
                        {
                            rsp_param.rsp_code = APPL_UCP_USER_NOT_AUTHORIZED;
                        }
                    }
                }
                else
                {
                    rsp_param.rsp_code = APPL_UCP_INVALID_PARAMETER;
                }
                break;
            default:
                rsp_param.rsp_code = APPL_UCP_OPCODE_NOT_SUPPORTED;
                break;
        }

        BT_start_timer
        (
             &appl_cntrl_pnt_timer_handle,
             1,
             appl_cntrl_point_timer_expiry_handler,
             &rsp_param,
             sizeof (rsp_param)
         );
    }
    else
    {
        return ATT_PROCEDURE_ALREADY_IN_PROGRESS | APPL_ERR_ID;
    }

    return API_SUCCESS;
}

API_RESULT appl_get_newly_regstd_user_index
           (
                /* IN */  UINT16  consent_code_of_user,
                /* OUT */ UCHAR   * user_idx,
                /* IN */  DEVICE_HANDLE dev_handle
           )
{
    UINT32 index;
    API_RESULT retval;

    retval = API_FAILURE;

    for(index = 0; index < APPL_MAX_NEW_USER_SUPPORT; index++)
    {
        if (appl_uds_cp[index].consent_code == 0xFFFF)
        {
            appl_uds_cp[index].dev_id = dev_handle;
            appl_uds_cp[index].consent_code = consent_code_of_user;
            (* user_idx) = (UCHAR) index;

            retval = API_SUCCESS;
            break;
        }
    }
    return retval;
}

API_RESULT appl_check_consent_code
           (
                UINT16 consent_code,
                UCHAR user_index
           )
{
    API_RESULT retval;

    retval = API_FAILURE;

    if (appl_uds_cp[user_index].consent_code == consent_code)
    {
        retval =  API_SUCCESS;
    }

    return retval;
}

API_RESULT appl_validate_user_data_access
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           )
{
    API_RESULT retval;
    int i;

    retval = API_SUCCESS;

    if ((GATT_SER_USER_DATA_INST == handle->service_id) &&
        ((GATT_CHAR_LANGAGE_INST >= handle->char_id) &&
        (GATT_CHAR_FIRST_NAME_INST <= handle->char_id)))
    {
        for (i = 0; i < APPL_MAX_NEW_USER_SUPPORT; i++)
        {
            if (0xFF != appl_uds_cp[i].dev_id)
            {
                if (handle->device_id == appl_uds_cp[i].dev_id)
                {
                    if (appl_uds_cp[i].valid_consent_code_sent == 1)
                    {
                        retval = API_SUCCESS;
                        /** When the requested device id is matched with one of the
                          * device id in the appl_uds_cp structure then break the loop
                          * no need to check with other device ids in the structure
                          */
                        break;
                    }
                    else
                    {
                        retval = APPL_USER_DATA_ACCESS_NOT_PERMITTED | APPL_ERR_ID;
                    }
                }
            }
            else
            {
                retval = APPL_USER_DATA_ACCESS_NOT_PERMITTED | APPL_ERR_ID;
            }
        }
    }

    return retval;
}


#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
void appl_profile_menu_handler (void)
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

    switch(choice)
    {
    case 0:
        CONSOLE_OUT ("\nExiting BlueLitE Menu Based Application ... \n\n");
        exit(0);

    case 1:
        CONSOLE_OUT ("\nRefreshing ...\r\n\n");
        break;

    case 10:
        appl_send_wss_measurement(&appl_hndl);
        break;

    case 11:
        appl_send_bcs_measurement(&appl_hndl);
        break;

    case 12:
        appl_send_dn_chng_measurement(&appl_hndl);
        break;
    }
}

#else /* BT_SW_TXP */

void console_app_read (UCHAR * choice, UCHAR * value, UINT16 length)
{
    /* Check the choice */
    switch (*choice)
    {
        case 0:
            return;

        case 1:
            CONSOLE_OUT("%s", main_options);
            *choice = 1;
            break;

        case 2:
            appl_power_on_device();
            *choice = 1;
        break;

        case 3:
            CONSOLE_OUT("Initiated Disconnection\n");
            /* Disconnect connected device */
            *choice = 1;
            appl_disconnect_device();
        break;

        case 4:
            appl_send_wss_measurement(&appl_hndl);
            *choice = 1;
        break;

        case 5:
            appl_send_bcs_measurement(&appl_hndl);
            *choice = 1;
            break;

        case 6:
            appl_send_dn_chng_measurement(&appl_hndl);
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

#endif /* WSS */
