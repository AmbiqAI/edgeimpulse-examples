
/**
 *  \file appl_cts.c
 *
 *  This file contains the Current Time application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef CTS

/* --------------------------------------------- External Global Variables */
extern BT_DEVICE_ADDR g_bd_addr;

/* --------------------------------------------- Exported Global Variables */
#define APPL_CUR_TIME_LENGTH                  10
#define APPL_LOC_TIME_LENGTH                  2
#define APPL_REF_TIME_LENGTH                  4
#define APPL_CUR_TIME_MEASUREMENT_INTERVAL    1
#define APPL_CUR_TIME_ILLEGAL_WRITE           0x80

/* Valid Year param */
#define APPL_YEAR_MAX                        9999
#define APPL_YEAR_MIN                        1582
#define APPL_YEAR_NOT_KNOWN                  0

/* Valid Month param */
#define APPL_MONTH_MAX                        12
#define APPL_MONTH_MIN                        0

/* Valid Day param */
#define APPL_DAY_MAX                          31
#define APPL_DAY_MIN                          0

/* Valid Hours param */
#define APPL_HOURS_MAX                        23
#define APPL_HOURS_MIN                        0

/* Valid Minutes param */
#define APPL_MINUTES_MAX                      59
#define APPL_MINUTES_MIN                      0

/* Valid Minutes param */
#define APPL_SECONDS_MAX                      59
#define APPL_SECONDS_MIN                      0

/* Validate Day of Week param*/
#define APPL_DAY_OF_WEEK_MAX                  7
#define APPL_DAY_OF_WEEK_MIN                  0

/* Adjust Reason Value Masks */
#define APPL_ADJUST_REASON_RFU_BITMASK        0xF0

/* Valid Timezone param */
#define APPL_TIMEZONE_MAX                     56
#define APPL_TIMEZONE_MIN                    -48
#define APPL_TIMEZONE_NOT_KNOWN              -128

/* Valid DST Offset param */
#define APPL_DSTOFFSET_MAX                    8
#define APPL_DSTOFFSET_MIN                    0
#define APPL_DSTOFFSET_NOT_KNOWN              255

/* --------------------------------------------- Static Global Variables */
static GATT_DB_HANDLE appl_cts_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_CUR_TIME_INST,
    GATT_CHAR_CUR_TIME_INST
};

static BT_timer_handle timer_handle;
static ATT_ATTR_HANDLE appl_cur_time_hndl;

/* Using Adjust Reason as 0x01 (Manual Time Update) */
static UCHAR cur_time[APPL_CUR_TIME_LENGTH] =
             {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F};

static UCHAR loc_time[APPL_LOC_TIME_LENGTH] = {0x33, 0x02};

static UCHAR ref_time[APPL_REF_TIME_LENGTH] = {0x04, 0x32, 0x32, 0x14};


UCHAR        curr_time_set;
static UCHAR cur_time_count;

static APPL_HANDLE    appl_handle;

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
const char menu[] = " \r\n\
========= CTS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
 \r\n\
    2. Register Peer/Slave Device Address \r\n\
    3. Initiate Disconnection \r\n\
\r\n\
    Your Option ?\r\n";
#else /* BT_SW_TXP */
static char cts_menu[] = "\n\
-------- CTS Menu ------- \n\
    0.  Exit \n\
    1.  Refresh menu \n\n\
    2.  External Time Reference Update with Adjusting Time less than 1 min\n\
    3.  External Time Reference Update with Adjusting Time more than 1 min\n\
    4.  Initiate disconnection\n\
    5.  Bond\n\
\n\
Your Option -> \0";
#endif /* BT_SW_TXP */
static UCHAR adj_time;
#endif /* APPL_MENU_OPS */

/*--------------------------------------------Global variables */
UCHAR datetime;

/* --------------------------------------------- Functions */

void appl_cts_init(void)
{
    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    appl_cts_server_reinitialize ();
}

void appl_cts_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;

    UINT16            cli_cnfg;

    cli_cnfg = 0;

    appl_cts_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    /* If Current Time is configured, initiate transfer */
    BT_gatt_db_get_char_val_hndl (&appl_cts_db_handle,&appl_cur_time_hndl);
    BT_gatt_db_get_char_cli_cnfg(&appl_cts_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));

    appl_manage_trasnfer(appl_cts_db_handle,cli_cnfg);
}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    API_RESULT     retval;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_cts_db_handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_cts_db_handle,&ekey_size);

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

    appl_get_handle_from_device_handle (handle.device_id,&appl_handle);
    if (GATT_CLI_CNFG_NOTIFICATION == config)
    {
        /* Start Transfer Timer to Send measurement every time interval */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            BT_stop_timer (timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }


        BT_start_timer
        (
            &timer_handle,
            APPL_CUR_TIME_MEASUREMENT_INTERVAL,
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

void appl_send_cur_time (APPL_HANDLE   * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    APPL_TRC (
    "Sending measurement On Handle 0x%04X\n", appl_cur_time_hndl);

    /* Read Current Time */
    appl_get_local_time (cur_time, sizeof(cur_time));

#ifdef APPL_MENU_OPS
    if (adj_time)
    {
        cur_time[5] += adj_time;
        adj_time = 20;
    }
#endif /* APPL_MENU_OPS */

    /* Using Adjust Reason always as 0x00 */
    hndl_val_param.handle =  appl_cur_time_hndl;
    hndl_val_param.value.val = cur_time;
    hndl_val_param.value.len = APPL_CUR_TIME_LENGTH;

    retval = BT_att_send_hndl_val_ntf
             (
                 &APPL_GET_ATT_INSTANCE(*handle),
                 &hndl_val_param
              );
    cur_time_count++;
    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "[** ERR **]: Failed to send measurement, reason 0x%04X",retval);
    }
}

