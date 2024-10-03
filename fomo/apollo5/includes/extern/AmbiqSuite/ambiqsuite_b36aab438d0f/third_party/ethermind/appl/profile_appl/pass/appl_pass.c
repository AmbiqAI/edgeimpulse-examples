
/**
 *  \file appl_pass.c
 *
 *  This file contains the Sample Phone Alert Status Sensor application.
 *  Sample applications detailed below:
 *      a. The Sensor, as defined by the Specification plays the GAP Central
 *         role.
 *      b. The Sensor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Battery and
 *           - Phone Alert Status
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Phone Alert Status Service exposes:
 *           - Alert Status
 *           - Ringer Settings
 *           - Ringer Control Point
 *      d. Alert Status and Ringer Settings can be configured to send
 *         Notifications.
 *      e. appl_start_transfer routine takes care of handling peer
 *         configuration. This handling would be needed:
 *           - When Peer Configures Alert Status Notifications Transfer by
 *             writing to the Characteristic Client Configuration of Alert
 *             Status.
 *           - Subsequent reconnection with bonded device that had already
 *             configured the device for transfer. Please note it is mandatory
 *             for GATT Servers to remember the configurations of bonded GATT
 *             clients.
 *         In order to ensure the above mentioned configurations are correctly
 *         handled, the routine, appl_start_transfer, is therefore called from:
 *           - gatt_db_pass_handler and
 *           - appl_pass_connect
 *         [NOTE]: If link does not have the needed secruity for the service,
 *         transfer will not be initiated.
 *      f. This sample application, once configured, send measurements every
 *         5s. This interval can be altered using the define
 *         APPL_VALUE_CHANGE_INTERVAL. In absence of a real sensor to provide
 *         alert status and ringer settings readings, appl_update_alert_status
 *         and appl_update_ringer_setting is used to simulate value change of
 *         alert status and ringer setting characteristic respectively.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef PASS

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

#define APPL_AUTO_SEND_NOTIFICATION   0
#define APPL_MENU_SEND_NOTIFICATION   1

#define APPL_ALERT_STATUS_INTERVAL    3
#define APPL_RINGER_SETTING_INTERVAL  3

static BT_timer_handle alert_status_timer_handle;
static BT_timer_handle ringer_setting_timer_handle;

static ATT_ATTR_HANDLE appl_alert_status_hndl;
static ATT_ATTR_HANDLE appl_ringer_settings_hndl;
static ATT_ATTR_HANDLE appl_ringer_control_point_hndl;
static UCHAR           alert_status_enabled;
static UCHAR           ringer_setting_enabled;

/**
 *  This configuration flag controls whether the notifications
 *  are sent automatically or from the Menu
 */
static UCHAR           appl_notification_state;

static GATT_DB_HANDLE  appl_alert_status_db_hndl =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PHN_ALRT_STATUS_INST,
    GATT_CHAR_ALRT_STATUS_INST
};

static GATT_DB_HANDLE  appl_ringer_settings_db_hndl =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PHN_ALRT_STATUS_INST,
    GATT_CHAR_RING_SET_INST
};

static GATT_DB_HANDLE  appl_ringer_cntrl_point_db_hndl =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PHN_ALRT_STATUS_INST,
    GATT_CHAR_RING_CTRL_PNT_INST
};


#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
const char menu[] = " \r\n\
========= PASS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
 \r\n\
    2. Register Peer/Slave Device Address \r\n\
    3. Initiate Disconnection \r\n\
\r\n\
    4. Change Application Notification State \r\n\
\r\n\
    5. Set Alert Status \r\n\
    6. Set Ringer Settings \r\n\
\r\n\
    7. Send Alert Status Notification \r\n\
    8. Send Ringer Settings Notification \r\n\
\r\n\
    Your Option ?\r\n";
#else /* BT_SW_TXP */
const char pass_menu[] = " \r\n\
========= PASS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
\r\n\
    2. Initiate Disconnection \r\n\
\r\n\
    3. Change Application Notification State \r\n\
