
/**
 *  \file appl_hid.c
 *
 *  This file contains the Sample HID Boot Mouse application.
 *  Sample applications detailed below:
 *      a. The Sensor, as defined by the Sepcification plays the GAP Peripheral
 *         role.
 *      b. The Sensor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Device Information
 *           - Battery
 *           - Human Interface Device
 *           - Scan Parameter
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Human Interface Device Service exposes:
 *           - Protocol Mode
 *           - Report Map
 *           - Report
 *           - Report Map
 *           - HID Control Point
 *           - HID Information
 *           - Boot Keyboard Input Report
 *           - Boot Keyboard Output Report
 *           - Boot Mouse Input Report
 *      d. Boot Mouse Input Report can be configured to send
 *         Indciations and Notifications repectively.
 *      e. appl_manage_transfer routine takes care of handling peer
 *         configuration. This handling would be needed:
 *           - When Peer Configures Event Transfer by writting to the
 *             Characteristic Client Configuration of Boot Mouse Input Report.
 *           - Subsequent reconnection with bonded device that had already
 *             configured the device for transfer. Please note it is mandatory
 *             for GATT Servers to remember the configurations of bonded GATT
 *             clients.
 *         In order to ensure the above mentioned configurations are correctly
 *         handled, the routine, appl_manage_transfer, is therefore called from:
 *           - gatt_db_hid_boot_mouse_event_handler and
 *           - appl_hid_connect
 *         [NOTE]: If link does not have the needed secruity for the service,
 *         transfer will not be initiated.
 *      f. This sample application, once configured, send different mouse events
 *         every 10s. This interval can be altered using the define
 *         APPL_HID_EVENT_INTERVAL.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef HID

/* --------------------------------------------- Global Definitions */
#define APPL_HID_EVENT_INTERVAL                   1 /* 5 */
#define APPL_HID_BOOT_MOUSE_REPORT_LENGTH         4
#define APPL_HID_BOOT_MOUSE_EVENT_COUNT           6
#define APPL_HID_BOOT_KBD_REPORT_LENGTH           9
#define APPL_HID_BOOT_KBD_EVENT_COUNT             12
#define APPL_BOOT_EVENT_OFFSET                    1

#define APPL_HID_BOOT_MODE                        0x00
#define APPL_HID_REPORT_MODE                      0x01

#define APPL_SERVER_REQUIRES_SCAN_REFRESH         0x00
#define APPL_SCAN_REFRESH_DATA_LENGTH             1
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

static GATT_DB_HANDLE  appl_hid_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_HID_INST,
    GATT_CHAR_HID_MOUSE_IN_INST
};

static GATT_DB_HANDLE  appl_sps_scan_refresh_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_SCAN_PARAM_INST,
    GATT_CHAR_SCAN_REFRESH_INST
};

static ATT_ATTR_HANDLE appl_hid_report_input_hndl;
static ATT_ATTR_HANDLE appl_hid_boot_mouse_input_hndl;
static ATT_ATTR_HANDLE appl_hid_boot_kbd_input_hndl;
static ATT_ATTR_HANDLE appl_hid_bat_lvl_hndl;
static UCHAR           appl_batt_update_count;
static UCHAR           appl_batt_percent;
static UCHAR           mouse_event_index;
static UCHAR           kbd_event_index;
static UCHAR           appl_boot_mouse_input;
static UCHAR           appl_boot_keyboard_input;
static UCHAR           appl_report_input;
static UCHAR           appl_bat_lvl;

static ATT_ATTR_HANDLE appl_sps_scan_refresh_hndl;
static UCHAR           appl_scan_ref_enabled;
/* This flag is used to send one one scan refresh notification when CCCD is configured */
static UCHAR           appl_scan_ref_sent;

/* Protocol Mode */
/* TODO: Not maintaining per bonded peer device */
static UCHAR appl_hid_mouse_proto_mode[1];

