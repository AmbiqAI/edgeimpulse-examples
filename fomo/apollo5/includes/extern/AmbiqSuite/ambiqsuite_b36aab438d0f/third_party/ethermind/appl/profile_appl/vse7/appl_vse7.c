
/**
 *  \file appl_vse7.c
 *
 */

/*
 *  Copyright (C) 2017. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#ifdef VSE7
#include "appl.h"
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_att_api.h"
#include "BT_smp_api.h"
#include "smp_pl.h"
#include "l2cap.h"
#include "gatt_defines.h"

/* ----------------------------------------- Configuration Defines */
/* Advertising Data Length */
#define APPL_ANP_GAP_ACCESS_ADV_DATA_LEN()                       9

/* Advertising Data */
#define APPL_ANP_GAP_ACCESS_ADV_DATA()                           "EtherMind"

/* Advertising Min and Max Intervals */
#define APPL_ANP_GAP_GET_ADV_MIN_INTERVAL()                      32
#define APPL_ANP_GAP_GET_ADV_MAX_INTERVAL()                      32

/* Advertising type */
#define APPL_ANP_GAP_GET_ADV_TYPE()                              0

/* Advertising Address types */
#define APPL_CONFIG_PERIPHERAL_UNBONDED_OWN_ADDR_TYPE()          0
#define APPL_CONFIG_PERIPHERAL_UNBONDED_DIRECT_ADDR_TYPE()       0

/* Peer Address for Directed Adv */
#define APPL_ANP_CONFIG_PEER_DEVICE_ADDR()                       "000000"

/* Advertising channel map */
#define APPL_ANP_GAP_GET_ADV_CHANNEL_MAP()                       7

/* Advertising filter policy */
#define APPL_ANP_GAP_GET_ADV_FILTER_POLICY()                     0

/* ----------------------------------------- Macro Defines */

#define DEVICE_CONNECTED     0x00
#define APPL_SET_STATE(mask) 0x00

/* ----------------------------------------- External Global Variables */
const char * appl_get_profile_menu (void);
void appl_profile_operations (void);

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */
/* Application specific information used to register with ATT */
DECL_STATIC ATT_APPLICATION att_app;

/* BLE Connection Handle */
DECL_STATIC UINT16 appl_ble_connection_handle;

/** ANP Characteristic related information */
typedef struct
{
    /* ANS - Alert Status Service */

    /* Supported New Alert Category */
    ATT_ATTR_HANDLE ans_sup_new_alert_cat_hdl;

    /* New Alert */
    ATT_ATTR_HANDLE ans_new_alert_hdl;

    /* New Alert - CCC */
    ATT_ATTR_HANDLE ans_new_alert_ccc_hdl;

    /* Supported Unread Alert Category */
    ATT_ATTR_HANDLE ans_sup_unread_alert_cat_hdl;

    /* Unread Alert */
    ATT_ATTR_HANDLE ans_unread_alert_hdl;

    /* Unread Alert - CCC */
    ATT_ATTR_HANDLE ans_unread_alert_ccc_hdl;

    /* Alert Notification Control Point */
    ATT_ATTR_HANDLE ans_alert_ntf_cntrl_pnt_hdl;

    /* BAS - Battery Service */

    /* Battery Level */
    ATT_ATTR_HANDLE bas_batt_level_hdl;
} ANP_CHAR_INFO;


/* ANS - Alert Status Service */

/* Supported New Alert Category */
#define ANS_SUP_NEW_ALRT_CAT_HDL     (anp_char_info.ans_sup_new_alert_cat_hdl)

/* New Alert */
#define ANS_NEW_ALRT_HDL             (anp_char_info.ans_new_alert_hdl)

/* New Alert - CCC */
#define ANS_NEW_ALRT_CCC_HDL         (anp_char_info.ans_new_alert_ccc_hdl)

/* Supported Unread Alert Category */
#define ANS_SUP_UNREAD_ALRT_CAT_HDL  (anp_char_info.ans_sup_unread_alert_cat_hdl)

/* Unread Alert */
#define ANS_UNREAD_ALRT_HDL          (anp_char_info.ans_unread_alert_hdl)

