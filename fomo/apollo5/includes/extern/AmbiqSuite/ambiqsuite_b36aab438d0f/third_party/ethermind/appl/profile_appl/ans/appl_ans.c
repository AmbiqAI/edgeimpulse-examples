
/**
 *  \file appl_ans.c
 *
 *  This file contains the Alert Notification application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef ANS

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */


#define APPL_NEW_ALRT_LENGTH                  2
#define APPL_NEW_ALRT_INTERVAL                10

#define APPL_UNREAD_ALRT_LENGTH               2
#define APPL_UNREAD_ALRT_INTERVAL             10

/**
 *  New Alert Supported Category
 *
 *  Category ID Bit Mask 0
 *
 *     Bit   -    Name
 *    Field
 *
 *      0    -   Simple Alert
 *      1    -   Email
 *      2    -   News
 *      3    -   Call
 *      4    -   Missed Call
 *      5    -   SMS/MMS
 *      6    -   Voice Mail
 *      7    -   Schedule
 *
 *
 *  Category ID Bit Mask 1
 *
 *     Bit   -    Name
 *    Field
 *
 *      0    -   High Prioritized Alert
 *      1    -   Instant Message
 *      2-6  -   Reserved
 */

DECL_CONST DECL_STATIC UCHAR new_alrt_supp_category[2] =
                             {
                                 ANS_SUPPORTED_NEW_ALRT_CTGRY,
                                 ANS_SUPPORTED_NEW_ALRT_CTGRY_ID
                             };

/**
 *  Unread Alert Supported Category
 *
 *  Category ID Bit Mask 0
 *
 *     Bit   -    Name
 *    Field
 *
 *      0    -   Simple Alert
 *      1    -   Email
 *      2    -   News
 *      3    -   Call
 *      4    -   Missed Call
 *      5    -   SMS/MMS
 *      6    -   Voice Mail
 *      7    -   Schedule
 *
 *
 *  Category ID Bit Mask 1
 *
 *     Bit   -    Name
 *    Field
 *
 *      0    -   High Prioritized Alert
 *      1    -   Instant Message
 *      2-6  -   Reserved
 */
const static UCHAR unread_alrt_supp_category[2] =
             {
                 ANS_SUPPORTED_UNREAD_ALRT_CTGRY,
                 ANS_SUPPORTED_UNREAD_ALRT_CTGRY_ID
             };

static UCHAR new_alert[APPL_NEW_ALRT_LENGTH] = {0x00, 0xFF};
static UCHAR new_alert_count;

static UCHAR unread_alert[APPL_UNREAD_ALRT_LENGTH] = {0x00, 0xFF};
static UCHAR unread_alert_count;

/** Flag to highlight which of the supported categories is Enabled */
static UCHAR peer_enabled_new_alrts;
static UCHAR peer_enabled_unread_alrts;

static BT_timer_handle new_alert_timer_handle;
static BT_timer_handle unread_alert_timer_handle;
static ATT_ATTR_HANDLE appl_new_alert_hndl;
static ATT_ATTR_HANDLE appl_unread_alert_hndl;
static GATT_DB_HANDLE appl_ans_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_ALRT_NTF_INST,
    GATT_CHAR_NEW_ALRT_INST
};
static UCHAR new_alert_enabled, unread_alert_enabled;

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
const char main_options[] = " \r\n\
========= ANS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
 \r\n\
    2. Initiate Disconnection \r\n\
\r\n\
    Your Option ?\r\n";
#else /* BT_SW_TXP */
const char menu[] = " \r\n\
========= ANS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
 \r\n\
    2. Register Peer/Slave Device Address \r\n\
    3. Initiate Disconnection \r\n\
\r\n\
    Your Option ?\r\n";
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

/* --------------------------------------------- Functions */
UCHAR get_category_id (UCHAR category_id_bit_mask);