static UCHAR appl_hid_boot_mouse_event_make[APPL_HID_BOOT_MOUSE_REPORT_LENGTH * APPL_HID_BOOT_MOUSE_EVENT_COUNT] =
{
   /* Mouse Left Button */
   0x01, 0x00, 0xF5, 0x00,

   /* Mouse Right Button*/
   0x01, 0x02, 0x00, 0x00,

   /* Mouse Move Left */
   0x01,0x01, 0x10, 0x00,

   /* Mouse Move Right */
   0x01, 0x00, 0x0F, 0x00,

   /* Mouse Move Up */
   0x01, 0x00, 0x00, 0xF5,

   /* Mouse Move Down */
   0x01, 0x00, 0x00, 0x0F

};
static UCHAR appl_hid_boot_kbd_event_make[APPL_HID_BOOT_KBD_REPORT_LENGTH * APPL_HID_BOOT_KBD_EVENT_COUNT] =
{
    /* small letters */
    0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x02, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x02, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x02, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* --------------------------------------------- Functions */

void appl_hid_init(void)
{
    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    appl_batt_update_count = 0;
    appl_batt_percent = 100;

    /* By default, Report Mode */
    appl_hid_mouse_proto_mode[0] = APPL_HID_REPORT_MODE;

    appl_hid_server_reinitialize ();
}

void appl_hid_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    API_RESULT        retval;
    UINT16            cli_cnfg;

    cli_cnfg = 0;

    appl_hid_db_handle.device_id  = APPL_GET_DEVICE_HANDLE((*appl_handle));
    appl_batt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    value.len = 1;
    value.val = &appl_hid_mouse_proto_mode[0];

    /* Update Protocol Mode Value with Report Mode */
    appl_hid_db_handle.char_id = GATT_CHAR_PROTO_MODE_INST;
    retval = BT_gatt_db_set_char_val
             (
                 &appl_hid_db_handle,
                 &value
             );

    /* If HID Mouse Input Report is configured, initiate transfer */
    appl_hid_db_handle.char_id = GATT_CHAR_HID_REPORT_INST0;

    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_hid_db_handle,
                  &appl_hid_report_input_hndl
             );
    retval = BT_gatt_db_get_char_cli_cnfg(&appl_hid_db_handle,&value);

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer (appl_hid_db_handle,cli_cnfg);
    }

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X) with "
    "result 0x%04X for Attr Handle 0x%04X", cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)), retval, appl_hid_report_input_hndl);

    /* If HID Boot Mouse Input Report is configured, initiate transfer */
    appl_hid_db_handle.char_id = GATT_CHAR_HID_MOUSE_IN_INST;

    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_hid_db_handle,
                  &appl_hid_boot_mouse_input_hndl
             );
    retval = BT_gatt_db_get_char_cli_cnfg(&appl_hid_db_handle,&value);

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer (appl_hid_db_handle,cli_cnfg);
    }

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X) with "
    "result 0x%04X for Attr Handle 0x%04X", cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)), retval, appl_hid_boot_mouse_input_hndl);

    /* If HID Boot Keyboard Input Report is configured, initiate transfer */
    appl_hid_db_handle.char_id = GATT_CHAR_HID_KBD_IN_INST;

    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_hid_db_handle,
                  &appl_hid_boot_kbd_input_hndl
             );
    retval = BT_gatt_db_get_char_cli_cnfg(&appl_hid_db_handle,&value);

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer (appl_hid_db_handle,cli_cnfg);
    }

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X) with "
    "result 0x%04X for Attr Handle 0x%04X", cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)), retval, appl_hid_boot_kbd_input_hndl);

    /* Scan Parameter Service Related handlings */
    appl_sps_scan_refresh_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_sps_scan_refresh_db_handle,
        &appl_sps_scan_refresh_hndl
    );
    BT_gatt_db_get_char_cli_cnfg(&appl_sps_scan_refresh_db_handle,&value);

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_sps_scan_refresh_db_handle,cli_cnfg);
    }

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X) with "
    "result 0x%04X for Attr Handle 0x%04X", cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)), retval, appl_sps_scan_refresh_hndl);
}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    API_RESULT     retval;
    APPL_HANDLE    appl_handle;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_hid_db_handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_hid_db_handle,&ekey_size);

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

    /**
     *  Security requirements satisfied.
     *
     *  Check if CCCD configured for HID Boot/Report Mode Input.
     *  Initiate transfer of simulated reported.
     */

    /**
     *  TODO: If the Input Report Descriptor position changes
     *  in the database, the following piece of code also to
     *  be modified.
     */
    appl_get_handle_from_device_handle (handle.device_id,&appl_handle);

    if (GATT_CLI_CNFG_NOTIFICATION == config)
    {
        if (GATT_CHAR_HID_MOUSE_IN_INST == handle.char_id)
        {
            appl_boot_mouse_input = BT_TRUE;
        }
        else if (GATT_CHAR_HID_KBD_IN_INST == handle.char_id)
        {
            appl_boot_keyboard_input = BT_TRUE;
        }
        else if (GATT_CHAR_HID_REPORT_INST0 == handle.char_id)
        {
            appl_report_input = BT_TRUE;
        }
        else if (GATT_CHAR_SCAN_REFRESH_INST == handle.char_id)
        {
            appl_scan_ref_enabled = BT_TRUE;

            /* Set flag as not sent */
            appl_scan_ref_sent    = BT_FALSE;
        }
        else
        {
            /* Do Nothing */
            return;
        }


        /* Start timer to simulate HID boot mouse events */
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
                     APPL_HID_EVENT_INTERVAL,
                     appl_timer_expiry_handler,
                     &appl_handle,
                     sizeof (appl_handle)
                 );
        APPL_TRC (
        "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        if (GATT_CHAR_HID_MOUSE_IN_INST == handle.char_id)
        {
            appl_boot_mouse_input = BT_FALSE;
        }
        else if (GATT_CHAR_HID_KBD_IN_INST == handle.char_id)
        {
            appl_boot_keyboard_input = BT_FALSE;
        }
        else if (GATT_CHAR_HID_REPORT_INST0 == handle.char_id)
        {
            appl_report_input = BT_FALSE;
        }
        else if (GATT_CHAR_SCAN_REFRESH_INST == handle.char_id)
        {
            appl_scan_ref_enabled = BT_FALSE;

            /* Set Flag as Sent */
            appl_scan_ref_sent    = BT_TRUE;
        }
        else
        {
            /* Do Nothing */
            return;
        }

        /* Stop timer to simulate HID boot mouse events */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            retval = BT_stop_timer (timer_handle);
            APPL_TRC (
            "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
            retval,timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
    }
}

