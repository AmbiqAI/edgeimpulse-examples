
/**
 *  \file gatt_db_pl.c
 *
 *  This file contains source code for the platform specific portions of
 *  GATT Database.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* -------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "gatt_db_pl.h"
#include "gatt_db.h"
#include "BT_smp_api.h"
#include "appl.h"

#define GL_CONFIGURED_STATE                  0x07

#define GATT_GLS_MSRMT_BIT_SET_MASK          0x01
#define GATT_GLS_CNTXT_MSRMT_BIT_SET_MASK    0x02
#define GATT_GLS_RACP_BIT_SET_MASK           0x04
#define GATT_GLS_MSRMT_BIT_RESET_MASK        0xFE
#define GATT_GLS_CNTXT_MSRMT_BIT_RESET_MASK  0xFD
#define GATT_GLS_RACP_BIT_RESET_MASK         0xFB

/* CGM Related defines */
#define CGM_CONFIGURED_STATE                 0x03

#define GATT_CGMS_MSRMT_BIT_SET_MASK         0x01
#define GATT_CGMS_RACP_BIT_SET_MASK          0x02
#define GATT_CGMS_MSRMT_BIT_RESET_MASK       0xFE
#define GATT_CGMS_RACP_BIT_RESET_MASK        0xFD


/* POS Related defines */
#define PLX_CONFIGURED_STATE                   0x03

#define GATT_PLX_SPOT_CHK_MSRMT_BIT_SET_MASK   0x01
#define GATT_PLX_RACP_BIT_SET_MASK             0x02
#define GATT_PLX_SPOT_CHK_MSRMT_BIT_RESET_MASK 0xFE
#define GATT_PLX_RACP_BIT_RESET_MASK           0xFD

#ifndef GATT_DB_PROFILE_HANDLER
#define GATT_DB_PROFILE_HANDLER            NULL
#endif /* GATT_DB_PROFILE_HANDLER */

UCHAR cgm_flag;

/* -------------------------------------- Exported Global Variables */

#ifdef GATT
API_RESULT gatt_db_service_change_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16 retval;
    UINT16 config;

    retval = API_SUCCESS;

    if (GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ == params->db_op)
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        retval = appl_service_change_handler (handle,config);
    }

    return retval;
}
#endif /* GATT */

API_RESULT gatt_db_battery_level_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16 retval;
    UINT16 config;

    retval = API_SUCCESS;

    /**
     *  Note: Adding a generic handler for Battery Level Notification.
     *  Incase the profile does not support Battery Service. Please add a dummy
     *  GATT_CHAR_BATTERY_LVL_INST set to 0xFF in corresponding gatt_db.h to
     *  resolve the build issue.
     */
    if ((GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ == params->db_op) &&
       (GATT_CHAR_BATTERY_LVL_INST == handle->char_id))
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        retval = appl_battery_level_handler (handle,config);
    }

    return retval;
}

#ifdef AIOS
API_RESULT gatt_db_aios_digital_io_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16 retval;
    UINT16 config;
    UINT16 uuid_val;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_DIGITAL_OUTPUT_INST == handle->char_id)
        {
            retval = appl_digital_output_signal_handler (handle,&params->value);
        }
    case GATT_DB_CHAR_PEER_HLD_DESC_WRITE_REQ:
        if ((GATT_CHAR_DIGITAL_INPUT_INST == handle->char_id) || (GATT_CHAR_ANALOG_INPUT_INST == handle->char_id))
        {
            BT_UNPACK_LE_2_BYTE (&uuid_val,GATT_DB_GET_UUID(params->handle))
            retval = appl_analog_and_digital_input_signal_handler (handle, &params->value,uuid_val);
        }
        break;
    case GATT_DB_CHAR_PEER_HLD_DESC_READ_REQ:
        BT_UNPACK_LE_2_BYTE(&uuid_val, GATT_DB_GET_UUID(params->handle))
        retval = appl_aios_read_req_handler(handle, uuid_val);
        break;
    default:
        break;
    }
    return retval;

}
#endif /* AIOS */

