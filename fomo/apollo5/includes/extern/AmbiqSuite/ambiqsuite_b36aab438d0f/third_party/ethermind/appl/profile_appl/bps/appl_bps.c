/**
 *  \file appl_bps.c
 *
 *  This file contains the Sample Blood Pressure Monitor application.
 *  Sample applications detailed below:
 *      a. The Monitor, as defined by the Sepcification plays the GAP Peripheral
 *         role.
 *      b. The Monitor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Battery
 *           - Device Information and
 *           - Blood Pressure
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Blood Pressure Service exposes:
 *           - Blood Pressure Measurement
 *           - Intermediate Cuff Pressure
 *           - Blood Pressure Feature
 *      d. Blood Pressure Measurement and Intermediate Cuff Pressure can
 *         be configured to send Indciations and Notifications repectively.
 *      e. appl_manage_transfer routine takes care of handling peer
 *         configuration. This handling would be needed:
 *           - When Peer Configures Measurement Transfer by writting to the
 *             Characteristic Client Configuration of Blood Pressure Measurement.
 *           - Subsequent reconnection with bonded device that had already
 *             configured the device for transfer. Please note it is mandatory
 *             for GATT Servers to remember the configurations of bonded GATT
 *             clients.
 *         In order to ensure the above mentioned configurations are correctly
 *         handled, the routine, appl_manage_transfer, is therefore called from:
 *           - gatt_db_bps_handler and
 *           - appl_bps_connect
 *         [NOTE]: If link does not have the needed secruity for the service,
 *         transfer will not be initiated.
 *      f. This sample application, once configured, send measurements every
 *         10s. This interval can be altered using the define
 *         APPL_BP_MEASUREMENT_INTERVAL. In absence of a real sensor to provide
 *         blood pressure readings, a static reading of 120-80-100 mmHg.
 *         Three intermediate cuff pressures of 130-95-80 mmHg, 126-90-85 mmHg
 *         and 123-85-90 mmHg are sent before the sending the Blood Pressure
 *         Measurement if collector has configured the device to send
 *         Intermediate Cuff Pressure values.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef BPS

/* --------------------------------------------- Global Definitions */
#define APPL_BP_MEASUREMENT_INTERVAL                 5
#define APPL_BLOOD_PRESSURE_MEASUREMENT_LENGTH       19
#define APPL_IM_BLOOD_PRESSURE_MEASUREMENT_LENGTH    19
#define APPL_IM_BLOOD_PRESSURE_COUNT                 3

#define APPL_BPS_ADV_DATA_LEN                        21

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

static BT_timer_handle timer_handle;

static GATT_DB_HANDLE  appl_batt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BATTERY_INST,
    GATT_CHAR_BATTERY_LVL_INST
};

static GATT_DB_HANDLE  appl_bps_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BP_INST,
    GATT_CHAR_BP_MSRMNT_INST
};

static ATT_ATTR_HANDLE appl_blood_pressure_hndl;
static ATT_ATTR_HANDLE appl_im_blood_pressure_hndl;
static UCHAR           im_bpm_enabled = BT_FALSE;
static UCHAR           bpm_enabled;
static UCHAR           appl_batt_update_count;
static UCHAR           appl_batt_percent;
UCHAR                  timer_started;

/* Blood Pressure Measurement Format SFLOAT */
static UCHAR bp_obs_data[APPL_BLOOD_PRESSURE_MEASUREMENT_LENGTH] =
{
    /**
     *  1st Octet         - Flag
     *  2nd & 3rd Octet   - Systolic  - 120 mmHg corresponds to 0x0078
     *  4th & 5th Octet   - Diastolic -  80 mmHg corresponds to 0x0050
     *  6th & 7th Octet   - MAP       - 100 mmHg corresponds to 0x0064
     *  8th - 14th Octet  - Time Stamp
     *  15th & 16th Octet - Pulse Rate
     *  17th Octet        - User Id
     *  18th & 19th Octet - Measurement Status
     */
    0x1E, 0x78, 0x00, 0x50, 0x00, 0x64, 0x00,
    0xDE, 0x07, 0x04, 0x1D, 0x06, 0x00, 0x1E,
    0x50, 0x00, 0x00, 0x00, 0x00
};