API_RESULT appl_manage_wwr (GATT_DB_HANDLE * handle, GATT_DB_PARAMS * params)
{
    APPL_HANDLE    appl_handle;
    API_RESULT     retval;

    retval = API_SUCCESS;

    appl_get_handle_from_device_handle (handle->device_id,&appl_handle);

    if (GATT_CHAR_PROTO_MODE_INST == handle->char_id)
    {
        /* Verify and Save Protocol Mode */
        if ((1 == params->value.len) &&
            (APPL_HID_REPORT_MODE >= params->value.val[0]))
        {
            /* Update Protocol Mode Value */
            BT_gatt_db_set_char_val
            (
                handle,
                &params->value
            );
            APPL_TRC (
            "[APPL]: Successfully updated Protocol Mode Value\n");
        }
        else
        {
            retval = API_FAILURE;
            APPL_TRC (
            "[APPL]: Failed to update Protocol Mode Value\n");
        }
    }
    return retval;
}

void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_VALUE          value;
    API_RESULT         retval;

    fsm_param.handle = (*((APPL_HANDLE *)data));

    /* Restart timer to simulate HID Boot Mouse events */
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
                 APPL_HID_EVENT_INTERVAL,
                 appl_timer_expiry_handler,
                 &fsm_param.handle,
                 sizeof(fsm_param.handle)
             );

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

    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,timer_handle);
}


void appl_hid_server_reinitialize (void)
{
    /* Stop Timer for HID boot mouse events */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
    }
    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
}