#ifdef POS
API_RESULT gatt_db_pulse_ox_msrmt_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    static UCHAR     pos_config_state;
    API_RESULT       retval;
    UINT16           config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_PLX_SPOT_CHECK_MSRMNT_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                pos_config_state &= GATT_PLX_SPOT_CHK_MSRMT_BIT_RESET_MASK;
            }
            else
            {
                pos_config_state |= GATT_PLX_SPOT_CHK_MSRMT_BIT_SET_MASK;
                appl_manage_trasnfer ((*handle),config);
            }
        }
        else if(GATT_CHAR_RACP_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                pos_config_state &= GATT_PLX_RACP_BIT_RESET_MASK;
            }
            else
            {
                pos_config_state |= GATT_PLX_RACP_BIT_SET_MASK;
            }
        }
        else if (GATT_CHAR_PLX_CONTINUS_MSRMNT_INST == handle->char_id)
        {
            appl_manage_trasnfer ((*handle),config);
        }
        else
        {
            /* Not a valid request */
        }
    }
    break;

    case GATT_DB_CHAR_PEER_WRITE_REQ:
    {
        if (GATT_CHAR_RACP_INST == handle->char_id)
        {
            if (PLX_CONFIGURED_STATE  == pos_config_state)
            {
                retval = appl_pos_racp_write_handler(&params->value);
            }
            else
            {
                retval = ATT_CCD_IMPROPERLY_CONFIGURED | APPL_ERR_ID ;
            }
        }
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* POS */

#ifdef HID
API_RESULT gatt_db_hid_event_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;

    case GATT_DB_CHAR_PEER_WRITE_CMD:
    {
        retval = appl_manage_wwr (handle, params);
    }
    break;

    default:
        break;
    }
    return retval;
}
#endif /* HID */

#ifdef HTS
API_RESULT gatt_db_temperature_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        retval = appl_validate_msrmt_interval(handle,config);
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* HTS */

#ifdef CSCS
API_RESULT gatt_db_cscs_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_SC_CNTRL_PNT_INST == handle->char_id)
        {
            appl_csc_cntrl_point_cnfg_handler (handle,config);
        }
        else
        {
           appl_manage_trasnfer ((*handle),config);
        }
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_SC_CNTRL_PNT_INST == handle->char_id)
        {
            retval = appl_csc_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* CSCS */

#ifdef RSCS
API_RESULT gatt_db_rscs_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_SC_CNTRL_PNT_INST == handle->char_id)
        {
            appl_rsc_cntrl_point_cnfg_handler (handle,config);
        }
        else
        {
           appl_manage_trasnfer ((*handle),config);
        }
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_SC_CNTRL_PNT_INST == handle->char_id)
        {
            retval = appl_rsc_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* RSCS */

#ifdef VSE1
API_RESULT gatt_db_vse1_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* VSE1 */

#ifdef VSE2
API_RESULT gatt_db_vse2_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_REQ:
    {
        appl_rx_data_handler ((*handle),&params->value);
    }
    break;
    default:
        break;
    }
    return API_SUCCESS;
}
#endif /* VSE2 */

#ifdef VSE3
/* GATT DB Handler for VSE3 related DB entries */
API_RESULT gatt_db_vse3_event_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;

    case GATT_DB_CHAR_PEER_WRITE_CMD:
    {
        retval = appl_manage_wwr (handle, params);
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_HR_CNTRL_PNT_INST == handle->char_id)
        {
            retval = appl_hr_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* VSE3 */

#ifdef VSE4
API_RESULT gatt_db_vse4_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_transfer ((*handle),config);
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* VSE4 */

#ifdef VSE5
API_RESULT gatt_db_vse5_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_transfer ((*handle),config);
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* VSE5 */

#ifdef VSE8
API_RESULT gatt_db_vse8_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* VSE8 */

#ifdef PASS
API_RESULT gatt_db_pass_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    if (GATT_CHAR_RING_CTRL_PNT_INST == handle->char_id)
    {
        if (GATT_DB_CHAR_PEER_WRITE_CMD == params->db_op)
        {
            appl_pass_ringer_control_point_write_handler (handle, &params->value);
        }
    }
    else if ((GATT_CHAR_ALRT_STATUS_INST == handle->char_id) ||
             (GATT_CHAR_RING_SET_INST == handle->char_id))
    {
        switch (params->db_op)
        {
            case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
                {
                    BT_UNPACK_LE_2_BYTE (&config,params->value.val);
                    appl_start_trasnfer ((*handle),config);
                }
                break;
            case GATT_DB_CHAR_PEER_READ_REQ:
                {
                    appl_pass_read_request_handler (handle);
                }
                break;
            default:
                break;
        }
    }
    return retval;
}
#endif /* PASS */