/* Intermediate Cuff Pressure Format SFLOAT */
static UCHAR intrm_bp_obs_data\
[APPL_IM_BLOOD_PRESSURE_MEASUREMENT_LENGTH * APPL_IM_BLOOD_PRESSURE_COUNT] =
{
    /**
     *  1st Octet         - Flag
     *  2nd & 3rd Octet   - Systolic  - 130 mmHg corresponds to 0x0082
     *  4th & 5th Octet   - Diastolic -  95 mmHg corresponds to 0x005F
     *  6th & 7th Octet   - MAP       -  80 mmHg corresponds to 0x0050
     *  8th - 14th Octet  - Time Stamp
     *  15th & 16th Octet - Pulse Rate
     *  17th Octet        - User Id
     *  18th & 19th Octet - Measurement Status
     */
    0x1E, 0x82, 0x00, 0x5F, 0x00, 0x50, 0x00,
    0xDE, 0x07, 0x04, 0x1D, 0x06, 0x00, 0x01,
    0x50, 0x00, 0x00, 0x00, 0x00,

    /**
     *  1st Octet         - Flag
     *  2nd & 3rd Octet   - Systolic  - 126 mmHg corresponds to 0x007E
     *  4th & 5th Octet   - Diastolic -  90 mmHg corresponds to 0x005A
     *  6th & 7th Octet   - MAP       -  85 mmHg corresponds to 0x0055
     *  8th - 14th Octet  - Time Stamp
     *  15th & 16th Octet - Pulse Rate
     *  17th Octet        - User Id
     *  18th & 19th Octet - Measurement Status
     */
    0x1E, 0x7E, 0x00, 0x5A, 0x00, 0x55, 0x00,
    0xDE, 0x07, 0x04, 0x1D, 0x06, 0x00, 0x0A,
    0x50, 0x00, 0x00, 0x00, 0x00,

    /**
     *  1st Octet         - Flag
     *  2nd & 3rd Octet   - Systolic  - 123 mmHg corresponds to 0x007B
     *  4th & 5th Octet   - Diastolic -  85 mmHg corresponds to 0x0055
     *  6th & 7th Octet   - MAP       -  90 mmHg corresponds to 0x005A
     *  8th - 14th Octet  - Time Stamp
     *  15th & 16th Octet - Pulse Rate
     *  17th Octet        - User Id
     *  18th & 19th Octet - Measurement Status
     */
    0x1E, 0x7B, 0x00, 0x55, 0x00, 0x5A, 0x00,
    0xDE, 0x07, 0x04, 0x1D, 0x06, 0x00, 0x12,
    0x50, 0x00, 0x00, 0x00, 0x00

};

UCHAR appl_bps_feature_value[] = { 0x00, 0x00 };

/* --------------------------------------------- Functions */

void appl_bps_init(void)
{
    appl_batt_update_count = 0;
    appl_batt_percent = 100;

    appl_bps_server_reinitialize ();
}

void appl_bps_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    UINT16            cli_cnfg;

    cli_cnfg = 0;

#ifndef APPL_BPS_SINGLE_BOND_SUPPORT
    appl_set_target_address_in_adv_data(APPL_BPS_ADV_DATA_LEN);
#endif /* APPL_BPS_SINGLE_BOND_SUPPORT */

    appl_bps_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_batt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_bps_db_handle.char_id = GATT_CHAR_BP_MSRMNT_INST;

    BT_gatt_db_get_char_val_hndl
    (
        &appl_bps_db_handle,
        &appl_blood_pressure_hndl
    );
    BT_gatt_db_get_char_cli_cnfg (&appl_bps_db_handle,&value);

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));

    appl_manage_trasnfer(appl_bps_db_handle,cli_cnfg);

    /* Check if intermediate cuff pressure is configured by the client */
    appl_bps_db_handle.char_id = GATT_CHAR_INTRM_CUFF_PRSR_INST;
    BT_gatt_db_get_char_cli_cnfg(&appl_bps_db_handle,&value);
    BT_gatt_db_get_char_val_hndl
    (
        &appl_bps_db_handle,
        &appl_im_blood_pressure_hndl
    );
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);
    if (cli_cnfg)
    {
        im_bpm_enabled = BT_TRUE;
    }

#ifdef APPL_BPS_SINGLE_BOND_SUPPORT
    appl_bps_db_handle.char_id = GATT_CHAR_BP_FEATURE_INST;

    value.val = appl_bps_feature_value;
    value.len = sizeof(appl_bps_feature_value);

    /* Set Multi Bond bit in Glucose feature to zero */
    retval = BT_gatt_db_set_char_val
             (
                &appl_bps_db_handle,
                &value
             );