void appl_send_hid_event (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    /*  Simulation to send HID events perodically */
    if (APPL_HID_BOOT_MOUSE_EVENT_COUNT == mouse_event_index)
    {
        mouse_event_index = 0;
        kbd_event_index = 0;
    }

    if (appl_boot_mouse_input)
    {
        hndl_val_param.handle =  appl_hid_boot_mouse_input_hndl;
        hndl_val_param.value.val = (appl_hid_boot_mouse_event_make+
                        (APPL_HID_BOOT_MOUSE_REPORT_LENGTH*mouse_event_index++)
                        + APPL_BOOT_EVENT_OFFSET);
        hndl_val_param.value.len = APPL_HID_BOOT_MOUSE_REPORT_LENGTH;

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
            retval);
        }

        hndl_val_param.value.val = (appl_hid_boot_mouse_event_make+
                        (APPL_HID_BOOT_MOUSE_REPORT_LENGTH*mouse_event_index++)
                        + APPL_BOOT_EVENT_OFFSET);

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
            retval);
        }
    }

    if (appl_boot_keyboard_input)
    {
        /* Send Make Event */
        hndl_val_param.handle =  appl_hid_boot_kbd_input_hndl;
        hndl_val_param.value.val = (appl_hid_boot_kbd_event_make +
                        (APPL_HID_BOOT_KBD_REPORT_LENGTH*kbd_event_index++) +
                        APPL_BOOT_EVENT_OFFSET);
        hndl_val_param.value.len = APPL_HID_BOOT_KBD_REPORT_LENGTH;

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
            retval);
        }

        hndl_val_param.value.val = (appl_hid_boot_kbd_event_make +
                        (APPL_HID_BOOT_KBD_REPORT_LENGTH*kbd_event_index++) +
                        APPL_BOOT_EVENT_OFFSET);

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
            retval);
        }

        /* Send Break Event */
        hndl_val_param.value.val = (appl_hid_boot_kbd_event_make+
                        (APPL_HID_BOOT_KBD_REPORT_LENGTH*kbd_event_index++));

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                  );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
            retval);
        }
    }

    if (appl_report_input)
    {
        hndl_val_param.handle =  appl_hid_report_input_hndl;

        /* Send Mouse Event */
        if (APPL_HID_BOOT_KBD_EVENT_COUNT > kbd_event_index)
        {
            /* Send Keyboard Event */
            hndl_val_param.value.val = (appl_hid_boot_kbd_event_make+
                            (APPL_HID_BOOT_KBD_REPORT_LENGTH*kbd_event_index++));
            hndl_val_param.value.len = APPL_HID_BOOT_KBD_REPORT_LENGTH;

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
                retval);
            }

            /* Send Break Event */
            hndl_val_param.value.val = (appl_hid_boot_kbd_event_make+
                            (APPL_HID_BOOT_KBD_REPORT_LENGTH*kbd_event_index++));

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
                retval);
            }
        }
        else
        {
            hndl_val_param.value.val = (appl_hid_boot_mouse_event_make+
                        (APPL_HID_BOOT_MOUSE_REPORT_LENGTH*mouse_event_index++));
            hndl_val_param.value.len = APPL_HID_BOOT_MOUSE_REPORT_LENGTH;

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
                retval);
            }

            hndl_val_param.value.val = (appl_hid_boot_mouse_event_make+
                            (APPL_HID_BOOT_MOUSE_REPORT_LENGTH*mouse_event_index++));

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send Mouse make event, reason 0x%04X",
                retval);
            }
        }
    }

    if ((BT_TRUE == appl_scan_ref_enabled) && (BT_TRUE != appl_scan_ref_sent))
    {
        UCHAR                 value;

        value = APPL_SERVER_REQUIRES_SCAN_REFRESH;

        hndl_val_param.handle =  appl_sps_scan_refresh_hndl;
        hndl_val_param.value.val = &value;
        hndl_val_param.value.len = APPL_SCAN_REFRESH_DATA_LENGTH;
        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );

        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to scan refresh ntf, reason 0x%04X",
            retval);

            /* Flag Scan Refresh as Not-Sent */
            appl_scan_ref_sent = BT_FALSE;
        }
        else
        {
            /* Flag Scan Refresh as Sent */
            appl_scan_ref_sent = BT_TRUE;
        }
    }
}


#endif /* HID */