API_RESULT appl_cts_get_current_time
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    API_RESULT     retval;
    ATT_VALUE      updated_time;
    ATT_VALUE      local_time;
    ATT_VALUE      reference_time;

    retval = API_SUCCESS;

    if (GATT_CHAR_CUR_TIME_INST == handle->char_id)
    {
        if (curr_time_set != BT_TRUE)
        {
            /* Read Current Time */
            appl_get_local_time(cur_time, sizeof(cur_time));
        }

        /* Update Current Time in GATT DB */
        updated_time.val = cur_time;
        updated_time.len = sizeof(cur_time);

        BT_gatt_db_set_char_val
        (
            handle,
            &updated_time
        );
    }
    else if(GATT_CHAR_LCL_TIME_INFO_INST == handle->char_id)
    {
        local_time.val = loc_time;
        local_time.len = sizeof(loc_time);

        BT_gatt_db_set_char_val
        (
            handle,
            &local_time
        );
    }
    else if (GATT_CHAR_REF_TIME_INFO_INST == handle->char_id)
    {
        reference_time.val = ref_time;
        reference_time.len = sizeof(ref_time);

        BT_gatt_db_set_char_val
        (
            handle,
            &reference_time
        );
    }
    else
    {
        retval = GATT_DB_INVALID_OPERATION;
    }

    return retval;
}

void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM    fsm_param;

    fsm_param.handle = (*((APPL_HANDLE *)data));

    /* Restart timer to simulate Temperature measurements */
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
        APPL_CUR_TIME_MEASUREMENT_INTERVAL,
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

void appl_cts_server_reinitialize (void)
{
    /* Turn Off Transfer Timer if On.  */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    cur_time_count = 0;
    datetime = 0x00;
    curr_time_set = BT_FALSE;

#ifdef APPL_MENU_OPS
    adj_time = 0;
#endif /* APPL_MENU_OPS */
}

