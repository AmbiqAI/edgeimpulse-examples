
/**
 *  \file appl_cscs.c
 *
 *  This file contains the Cycling Speed & Cadence Sensor application.
 *  Sample applications detailed below:
 *      a. The Sensor, as defined by the Sepcification plays the GAP Peripheral
 *         role.
 *      b. The Sensor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Battery
 *           - Device Information and
 *           - Cycling Speed & Cadence
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Cycling Speed and Cadence Service exposes:
 *           - Cycling Speed & Cadence Measurement
 *           - Sensor Location
 *           - Cycling Speed & Cadence Control Point
 *      d. Cycling Speed & Cadence Measurement can be configured to send
 *         Notifications.
 *      e. appl_manage_transfer routine takes care of handling peer
 *         configuration. This handling would be needed:
 *           - When Peer Configures Measurement Transfer by writting to the
 *             Characteristic Client Configuration of CSC Measurement.
 *           - Subsequent reconnection with bonded device that had already
 *             configured the device for transfer. Please note it is mandatory
 *             for GATT Servers to remember the configurations of bonded GATT
 *             clients.
 *         In order to ensure the above mentioned configurations are correctly
 *         handled, the routine, appl_manage_transfer, is therefore called from:
 *           - gatt_db_cscs_handler and
 *           - appl_cscs_connect
 *         [NOTE]: If link does not have the needed secruity for the service,
 *         transfer will not be initiated.
 *      f. This sample application, once configured, send measurements every
 *         1s. This interval can be altered using the define
 *         APPL_CSC_MSURMNT_INTERVAL. In absence of a real sensor
 *         to provide speed & cadence readings, a static reading of 86 bpm is being
 *         sent. Distance is incremented every 5 seconds. This value can
 *         be reset by the Collector device through the Cycling Speed
 *         & Cadence Control Point. See appl_csc_control_point_handler for more
 *         details.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef CSCS

/* --------------------------------------------- Constants */
#define APPL_CSC_MSRMNT_LENGTH           11
#define APPL_CSC_MSURMNT_INTERVAL        5

/* Speed and Cadence Control Point Commands */
#define APPL_CSC_SET_CUMMULATIVE_VALUE_OPCODE               1
#define APPL_CSC_START_SENSOR_CALIBRATION_OPCODE            2
#define APPL_CSC_UPDATE_SENSOR_LOCATION_OPCODE              3
#define APPL_CSC_REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE   4
#define APPL_CSC_RESPONSE_CODE_OPCODE                       16

/* Speed and Cadence Control Point Response Codes */
#define APPL_CSC_SUCCESS                                1
#define APPL_CSC_OPCODE_NOT_SUPPORTED                   2
#define APPL_CSC_INVALID_PARAMETER                      3
#define APPL_CSC_OPERATION_FAILED                       4

#define APPL_CSC_CUMMULATIVE_VALUE_LENGTH               4
#define APPL_CSC_RESPONSE_LENGTH                        3

/* Supported Sensor Location Count */
#define APPL_CSC_SUPPORTED_SENSOR_LOC_COUNT             3

/* Application Error Codes */
#define APPL_CSC_PROCEDURE_ALREADY_IN_PROGRESS          (APPL_ERR_ID | 0x80)
#define APPL_CSC_CCD_IMPROPERLY_CONFIGURED              (APPL_ERR_ID | 0x81)

/* --------------------------------------------- External Global Variables */
extern UCHAR appl_hvc_flag;
/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

