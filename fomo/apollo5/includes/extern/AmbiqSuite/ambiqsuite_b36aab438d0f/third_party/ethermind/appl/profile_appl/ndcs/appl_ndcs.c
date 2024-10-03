
/**
 *  \file appl_ndcs.c
 *
 *  This file contains the Next DST Change application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef NDCS

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */
/*
 * Read Current Time is used to read current time.
 * Read Current Time function is also used by Current Time Service,
 * and it returns Day of Week and Fraction256 fields
 * which are not required for Next DST Change (NDCS).
 *
 * NDCS has one additional field of one octet, DST Offset.
 */
#define APPL_NDCS_TIME_LENGTH                  (8 + 1)

/* Default DST Offset set as Standard Time */
#define APPL_NDCS_DEFAULT_DST_OFFSET           0x00
#define APPL_NDCS_DST_INDEX                    7

/* --------------------------------------------- Static Global Variables */
static GATT_DB_HANDLE appl_ndcs_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_NDCS_INST,
    GATT_CHAR_TIME_W_DST_INST
};

static ATT_ATTR_HANDLE appl_ndcs_time_hndl;

/* Using Adjust Reason as 0x01 (Manual Time Update) */
static UCHAR ndcs_time[APPL_NDCS_TIME_LENGTH] =
             {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UCHAR ndcs_time_count;

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
const char main_options[] = " \r\n\
========= NDCS : M A I N   M E N U ========= \r\n\
 \r\n\
    0. Exit \r\n\
    1. Refresh \r\n\
 \r\n\
    2. Initiate Disconnection \r\n\
\r\n\
    Your Option ?\r\n";
#else /* BT_SW_TXP */
const char menu[] = " \r\n\
========= NDCS : M A I N   M E N U ========= \r\n\
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

void appl_ndcs_init(void)
{
    appl_ndcs_server_reinitialize ();
}

void appl_ndcs_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    UCHAR             val_arr[2];
    UINT16            cli_cnfg;

    cli_cnfg = 0;

    value.len = 2;
    value.val = &val_arr[0];

    appl_ndcs_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    /* If Current Time is configured, initiate transfer */
    BT_gatt_db_get_char_val_hndl (&appl_ndcs_db_handle,&appl_ndcs_time_hndl);
    BT_gatt_db_get_char_cli_cnfg(&appl_ndcs_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,val_arr);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X)\n",
    cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)));
}

API_RESULT appl_ndcs_get_current_time
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    API_RESULT     retval;
    ATT_VALUE      updated_time;

    if (GATT_CHAR_TIME_W_DST_INST == handle->char_id)
    {
        /*
         * Read Current Time returns Day of Week and Fraction256 fields
         * which are not required for NDCS.
         *
         * NDCS has DST offset field.
         * In this sample example, it is set as Standard Time.
         */

        appl_get_local_time (ndcs_time, sizeof(ndcs_time));

        ndcs_time[APPL_NDCS_DST_INDEX] = APPL_NDCS_DEFAULT_DST_OFFSET;

        /* Update Current Time in GATT DB */
        updated_time.val = ndcs_time;
        updated_time.len = sizeof(ndcs_time) - 1;

        BT_gatt_db_set_char_val
        (
            handle,
            &updated_time
        );
        /* Return Success */
        retval = API_SUCCESS;
    }
    else
    {
        retval = GATT_DB_INVALID_OPERATION;
    }

    return retval;
}

void appl_ndcs_server_reinitialize (void)
{
    ndcs_time_count = 0;
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

#endif /* NDCS */