void appl_ans_init(void)
{
    ATT_VALUE         value;


    value.len = 2;

    new_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    unread_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;

    new_alert_enabled = BT_FALSE;
    unread_alert_enabled = BT_FALSE;

    peer_enabled_new_alrts = 0x00;
    peer_enabled_unread_alrts = 0x00;

    /* Device Id is set to 0x00 */
    appl_ans_db_handle.device_id = 0x00;

    value.val = (UCHAR *) &new_alrt_supp_category[0];

    /* Set the Supported New Alert Category */
    appl_ans_db_handle.char_id = GATT_CHAR_SUP_NEW_ALRT_CAT_INST;

    BT_gatt_db_set_char_val
    (
        &appl_ans_db_handle,
        &value
    );

    value.val = (UCHAR *) &unread_alrt_supp_category[0];

    /* Set the Supported Unread Alert Category */
    appl_ans_db_handle.char_id = GATT_CHAR_SUP_UNREAD_ALRT_CAT_INST;

    BT_gatt_db_set_char_val
    (
        &appl_ans_db_handle,
        &value
    );

    appl_ans_server_reinitialize();
}

void appl_ans_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;

    UINT16            cli_cnfg;

    cli_cnfg = 0;


    appl_ans_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    /* If New Alert is configured, initiate transfer */
    appl_ans_db_handle.char_id = GATT_CHAR_NEW_ALRT_INST;

    BT_gatt_db_get_char_val_hndl
    (
         &appl_ans_db_handle,
         &appl_new_alert_hndl
    );
    BT_gatt_db_get_char_cli_cnfg(&appl_ans_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));

    appl_manage_trasnfer(appl_ans_db_handle,cli_cnfg);

    /* If Unread Alert Status is configured, initiate transfer */
    appl_ans_db_handle.char_id = GATT_UNREAD_ALRT_STATUS_INST;

    /* service_id is already configured above. Not setting again */
    BT_gatt_db_get_char_val_hndl
    (
         &appl_ans_db_handle,
         &appl_unread_alert_hndl
    );
    BT_gatt_db_get_char_cli_cnfg (&appl_ans_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));

    appl_manage_trasnfer(appl_ans_db_handle,cli_cnfg);
}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    APPL_HANDLE    appl_handle;
    API_RESULT     retval;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_ans_db_handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_ans_db_handle,&ekey_size);

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

    if (GATT_CLI_CNFG_NOTIFICATION == config)
    {
        appl_get_handle_from_device_handle (handle.device_id,&appl_handle);

        if (GATT_CHAR_NEW_ALRT_INST == handle.char_id)
        {
            /* Start Transfer Timer to Send measurement every time interval */
            if (BT_TIMER_HANDLE_INIT_VAL != new_alert_timer_handle)
            {
                BT_stop_timer (new_alert_timer_handle);
                new_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            new_alert_enabled = BT_TRUE;

            BT_start_timer
            (
                &new_alert_timer_handle,
                APPL_NEW_ALRT_INTERVAL,
                appl_new_alert_timer_expiry_handler,
                &appl_handle,
                sizeof (appl_handle)
            );
            APPL_TRC (
            "[APPL]: Started Timer with result %p\n",
            appl_new_alert_timer_expiry_handler);
        }
        else if (GATT_UNREAD_ALRT_STATUS_INST == handle.char_id)
        {
            /* Start Transfer Timer to Send measurement every time interval */
            if (BT_TIMER_HANDLE_INIT_VAL != unread_alert_timer_handle)
            {
                BT_stop_timer (unread_alert_timer_handle);
                unread_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            unread_alert_enabled = BT_TRUE;

            BT_start_timer
            (
                &unread_alert_timer_handle,
                APPL_UNREAD_ALRT_INTERVAL,
                appl_unread_alert_timer_expiry_handler,
                &appl_handle,
                sizeof (appl_handle)
            );
            APPL_TRC (
            "[APPL]: Started Timer %p\n", unread_alert_timer_handle);
        }
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        if (GATT_CHAR_NEW_ALRT_INST == handle.char_id)
        {
            /* Turn Off Transfer Timer if On */
            if (BT_TIMER_HANDLE_INIT_VAL != new_alert_timer_handle)
            {
                BT_stop_timer (new_alert_timer_handle);
                new_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            new_alert_enabled = BT_FALSE;
        }
        else if (GATT_UNREAD_ALRT_STATUS_INST == handle.char_id)
        {
            /* Turn Off Transfer Timer if On */
            if (BT_TIMER_HANDLE_INIT_VAL != unread_alert_timer_handle)
            {
                BT_stop_timer (unread_alert_timer_handle);
                unread_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            unread_alert_enabled = BT_FALSE;
        }
    }
    else
    {
        /* Incorrect Configuration */
    }
}

void appl_send_notification (APPL_HANDLE * handle)
{
    APPL_TRC(
    "Send NTF: New Altert %d, Unread Alert %d\n", new_alert_enabled, unread_alert_enabled);

    if (BT_FALSE != new_alert_enabled)
    {
        APPL_TRC(
        "New Alert Mask 0x%02X\n", peer_enabled_new_alrts);

        if (peer_enabled_new_alrts)
        {
            new_alert[0] = get_category_id(peer_enabled_new_alrts);
            new_alert[1] = new_alert_count;
            appl_send_alert_ntf
            (
                 handle,
                 &appl_new_alert_hndl,
                 new_alert,
                 APPL_NEW_ALRT_LENGTH
             );
            new_alert_count++;
        }
    }
    if (BT_FALSE != unread_alert_enabled)
    {
        APPL_TRC(
        "Unread Alert Mask 0x%02X\n", peer_enabled_unread_alrts);

        if (peer_enabled_unread_alrts)
        {
            unread_alert[0] = get_category_id(peer_enabled_unread_alrts);
            unread_alert[1] = unread_alert_count;
            appl_send_alert_ntf
            (
                handle,
                &appl_unread_alert_hndl,
                unread_alert,
                APPL_UNREAD_ALRT_LENGTH
            );
            unread_alert_count++;
        }
    }
}

void appl_send_alert_ntf (
                             APPL_HANDLE * handle,
                             ATT_ATTR_HANDLE * appl_alert_handle,
                             UCHAR * alert_value,
                             UINT16 len
                         )
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    APPL_TRC (
    "Sending notification On Handle 0x%04X\n", appl_new_alert_hndl);

    hndl_val_param.handle =  *appl_alert_handle;
    hndl_val_param.value.val = alert_value;
    hndl_val_param.value.len = len;

    appl_dump_bytes(alert_value,len);

    retval = BT_att_send_hndl_val_ntf
             (
                 &APPL_GET_ATT_INSTANCE(*handle),
                 &hndl_val_param
              );
    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "[** ERR **]: Failed to send notification, reason 0x%04X",retval);
    }
}