#endif /* APPL_BPS_SINGLE_BOND_SUPPORT */

}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    API_RESULT     retval;
    APPL_HANDLE    appl_handle;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_bps_db_handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_bps_db_handle,&ekey_size);

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

    if (GATT_CHAR_BP_MSRMNT_INST == handle.char_id)
    {
        appl_get_handle_from_device_handle (handle.device_id,&appl_handle);

        if (GATT_CLI_CNFG_INDICATION == config)
        {
            /* Set the Flag for BP Measurements requested to True */
            bpm_enabled = BT_TRUE;

            if (timer_started != BT_TRUE)
            {
                appl_bps_start_timer(&appl_handle, sizeof(appl_handle));
            }
        }
        else if (GATT_CLI_CNFG_DEFAULT == config)
        {
            bpm_enabled = BT_FALSE;

            if ((BT_FALSE == bpm_enabled) && (BT_FALSE == im_bpm_enabled))
            {
               /* Stop tranfer simulate Blood Pressure measurements
                * only if both BPM and IM_CUFF are disabled
                */
                if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
                {
                    BT_stop_timer (timer_handle);
                    APPL_TRC (
                    "[APPL]: Stopped Timer %p\n",timer_handle);
                    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                }

                timer_started = BT_FALSE;
            }
        }
    }
    else
    {
        if (GATT_CHAR_INTRM_CUFF_PRSR_INST == handle.char_id)
        {
            appl_get_handle_from_device_handle(handle.device_id, &appl_handle);

            if (GATT_CLI_CNFG_NOTIFICATION == config)
            {
                im_bpm_enabled = BT_TRUE;

                if (timer_started != BT_TRUE)
                {
                    appl_bps_start_timer(&appl_handle, sizeof(appl_handle));
                }
            }
            else if (GATT_CLI_CNFG_DEFAULT == config)
            {
                im_bpm_enabled = BT_FALSE;

                if ((BT_FALSE == bpm_enabled) && (BT_FALSE == im_bpm_enabled))
                {
                    /* Stop tranfer simulate Blood Pressure measurements
                     * only if both BPM and IM_CUFF are disabled
                     */
                    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
                    {
                        BT_stop_timer (timer_handle);
                        APPL_TRC (
                        "[APPL]: Stopped Timer %p\n",timer_handle);
                        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                    }

                    timer_started = BT_FALSE;
                }
            }
            else
            {
                APPL_TRC ("[*** ERR ***]:Not the right transfer type!");
            }
        }
        else
        {
            APPL_TRC ("[*** ERR ***]:Not the right transfer type!");
        }
    }
}


void appl_bps_start_timer(APPL_HANDLE * hndl, UINT16 hndl_len)
{
    timer_started = BT_TRUE;

    /* Start timer to simulate Blood Pressure measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer(timer_handle);
        APPL_TRC(
            "[APPL]: Stopped Timer %p\n", timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &timer_handle,
        APPL_BP_MEASUREMENT_INTERVAL,
        appl_timer_expiry_handler,
        (void *) hndl,
        hndl_len
    );

    APPL_TRC(
        "[APPL]: Started Timer %p\n", timer_handle);
}

void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_VALUE          value;

    /* Setting Timer Start Flag to False */
    timer_started = BT_FALSE;

    fsm_param.handle = (*((APPL_HANDLE *)data));

    appl_bps_start_timer(&fsm_param.handle, sizeof(fsm_param.handle));

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


void appl_bps_server_reinitialize (void)
{
    /* Stop Timer for Blood Pressure measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    timer_started = BT_FALSE;

    im_bpm_enabled = BT_FALSE;
    bpm_enabled    = BT_FALSE;
}

void appl_send_blood_pressure_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 im_index;

    /**
     *  Transfer intermediate measurements before transferring the
     *  stable measurement
     */
    im_index = 0;
    if (BT_FALSE != im_bpm_enabled)
    {
        do
        {
            hndl_val_param.handle =  appl_im_blood_pressure_hndl;
            hndl_val_param.value.val = \
                (intrm_bp_obs_data+(APPL_BLOOD_PRESSURE_MEASUREMENT_LENGTH*im_index));
            hndl_val_param.value.len = APPL_BLOOD_PRESSURE_MEASUREMENT_LENGTH;
            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
            im_index++;
        }while (im_index < APPL_IM_BLOOD_PRESSURE_COUNT);
    }

    if (BT_FALSE != bpm_enabled)
    {
        hndl_val_param.handle =  appl_blood_pressure_hndl;
        hndl_val_param.value.val = bp_obs_data;
        hndl_val_param.value.len = APPL_BLOOD_PRESSURE_MEASUREMENT_LENGTH;

        retval = BT_att_send_hndl_val_ind
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send measurement, reason 0x%04X",
            retval);
        }
    }
}

#endif /* BPS */