#ifdef HRS
API_RESULT gatt_db_hrs_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_HR_CNTRL_PNT_INST == handle->char_id)
        {
            retval = appl_hr_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* HRS */

#ifdef GLS
API_RESULT gatt_db_glucose_meter_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    static UCHAR     gl_config_state;
    UINT16           config;
    API_RESULT       retval;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_GL_MSRMNT_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                gl_config_state &= GATT_GLS_MSRMT_BIT_RESET_MASK;
            }
            else
            {
                gl_config_state |= GATT_GLS_MSRMT_BIT_SET_MASK;
            }
        }
        else if (GATT_CHAR_GL_CNTXT_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                gl_config_state &= GATT_GLS_CNTXT_MSRMT_BIT_RESET_MASK;
            }
            else
            {
                gl_config_state |= GATT_GLS_CNTXT_MSRMT_BIT_SET_MASK;
            }
        }
        else if (GATT_CHAR_GL_RACP_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                gl_config_state &= GATT_GLS_RACP_BIT_RESET_MASK;
            }
            else
            {
                gl_config_state |= GATT_GLS_RACP_BIT_SET_MASK;
            }
        }
        else
        {
            /* Not a valid request */
        }
        break;
    }
    case GATT_DB_CHAR_PEER_WRITE_REQ:
    {
        /* Verify if security requirements are available with the link */
        retval = appl_smp_assert_security
                 (
                     &handle->device_id,
                     GATT_DB_SER_SECURITY_MODE1 | GATT_DB_SER_SECURITY_LEVEL1,
                     GATT_DB_SER_ENC_KEY_SIZE_DONT_CARE
                 );

        /* Security requirements satisfied? */
        if (API_SUCCESS != retval)
        {
            retval = GATT_DB_INSUFFICIENT_SECURITY;
            return retval;
        }

        /* Security requirements satisfied, go ahead with data transfer */
        if (GATT_CHAR_GL_RACP_INST == handle->char_id)
        {
            if (GL_CONFIGURED_STATE == gl_config_state)
            {
                retval = appl_gls_racp_write_handler(&params->value);
            }
            else
            {
                retval = ATT_APPL_CCD_IMPROPERLY_CONFIGURED | APPL_ERR_ID;
            }
        }
        
        break;
    }

    default:
        break;
    }

    return retval;
}
#endif /* GLS */

#ifdef PXR
API_RESULT gatt_db_pxr_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    API_RESULT   retval;

    retval = API_SUCCESS;

    switch(params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_REQ:
    case GATT_DB_CHAR_PEER_WRITE_CMD:
        retval = appl_pxr_set_alert_level (handle, &params->value);
        break;

    case GATT_DB_CHAR_PEER_READ_REQ:
        if (GATT_CHAR_TX_POWER_INST == handle->char_id)
        {
            retval = appl_pxr_get_tx_power_level (handle, &params->value);
        }
        break;

    default:
        break;
    }

    return retval;
}
#endif /* PXR */

#ifdef FMT
API_RESULT gatt_db_fmt_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    API_RESULT   retval;

    retval = API_SUCCESS;

    switch(params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        retval = appl_fmt_set_alert_level (handle, &params->value);
        break;

    default:
        break;
    }

    return retval;
}
#endif /* FMT */

#ifdef BPS
API_RESULT gatt_db_bps_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;
    retval = API_SUCCESS;
    switch (params->db_op)
    {
        case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
        {
            BT_UNPACK_LE_2_BYTE (&config,params->value.val);
            appl_manage_trasnfer ((*handle),config);
        }
        break;
        default:
            break;
    }
    return retval;
}
#endif /* BPS */

