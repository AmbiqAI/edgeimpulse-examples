
/**
 *  \file appl_hts.c
 *
 *  This file contains the Sample Health Thermometer Sensor application.
 *  Sample applications detailed below:
 *      a. The Sensor, as defined by the Sepcification plays the GAP Peripheral
 *         role.
 *      b. The Sensor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Battery
 *           - Device Information and
 *           - Health Thermometer
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Health Thermometer Service exposes:
 *           - Temperature Measurement
 *           - Intermediate Temperature Measurement
 *           - Temperature Type
 *      d. Temperature Measurement and Intermediate Temperature Measurement can
 *         be configured to send Indciations and Notifications repectively.
 *      e. appl_manage_transfer routine takes care of handling peer
 *         configuration. This handling would be needed:
 *           - When Peer Configures Measurement Transfer by writting to the
 *             Characteristic Client Configuration of Temperature Measurement.
 *           - Subsequent reconnection with bonded device that had already
 *             configured the device for transfer. Please note it is mandatory
 *             for GATT Servers to remember the configurations of bonded GATT
 *             clients.
 *         In order to ensure the above mentioned configurations are correctly
 *         handled, the routine, appl_manage_transfer, is therefore called from:
 *           - gatt_db_temperature_measurment_handler and
 *           - appl_hts_connect
 *         [NOTE]: If link does not have the needed secruity for the service,
 *         transfer will not be initiated.
 *      f. This sample application, once configured, send measurements every
 *         10s. This interval can be altered using the define
 *         APPL_TEMPERATURE_MEASUREMENT_INTERVAL. In absence of a real sensor
 *         to provide temperature readings, a static reading of 36.4 C.
 *         Three intermediate temperature measurements of 35.9 C, 36.1 C and
 *         36.3 C are sent before the sending the Temperature Measurement if
 *         collector has configured the device to send Intermediate Temperature
 *         values.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef HTS

/* --------------------------------------------- Global Definitions */
#define APPL_TEMPERATURE_MEASUREMENT_INTERVAL    10
#define APPL_TEMPERATURE_MEASUREMENT_LENGTH      12
#define APPL_IM_TEMPERATURE_MEASUREMENT_LENGTH    5
#define APPL_IM_TEMPERATURE_COUNT                 3
#define APPL_MSRMT_INTRVL_DATA_INTERVAL           1
#define APPL_MSRMT_INTRVL_DATA_LENGTH             2
#define APPL_OUT_OF_RANGE_ERROR                   (APPL_ERR_ID | 0x80)
#define APPL_VALID_RANGE_DESC                     0x2906

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

static BT_timer_handle timer_handle;
static BT_timer_handle msrmt_intrvl_timer_handle;
static GATT_DB_HANDLE  appl_batt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BATTERY_INST,
    GATT_CHAR_BATTERY_LVL_INST
};

static GATT_DB_HANDLE  appl_temp_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_HEALTH_THERM_INST,
    GATT_CHAR_TEMP_MSRMNT_INST
};

static ATT_ATTR_HANDLE appl_temperature_hndl;
static ATT_ATTR_HANDLE appl_im_temperature_hndl;
static ATT_ATTR_HANDLE appl_msrmt_intrvl_hndl;
static UCHAR           im_temp_enabled;
static UCHAR           appl_batt_update_count;
static UCHAR           appl_batt_percent;
static UINT16          appl_msrmt_intrvl;
static UINT16          appl_min_msrmt_intrvl;
static UINT16          appl_max_msrmt_intrvl;


/* Temperature Measurement Format Float. Value 36.4 corresponds to 0xFB378AC0 */
static UCHAR temp_obs_data[APPL_TEMPERATURE_MEASUREMENT_LENGTH] = 
{
    /* Flag */
    0x02, 

    /* Temperature Measurement Value */
    0xC0, 0x8A, 0x37, 0xFB,

    /* Time Stamp */
    0xDE, 0x07, 0x04, 0x1D, 0x06, 0x00, 0x01
};

/**
 * Temperature Measurement Format Float. Value(s)
 *  -  35.9 corresponds to 0xFB36C770
 *  -  36.1 corresponds to 0xFB371590
 *  -  36.3 corresponds to 0xFB3763B0
 */