\r\n\
    4. Set Alert Status \r\n\
    5. Set Ringer Settings \r\n\
\r\n\
    6. Send Alert Status Notification \r\n\
    7. Send Ringer Settings Notification \r\n\
\r\n\
    Your Option ?\r\n";

#endif /* BT_SW_TXP */
const char alrt_status_menu[] = " \r\n\
 0 - No Alerts \r\n\
 1 - Set Ringer State not active \r\n\
 2 - Set Ringer State active \r\n\
 3 - Set Vibrate State not active \r\n\
 4 - Set Vibrate State active \r\n\
 5 - Set Display Alert Status not active \r\n\
 6 - Set Display Alert Status active \r\n";
#endif /* APPL_MENU_OPS */
/* --------------------------------------------- Functions */

void appl_pass_init(void)
{
    alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;

    alert_status_enabled = BT_FALSE;
    ringer_setting_enabled = BT_FALSE;

    appl_notification_state = APPL_AUTO_SEND_NOTIFICATION;

    appl_pass_server_reinitialize ();
}

void appl_pass_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    UINT16            cli_cnfg;
    cli_cnfg = 0;


    appl_alert_status_db_hndl.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_alert_status_db_hndl,
        &appl_alert_status_hndl
    );

    BT_gatt_db_get_char_cli_cnfg
    (
        &appl_alert_status_db_hndl,
        &value
    );

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));

    if (cli_cnfg)
    {
        appl_start_trasnfer(appl_alert_status_db_hndl,cli_cnfg);
    }

    /* Check if Ringer Setting is configured by the client */
    appl_ringer_settings_db_hndl.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_ringer_settings_db_hndl,
        &appl_ringer_settings_hndl
    );

    BT_gatt_db_get_char_cli_cnfg
    (
        &appl_ringer_settings_db_hndl,
        &value
    );

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_start_trasnfer(appl_ringer_settings_db_hndl,cli_cnfg);
    }

    /* Check if Ringer Control Point is configured by the client */
    appl_ringer_cntrl_point_db_hndl.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    BT_gatt_db_get_char_val_hndl
    (
        &appl_ringer_cntrl_point_db_hndl,
        &appl_ringer_control_point_hndl
    );
}


void appl_start_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    API_RESULT   retval;
    APPL_HANDLE    appl_handle;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_alert_status_db_hndl,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_alert_status_db_hndl,&ekey_size);

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

    if ((GATT_CHAR_ALRT_STATUS_INST == handle.char_id) ||
        (GATT_CHAR_RING_SET_INST == handle.char_id))
    {
        appl_get_handle_from_device_handle (handle.device_id, &appl_handle);

        if (GATT_CLI_CNFG_NOTIFICATION == config)
        {
            if (GATT_CHAR_ALRT_STATUS_INST == handle.char_id)
            {
                /* Start timer to simulate Status Change */
                if (BT_TIMER_HANDLE_INIT_VAL != alert_status_timer_handle)
                {
                    BT_stop_timer (alert_status_timer_handle);
                    APPL_TRC (
                    "[APPL]: Stopped Timer %p\n",alert_status_timer_handle);

                    alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                }

                alert_status_enabled = BT_TRUE;

                BT_start_timer
                (
                    &alert_status_timer_handle,
                    APPL_ALERT_STATUS_INTERVAL,
                    appl_alert_status_timer_expiry_handler,
                    &appl_handle,
                    sizeof (APPL_HANDLE)
                );
                APPL_TRC (
                "[APPL]: Started Timer %p\n", alert_status_timer_handle);
            }
            else if (GATT_CHAR_RING_SET_INST == handle.char_id)
            {
                /* Start Transfer Timer to Send measurement every time interval */
                if (BT_TIMER_HANDLE_INIT_VAL != ringer_setting_timer_handle)
                {
                    BT_stop_timer (ringer_setting_timer_handle);
                    ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                }

                ringer_setting_enabled = BT_TRUE;

                BT_start_timer
                (
                    &ringer_setting_timer_handle,
                    APPL_RINGER_SETTING_INTERVAL,
                    appl_ringer_setting_timer_expiry_handler,
                    &appl_handle,
                    sizeof (appl_handle)
                );
                APPL_TRC (
                "[APPL]: Started Timer %p\n", ringer_setting_timer_handle);
            }
        }
        else if (GATT_CLI_CNFG_DEFAULT == config)
        {
            if (GATT_CHAR_ALRT_STATUS_INST == handle.char_id)
            {
                /* Stop tranfer simulate Status Change */
                if (BT_TIMER_HANDLE_INIT_VAL != alert_status_timer_handle)
                {
                    BT_stop_timer (alert_status_timer_handle);
                    APPL_TRC (
                    "[APPL]: Stopped Timer %p\n", alert_status_timer_handle);
                    alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                }

                alert_status_enabled = BT_FALSE;
            }
            else if (GATT_CHAR_RING_SET_INST == handle.char_id)
            {
                /* Stop tranfer simulate Status Change */
                if (BT_TIMER_HANDLE_INIT_VAL != ringer_setting_timer_handle)
                {
                    BT_stop_timer (ringer_setting_timer_handle);
                    APPL_TRC (
                    "[APPL]: Stopped Timer %p\n", ringer_setting_timer_handle);
                    ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                }

                ringer_setting_enabled = BT_FALSE;
            }
        }
    }
}