void appl_new_alert_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM    fsm_param;

    fsm_param.handle = (*((APPL_HANDLE *)data));

    /* Restart timer to simulate Temperature measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != new_alert_timer_handle)
    {
        BT_stop_timer (new_alert_timer_handle);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", new_alert_timer_handle);
        new_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    APPL_TRC (
    "[APPL]: Started Timer %p\n", new_alert_timer_handle);

    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
        APPL_FSM_ID, 
        ev_appl_measurement_ind,
        &fsm_param
    );


}

void appl_unread_alert_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM    fsm_param;

    fsm_param.handle = (*((APPL_HANDLE *)data));

    /* Restart timer to simulate Temperature measurements */
    if (BT_TIMER_HANDLE_INIT_VAL != unread_alert_timer_handle)
    {
        BT_stop_timer (unread_alert_timer_handle);
        APPL_TRC (
        "[APPL]: Stopped Timer %p\n", unread_alert_timer_handle);
        unread_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    APPL_TRC (
    "[APPL]: Started Timer %p\n",unread_alert_timer_handle);
    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
         APPL_FSM_ID,
         ev_appl_measurement_ind,
         &fsm_param
    );
}

API_RESULT appl_alrt_ntf_cntrl_point
    (GATT_DB_HANDLE handle, UCHAR command_id, UCHAR category_id)
{
    APPL_HANDLE  appl_handle;
    API_RESULT   retval;

    retval = API_SUCCESS;

    appl_get_handle_from_device_handle (handle.device_id,&appl_handle);

    APPL_TRC(
    "NTF Control Point. CMD ID: 0x%02X, CAT ID: 0x%02X\n",
    command_id, category_id);

    switch (command_id)
    {
        case ENABLE_NEW_INCOMING_ALRT_NTF:
            peer_enabled_new_alrts |= ((1 << category_id) & new_alrt_supp_category[0]);
            break;

        case ENABLE_UNREAD_CATEGORY_STATUS_NTF:
            peer_enabled_unread_alrts |= ((1 << category_id) & unread_alrt_supp_category[0]);
            break;

        case DISABLE_NEW_INCOMING_ALRT_NTF:
            peer_enabled_new_alrts &= ~(1 << category_id);
            break;

        case DISABLE_UNREAD_CATEGORY_STATUS_NTF:
            peer_enabled_unread_alrts &= ~(1 << category_id);
            break;

        case NOTIFY_NEW_INCOMING_ALRT_IMMEDIATELY:
            /* If New Alert category is Enabled and suppported */
            if (peer_enabled_new_alrts & (1 << category_id))
            {
                if (BT_FALSE != new_alert_enabled)
                {
                    if (BT_TIMER_HANDLE_INIT_VAL != new_alert_timer_handle)
                    {
                        BT_stop_timer (new_alert_timer_handle);
                        APPL_TRC (
                        "[APPL]: Stopped Timer %p\n", new_alert_timer_handle);
                        new_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                    }

                    BT_start_timer
                    (
                        &new_alert_timer_handle,
                        APPL_NEW_ALRT_INTERVAL,
                        appl_new_alert_timer_expiry_handler,
                        &appl_handle,
                        sizeof (appl_handle)
                    );

                    APPL_TRC (
                    "[APPL]: Started Timer with result %p\n",
                    appl_new_alert_timer_expiry_handler);
                }
            }
            break;

        case NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY:
            /* If Unread Alert category is Enabled and suppported */
            if (peer_enabled_unread_alrts & (1 << category_id))
            {
                if (BT_FALSE != unread_alert_enabled)
                {
                    if (BT_TIMER_HANDLE_INIT_VAL != unread_alert_timer_handle)
                    {
                        BT_stop_timer (unread_alert_timer_handle);
                        APPL_TRC (
                        "[APPL]: Stopped Timer %p\n", unread_alert_timer_handle);
                        unread_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                    }

                    BT_start_timer
                    (
                        &unread_alert_timer_handle,
                        APPL_UNREAD_ALRT_INTERVAL,
                        appl_unread_alert_timer_expiry_handler,
                        &appl_handle,
                        sizeof (appl_handle)
                    );
                    APPL_TRC (
                    "[APPL]: Started Timer %p\n", unread_alert_timer_handle);
                }
            }
            break;

        default:
            retval = ALRT_NTF_CNTRL_PNT_CMD_NOT_SUPPORTED;
            break;
    }

    return retval;
}