#ifdef CTS
API_RESULT gatt_db_cts_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_CUR_TIME_INST == handle->char_id)
        {
            retval = appl_update_current_time (&params->value);
        }
        else if (GATT_CHAR_LCL_TIME_INFO_INST == handle->char_id)
        {
            retval = appl_update_local_time (&params->value);
        }
        else
        {
            /* Do Nothing */
        }
    break;

    case GATT_DB_CHAR_PEER_WRITE_CMD:
    {
        retval = appl_rtus_config_time_update (handle, &params->value);
    }
    break;

    case GATT_DB_CHAR_PEER_READ_REQ:
    if (GATT_CHAR_TIME_UPDATE_CP_INST == handle->char_id)
    {
        retval = appl_rtus_get_time_update_state (handle, &params->value);
    }
    else
    {
        retval = appl_cts_get_current_time (handle, &params->value);
    }
    break;
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    default:
        break;
    }
    return retval;
}
#endif /* CTS */

#ifdef RTUS
API_RESULT gatt_db_rtus_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_CMD:
    {
        retval = appl_rtus_config_time_update (handle, &params->value);
    }
    break;

    case GATT_DB_CHAR_PEER_READ_REQ:
        retval = appl_rtus_get_time_update_state (handle, &params->value);
        break;

    default:
        break;
    }

    return retval;
}
#endif /* RTUS */

#ifdef NDCS
API_RESULT gatt_db_ndcs_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_READ_REQ:
        retval = appl_ndcs_get_current_time (handle, &params->value);
        break;

    default:
        break;
    }
    return retval;
}
#endif /* NDCS */

#ifdef ANS
API_RESULT gatt_db_ans_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;
    UCHAR      command_id;
    UCHAR      category_id;

    retval = API_SUCCESS;

    if (GATT_CHAR_ALRT_NTF_CP_INST == handle->char_id)
    {
        switch (params->db_op)
        {
            case GATT_DB_CHAR_PEER_WRITE_REQ:
                BT_UNPACK_LE_1_BYTE (&command_id,params->value.val);
                BT_UNPACK_LE_1_BYTE (&category_id,params->value.val+1);
                retval = appl_alrt_ntf_cntrl_point((*handle), command_id, category_id);
                break;

            default:
                break;
        }
    }
    else if ((GATT_CHAR_NEW_ALRT_INST == handle->char_id) ||
          (GATT_UNREAD_ALRT_STATUS_INST == handle->char_id))
    {
        switch (params->db_op)
        {
            case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
                BT_UNPACK_LE_2_BYTE (&config,params->value.val);
                appl_manage_trasnfer ((*handle), config);
                break;

            default:
                break;
        }
    }
    return retval;
}
#endif /* ANS */

#ifdef CGMS
API_RESULT gatt_db_cgms_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    static UCHAR cgm_config_state;
    UINT16       config;
    API_RESULT   retval;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_CGM_MSRMT_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                cgm_config_state &= GATT_CGMS_MSRMT_BIT_RESET_MASK;
                cgm_flag = BT_FALSE;
            }
            else
            {
                cgm_config_state |= GATT_CGMS_MSRMT_BIT_SET_MASK;
                cgm_flag = BT_TRUE;
            }
        }
        else if (GATT_CHAR_CGM_RACP_INST == handle->char_id)
        {
            if (GATT_CLI_CNFG_DEFAULT == config)
            {
                cgm_config_state &= GATT_CGMS_RACP_BIT_RESET_MASK;
            }
            else
            {
                cgm_config_state |= GATT_CGMS_RACP_BIT_SET_MASK;
            }
        }
        else if (GATT_CHAR_CGM_CTRL_PNT_INST == handle->char_id)
        {
            appl_cgm_cntrl_point_cnfg_handler (handle,config);
        }
        else
        {
            /* Invalid Configuration */
        }
        break;
    }
    case GATT_DB_CHAR_PEER_WRITE_REQ:

        switch (handle->char_id)
        {
        case GATT_CHAR_CGM_SSN_STRT_TIME_INST:
            retval = appl_cgm_set_ssn_start_time
                     (
                         handle,
                         &params->value
                     );
            break;
        case GATT_CHAR_CGM_CTRL_PNT_INST:
            retval = appl_cgm_cp_handler
                     (
                         handle,
                         &params->value
                     );
            break;
        case GATT_CHAR_CGM_RACP_INST:
            if (CGM_CONFIGURED_STATE == cgm_config_state)
            {
                retval = appl_cgms_racp_write_handler(&params->value);
            }
            else
            {
                retval = ATT_CCD_IMPROPERLY_CONFIGURED | APPL_ERR_ID;
            }
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }

    return retval;
}
#endif /* CGMS */