void appl_send_phone_alert_notifications (APPL_HANDLE  * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 alert_status_value;
    UCHAR                 ringer_settings_value;

    if (APPL_AUTO_SEND_NOTIFICATION == appl_notification_state)
    {
        hndl_val_param.value.len = 1;

        /** Send Notifications for Alert Status Change */
        if (BT_FALSE != alert_status_enabled)
        {
            hndl_val_param.handle =  appl_alert_status_hndl;
            hndl_val_param.value.val = &alert_status_value;

            retval = BT_gatt_db_get_char_val
                     (
                         &appl_alert_status_db_hndl,
                         &hndl_val_param.value
                     );

            appl_update_alert_status (hndl_val_param.value.val);

            retval = BT_gatt_db_set_char_val
                     (
                         &appl_alert_status_db_hndl,
                         &hndl_val_param.value
                     );

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
        }
        /** Send Notifications for Ringer Settings Change */
        if (BT_FALSE != ringer_setting_enabled)
        {
            hndl_val_param.handle =  appl_ringer_settings_hndl;
            hndl_val_param.value.val = &ringer_settings_value;

            retval = BT_gatt_db_get_char_val
                     (
                         &appl_ringer_settings_db_hndl,
                         &hndl_val_param.value
                     );

            appl_update_ringer_setting (hndl_val_param.value.val);

            retval = BT_gatt_db_set_char_val
                     (
                         &appl_ringer_settings_db_hndl,
                         &hndl_val_param.value
                     );

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(*handle),
                         &hndl_val_param
                      );
        }
    }
}

void appl_pass_read_request_handler
     (
        GATT_DB_HANDLE    * handle
     )
{
    ATT_VALUE                att_value;
    API_RESULT               retval;
    APPL_HANDLE              appl_handle;
    UCHAR                    value;

    retval = API_SUCCESS;
    att_value.len = 1;
    att_value.val = &value;

    if (GATT_CHAR_ALRT_STATUS_INST == handle->char_id)
    {
        retval = BT_gatt_db_get_char_val
                 (
                     &appl_alert_status_db_hndl,
                     &att_value
                 );
    }
    else if (GATT_CHAR_RING_SET_INST == handle->char_id)
    {
        retval = BT_gatt_db_get_char_val
                 (
                     &appl_ringer_settings_db_hndl,
                     &att_value
                 );
    }

    if (API_SUCCESS == retval)
    {
        /* Read Request */
        appl_get_handle_from_device_handle (handle->device_id,&appl_handle);
        retval = BT_att_read_rsp
                 (
                      &APPL_GET_ATT_INSTANCE(appl_handle),
                      &att_value
                 );

        APPL_TRC (
        "[APPL]:Read Response sent with result 0x%04X\n",retval);
    }
}