/*static UCHAR csc_obs_data[APPL_CSC_MSRMNT_LENGTH] = {0x0E, 0x56, 0x00, 0x00};*/
static UCHAR csc_obs_data[APPL_CSC_MSRMNT_LENGTH] =
{
    0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00
};
static UCHAR csc_supported_sensor_location[APPL_CSC_SUPPORTED_SENSOR_LOC_COUNT] =
{
    0x04, 0x05, 0x06
};
static UCHAR csc_count;
static GATT_DB_HANDLE appl_cscs_handle;
static BT_timer_handle timer_handle;
static ATT_ATTR_HANDLE appl_csc_hndl;
BT_DEFINE_MUTEX (appl_cscs_mutex)
static BT_timer_handle cntrl_point_handle = BT_TIMER_HANDLE_INIT_VAL;
static UCHAR appl_cntrl_point_cnfgd = BT_FALSE;
static GATT_DB_HANDLE  appl_csc_sc_cp_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_CSC_INST,
    GATT_CHAR_SC_CNTRL_PNT_INST
};
static ATT_ATTR_HANDLE appl_csc_sc_cp_hndl;
static UCHAR csc_reverse;

/* --------------------------------------------- Functions */

void appl_cscs_init(void)
{
    /* Module Mutex Initialization */
    BT_MUTEX_INIT_VOID (appl_cscs_mutex,APPL_CSCS);
    appl_cscs_server_reinitialize ();
}

void appl_cscs_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    API_RESULT        retval;   
    UINT16            cli_cnfg;

    APPL_CSCS_LOCK_VOID ();

    cli_cnfg = 0;

    appl_csc_sc_cp_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    retval = BT_gatt_db_get_char_val_hndl
             (
                 &appl_csc_sc_cp_db_handle,
                 &appl_csc_sc_cp_hndl
             );

    appl_cscs_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_cscs_handle.char_id = GATT_CHAR_CSC_MSRMNT_INST;
    appl_cscs_handle.service_id = GATT_SER_CSC_INST;

    retval = BT_gatt_db_get_char_val_hndl (&appl_cscs_handle,&appl_csc_hndl);
    retval = BT_gatt_db_get_char_cli_cnfg(&appl_cscs_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X) with "
    "result 0x%04X", cli_cnfg,APPL_GET_DEVICE_HANDLE((*appl_handle)),
    retval);
    
    APPL_CSCS_UNLOCK_VOID ();

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_cscs_handle,cli_cnfg);
    }

}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    APPL_HANDLE    appl_handle;
    API_RESULT     retval;

    UCHAR          security, ekey_size;

    APPL_CSCS_LOCK_VOID ();

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
    retval = appl_get_handle_from_device_handle(handle.device_id,&appl_handle);

    if (API_SUCCESS != retval)
    {
        return;
    }

    if (GATT_CLI_CNFG_NOTIFICATION == config)
    {
        /* Start Transfer Timer to Send measurement every second */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            BT_stop_timer (timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
        retval = BT_start_timer
                 (
                     &timer_handle,
                     APPL_CSC_MSURMNT_INTERVAL,
                     appl_timer_expiry_handler,
                     &appl_handle,
                     sizeof (appl_handle)
                 );
        APPL_TRC (
        "[APPL]:[%02X] Started Timer with result 0x%04X, timer handle %p\n",
        appl_handle,retval,timer_handle);

        if (API_SUCCESS != retval)
        {
            /**
             *  Transfer could not be started, take appropriate action. One
             *  option could be to indicate an application error for
             *  configuration request
             */
        }
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        /* Turn Off Transfer Timer if On */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            BT_stop_timer (timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
    }
    else
    {
        /* Incorrect Configuration */
    }

    APPL_CSCS_UNLOCK_VOID ();
}

void appl_send_csc_measurement (APPL_HANDLE   * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    APPL_TRC (
    "[0x%02X]:Sending measurement On Handle 0x%04X\n",
    (*handle),appl_cscs_handle);

    APPL_CSCS_LOCK_VOID ();

    hndl_val_param.handle =  appl_csc_hndl;
    hndl_val_param.value.val = csc_obs_data;
    hndl_val_param.value.len = APPL_CSC_MSRMNT_LENGTH;

    retval = BT_att_send_hndl_val_ntf
             (
                 &APPL_GET_ATT_INSTANCE(*handle),
                 &hndl_val_param
              );
    if (2 == csc_count)
    {
        if (BT_TRUE != csc_reverse)
        {
            /* Simulate Cummulative Wheel Revolutions */
            if (0xFF == csc_obs_data[0x01])
            {
                csc_obs_data[0x02]++;
            }
            csc_obs_data[0x01]++;

            /* Simulate Last Wheel Event Time */
            if (0x40 == csc_obs_data[0x05])
            {
                csc_obs_data[0x05] = 0;
            }
            csc_obs_data[0x05]++;
        }
        else
        {
            csc_obs_data[0x01]--;
            if (0x00 == csc_obs_data[0x01])
            {
                csc_reverse = BT_FALSE;
            }
        }
        csc_count = 0;
    }
    csc_count++;
    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "[** ERR **]: Failed to send measurement, reason 0x%04X",
        retval);
    }

    APPL_CSCS_UNLOCK_VOID ();
}