void appl_ans_server_reinitialize (void)
{
    /* Turn Off Transfer Timer if On.  */
    if (BT_TIMER_HANDLE_INIT_VAL != new_alert_timer_handle)
    {
        BT_stop_timer (new_alert_timer_handle);
        new_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    /* Turn Off Transfer Timer if On.  */
    if (BT_TIMER_HANDLE_INIT_VAL != unread_alert_timer_handle)
    {
        BT_stop_timer (unread_alert_timer_handle);
        unread_alert_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    new_alert_count = 0;
    unread_alert_count = 0;

    peer_enabled_new_alrts = 0x00;
    peer_enabled_unread_alrts = 0x00;
}

UCHAR get_category_id (UCHAR category_id_bit_mask)
{
    UCHAR val;
    UCHAR count;

    if (1 == category_id_bit_mask)
    {
        return 0;
    }
    else
    {
        count = 1;
        do
        {
            val = 2 * count;
            if (val == category_id_bit_mask)
            {
                return count;
            }
            else
            {
                count++;
            }
        }while (8 != count);
    }
    return 0;
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
        CONSOLE_OUT ("\nExiting Menu Based Application ... \n\n");
        exit(0);

    case 1:
        CONSOLE_OUT ("\nRefreshing ...\r\n\n");
        break;

    case 2:
        appl_disconnect_device();
        break;
    }
}
#else /* BT_SW_TXP */
void console_app_read (UCHAR * choice, UCHAR * value, UINT16 length)
{
    static BT_DEVICE_ADDR peer_addr;

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

            case 3:
            CONSOLE_OUT("Initiated Disconnection\n");
            /* Disconnect connected device */
            *choice = 1;
            appl_disconnect_device();
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
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

#endif /* ANS */