void appl_pass_ringer_control_point_write_handler
     (
        GATT_DB_HANDLE    * handle,
        ATT_VALUE         * value
     )
{
    APPL_HANDLE  appl_handle;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 alert_status_value;
    UCHAR                 ringer_settings_value;

    hndl_val_param.value.len = 1;
    appl_get_handle_from_device_handle (handle->device_id,&appl_handle);

    switch (*(value->val))
    {
        case APPL_PASS_RINGER_CNTRL_PNT_SILENT_MODE:
            APPL_TRC ("\n Set Silent Mode \n Ringer Silent\n");
            /** Send Notifications for Ringer Settings Change */
            if (BT_FALSE != ringer_setting_enabled)
            {
                hndl_val_param.handle =  appl_ringer_settings_hndl;
                hndl_val_param.value.val = &ringer_settings_value;

                (*hndl_val_param.value.val) = RINGER_SETTING_RINGER_SILENT;

                retval = BT_gatt_db_set_char_val
                         (
                             &appl_ringer_settings_db_hndl,
                             &hndl_val_param.value
                         );

                retval = BT_att_send_hndl_val_ntf
                         (
                             &APPL_GET_ATT_INSTANCE(appl_handle),
                             &hndl_val_param
                          );
            }
            break;
        case APPL_PASS_RINGER_CNTRL_PNT_MUTE_ONCE:
            APPL_TRC ("\n Mute Once \n");
            /** Send Notifications for Alert Status Change */
            if (BT_FALSE != alert_status_enabled)
            {
                hndl_val_param.handle =  appl_alert_status_hndl;
                hndl_val_param.value.val = &alert_status_value;

                retval = BT_gatt_db_get_char_val
                         (
                             &appl_alert_status_db_hndl,
                             &hndl_val_param.value
                         );

                appl_clear_bit_value(alert_status_value,ALRT_STATUS_RINGER_STATE_BIT);

                retval = BT_gatt_db_set_char_val
                         (
                             &appl_alert_status_db_hndl,
                             &hndl_val_param.value
                         );

                retval = BT_att_send_hndl_val_ntf
                         (
                             &APPL_GET_ATT_INSTANCE(appl_handle),
                             &hndl_val_param
                          );
            }
            break;
        case APPL_PASS_RINGER_CNTRL_PNT_CANCEL_SILENT_MODE:
            APPL_TRC ("\n Cancel Silent Mode \n Ringer Normal\n");
            /** Send Notifications for Ringer Settings Change */
            if (BT_FALSE != ringer_setting_enabled)
            {
                hndl_val_param.handle =  appl_ringer_settings_hndl;
                hndl_val_param.value.val = &ringer_settings_value;

                (*hndl_val_param.value.val) = RINGER_SETTING_RINGER_NORMAL;

                retval = BT_gatt_db_set_char_val
                         (
                             &appl_ringer_settings_db_hndl,
                             &hndl_val_param.value
                         );

                retval = BT_att_send_hndl_val_ntf
                         (
                             &APPL_GET_ATT_INSTANCE(appl_handle),
                             &hndl_val_param
                          );
            }
            break;
        default:
            APPL_TRC ("\n Unknown Ringer Control Command \n");
            if (BT_TIMER_HANDLE_INIT_VAL != ringer_setting_timer_handle)
            {
                BT_stop_timer (ringer_setting_timer_handle);
                APPL_TRC (
                "[APPL]: Stopping Timer %p\n", ringer_setting_timer_handle);
                ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
            if (BT_TIMER_HANDLE_INIT_VAL != alert_status_timer_handle)
            {
                BT_stop_timer (alert_status_timer_handle);
                APPL_TRC (
                "[APPL]: Stopping Timer %p\n", alert_status_timer_handle);
                alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
            break;
    }
}

void appl_alert_status_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;


    fsm_param.handle = *((APPL_HANDLE *)data);

    /* Restart timer to simulate Temperature measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != alert_status_timer_handle)
    {
        BT_stop_timer (alert_status_timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer %p\n", alert_status_timer_handle);
        alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &alert_status_timer_handle,
        APPL_ALERT_STATUS_INTERVAL,
        appl_alert_status_timer_expiry_handler,
        &fsm_param.handle,
        sizeof(fsm_param.handle)
    );

    APPL_TRC (
    "[APPL]: Started Timer %p\n", alert_status_timer_handle);

    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
         APPL_FSM_ID,
         ev_appl_measurement_ind,
         &fsm_param
    );
}

void appl_ringer_setting_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    fsm_param.handle = (*((APPL_HANDLE *)data));

    /* Restart timer to simulate Temperature measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != ringer_setting_timer_handle)
    {
        BT_stop_timer (ringer_setting_timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer %p\n", ringer_setting_timer_handle);
        ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    BT_start_timer
    (
        &ringer_setting_timer_handle,
        APPL_RINGER_SETTING_INTERVAL,
        appl_ringer_setting_timer_expiry_handler,
        &fsm_param.handle,
        sizeof(fsm_param.handle)
    );


    APPL_TRC (
    "[APPL]: Started Timer %p\n", ringer_setting_timer_handle);
    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
         APPL_FSM_ID,
         ev_appl_measurement_ind,
         &fsm_param
    );
}

void appl_pass_server_reinitialize (void)
{
    /* Stop Timer for Phone Alert Status Change */
    if (BT_TIMER_HANDLE_INIT_VAL != alert_status_timer_handle)
    {
        BT_stop_timer (alert_status_timer_handle);
        alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    /* Stop Timer for Phone Alert Status Change */
    if (BT_TIMER_HANDLE_INIT_VAL != ringer_setting_timer_handle)
    {
        BT_stop_timer (ringer_setting_timer_handle);
        ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    appl_notification_state = APPL_AUTO_SEND_NOTIFICATION;
}

void appl_update_alert_status (UCHAR * val)
{
    /** Toggle ALRT_STATUS_RINGER_STATE_BIT */
    if (appl_get_bit_value(*val,ALRT_STATUS_RINGER_STATE_BIT))
    {
        appl_clear_bit_value(*val,ALRT_STATUS_RINGER_STATE_BIT);
    }
    else
    {
        appl_set_bit_value(*val,ALRT_STATUS_RINGER_STATE_BIT);
    }

    /** Toggle ALRT_STATUS_VIBRATOR_STATE_BIT */
    if (appl_get_bit_value(*val,ALRT_STATUS_VIBRATOR_STATE_BIT))
    {
        appl_clear_bit_value(*val,ALRT_STATUS_VIBRATOR_STATE_BIT);
    }
    else
    {
        appl_set_bit_value(*val,ALRT_STATUS_VIBRATOR_STATE_BIT);
    }

    /** Toggle ALRT_STATUS_DISP_ALRT_STATE_BIT */
}

void appl_update_ringer_setting (UCHAR * val)
{
    if (RINGER_SETTING_RINGER_SILENT == (*val))
    {
        (*val) = RINGER_SETTING_RINGER_NORMAL;
    }
    else
    {
        (*val) = RINGER_SETTING_RINGER_SILENT;
    }
}

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
void console_app_read (UCHAR * choice, UCHAR * value, UINT16 length)
{
    static BT_DEVICE_ADDR peer_addr;
    UCHAR val;
    ATT_VALUE att_value;

    att_value.len = 1;
    att_value.val = &val;

    /* Check the choice */
    switch (*choice)
    {

        case 0:
            return;

        case 1:
            CONSOLE_OUT("%s", menu);
            break;

        case 2:
            CONSOLE_OUT("Enter Peer/Slave BD Address (0xB5B4B3B2B1B0): \n");
            break;

        case (0x80 | 2):
            /* If 6 octets received, it is BD Address. Store it and ask for type */
            if (6 == length)
            {
                /* Store BD Address */
                CONSOLE_OUT("BD Address - %02X %02X %02X %02X %02X %02X\n",
                value[0], value[1], value[2],
                value[3], value[4], value[5]);

                BT_COPY_BD_ADDR(peer_addr.addr,value);

                CONSOLE_OUT("\r\nEnter peer BD Address Type:\n");
            }
            else if (1 == length)
            {
                /* Store BD Address type */
                CONSOLE_OUT("BD Address type - %02X\n", value[0]);
                *choice = 1;
                BT_COPY_TYPE(peer_addr.type,value[0]);
                appl_store_peer_dev_addr(&peer_addr);
            }
            break;

        case 3:
            CONSOLE_OUT("Initiated Disconnection\n");
            /* Disconnect connected device */
            *choice = 1;
            appl_disconnect_device();
            CONSOLE_OUT (menu);
            break;

        case 4:
            {
                /* Set Application Notification State */
                CONSOLE_OUT(" 1 - Auto Send Notification \n");
                CONSOLE_OUT(" 2 - Menu Based Notification \n");
            }
            break;

        case (0x80 | 4):
            if (1 == length)
            {
                *choice = 1;
                /* Handle the Sub Menu for Alert Status */
                switch (value[0])
                {
                    case 1:
                        {
                            appl_notification_state = APPL_AUTO_SEND_NOTIFICATION;
                        }
                        break;

                    case 2:
                        {
                            appl_notification_state = APPL_MENU_SEND_NOTIFICATION;
                        }
                        break;

                    default:
                        CONSOLE_OUT("\r\nInvalid Option\n");
                        *choice = 1;
                        break;
                }
            }
            CONSOLE_OUT (menu);
            break;

        case 5:
            /* Set Alert Status */
#if 0
            CONSOLE_OUT(" 0 - No Alerts \n");
            CONSOLE_OUT(" 1 - Set Ringer State not active \n");
            CONSOLE_OUT(" 2 - Set Ringer State active \n");
            CONSOLE_OUT(" 3 - Set Vibrate State not active \n");
            CONSOLE_OUT(" 4 - Set Vibrate State active \n");
            CONSOLE_OUT(" 5 - Set Display Alert Status not active \n");
            CONSOLE_OUT(" 6 - Set Display Alert Status active \n");
#else /* 0 */
            CONSOLE_OUT(alrt_status_menu);
#endif /* 0 */

            break;

        case (0x80 | 5):
            if (1 == length)
            {
                *choice = 1;
                /* Handle the Sub Menu for Alert Status */
                switch (value[0])
                {
                    case 0:
                         {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_clear_bit_value(*(att_value.val),ALRT_STATUS_RINGER_STATE_BIT);
                           appl_clear_bit_value(*(att_value.val),ALRT_STATUS_VIBRATOR_STATE_BIT);
                           appl_clear_bit_value(*(att_value.val),ALRT_STATUS_DISP_ALRT_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    case 1:
                        {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_clear_bit_value(*(att_value.val),ALRT_STATUS_RINGER_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    case 2:
                        {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_set_bit_value(*(att_value.val),ALRT_STATUS_RINGER_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    case 3:
                        {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_clear_bit_value(*(att_value.val),ALRT_STATUS_VIBRATOR_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    case 4:
                        {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_set_bit_value(*(att_value.val),ALRT_STATUS_VIBRATOR_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    case 5:
                        {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_clear_bit_value(*(att_value.val),ALRT_STATUS_DISP_ALRT_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    case 6:
                        {
                           BT_gatt_db_get_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );

                           appl_set_bit_value(*(att_value.val),ALRT_STATUS_DISP_ALRT_STATE_BIT);

                           BT_gatt_db_set_char_val
                           (
                               &appl_alert_status_db_hndl,
                               &att_value
                           );
                        }
                        break;
                    default:
                        CONSOLE_OUT("\r\nInvalid Option\n");
                        *choice = 1;
                        break;
                }
            }
            CONSOLE_OUT (menu);
            break;

        case 6:
            /* Set Ringer Settings */
            CONSOLE_OUT(" 1 - Set Ringer Silent \n");
            CONSOLE_OUT(" 2 - Set Ringer Normal \n");
            break;

        case (0x80 | 6):
            if (1 == length)
            {
                *choice = 1;
                /* Handle the Sub Menu for Ringer Settings */
                switch (value[0])
                {
                    case 1:
                        {
                           *(att_value.val) = RINGER_SETTING_RINGER_SILENT;

                           BT_gatt_db_set_char_val
                           (
                               &appl_ringer_settings_db_hndl,
                               &att_value
                           );
                        }
                        break;

                    case 2:
                        {
                           *(att_value.val) = RINGER_SETTING_RINGER_NORMAL;

                           BT_gatt_db_set_char_val
                           (
                               &appl_ringer_settings_db_hndl,
                               &att_value
                           );
                        }
                        break;

                    default:
                       CONSOLE_OUT("\r\nInvalid Option\n");
                       *choice = 1;
                        break;
                }
            }
            CONSOLE_OUT (menu);
            break;

        case 7:
            /* Send Alert Status notifications */
            appl_send_alert_status_ntf();
            CONSOLE_OUT (menu);
            break;

        case 8:
            /* Send Ringer Settings notifications */
            appl_send_ringer_settings_ntf();
            CONSOLE_OUT (menu);
            break;

        default:
            CONSOLE_OUT("\r\nInvalid Option\n");
            *choice = 1;
            break;
    }
}

const char * appl_get_profile_menu (void)
{
    return menu;
}
#else /* BT_SW_TXP */
void appl_profile_menu_handler (void)
{
    UCHAR val;
    ATT_VALUE att_value;
    int choice;
    int temp;

    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_device_init_req,
        NULL
    );

    att_value.len = 1;
    att_value.val = &val;
    while (1)
    {
        printf("\n");
        printf("%s", pass_menu);
        scanf("%d", &choice);

        switch(choice)
        {
        case 0: /* exit */
            return;

        case 1: /* Refresh */
            break;

        case 2:
            appl_disconnect_device();
            break;

        case 3:
            /* Set Application Notification State */
            CONSOLE_OUT (" 1 - Auto Send Notification \n");
            CONSOLE_OUT (" 2 - Menu Based Notification \n");
            CONSOLE_IN ("%d", &temp);
            /* Handle the Sub Menu for Alert Status */
            switch (temp)
            {
                case 1:
                    appl_notification_state = APPL_AUTO_SEND_NOTIFICATION;
                    break;

                case 2:
                    appl_notification_state = APPL_MENU_SEND_NOTIFICATION;
                    if (BT_TIMER_HANDLE_INIT_VAL != ringer_setting_timer_handle)
                    {
                        BT_stop_timer (ringer_setting_timer_handle);
                        APPL_TRC (
                        "[APPL]: Stopping Timer %p\n", ringer_setting_timer_handle);
                        ringer_setting_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                    }
                    if (BT_TIMER_HANDLE_INIT_VAL != alert_status_timer_handle)
                    {
                        BT_stop_timer (alert_status_timer_handle);
                        APPL_TRC (
                        "[APPL]: Stopping Timer %p\n", alert_status_timer_handle);
                        alert_status_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                    }

                    break;

                default:
                    CONSOLE_OUT("\r\nInvalid Option\n");
                    break;
            }
            break;

        case 4:
            /* Set Alert Status */
            CONSOLE_OUT(alrt_status_menu);
            CONSOLE_IN ("%d", &temp);
            /* Handle the Sub Menu for Alert Status */
            switch (temp)
            {
                case 0:
                     {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_clear_bit_value(*(att_value.val),ALRT_STATUS_RINGER_STATE_BIT);
                       appl_clear_bit_value(*(att_value.val),ALRT_STATUS_VIBRATOR_STATE_BIT);
                       appl_clear_bit_value(*(att_value.val),ALRT_STATUS_DISP_ALRT_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                case 1:
                    {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_clear_bit_value(*(att_value.val),ALRT_STATUS_RINGER_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                case 2:
                    {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_set_bit_value(*(att_value.val),ALRT_STATUS_RINGER_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                case 3:
                    {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_clear_bit_value(*(att_value.val),ALRT_STATUS_VIBRATOR_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                case 4:
                    {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_set_bit_value(*(att_value.val),ALRT_STATUS_VIBRATOR_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                case 5:
                    {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_clear_bit_value(*(att_value.val),ALRT_STATUS_DISP_ALRT_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                case 6:
                    {
                       BT_gatt_db_get_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );

                       appl_set_bit_value(*(att_value.val),ALRT_STATUS_DISP_ALRT_STATE_BIT);

                       BT_gatt_db_set_char_val
                       (
                           &appl_alert_status_db_hndl,
                           &att_value
                       );
                    }
                    break;
                default:
                    CONSOLE_OUT("\r\nInvalid Option\n");
                    break;
            }
            break;

        case 5:
            /* Set Ringer Settings */
            CONSOLE_OUT(" 1 - Set Ringer Silent \n");
            CONSOLE_OUT(" 2 - Set Ringer Normal \n");
            CONSOLE_IN ("%d", &temp);

            /* Handle the Sub Menu for Ringer Settings */
            switch (temp)
            {
                case 1:
                    {
                       *(att_value.val) = RINGER_SETTING_RINGER_SILENT;

                       BT_gatt_db_set_char_val
                       (
                           &appl_ringer_settings_db_hndl,
                           &att_value
                       );
                    }
                    break;

                case 2:
                    {
                       *(att_value.val) = RINGER_SETTING_RINGER_NORMAL;

                       BT_gatt_db_set_char_val
                       (
                           &appl_ringer_settings_db_hndl,
                           &att_value
                       );
                    }
                    break;

                default:
                   CONSOLE_OUT("\r\nInvalid Option\n");
                    break;
            }
            break;

        case 6:
            /* Send Alert Status notifications */
            appl_send_alert_status_ntf();
            break;

        case 7:
            /* Send Ringer Settings notifications */
            appl_send_ringer_settings_ntf();
            break;

        default:
            CONSOLE_OUT("\r\nInvalid Option\n");
            break;
        }
    }
}
#endif /* BT_SW_TXP */

void appl_send_alert_status_ntf (void)
{
    APPL_HANDLE           appl_handle;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 alert_status_value;

    if (APPL_MENU_SEND_NOTIFICATION == appl_notification_state)
    {
        appl_get_handle_from_device_handle
        (appl_alert_status_db_hndl.device_id,&appl_handle);

        hndl_val_param.value.len = 1;

        /** Send Notifications for Alert Status Change */
        if (BT_FALSE != alert_status_enabled)
        {
            hndl_val_param.handle =  appl_alert_status_hndl;
            hndl_val_param.value.val = &alert_status_value;

            retval = BT_gatt_db_get_char_val
                     (
                         &appl_alert_status_db_hndl,
                         &hndl_val_param.value
                     );

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(appl_handle),
                         &hndl_val_param
                      );
        }
    }
}

void appl_send_ringer_settings_ntf (void)
{
    APPL_HANDLE           appl_handle;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UCHAR                 ringer_settings_value;

    if (APPL_MENU_SEND_NOTIFICATION == appl_notification_state)
    {
        appl_get_handle_from_device_handle
        (appl_ringer_settings_db_hndl.device_id,&appl_handle);

        hndl_val_param.value.len = 1;

        /** Send Notifications for Ringer Settings Change */
        if (BT_FALSE != ringer_setting_enabled)
        {
            hndl_val_param.handle =  appl_ringer_settings_hndl;
            hndl_val_param.value.val = &ringer_settings_value;

            retval = BT_gatt_db_get_char_val
                     (
                         &appl_ringer_settings_db_hndl,
                         &hndl_val_param.value
                     );

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(appl_handle),
                         &hndl_val_param
                      );
        }
    }
}

#endif /* APPL_MENU_OPS */

#endif /* PASS */