static UCHAR intrm_temp_obs_data[APPL_IM_TEMPERATURE_MEASUREMENT_LENGTH * APPL_IM_TEMPERATURE_COUNT] =
{
    /* 35.9 */
    0x00, 0x70, 0xC7, 0x36, 0xFB,

    /* 36.1 */
    0x00, 0x90, 0x15, 0x37, 0xFB,

    /* 36.3 */
    0x00, 0xB0, 0x63, 0x37, 0xFB
};

/* --------------------------------------------- Functions */

void appl_hts_init(void)
{
    im_temp_enabled = BT_FALSE;
    appl_batt_update_count = 0;
    appl_batt_percent = 100;
    appl_msrmt_intrvl = 5;

    appl_hts_server_reinitialize ();
}

void appl_hts_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    UINT16            cli_cnfg;

    cli_cnfg = 0;

    appl_temp_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_batt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    /* If Temperature measurement is configured, initiate transfer */
    appl_temp_db_handle.char_id = GATT_CHAR_TEMP_MSRMNT_INST;

    BT_gatt_db_get_char_val_hndl
    (
        &appl_temp_db_handle,
        &appl_temperature_hndl
    );
    BT_gatt_db_get_char_cli_cnfg(&appl_temp_db_handle,&value);


    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_temp_db_handle,cli_cnfg);
    }

    /* Check if intermediate measurement is configured by the client */
    appl_temp_db_handle.char_id = GATT_CHAR_INTRM_TEMP_INST;
    BT_gatt_db_get_char_cli_cnfg(&appl_temp_db_handle,&value);
    BT_gatt_db_get_char_val_hndl
    (
        &appl_temp_db_handle,
        &appl_im_temperature_hndl
    );
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);
    if (cli_cnfg)
    {
        im_temp_enabled = BT_TRUE;
    }

    appl_temp_db_handle.char_id = GATT_CHAR_MSRMT_INTERVAL_INST;
    /* Get Valid Range */
    BT_gatt_db_get_hld_char_desc_val
    (
        &appl_temp_db_handle,
        &value,
        APPL_VALID_RANGE_DESC
    );

    BT_PACK_LE_2_BYTE(&appl_min_msrmt_intrvl,&value.val[0]);
    BT_PACK_LE_2_BYTE(&appl_max_msrmt_intrvl,&value.val[2]);

    BT_gatt_db_get_char_cli_cnfg(&appl_temp_db_handle,&value);
    BT_gatt_db_get_char_val_hndl
    (
        &appl_temp_db_handle,
        &appl_msrmt_intrvl_hndl
    );
}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    API_RESULT     retval;
    APPL_HANDLE    appl_handle;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_temp_db_handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_temp_db_handle,&ekey_size);

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

    appl_get_handle_from_device_handle (handle.device_id,&appl_handle);

    /* Security requirements satisfied, go ahead with data transfer */
    if (GATT_CHAR_TEMP_MSRMNT_INST == handle.char_id)
    {
        if (GATT_CLI_CNFG_INDICATION == config)
        {
            /* Start timer to simulate Temperature measurements */
            if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
            {
                BT_stop_timer (timer_handle);
                APPL_TRC (
                "[APPL]: Stopped Timer %p\n",timer_handle);
                timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            BT_start_timer
            (
                &timer_handle,
                appl_msrmt_intrvl,
                appl_timer_expiry_handler,
                &appl_handle,
                sizeof (appl_handle)
            );
            APPL_TRC (
            "[APPL]: Started Timer %p\n", timer_handle);
        }
        else if (GATT_CLI_CNFG_DEFAULT == config)
        {
            /* Stop tranfer simulate Temperature measurements */
            if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
            {
                BT_stop_timer (timer_handle);
                APPL_TRC (
                "[APPL]: Stopped Timer %p\n", timer_handle);
                timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
        }
    }
    else if (GATT_CHAR_INTRM_TEMP_INST == handle.char_id)
    {
        if (GATT_CLI_CNFG_NOTIFICATION == config)
        {
            im_temp_enabled = BT_TRUE;
        }
        else if (GATT_CLI_CNFG_DEFAULT == config)
        {
            im_temp_enabled = BT_FALSE;
        }
        else
        {
            APPL_TRC ("[*** ERR ***]:Not the right transfer type!");
        }
    }
    else if (GATT_CHAR_MSRMT_INTERVAL_INST == handle.char_id)
    {
        if (GATT_CLI_CNFG_INDICATION == config)
        {
            /* Start timer to simulate Measurment Interval Indication */
            if (BT_TIMER_HANDLE_INIT_VAL != msrmt_intrvl_timer_handle)
            {
                BT_stop_timer (msrmt_intrvl_timer_handle);
                APPL_TRC (
                "[APPL]: Stopped Timer %p\n",msrmt_intrvl_timer_handle);
                msrmt_intrvl_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            BT_start_timer
            (
                &msrmt_intrvl_timer_handle,
                APPL_MSRMT_INTRVL_DATA_INTERVAL,
                appl_msrmt_intrvl_timer_expiry_handler,
                &appl_handle,
                sizeof (appl_handle)
            );
            APPL_TRC (
            "[APPL]: Started Timer %p\n", timer_handle);

        }
    }
    else
    {
        APPL_TRC ("[*** ERR ***]:Not the right transfer type!");
    }
}

API_RESULT appl_validate_msrmt_interval (GATT_DB_HANDLE * handle, UINT16 data)
{
    if (0x0000 == data)
    {
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            BT_stop_timer (timer_handle);
            APPL_TRC (
            "[APPL]: Stopped Timer %p\n", timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
    }
    else if (!(data >= appl_min_msrmt_intrvl && data <= appl_max_msrmt_intrvl))
    {
        return APPL_OUT_OF_RANGE_ERROR;
    }
    appl_msrmt_intrvl = data;
    return API_SUCCESS;
}

void appl_msrmt_intrvl_timer_expiry_handler (void *data_param, UINT16 datalen)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    UCHAR temp_msrmt_int_buff[2];
    API_RESULT retval;

    /* Stop timer */
    if (BT_TIMER_HANDLE_INIT_VAL != msrmt_intrvl_timer_handle)
    {
        BT_stop_timer (msrmt_intrvl_timer_handle);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", msrmt_intrvl_timer_handle);
        msrmt_intrvl_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    hndl_val_param.handle     = appl_msrmt_intrvl_hndl;
    hndl_val_param.value.val  = temp_msrmt_int_buff;
    BT_PACK_LE_2_BYTE(hndl_val_param.value.val,&appl_msrmt_intrvl);
    hndl_val_param.value.len  = APPL_MSRMT_INTRVL_DATA_LENGTH;

    retval = BT_att_send_hndl_val_ind
             (
                 &APPL_GET_ATT_INSTANCE(*((APPL_HANDLE *)data_param)),
                 &hndl_val_param
              );
    if (API_SUCCESS != retval)
    {
        APPL_TRC ("[** ERR **]: Failed to send measurement, reason 0x%04X",
        retval);
    }
}

void appl_timer_expiry_handler (void *data_param, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_VALUE          value;

    fsm_param.handle = (*((APPL_HANDLE *)data_param));
    /* Restart timer to simulate Temperature measurements */
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
        appl_msrmt_intrvl,
        appl_timer_expiry_handler,
        &fsm_param.handle,
        sizeof(fsm_param.handle)
    );

    APPL_TRC (
    "[APPL]: Started Timer %p\n",timer_handle);

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


void appl_hts_server_reinitialize (void)
{
    /* Stop Timer for Temperature measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    if (BT_TIMER_HANDLE_INIT_VAL != msrmt_intrvl_timer_handle)
    {
        BT_stop_timer (msrmt_intrvl_timer_handle);
        msrmt_intrvl_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
}

void appl_send_temperature_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 im_index;

    /**
     *  Transfer intermediate measurements before transferring the
     *  stable measurement
     */
    im_index = 0;
    if (BT_FALSE != im_temp_enabled)
    {
        do
        {
            hndl_val_param.handle =  appl_im_temperature_hndl;
            hndl_val_param.value.val = (intrm_temp_obs_data+(5*im_index));
            hndl_val_param.value.len = APPL_IM_TEMPERATURE_MEASUREMENT_LENGTH;
            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
            im_index++;
        }while (im_index < APPL_IM_TEMPERATURE_COUNT);
    }
    hndl_val_param.handle =  appl_temperature_hndl;
    hndl_val_param.value.val = temp_obs_data;
    hndl_val_param.value.len = APPL_TEMPERATURE_MEASUREMENT_LENGTH;

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


#endif /* HTS */