/* Unread Alert - CCC */
#define ANS_UNREAD_ALRT_CCC_HDL      (anp_char_info.ans_unread_alert_ccc_hdl)

/* Alert Notification Control Point */
#define ANS_ALRT_NTF_CNTRL_PNT_HDL   (anp_char_info.ans_alert_ntf_cntrl_pnt_hdl)

/* BAS - Battery Service */

/* Battery Level */
#define BAS_BATTERY_LEVEL_HDL        (anp_char_info.bas_batt_level_hdl)

#define APPL_CLI_CNFG_VAL_LENGTH    2
#define APPL_ANP_INFO_INIT()\
        ANS_NEW_ALRT_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        ANS_NEW_ALRT_CCC_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        ANS_SUP_UNREAD_ALRT_CAT_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        ANS_UNREAD_ALRT_CCC_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        ANS_ALRT_NTF_CNTRL_PNT_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        BAS_BATTERY_LEVEL_HDL = ATT_INVALID_ATTR_HANDLE_VAL;

#define ENABLE_NEW_INCOMING_ALRT_NTF                  0
#define ENABLE_UNREAD_CATEGORY_STATUS_NTF             1
#define DISABLE_NEW_INCOMING_ALRT_NTF                 2
#define DISABLE_UNREAD_CATEGORY_STATUS_NTF            3
#define NOTIFY_NEW_INCOMING_ALRT_IMMEDIATELY          4
#define NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY     5

DECL_STATIC ANP_CHAR_INFO anp_char_info;

#ifdef APPL_MENU_OPS
BT_DEVICE_ADDR g_peer_bd_addr;
const char main_menu[] = " \n\
========= M A I N   M E N U ========= \n\
 \n\
    0. Exit \n\
    1. Refresh \n\
\n\
    2. ANP menu operation\n\
\n\
    Your Option ?\n";

static UCHAR anp_client_menu[] =
"\n\
    0.  Exit \r\n\
\r\n\
    1.  Refresh \r\n\
\r\n\
    2.  Disconnect \r\n\
\r\n\
       === Alert Notification Service ===\r\n\n\
   10. Discover Alert Notification Service \r\n\
\r\n\
        -- Supported New Alert Category -- \r\n\
   11.  Read Supported New Alert Category \r\n\
\r\n\
        -- New Alert -- \r\n\
   12.  Configure New Alert CCC \r\n\
   13.  Stop New Alert CCC \r\n\
\r\n\
        -- Supported Unread Alert Category -- \r\n\
   14.  Read Supported Unread Alert Category \r\n\
\r\n\
        -- Unread Alert -- \r\n\
   15.  Configure Unread Alert CCC \r\n\
   16.  Stop Unread Alert CCC \r\n\
\r\n\
        -- Alert Notification Control Point -- \r\n\
   17.  Enable New Alert Notification through Control Point \r\n\
   18.  Disable New Alert Notification through Control Point \r\n\
   19.  Enable Unread Alert Notification through Control Point \r\n\
   20.  Disable Unread Alert Notification through Control Point \r\n\
   21.  Notify New Alert Immediately \r\n\
   22.  Notify Unread Alert Immediately \r\n\
\r\n\n\
        === Battery Service === \r\n\n\
   60 - Discover Battery Service \r\n\
\r\n\
   61 - Read Battery Level \r\n\n\
   Your Option? \0";
#endif /* APPL_MENU_OPS */

/* --------------------------------------------- Constants */

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

void appl_vse7_init(void)
{
    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    appl_vse7_server_reinitialize ();
}

void appl_vse7_connect(APPL_HANDLE  * appl_handle)
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

void appl_vse7_server_reinitialize (void)
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

#ifdef APPL_MENU_OPS
void appl_profile_menu_handler (void)
{
    int choice;

    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_device_init_req,
        NULL
    );

    while (1)
    {
        CONSOLE_OUT ("\n");
        CONSOLE_OUT ("%s", main_menu);
        CONSOLE_IN ("%d", &choice);

        switch(choice)
        {
        case 0: /* exit */
            return;

        case 1: /* Refresh */
            break;

        case 2:
            appl_profile_operations();
            break;

        default:
            break;
        }
    }
}
#endif /* APPL_MENU_OPS */