void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM    fsm_param;

    APPL_CSCS_LOCK_VOID ();

    fsm_param.handle = (*((APPL_HANDLE *)data));

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]:[0x%02X]: Stopping Timer %p\n",
        fsm_param.handle,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &timer_handle,
        APPL_CSC_MSURMNT_INTERVAL,
        appl_timer_expiry_handler,
        &fsm_param.handle,
        sizeof (fsm_param.handle)
    );

    APPL_TRC (
    "[APPL]:[0x%02X] Started Timer  %p\n",fsm_param.handle,timer_handle);

    APPL_CSCS_UNLOCK_VOID ();

    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_measurement_ind,
        &fsm_param
    );
}

void appl_csc_cntrl_point_cnfg_handler (GATT_DB_HANDLE  *handle,UINT16 config)
{
    if (GATT_CLI_CNFG_INDICATION == config)
    {
        appl_cntrl_point_cnfgd = BT_TRUE;
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        appl_cntrl_point_cnfgd = BT_FALSE;
    }
}

void appl_cscs_server_reinitialize (void)
{
    APPL_CSCS_LOCK_VOID ();

    /* Turn Off Transfer Timer if On.  */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    if (BT_TIMER_HANDLE_INIT_VAL != cntrl_point_handle)
    {
        BT_stop_timer (cntrl_point_handle);
        cntrl_point_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    csc_count = 0;
    appl_cntrl_point_cnfgd = BT_FALSE;
    appl_hvc_flag = BT_FALSE;

    APPL_CSCS_UNLOCK_VOID ();
}

void appl_cscs_cntrl_point_timer_handle (void * data, UINT16 datalen)
{
    /* Send Indication */
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    UCHAR                 rsp[6];
    API_RESULT            retval;

    rsp[0] = APPL_CSC_RESPONSE_CODE_OPCODE;
    rsp[1] = ((APPL_CSCS_CNTRL_POINT_RSP_PARAM *)data)->op_code;
    rsp[2] = ((APPL_CSCS_CNTRL_POINT_RSP_PARAM *)data)->rsp_code;

    if (BT_TIMER_HANDLE_INIT_VAL != cntrl_point_handle)
    {
        BT_stop_timer (cntrl_point_handle);
        cntrl_point_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    if (APPL_CSC_REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE == rsp[1])
    {
        hndl_val_param.value.len = APPL_CSC_RESPONSE_LENGTH +
            APPL_CSC_SUPPORTED_SENSOR_LOC_COUNT;
        BT_mem_copy
        (
            &rsp[3],
            &csc_supported_sensor_location,
            APPL_CSC_SUPPORTED_SENSOR_LOC_COUNT
        );
    }
    else
    {
        hndl_val_param.value.len = APPL_CSC_RESPONSE_LENGTH;
    }

    hndl_val_param.handle =  appl_csc_sc_cp_hndl;
    hndl_val_param.value.val = rsp;

    retval = BT_att_send_hndl_val_ind
             (
                 &APPL_GET_ATT_INSTANCE(((APPL_CSCS_CNTRL_POINT_RSP_PARAM *)data)->appl_handle),
                 &hndl_val_param
              );

    if (API_FAILURE != retval)
    {
        appl_hvc_flag = BT_TRUE;
    }
}

API_RESULT appl_csc_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           )
{
    API_RESULT retval;
    APPL_CSCS_CNTRL_POINT_RSP_PARAM   rsp_param;

    appl_get_handle_from_device_handle
    (
        handle->device_id,
        &rsp_param.appl_handle
    );
    retval = API_SUCCESS;

    rsp_param.op_code = value->val[0];

    if (BT_FALSE == appl_cntrl_point_cnfgd)
    {
        retval = APPL_CSC_CCD_IMPROPERLY_CONFIGURED;
    }
    else if (BT_TRUE != appl_hvc_flag)
    {
        if ((value->len >= 1) && (value->val[0] < 0x05))
        {
            switch (value->val[0])
            {
            case APPL_CSC_SET_CUMMULATIVE_VALUE_OPCODE:
                if (value->len >= (APPL_CSC_CUMMULATIVE_VALUE_LENGTH + 1))
                {
                    BT_mem_copy
                    (
                        csc_obs_data+1,
                        &(value->val[1]),
                        APPL_CSC_CUMMULATIVE_VALUE_LENGTH
                    );
                    csc_count = 0;
                    if (0x05 == value->val[1])
                    {
                        csc_reverse = BT_TRUE;
                    }
                    rsp_param.rsp_code = APPL_CSC_SUCCESS;
                }
                else
                {
                    rsp_param.rsp_code = APPL_CSC_INVALID_PARAMETER;
                }
                retval = API_SUCCESS;
                break;
            case APPL_CSC_UPDATE_SENSOR_LOCATION_OPCODE:
                {
                    if (2 == value->len)
                    {
                        GATT_DB_HANDLE   db_handle;
                        ATT_VALUE        att_value;
                        UCHAR            count;
                        count = 0;
                        rsp_param.rsp_code = APPL_CSC_INVALID_PARAMETER;

                        while (count < APPL_CSC_SUPPORTED_SENSOR_LOC_COUNT)
                        {
                            if (value->val[1] == csc_supported_sensor_location[count])
                            {
                                att_value.len = 1;
                                att_value.val = value->val+1;

                                db_handle = (*handle);
                                db_handle.char_id = GATT_CHAR_SNSR_LOC_INST;
                                BT_gatt_db_set_char_val (&db_handle,&att_value);
                                rsp_param.rsp_code = APPL_CSC_SUCCESS;
                                break;
                            }
                            count ++;
                        }
                    }
                    else
                    {
                        rsp_param.rsp_code = APPL_CSC_INVALID_PARAMETER;
                    }
                }
                break;
            case APPL_CSC_REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE:
                if (1 == value->len)
                {
                    rsp_param.rsp_code = APPL_CSC_SUCCESS;
                }
                else
                {
                    rsp_param.rsp_code = APPL_CSC_INVALID_PARAMETER;
                }
                break;
            default:
                retval = API_SUCCESS;
                rsp_param.rsp_code = APPL_CSC_OPCODE_NOT_SUPPORTED;
                break;
            }
        }
        else
        {
            rsp_param.rsp_code = APPL_CSC_OPCODE_NOT_SUPPORTED;
        }

        BT_start_timer
        (
             &cntrl_point_handle,
             1,
             appl_cscs_cntrl_point_timer_handle,
             &rsp_param,
             sizeof (rsp_param)
         );
    }
    else
    {
        retval = APPL_CSC_PROCEDURE_ALREADY_IN_PROGRESS;
    }
    return retval;
}

#endif /* CSCS */