#ifdef CPMS
API_RESULT gatt_db_cpms_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    API_RESULT retval;
    UINT16     config;

    retval = API_SUCCESS;


    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_CPM_CNTRL_POINT_INST == handle->char_id)
        {
            appl_cpm_cntrl_point_cnfg_handler (handle,config);
        }
        else
        {
           appl_manage_trasnfer ((*handle),config);

           if ((GATT_CHAR_CPM_VECTOR_INST == handle->char_id) &&
               (GATT_CLI_CNFG_NOTIFICATION == config))
           {
               retval = GATT_DB_DELAYED_RESPONSE;
           }
        }
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_CPM_CNTRL_POINT_INST == handle->char_id)
        {
            retval = appl_cpm_control_point_handler (handle,&params->value);
        }
        break;
    case GATT_DB_CHAR_PEER_SER_CNFG_WRITE_REQ:
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
#ifdef APPL_GAP_BROADCASTER_SUPPORT
        appl_cpm_manage_brodcast (config);
#endif /* APPL_GAP_BROADCASTER_SUPPORT */
        break;
    default:
        break;
    }
    return retval;
}
#endif /* CPMS */

#ifdef OTS
API_RESULT gatt_db_ots_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    API_RESULT retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
        case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
        {
            BT_UNPACK_LE_2_BYTE (&config,params->value.val);
            if (GATT_CHAR_OACP_INST == handle->char_id)
            {
                appl_oacp_cnfg_handler (handle,config);
            }
            else if (GATT_CHAR_OLCP_INST == handle->char_id)
            {
                appl_olcp_cnfg_handler (handle,config);
            }
            else if (GATT_CHAR_OBJ_CHNGD_INST == handle->char_id)
            {
                appl_ots_obj_changed_handler(handle, config);
            }
            else
            {
                /* Do Nothing */
            }
        }
            break;
        case GATT_DB_CHAR_PEER_WRITE_REQ:
            if (GATT_CHAR_OACP_INST == handle->char_id)
            {
                retval = appl_oacp_handler (handle,&params->value);
            }
            else if (GATT_CHAR_OLCP_INST == handle->char_id)
            {
                retval = appl_olcp_handler (handle, &params->value);
            }
            else if ((GATT_CHAR_OLF_INST_1 == handle->char_id) ||
                (GATT_CHAR_OLF_INST_2 == handle->char_id) ||
                (GATT_CHAR_OLF_INST_3 == handle->char_id))
            {
                retval = appl_obj_list_filter_handler (handle, params);
            }
            else
            {
                retval = appl_obj_metadata_handler(handle, params);
            }
            break;
        case GATT_DB_CHAR_PEER_READ_REQ:
            if ((GATT_CHAR_OLF_INST_1 == handle->char_id) ||
                (GATT_CHAR_OLF_INST_2 == handle->char_id) ||
                (GATT_CHAR_OLF_INST_3 == handle->char_id))
            {
                /* retval = appl_obj_list_filter_handler (handle, params); */
            }
            break;
        default:
            break;
    }

    return retval;
}
#endif /* OTS */