/* ------------------------------- ATT related Functions */

void appl_rcv_service_desc (UINT16 config, UINT16 uuid, UINT16 value_handle)
{
    /* Populate Needed CCCDs here */
    if (GATT_CLIENT_CONFIG == config)
    {
        if (GATT_NEW_ALERT_CHARACTERISTIC == uuid)
        {
            ANS_NEW_ALRT_CCC_HDL = value_handle;
        }
        else if (GATT_UNREAD_ALRT_STATUS_CHARACTERISTIC == uuid)
        {
            ANS_UNREAD_ALRT_CCC_HDL = value_handle;
        }
    }
}

void appl_rcv_service_char (UINT16 uuid, UINT16 value_handle)
{
#ifdef APPL_MENU_OPS
    switch (uuid)
    {
    case GATT_SPRTD_NEW_ALRT_CTGRY_CHARACTERISTIC:
        ANS_SUP_NEW_ALRT_CAT_HDL = value_handle;
        break;

    case GATT_NEW_ALERT_CHARACTERISTIC:
        ANS_NEW_ALRT_HDL = value_handle;
        break;

    case GATT_SPRT_UNRD_ALRT_CTGRY_CHARACTERISTIC:
        ANS_SUP_UNREAD_ALRT_CAT_HDL = value_handle;
        break;

    case GATT_UNREAD_ALRT_STATUS_CHARACTERISTIC:
        ANS_UNREAD_ALRT_HDL = value_handle;
        break;

    case GATT_ALERT_NTF_CONTROL_POINT:
        ANS_ALRT_NTF_CNTRL_PNT_HDL = value_handle;
        break;

    case GATT_BATTERY_LEVEL_CHARACTERISTIC:
        BAS_BATTERY_LEVEL_HDL = value_handle;
        break;

    default:
        APPL_TRC ("Unknown\n");
        break;
    }
#endif /* APPL_MENU_OPS */
    APPL_TRC ("----\n\n");
}

/**
 *  Function to initiate Service Discovery for
 *  Alert Notification Service.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_alert_notification_service(void)
{
    ATT_UUID16             uuid16;
    API_RESULT             retval;

    uuid16 = GATT_ALERT_NOTIFICATION_SERVICE;

    /* Discover Alert Notification Service */
    retval = appl_discover_service
             (
                 &uuid16,
                 ATT_16_BIT_UUID_FORMAT
             );

    return;
}

