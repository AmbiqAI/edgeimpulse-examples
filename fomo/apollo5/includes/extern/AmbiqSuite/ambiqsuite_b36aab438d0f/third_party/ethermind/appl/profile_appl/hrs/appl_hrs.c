
/**
 *  \file appl_hrs.c
 *
 *  This file contains the Heart Rate Sensor application.
  *  Sample applications detailed below:
 *      a. The Sensor, as defined by the Sepcification plays the GAP Peripheral
 *         role.
 *      b. The Sensor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Battery
 *           - Device Information and
 *           - Heart Rate
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Heart Rate Service exposes:
 *           - Heart Rate Measurement
 *           - Sensor Location
 *           - Heart Rate Control Point
 *      d. Heart Rate Measurement can be configured to send Notifications.
 *      e. appl_manage_transfer routine takes care of handling peer
 *         configuration. This handling would be needed:
 *           - When Peer Configures Measurement Transfer by writting to the
 *             Characteristic Client Configuration of Heart Rate Measurement.
 *           - Subsequent reconnection with bonded device that had already
 *             configured the device for transfer. Please note it is mandatory
 *             for GATT Servers to remember the configurations of bonded GATT
 *             clients.
 *         In order to ensure the above mentioned configurations are correctly
 *         handled, the routine, appl_manage_transfer, is therefore called from:
 *           - gatt_db_hrs_handler and
 *           - appl_hrs_connect
 *         [NOTE]: If link does not have the needed secruity for the service,
 *         transfer will not be initiated.
 *      f. This sample application, once configured, send measurements every
 *         1s. This interval can be altered using the define
 *         APPL_HEART_RATE_MSURMNT_INTERVAL. In absence of a real sensor
 *         to provide heart rate readings, a static reading of 86 bpm is being
 *         sent. Energy Expended is incremented every 5 seconds. This value can
 *         be reset reset by the Collector device through the Heart Rate
 *         Control Point. See appl_hr_control_point_handler for more details.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef HRS

/* --------------------------------------------- Constants */
#define APPL_HEART_RATE_MSRMNT_LENGTH          4
#define APPL_HEART_RATE_MSURMNT_INTERVAL       1
#define APPL_HEART_RATE_MSURMNT_COUNT          2

/* --------------------------------------------- Static Global Variables */

static UCHAR hr_obs_data[] = 
{
    /* Heart Rate Value 1 Byte with Energy Expended */
    0x08, 0x56, 0x00, 0x00,

    /* Heart Rate Value 2 Bytes with RR Interval and Sensor Contact */
    0x15, 0x56, 0x00, 0x00, 0x00
};

static APPL_HRS_OBS_DATA_INFO hr_obs_data_info[APPL_HEART_RATE_MSURMNT_COUNT] =
{
    {
        /* Index */
        0x00,

        /* Length*/
        0x04
    },
    {
        /* Index */
        0x04,

        /* Length */
        0x05
    }
};

static UCHAR hr_count;
static GATT_DB_HANDLE appl_hrs_handle;
static BT_timer_handle timer_handle;
static ATT_ATTR_HANDLE appl_hr_hndl;

/* --------------------------------------------- Functions */

void appl_hrs_init(void)
{
    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    appl_hrs_server_reinitialize ();
}

void appl_hrs_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    UINT16            cli_cnfg;

    cli_cnfg = 0;



    appl_hrs_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    /* If Heart Rate measurement is configured, initiate transfer */
    appl_hrs_handle.char_id = GATT_CHAR_HR_MSRMNT;
    appl_hrs_handle.service_id = GATT_SER_HEART_RATE_INST;

    BT_gatt_db_get_char_val_hndl (&appl_hrs_handle,&appl_hr_hndl);
    BT_gatt_db_get_char_cli_cnfg(&appl_hrs_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg,APPL_GET_DEVICE_HANDLE((*appl_handle)));

    appl_manage_trasnfer(appl_hrs_handle,cli_cnfg);
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
            timer_handle= BT_TIMER_HANDLE_INIT_VAL;
        }


        BT_start_timer
        (
            &timer_handle,
            APPL_HEART_RATE_MSURMNT_INTERVAL,
            appl_timer_expiry_handler,
            &appl_handle,
            sizeof (appl_handle)
        );
        APPL_TRC (
        "[APPL]: Started Timer %p\n", timer_handle);
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
}

void appl_send_heart_rate_measurement (APPL_HANDLE   * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UINT16                indx;

    APPL_TRC (
    "[0x%02X]:Sending measurement On Handle 0x%04X\n",
    (*handle), appl_hr_hndl);

    for (indx = 0; indx < APPL_HEART_RATE_MSURMNT_COUNT; indx++)
    {
        hndl_val_param.handle =  appl_hr_hndl;
        hndl_val_param.value.val = &hr_obs_data[hr_obs_data_info[indx].index];
        hndl_val_param.value.len = hr_obs_data_info[indx].length;

        if ((1 == indx) && (1 == hr_count))
        {
            hr_obs_data[hr_obs_data_info[indx].index] = 0x17;
        }

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
    }

    if (5 == hr_count)
    {
        if (0xFF == hr_obs_data[0x02])
        {
            hr_obs_data[0x03]++;
        }
        hr_obs_data[0x02]++;
        hr_obs_data[hr_obs_data_info[1].index] = 0x15;
        hr_count = 0;
    }
    hr_count++;
    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "[** ERR **]: Failed to send measurement, reason 0x%04X",
        retval);
    }
}


void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM    fsm_param;
    fsm_param.handle = (*((APPL_HANDLE *)data));

    /* Restart timer to simulate Heart Rate measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &timer_handle,
        APPL_HEART_RATE_MSURMNT_INTERVAL,
        appl_timer_expiry_handler,
        &fsm_param.handle,
        sizeof (fsm_param.handle)
    );

    APPL_TRC (
    "[APPL]: Started Timer %p\n",timer_handle);
    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_measurement_ind,
        &fsm_param
    );
}

void appl_hrs_server_reinitialize (void)
{
    /* Turn Off Transfer Timer if On.  */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    hr_count = 0;
}

API_RESULT appl_hr_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           )
{
    API_RESULT retval;

    retval = APPL_HR_CNTRL_PNT_NOT_SUPPORTED;

    if ((1 == value->len) && (0x01 == value->val[0]))
    {
        hr_obs_data[2] = 0x00;
        hr_obs_data[3] = 0x00;
        hr_count = 0;
        retval = API_SUCCESS;
    }

    return retval;
}

#endif /* HRS */