#ifdef LNS
API_RESULT gatt_db_lns_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    API_RESULT retval;
    UINT16     config;

    retval = API_SUCCESS;


    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_LN_CNTRL_POINT_INST == handle->char_id)
        {
            appl_ln_cntrl_point_cnfg_handler (handle,config);
        }
        else
        {
           appl_manage_trasnfer ((*handle),config);
        }
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_LN_CNTRL_POINT_INST == handle->char_id)
        {
            retval = appl_ln_control_point_handler (handle,&params->value);
        }
        break;
    case GATT_DB_CHAR_PEER_READ_REQ:
        if (GATT_CHAR_POSTN_QLTY_INST == handle->char_id)
        {
            appl_mask_pos_quality_content();
        }
        break;
    default:
        break;
    }

#ifdef OTS
    if (GATT_SER_OTS_INST == handle->service_id)
    {
        retval = gatt_db_ots_handler(handle, params);
    }
#endif /* OTS */

    return retval;
}
#endif /* LNS */

#ifdef SPS
API_RESULT gatt_db_sps_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16 config;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_scan_refresh_cnfg_handler (handle,config);
    }
    break;
    default:
        break;
    }
    return API_SUCCESS;
}
#endif /* SPS */

#ifdef WSS
API_RESULT gatt_db_wss_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    /* Fall through */
    case GATT_DB_CHAR_PEER_READ_REQ:
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_USER_CNTRL_PNT_INST == handle->char_id)
        {
            retval = appl_user_control_point_handler (handle,&params->value);
        }
        else
        {
            retval = appl_validate_user_data_access (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* WSS */

#ifdef ESS
API_RESULT gatt_db_ess_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;
    UINT16     uuid;

    retval = API_SUCCESS;

    BT_UNPACK_LE_2_BYTE(&uuid,GATT_DB_GET_UUID(params->handle));

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_ess_ccd_config_handlr ((*handle),config);
    }
    break;
    case GATT_DB_CHAR_PEER_HLD_DESC_WRITE_REQ:
    {
        if (ESS_TRIGGER_SETTING_DESC_UUID == uuid)
        {
            retval = appl_ess_set_trgr_desc(params);
        }
        else if (ESS_CONFIG_DESC_UUID == uuid)
        {
            retval = appl_ess_set_config_desc(params);
        }
        else if (ESS_VALID_RANGE_DESC_UUID == uuid)
        {

        }
        else
        {
            /* Do Nothing */
        }
    }
    break;
    case GATT_DB_CHAR_PEER_USR_DESC_WRITE_REQ:
        break;
    default:
        break;
    }
    return retval;
}
#endif /* ESS */

#ifdef BMS
API_RESULT gatt_db_bms_measurment_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    API_RESULT retval;

    retval = API_SUCCESS;


    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_BMCP_INST == handle->char_id)
        {
            retval = appl_bms_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }

    return retval;
}
#endif /* BMS */

#ifdef IPS
API_RESULT gatt_db_ips_handler
           (
                GATT_DB_HANDLE    * handle,
                GATT_DB_PARAMS    * params
           )
{
    API_RESULT retval;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_INDOOR_POS_CONFIG_INST == handle->char_id)
        {
            retval = appl_ips_handler (&params->value);
        }
        break;
    default:
        break;
    }
    return retval;

}
#endif /* IPS */