/**
 *  Function to initiate Service Discovery for Battery.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_battery_service(void)
{
    ATT_UUID16             uuid16;

    uuid16 = GATT_BATTERY_SERVICE;

    /* Discover Battery Service */
    appl_discover_service
    (
        &uuid16,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/* ------------------------------- Dummy Functions */
#include "fsm_defines.h"

#ifdef APPL_MENU_OPS
void appl_profile_operations (void)
{
    int          choice;
    ATT_VALUE    att_value;
    UINT16       cli_cfg;
    UCHAR        cfg_val[APPL_CLI_CNFG_VAL_LENGTH];
    UCHAR        value_arr[2];
    int read;
    UCHAR category_id;

    while (1)
    {
        CONSOLE_OUT(
        "%s", anp_client_menu);

        CONSOLE_IN ( "%d", &choice);

        switch (choice)
        {
            case 0:
                /* Exit */
                return;

            case 1:
                /* Refresh*/
                break;

            case 2:
                /* TBD! taking handle wrt 0th index */
                appl_ble_connection_handle = APPL_GET_CONNECTION_HANDLE(0);
                /* Initiate BLE Disconnection - Step 2(f) */
                BT_hci_disconnect (appl_ble_connection_handle, 0x13);
                break;

            case 10:
                appl_discover_alert_notification_service();
                break;

            case 11:
                appl_read_req(ANS_SUP_NEW_ALRT_CAT_HDL);
                break;

            case 12:
                cli_cfg = GATT_CLI_CNFG_NOTIFICATION;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                att_value.len = APPL_CLI_CNFG_VAL_LENGTH;
                att_value.val = cfg_val;
                appl_write_req (ANS_NEW_ALRT_CCC_HDL,&att_value);
                break;

            case 13:
                cli_cfg = GATT_CLI_CNFG_DEFAULT;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                att_value.len = APPL_CLI_CNFG_VAL_LENGTH;
                att_value.val = cfg_val;
                appl_write_req (ANS_NEW_ALRT_CCC_HDL,&att_value);
                break;

            case 14:
                appl_read_req(ANS_SUP_UNREAD_ALRT_CAT_HDL);
                break;

            case 15:
                cli_cfg = GATT_CLI_CNFG_NOTIFICATION;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                att_value.len = APPL_CLI_CNFG_VAL_LENGTH;
                att_value.val = cfg_val;
                appl_write_req (ANS_UNREAD_ALRT_CCC_HDL,&att_value);
                break;

            case 16:
                cli_cfg = GATT_CLI_CNFG_DEFAULT;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                att_value.len = APPL_CLI_CNFG_VAL_LENGTH;
                att_value.val = cfg_val;
                appl_write_req (ANS_UNREAD_ALRT_CCC_HDL,&att_value);
                break;

            case 17:
                CONSOLE_OUT (" Enter Category Id : \n");
                CONSOLE_IN ("%d", &read);

                category_id = (UCHAR) read;

                value_arr[0] = ENABLE_NEW_INCOMING_ALRT_NTF;
                value_arr[1] = category_id;
                att_value.len = 2;
                att_value.val = &value_arr[0];
                appl_write_req (ANS_ALRT_NTF_CNTRL_PNT_HDL,&att_value);
                break;

            case 18:
                CONSOLE_OUT (" Enter Category Id : \n");
                CONSOLE_IN ("%d", &read);

                category_id = (UCHAR) read;

                value_arr[0] = DISABLE_NEW_INCOMING_ALRT_NTF;
                value_arr[1] = category_id;
                att_value.len = 2;
                att_value.val = &value_arr[0];
                appl_write_req (ANS_ALRT_NTF_CNTRL_PNT_HDL,&att_value);
                break;

            case 19:
                CONSOLE_OUT (" Enter Category Id : \n");
                CONSOLE_IN ("%d", &read);

                category_id = (UCHAR) read;

                value_arr[0] = ENABLE_UNREAD_CATEGORY_STATUS_NTF;
                value_arr[1] = category_id;
                att_value.len = 2;
                att_value.val = &value_arr[0];
                appl_write_req (ANS_ALRT_NTF_CNTRL_PNT_HDL,&att_value);
                break;

            case 20:
                CONSOLE_OUT (" Enter Category Id : \n");
                CONSOLE_IN ("%d", &read);

                category_id = (UCHAR) read;

                value_arr[0] = DISABLE_UNREAD_CATEGORY_STATUS_NTF;
                value_arr[1] = category_id;
                att_value.len = 2;
                att_value.val = &value_arr[0];
                appl_write_req (ANS_ALRT_NTF_CNTRL_PNT_HDL,&att_value);
                break;

            case 21:
                CONSOLE_OUT (" Enter Category Id : \n");
                CONSOLE_IN ("%d", &read);

                category_id = (UCHAR) read;

                value_arr[0] = NOTIFY_NEW_INCOMING_ALRT_IMMEDIATELY;
                value_arr[1] = category_id;
                att_value.len = 2;
                att_value.val = &value_arr[0];
                appl_write_req (ANS_ALRT_NTF_CNTRL_PNT_HDL,&att_value);
                break;

            case 22:
                CONSOLE_OUT (" Enter Category Id : \n");
                CONSOLE_IN ("%d", &read);

                category_id = (UCHAR) read;

                value_arr[0] = NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY;
                value_arr[1] = category_id;
                att_value.len = 2;
                att_value.val = &value_arr[0];
                appl_write_req (ANS_ALRT_NTF_CNTRL_PNT_HDL,&att_value);
                break;

            case 60:
                appl_discover_battery_service ();
                break;
            case 61:
                appl_read_req (BAS_BATTERY_LEVEL_HDL);
                break;

            default:
                break;
        };
    }
}

#else

void appl_set_scan_parameters(UCHAR read)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_set_scan_mode(UCHAR mode)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_create_connection(void)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_bond_with_peer(void)
{
    API_RESULT retval;
    /* If connection is successful, initiate bonding [Step 2(c)] */

    SMP_AUTH_INFO auth;
    SMP_BD_ADDR   smp_peer_bd_addr;
    SMP_BD_HANDLE smp_bd_handle;

    APPL_SET_STATE(DEVICE_CONNECTED);

    auth.param = 1;
    auth.bonding = 1;
    auth.ekey_size = 12;
    auth.security = SMP_SEC_LEVEL_1;

    BT_COPY_BD_ADDR(smp_peer_bd_addr.addr, g_peer_bd_addr.addr);
    BT_COPY_TYPE(smp_peer_bd_addr.type, g_peer_bd_addr.type);

    retval = BT_smp_get_bd_handle
             (
                 &smp_peer_bd_addr,
                 &smp_bd_handle
             );

    if (API_SUCCESS == retval)
    {
        retval = BT_smp_authenticate (&smp_bd_handle, &auth);
    }

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "Initiation of Authentication Failed. Reason 0x%04X\n",
        retval);
    }

    /**
     *  Application will receive authentication complete event,
     *  in SMP Callback.
     *
     *  Look for 'SMP_AUTHENTICATION_COMPLETE' event handling in
     *  'appl_smp_callback'.
     */
}

