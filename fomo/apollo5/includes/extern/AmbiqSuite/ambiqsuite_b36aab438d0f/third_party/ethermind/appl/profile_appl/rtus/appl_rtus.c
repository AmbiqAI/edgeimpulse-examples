
/**
 *  \file appl_rtus.c
 *
 *  This file contains the Reference Time Update application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef RTUS

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
const char main_options[] = " \r\n\
========= RTUS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
 \r\n\
    2. Initiate Disconnection \r\n\
\r\n\
    Your Option ?\r\n";

#else
const char menu[] = " \r\n\
========= RTUS : M A I N   M E N U ========= \r\n\
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

void appl_rtus_init(void)
{
    appl_rtus_server_reinitialize ();
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

void appl_rtus_server_reinitialize (void)
{
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
        CONSOLE_OUT ("\nExiting BlueLitE Menu Based Application ... \n\n");
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

#endif /* RTUS */