#ifdef TDS
API_RESULT gatt_db_tds_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_TDS_CONTROL_POINT_INST == handle->char_id)
        {
            appl_tds_cntrl_point_cnfg_handler (handle,config);
        }
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_TDS_CONTROL_POINT_INST == handle->char_id)
        {
            retval = appl_tds_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* TDS */

#ifdef FTMS
API_RESULT gatt_db_ftms_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        if (GATT_CHAR_CONTROL_POINT_INST == handle->char_id)
        {
            appl_ftm_cntrl_point_cnfg_handler (handle,config);
        }
        else
        {
            appl_manage_trasnfer ((*handle),config);
        }

    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_CONTROL_POINT_INST == handle->char_id)
        {
            retval = appl_ftm_control_point_handler (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* FTMS */

#ifdef AMOTAS
API_RESULT gatt_db_amotas_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_CMD:
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_AMOTA_RX == handle->char_id)
        {
            retval = appl_amotas_write_cback (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* AMOTAS */

#ifdef AMDTPS
API_RESULT gatt_db_amdtps_handler
           (
               GATT_DB_HANDLE    * handle,
               GATT_DB_PARAMS    * params
           )
{
    UINT16     retval;
    UINT16     config;

    retval = API_SUCCESS;

    switch (params->db_op)
    {
    case GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ:
    {
        BT_UNPACK_LE_2_BYTE (&config,params->value.val);
        appl_manage_trasnfer ((*handle),config);
    }
    break;
    case GATT_DB_CHAR_PEER_WRITE_CMD:
    case GATT_DB_CHAR_PEER_WRITE_REQ:
        if (GATT_CHAR_AMDTP_RX == handle->char_id || GATT_CHAR_AMDTP_ACK == handle->char_id)
        {
            retval = appl_amdtps_write_cback (handle,&params->value);
        }
        break;
    default:
        break;
    }
    return retval;
}
#endif /* AMOTAS */

API_RESULT gatt_char_handler (GATT_DB_HANDLE * handle,GATT_DB_PARAMS * param)
{
    /**
     *  For calling common handlers before calling dedicated profile/service
     *  handler
     */
    if (GATT_CHAR_BATTERY_LVL_INST == handle->char_id)
    {
        return gatt_db_battery_level_handler(handle,param);
    }

#ifdef ANS
    return gatt_db_ans_handler (handle,param);
#elif defined AMOTAS
    return gatt_db_amotas_handler (handle,param);
#elif defined AMDTPS
    return gatt_db_amdtps_handler (handle,param);
#elif defined BPS
    return gatt_db_bps_handler (handle,param);
#elif defined CGMS
    return gatt_db_cgms_measurment_handler (handle,param);
#elif defined CPMS
    return gatt_db_cpms_measurment_handler (handle,param);
#elif defined CSCS
    return gatt_db_cscs_handler (handle,param);
#elif defined CTS
    return gatt_db_cts_handler (handle,param);
#elif defined FMT
    return gatt_db_fmt_handler (handle,param);
#elif defined FTMS
    return gatt_db_ftms_handler(handle, param);
#elif defined HID
    return gatt_db_hid_event_handler (handle,param);
#elif defined HRS
    return gatt_db_hrs_handler (handle,param);
#elif defined HTS
    return gatt_db_temperature_measurment_handler (handle,param);
#elif defined IPS
    return gatt_db_ips_handler(handle, param);
#elif defined LNS
    return gatt_db_lns_measurment_handler (handle,param);
#elif defined PASS
    return gatt_db_pass_handler (handle,param);
#elif defined POS
    return gatt_db_pulse_ox_msrmt_handler (handle,param);
#elif defined PXR
    return gatt_db_pxr_handler (handle,param);
#elif defined RSCS
    return gatt_db_rscs_handler (handle,param);
#elif defined RTUS
    return gatt_db_rtus_handler (handle,param);
#elif defined GLS
    return gatt_db_glucose_meter_handler (handle,param);
#elif defined NDCS
    return gatt_db_ndcs_handler (handle,param);
#elif defined SPS
    return gatt_db_sps_measurment_handler (handle,param);
#elif defined AIOS
    return gatt_db_aios_digital_io_handler (handle,param);
#elif defined WSS
    return gatt_db_wss_measurment_handler (handle,param);
#elif defined ESS
    return gatt_db_ess_handler (handle,param);
#elif defined BMS
    return gatt_db_bms_measurment_handler (handle,param);
#elif defined TDS
    return gatt_db_tds_handler(handle, param);
#elif defined VSE1
    return gatt_db_vse1_handler (handle,param);
#elif defined VSE2
    return gatt_db_vse2_handler (handle,param);
#elif defined VSE3
    return gatt_db_vse3_event_handler(handle, param);
#elif defined VSE4
    return gatt_db_vse4_handler(handle, param);
#elif defined VSE5
    return gatt_db_vse5_handler(handle, param);
#elif defined VSE8
    return gatt_db_vse8_handler(handle, param);
#elif defined GATT
    return gatt_db_service_change_handler(handle,param);
#else
    return API_SUCCESS;
#endif
}