void appl_initiate_connection_update(void)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_add_dev_to_white_list(BT_DEVICE_ADDR * peer_div_addr)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_set_adv_data(void)
{
    API_RESULT retval;
    /**
    *  Step 2(a). Initiate Advertising.
    *
    *  The operation is performed in three parts.
    *  First set the advertising data.
    *  Second set the advertising parameters.
    *  Finally, on completion of set advertising parameter, enable advertising.
    *
    *  Completion of set advertising data is informed through
    *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback 'appl_hci_callback'.
    *  Look for the handling of opcode 'HCI_LE_SET_ADVERTISING_DATA_OPCODE'.
    */
    retval = BT_hci_le_set_advertising_data
         (
              APPL_ANP_GAP_ACCESS_ADV_DATA_LEN(),
              APPL_ANP_GAP_ACCESS_ADV_DATA()
         );
    if (API_SUCCESS != retval)
    {
    APPL_ERR (
    "Failed to set Adv. Data. Result = 0x%04X\n", retval);
    }

}

void appl_set_adv_params(void)
{
    /*
     *  Completion of set advertising parameter is informed through
     *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback
     *  'appl_hci_callback'.
     *  Look for the handling of opcode
     *  'HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE'.
     */
    BT_hci_le_set_advertising_parameters
    (
        APPL_ANP_GAP_GET_ADV_MIN_INTERVAL(),
        APPL_ANP_GAP_GET_ADV_MAX_INTERVAL(),
        APPL_ANP_GAP_GET_ADV_TYPE(),
        APPL_CONFIG_PERIPHERAL_UNBONDED_OWN_ADDR_TYPE(),
        APPL_CONFIG_PERIPHERAL_UNBONDED_DIRECT_ADDR_TYPE(),
        APPL_ANP_CONFIG_PEER_DEVICE_ADDR(),
        APPL_ANP_GAP_GET_ADV_CHANNEL_MAP(),
        APPL_ANP_GAP_GET_ADV_FILTER_POLICY()
    );
}

void appl_set_adv_mode(UCHAR mode)
{
    /*
     *  Completion of set advertising enabled is informed through
     *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback
     *  'appl_hci_callback'.
     *  Look for the handling of opcode
     *  'HCI_LE_SET_ADVERTISING_ENABLE_OPCODE'.
     */

    /**
      *  Enable/Disable Advertising
      *  - Diable Advertising: 0x00
      *  - Enable Advertising: 0x01
      */

    BT_hci_le_set_advertising_enable(mode);
}
#endif /* APPL_MENU_OPS */
#endif /*  VSE7 */