API_RESULT appl_update_current_time (ATT_VALUE    * value)
{
    UINT16 data;

    /* Year */
    BT_UNPACK_LE_2_BYTE(&data, value->val);

    if (!((APPL_YEAR_MAX >= data) && \
        (APPL_YEAR_MIN <= data))||(APPL_YEAR_NOT_KNOWN == data))
    {
        APPL_ERR("[APPL] Illegal Year\n");
        return  (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Month */
    data = (UCHAR) value->val[2];

    if (APPL_MONTH_MAX < data)
    {
         APPL_ERR("[APPL] Illegal Month\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Date */
    data = (UCHAR) value->val[3];

    if (APPL_DAY_MAX < data)
    {
        APPL_ERR("[APPL] APPL Day Check FAILED\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Hours */
    data = (UCHAR) value->val[4];

    if (APPL_HOURS_MAX < data)
    {
        APPL_ERR("[APPL] Illegal Hours\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Minutes */
    data = (UCHAR) value->val[5];

    if (APPL_MINUTES_MAX < data)
    {
        APPL_ERR("[APPL] Illegal Minutes\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Seconds */
    data = (UCHAR) value->val[6];

    if (APPL_SECONDS_MAX < data)
    {
        APPL_ERR("[APPL] Illegal Seconds\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Day of the week */
    data = (UCHAR)value->val[7];

    if (APPL_DAY_OF_WEEK_MAX < data)
    {
        APPL_ERR("[APPL] Illegal Day of week\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Assigining Fractions256 here, but nothing to be done */
    data = (UCHAR)value->val[8];

    /* Adjust Reason */
    data = (UCHAR)value->val[9];

    if (0x00 != (((UCHAR)data) & APPL_ADJUST_REASON_RFU_BITMASK))
    {
        APPL_ERR("[APPL] Illegal Adjust Reason\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    BT_mem_copy(cur_time, &value->val[0], value->len);

    curr_time_set = BT_TRUE;

    return API_SUCCESS;
}

API_RESULT appl_update_local_time (ATT_VALUE    * value)
{
    INT8    data;

    /* Timezone */
    data = value->val[0];

    if (!((APPL_TIMEZONE_MAX >= data) && \
        (APPL_TIMEZONE_MIN <= data)) || \
        (APPL_TIMEZONE_NOT_KNOWN == data))
    {
        APPL_ERR("[APPL] APPL Timezone Illegal\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* DST Offset */
    data = (UCHAR) value->val[1];

    if (!((APPL_DSTOFFSET_MAX  >= data) && \
        (APPL_DSTOFFSET_MIN  <= data)) ||\
        (APPL_DSTOFFSET_NOT_KNOWN == data))

    {
        APPL_ERR("[APPL] APPL DST Illegal\n");
        return (APPL_CUR_TIME_ILLEGAL_WRITE | APPL_ERR_ID);
    }

    /* Copy the incoming to the application maintained local time */
    BT_mem_copy(loc_time, value->val, sizeof(loc_time));

    return API_SUCCESS;
}

API_RESULT appl_rtus_config_time_update
           (
               GATT_DB_HANDLE    * handle,
               ATT_VALUE         * value
           )
{
    UCHAR update_cp_value;

    /* Check Characteristic ID */
    if (GATT_CHAR_TIME_UPDATE_CP_INST == handle->char_id)
    {
        /* TODO: Have appropriate define for the CP value length */
        if (sizeof(UCHAR) == value->len)
        {
            update_cp_value = value->val[0];

            switch (update_cp_value)
            {
                case 0x01:
                    APPL_TRC ("Get Reference Update\n");
                    break;

                case 0x02:
                    APPL_TRC ("Cancel Reference Update\n");
                    break;

                default:
                    APPL_TRC ("Reserved value for Ref Update CP 0x%02X\n",
                    update_cp_value);
            }
        }
    }

    return API_SUCCESS;
}

API_RESULT appl_rtus_get_time_update_state
           (
               GATT_DB_HANDLE    * handle,
               ATT_VALUE         * value
           )
{
    ATT_READ_RSP_PARAM rd_rsp_param;
    APPL_HANDLE    appl_handle;
    UCHAR          time_update_state[2];
    API_RESULT     retval;

    retval = API_SUCCESS;
    appl_get_handle_from_device_handle (handle->device_id,&appl_handle);

    /* Check Characteristic ID */
    if (GATT_CHAR_TIME_UPDATE_STATE_INST == handle->char_id)
    {
        /**
         * Returning fixed value.
         *
         * Current State - Idle (0x00)
         * Result - Successful (0x00)
         */
        time_update_state[0] = 0x00;
        time_update_state[1] = 0x00;

        rd_rsp_param.val = time_update_state;
        rd_rsp_param.len = sizeof(time_update_state);
        rd_rsp_param.actual_len = sizeof(time_update_state);

        retval = BT_att_read_rsp
                 (
                     &APPL_GET_ATT_INSTANCE(appl_handle),
                     &rd_rsp_param
                  );

        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send Time Update State, reason 0x%04X",
            retval);
        }
    }

    return retval;
}

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
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
#else /* BT_SW_TXP */
void appl_profile_menu_handler (void)
{
    int    choice;
    SMP_AUTH_INFO auth;
    SMP_BD_ADDR   smp_peer_bd_addr;
    SMP_BD_HANDLE smp_bd_handle;
    API_RESULT retval;

    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_device_init_req,
        NULL
    );

    while (1)
    {
        printf("\n");
        printf("%s", cts_menu);
        scanf("%d", &choice);

        switch(choice)
        {
        case 0: /* exit */
            return;

        case 1: /* Refresh */
            break;

        case 2:
            /* Stop Notification if the Adjusting Time is less than 1 minute */
            if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
            {
                BT_stop_timer (timer_handle);
                timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
            break;

        case 3:
#ifdef APPL_MENU_OPS
            adj_time = 2;
#endif /* APPL_MENU_OPS */
            if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
            {
                BT_stop_timer (timer_handle);
                timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            BT_start_timer
            (
                &timer_handle,
                APPL_CUR_TIME_MEASUREMENT_INTERVAL,
                appl_timer_expiry_handler,
                &appl_handle,
                sizeof (appl_handle)
            );
            break;

        case 4:
            appl_disconnect_device();
            break;

        case 5:
            auth.param = 1;
            auth.bonding = 1;
            auth.ekey_size = 12;
            auth.security = SMP_SEC_LEVEL_1;
            BT_COPY_BD_ADDR(smp_peer_bd_addr.addr, g_bd_addr.addr);
            BT_COPY_TYPE(smp_peer_bd_addr.type, g_bd_addr.type);

            retval = BT_smp_get_bd_handle
                     (
                        &smp_peer_bd_addr,
                        &smp_bd_handle
                     );

            if (API_SUCCESS == retval)
            {
                retval = BT_smp_authenticate(&smp_bd_handle, &auth);
            }

            if (API_SUCCESS != retval)
            {
                APPL_TRC(
                    "Initiation of Authentication Failed. Reason 0x%04X\n",
                    retval);
            }
            break;
        }
    }
}
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

#endif /* CTS */

