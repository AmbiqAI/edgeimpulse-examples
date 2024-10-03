
/**
 *  \file appl.c
 *
 *  Main Application Source File
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "appl.h"
#include "appl_hci.h"
#include "appl_fsm_defines.h"

#ifdef BT_VENDOR_SPECIFIC_INIT
#include "vendor_specific_init.h"
#endif /* BT_VENDOR_SPECIFIC_INIT */

#ifdef BT_ANALYZER
#include "analyzer_interface.h"
/* #include "BT_sock_client.h" */
#endif /* BT_ANALYZER */

BT_timer_handle    appl_idle_timer;
APPL_PEER_INSTANCE appl_peer_device_list[BT_MAX_DEVICE_QUEUE_SIZE];
UCHAR              appl_active_connections;

#ifndef APPL_MENU_OPS
DECL_CONST
#endif /* APPL_MENU_OPS */
BT_DEVICE_ADDR     peer_bd_addr =
                   {
                       {0x99, 0x07, 0x00, 0x7A, 0x03, 0x00},

                       0x00
                   };

#ifdef FREERTOS
DECL_STATIC UCHAR curr_indx = 0;
#endif /* FREERTOS */

#ifdef APPL_CONFIG_LOCAL_DEV_ADDR_TYPE
/*
 * Local Device Address Type
 *
 * # Public Address (Both Central and Peripheral can use)
 *   >> BT_BD_PUBLIC_ADDRESS_TYPE
 * # Random Static Address (Both Central and Peripheral can use)
 *   >> BT_BD_STATIC_ADDRESS_TYPE
 * # Random Resolvable Private Address (According to the current Spec.
 *      Only Peripheral can use)
 *   >> BT_BD_RESOLVALE_PVT_ADDRESS_TYPE
 * # Random Non-Resolvable Private Address (According to the current Spec.
 *      Only Central can use)
 *   >> BT_BD_NON_RESOLVALE_PVT_ADDRESS_TYPE
 */
UCHAR appl_local_dev_addr_type = BT_BD_PUBLIC_ADDRESS_TYPE;

/*
 * Random Static Device Address
 * Two most significant bits equal to '1'
 *
 * Setting 0xFE-DC-BA-98-76-54 as default
 */
UCHAR local_static_address [BT_BD_ADDR_SIZE] = {0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE};
#endif /* APPL_CONFIG_LOCAL_DEV_ADDR_TYPE */

/* ----------------------------------------- External Global Variables */
API_RESULT appl_bluetooth_initialization_complete (void);
extern APPL_GAP_ADV_DATA appl_gap_adv_data[APPL_GAP_MAX_ADV_DATA_OPTIONS];
extern BT_DEVICE_ADDR g_bd_addr;

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */
static GATT_DB_HANDLE  appl_batt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BATTERY_INST,
    GATT_CHAR_BATTERY_LVL_INST
};
static ATT_ATTR_HANDLE    appl_bat_lvl_hndl;
static BT_timer_handle timer_handle;

#ifdef APPL_MENU_OPS
UCHAR appl_profile_bt_on_flag = 0x00;
#endif /* APPL_MENU_OPS */

/* ------------------------------- Functions */
/**
 *  \brief Entry point of the sample application.
 *
 *  \note
 *  BT_SW_TXP shall be defined for integrated (Controller & Host)
 *  single mode stack. When BT_SW_TXP is defined, appl_init() shall be
 *  the entry point for the application.
 */
#ifndef FREERTOS
int main (int argc, char **argv)
#else
int appl_init(void)
#endif /* FREERTOS */
{
    UINT32     index;

#ifndef APPL_MENU_OPS
#ifndef DONT_USE_STANDARD_IO
    API_RESULT retval;
#endif /* DONT_USE_STANDARD_IO */
#endif /* APPL_MENU_OPS */

#ifdef BT_UART
    /* Set Serial Parameters */
    hci_uart_set_serial_settings
    (
        APPL_CONFIG_UART_PORT(),
        APPL_CONFIG_UART_BAUDRATE()
    );
#endif /* BT_UART */

    /* Initialize SNOOP Related Functions */
    em_snoop_init();
    snoop_bt_init();

    /* Initialize stack */
    BT_ethermind_init ();

#ifdef BT_ANALYZER
    init_analyzer();
#endif /* BT_ANALYZER */

    /**
     *  Enable/disable debug logs runtime.
     */
#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
    /* Enable debug logging of all modules */
    BT_enable_module_debug_flag(0xFFFFFFFF);

    /* Disable debug logging of all modules */
    /* BT_disable_module_debug_flag(0xFFFFFFFF); */

    /* Disable debug logging of specific module - HCI as example */
    /* BT_disable_module_debug_flag(BT_MODULE_ID_HCI); */

    /* Set run level */
    /* Enable only Error Logs */
    /* BT_set_debug_level(EM_DEBUG_LEVEL_ERR); */

    /* Enable Error & Trace Logs */
    /* BT_set_debug_level(EM_DEBUG_LEVEL_TRC); */

#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

    /**
     *  Enable Storage Write only during
     *  - Authentication Update
     *  - BT Shutdown
     */
#ifdef STORAGE_HAVE_EVENT_MASK
    BT_storage_enable_events((STORAGE_EVENT_AUTH_UPDATE)|(STORAGE_EVENT_SHUTDOWN));
#endif /* STORAGE_HAVE_EVENT_MASK */

#ifdef HCI_TX_RUN_TIME_SELECTION
#ifdef BT_USB
    BT_hci_register_tx_cb(hci_usb_send_data);
#endif /* BT_USB */

#ifdef BT_UART
    BT_hci_register_tx_cb(hci_uart_send_data);
#endif /* BT_UART */

#ifdef BT_SOCKET
    BT_hci_register_tx_cb(hci_socket_send_data);
#endif /* BT_SOCKET */
#endif /* HCI_TX_RUN_TIME_SELECTION */

    index = BT_MAX_REMOTE_DEVICES;

    SET_ALL_APPL_STATE (SL_0_IDLE);
    do
    {
        index--;
        APPL_DEVICE_INIT(index);
    } while (index > 0);

    appl_idle_timer = BT_TIMER_HANDLE_INIT_VAL;
    appl_active_connections = 0;

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
    /* Initialize console */
    console_init(appl_get_profile_menu());
#endif /* BT_SW_TXP */

#else  /* APPL_MENU_OPS */
    /** Post Power on request to the application fsm */
#ifndef DONT_USE_STANDARD_IO
    retval =
#endif /* DONT_USE_STANDARD_IO */
             fsm_post_event
             (
                 APPL_FSM_ID,
                 ev_appl_device_init_req,
                 NULL
             );

    APPL_TRC (
    "[APPL]: FSM Post Event Result for Device Intialization Request = 0x%04X",
    retval);

#endif /* APPL_MENU_OPS */

#ifndef FREERTOS
    /** Wait-Loop - to avoid exiting from the process */
    while (1)
    {
#ifdef APPL_MENU_OPS
        appl_profile_menu_handler();
#else
        BT_sleep(1);
#endif /* APPL_MENU_OPS */
    }
#endif /* FREERTOS */
    return 0;
}

API_RESULT appl_bluetooth_initialization_complete ( void )
{
    ATT_APPLICATION   att_cb;
    API_RESULT retval;

#ifdef INCLUDE_SP_APP
    pf_notify(SP_IND_BT_INIT_DONE);
#endif /* INCLUDE_SP_APP */
    /* Register application security handling with SMP and configure */
#ifdef SMP
    appl_smp_register ();
    appl_smp_configure_platform ();
#endif /* SMP */

#ifndef GATT_CLIENT
    /* Initialize the ATT Server Application */
    appl_att_server_init();

    /** Register with ATT */
    att_cb.cb = &appl_att_cb;
    retval = BT_att_register(&att_cb);

    APPL_TRC (
    "[APPL]: ATT Registeration Result 0x%04X\n",retval);
#else /* GATT_CLIENT */
    appl_gatt_init();
#endif /* GATT_CLIENT */
#ifdef APPL_CONFIG_LOCAL_DEV_ADDR_TYPE
    if (BT_BD_STATIC_ADDRESS_TYPE == APPL_PRECISE_LOCAL_DEV_ADDR_TYPE())
    {
        /* Set Random Address */
        BT_hci_le_set_random_address(local_static_address);
    }
#endif /* APPL_CONFIG_LOCAL_DEV_ADDR_TYPE */

    retval = fsm_post_event
             (
                 APPL_FSM_ID,
                 ev_appl_power_on_complete_ind,
                 NULL
             );

    return retval;
}


API_RESULT appl_bluetooth_on_complete ( void )
{
#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
    /* Initialize console */
    console_init(appl_get_profile_menu());

    CONSOLE_OUT ("\r\nTurned ON Bluetooth.\r\n");
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

    /* Perform Bluetooth power on initialization */
    return appl_bluetooth_initialization_complete ();
}

void appl_get_bd_addr(UCHAR *bd_addr)
{
    int i, read;
    read = 0;
    for (i = 0; i < 6; i ++)
    {
        CONSOLE_IN (
        "%x", &read);
        bd_addr[i] = (UCHAR) read;
    }
}

void appl_dump_bytes (UCHAR *buffer, UINT16 length)
{
    char hex_stream[49];
    char char_stream[17];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;

    APPL_TRC ("\n");
    APPL_TRC ("-- Dumping %d Bytes --\n",
    (int)length);

    APPL_TRC (
    "-------------------------------------------------------------------\n");

    count = 0;
    offset = 0;
    for(i = 0; i < length; i ++)
    {
        c =  buffer[i];
        sprintf(hex_stream + offset, "%02X ", c);

        if ( (c >= 0x20) && (c <= 0x7E) )
        {
            char_stream[count] = c;
        }
        else
        {
            char_stream[count] = '.';
        }

        count ++;
        offset += 3;

        if(16 == count)
        {
            char_stream[count] = '\0';
            count = 0;
            offset = 0;

            APPL_TRC ("%s   %s\n",
            hex_stream, char_stream);

            BT_mem_set(hex_stream, 0, 49);
            BT_mem_set(char_stream, 0, 17);
        }
    }

    if(offset != 0)
    {
        char_stream[count] = '\0';

        /* Maintain the alignment */
        APPL_TRC ("%-48s   %s\n",
        hex_stream, char_stream);
    }

    APPL_TRC (
    "-------------------------------------------------------------------\n");

    APPL_TRC ("\n");

    return;
}

void appl_set_all_state(APPL_STATE_T state)
{
    UINT32 index;

    index = BT_MAX_REMOTE_DEVICES;
    do
    {
        index--;
        appl_set_state(index,state);
    } while (index > 0);
}

void appl_set_state(APPL_HANDLE  handle, APPL_STATE_T state)
{
    APPL_TRC (
    "Changing Application State. Previous 0x%02X, New 0x%02X\n",
    GET_APPL_STATE(handle), state);

    GET_APPL_STATE(handle) = state;
}

/** Application State Handler */
API_RESULT appl_access_state_handler
           (
                void       * param,
                STATE_T    * state
           )
{
    if ((NULL == param) ||
       (BT_MAX_REMOTE_DEVICES == ((APPL_EVENT_PARAM *)param)->handle))
    {
        /**
         *  For Device intitialization, power on and power off, there will not
         *  be any valid application instance handle, therefore use the first
         *  one as all application instances will have the same states during
         *  these stages
         */
        *state =  GET_APPL_STATE(0);
    }
    else
    {
        *state = GET_APPL_STATE(((APPL_EVENT_PARAM *)param)->handle);
    }

    return API_SUCCESS;
}


API_RESULT appl_add_device
           (
                /* IN */  ATT_HANDLE             * att_handle,
                /* OUT */ APPL_HANDLE            * handle
           )
{
    int index;
    API_RESULT   retval;

    index = BT_MAX_REMOTE_DEVICES;
    retval = API_FAILURE;

    do
    {
        index--;
        /* Allocates assuming duplication does not occur */
        if (DEVICE_HANDLE_INIT_VAL == APPL_GET_DEVICE_HANDLE(index))
        {
             APPL_GET_ATT_INSTANCE(index) = (*att_handle);

             APPL_SET_MTU(index, ATT_DEFAULT_MTU);

             (*handle) = index;
             retval = API_SUCCESS;
             break;
        }
    }while(index > 0);

    return retval;
}

API_RESULT appl_set_cxn_handle
           (
               /* IN */ UINT16           cxn_handle,
               /* IN */ DEVICE_HANDLE    dq_handle,
               /* OUT */ APPL_HANDLE     * handle
           )

{
    API_RESULT  retval;

    retval = appl_get_handle_from_device_handle (dq_handle,handle);

    if (API_SUCCESS == retval)
    {
        APPL_GET_CONNECTION_HANDLE((*handle)) = cxn_handle;
    }

    return retval;
}


API_RESULT appl_get_handle_from_cxn_handle
           (
                /* IN */  UINT16         cxn_handle,
                /* OUT */ APPL_HANDLE    * handle
           )
{
    int          index;
    API_RESULT   retval;

    index = BT_MAX_REMOTE_DEVICES;
    retval = API_FAILURE;

    do
    {
        index--;
        if (cxn_handle == APPL_GET_CONNECTION_HANDLE(index))
        {
            (*handle) =  (index);
            retval = API_SUCCESS;
            break;
        }
    }while(index > 0);

    return retval;
}


API_RESULT appl_get_handle_from_device_handle
           (
                /* IN */  DEVICE_HANDLE    dq_handle,
                /* OUT */ APPL_HANDLE      * handle
           )
{
    int          index;
    API_RESULT   retval;

    index = BT_MAX_REMOTE_DEVICES;
    retval = API_FAILURE;

    do
    {
        index--;
        if (dq_handle == APPL_GET_DEVICE_HANDLE(index))
        {
            (*handle) =  (index);
            retval = API_SUCCESS;
            break;
        }
    }while(index > 0);

    return retval;
}

API_RESULT appl_delete_device
           (
                /* IN */ UINT16 cxn_handle
           )
{
    int index; API_RESULT   retval;

    index = BT_MAX_REMOTE_DEVICES;
    retval = API_FAILURE;

    do
    {
        index--;
        if (cxn_handle == APPL_GET_CONNECTION_HANDLE(index))
        {
            APPL_DEVICE_INIT(index);
            retval = API_SUCCESS;
            break;
        }
    }while(index > 0);

    return retval;
}


API_RESULT appl_set_att_instance
           (
               /* IN */  ATT_HANDLE     * att_instance,
               /* OUT */ APPL_HANDLE    * handle
           )
{
    API_RESULT retval;

    retval = appl_get_handle_from_device_handle (att_instance->device_id,handle);

    if (API_SUCCESS == retval)
    {
        APPL_SET_ATT_INSTANCE ((*handle),(*att_instance));
    }

    return retval;
}


/** TBD: Implement. Now it always returns false */
UCHAR appl_is_device_bonded(void)
{
    return (UCHAR)BLE_FALSE;
}

#ifdef APPL_USE_IDLE_TIMER
void appl_idle_timer_expiry(void * data_param, UINT16 datalen)
{
    APPL_STOP_IDLE_TIMER ();

    fsm_post_event
    (
         APPL_FSM_ID,
         ev_appl_low_power_entry_req,
         NULL
    );
}
#endif /* APPL_USE_IDLE_TIMER */

/* Returns local time */
#ifndef CUSTOM_PROFILE
void appl_get_local_time (/* OUT */ UCHAR *buf, /* IN */ UINT16 buf_len)
{
#ifndef FREERTOS
    struct tm * now;
    time_t      time_value;
    UINT16      year;

    /* seconds after the minute - [0,59] */
    /* int tm_sec; */

    /* minutes after the hour - [0,59] */
    /* int tm_min; */

    /* hours since midnight - [0,23] */
    /* int tm_hour; */

    /* day of the month - [1,31] */
    /* int tm_mday; */

    /* months since January - [0,11] */
    /* int tm_mon; */

    /* years since 1900 */
    /* int tm_year; */

    /* days since Sunday - [0,6] */
    /* int tm_wday; */

    /* days since January 1 - [0,365] */
    /* int tm_yday; */

    /* daylight savings time flag */
    /* int tm_isdst; */

    time_value = time(NULL);          /* Get time value              */
    now = localtime(&time_value);     /* Get time and date structure */

    /* Year */
    year = (UINT16)(now->tm_year + 1900);

    buf[0] = (UCHAR)year;
    buf[1] = (UCHAR)(year >> 8);

    /* Month */
    buf[2] = (UCHAR)now->tm_mon;

    /* Date */
    buf[3] = (UCHAR)now->tm_mday;

    /* Hour */
    buf[4] = (UCHAR)now->tm_hour;

    /* Minute */
    buf[5] = (UCHAR)now->tm_min;

    /* Second */
    buf[6] = (UCHAR)now->tm_sec;

    /* Day of week */
    buf[7] = (UCHAR)now->tm_wday;

    /* Fraction256 */
    buf[8] = 0x00;

#else
    UCHAR timestamp [4][9] =
    {
        {0xDB, 0x07, 0x04, 0x13, 0x02, 0x1F, 0x00, 0x06, 0x00},
        {0xDB, 0x07, 0x04, 0x13, 0x07, 0x1F, 0x00, 0x06, 0x00},
        {0xDB, 0x07, 0x04, 0x13, 0x09, 0x1F, 0x00, 0x06, 0x00},
        {0xDB, 0x07, 0x04, 0x13, 0x0C, 0x1F, 0x00, 0x06, 0x00}
    };

    BT_mem_copy (buf, timestamp[curr_indx], 9);
    curr_indx++;

    curr_indx &= 0x03;
#endif /* FREERTOS */

    return;
}
#endif /* CUSTOM_PROFILE */

/* Generic Handler to update Battery level */
void appl_update_battery_lvl (DEVICE_HANDLE  * dq_handle)
{
    API_RESULT    retval;
    ATT_VALUE    value;
    UCHAR        data;

    data = APPL_BATTERY_PERCENT;
    value.val = &data;
    value.len = 1;

    appl_batt_db_handle.device_id = (*dq_handle);

    /* Get Battery Level Handle */
    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_batt_db_handle,
                  &appl_bat_lvl_hndl
             );

    retval = BT_gatt_db_set_char_val (&appl_batt_db_handle,&value);

    if (API_FAILURE != retval)
    {
        APPL_TRC ("Successfully updated Battery Level, reason 0x%04X",
        retval);
    }
    else
    {
        APPL_TRC ("[ERR]: Failed to update Battery Level, reason 0x%04X",
        retval);
    }

}

API_RESULT appl_battery_level_handler (GATT_DB_HANDLE * handle, UINT16 config)
{
    API_RESULT    retval;
    APPL_HANDLE   appl_handle;

    retval = API_SUCCESS;

    /* If Notification is Enabled then start Notification */
    if (GATT_CLI_CNFG_NOTIFICATION== config)
    {
        appl_get_handle_from_device_handle (handle->device_id,&appl_handle);

        /* Start timer to simulate Battery Level Notification */
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
                     1,
                     appl_batt_timer_expiry_handler,
                     &appl_handle,
                     sizeof (appl_handle)
                 );
        APPL_TRC (
        "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
    }

    return retval;
}

void appl_batt_timer_expiry_handler (void *data, UINT16 datalen)
{
    API_RESULT    retval;
    ATT_VALUE     value;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    APPL_HANDLE   appl_handle;

    appl_handle = (*((APPL_HANDLE *)data));

    retval = API_SUCCESS;

    BT_gatt_db_get_char_val (&appl_batt_db_handle,&value);

    hndl_val_param.handle =  appl_bat_lvl_hndl;
    hndl_val_param.value.val = value.val;
    hndl_val_param.value.len = 1;

    retval = BT_att_send_hndl_val_ntf
             (
                 &APPL_GET_ATT_INSTANCE(appl_handle),
                 &hndl_val_param
              );

    if (API_SUCCESS != retval)
    {
        APPL_TRC ("[** ERR **]: Failed to send Battery Level, reason 0x%04X",
        retval);
    }
}

#ifdef APPL_GAP_PERIPHERAL_SUPPORT
/*
 *  Add Target Address at the end of pre-configured
 *  advertising data associated with specific profile/service.
 *
 *  \param offset:
 *         End index of the pre-configured advertising data.
 *         Target Adress AD Type to be added from this offset.
 *
 *  \note:
 *  This function does not check if adding Target Address AD type
 *  exceeds specification defined advertising data length.
 */
void appl_set_target_address_in_adv_data (UCHAR offset)
{
    /* Add Target Address AD Type Length */
    appl_gap_adv_data[0].data_param[offset] =
                BT_BD_ADDR_SIZE + 1;

    /* Add Target Address AD Type. Public/Random */
    if (BT_BD_PUBLIC_ADDRESS_TYPE == g_bd_addr.type)
    {
        appl_gap_adv_data[0].data_param[offset + 1] =
                    HCI_AD_TYPE_PUBLIC_TARGET_ADDRESS;
    }
    else
    {
        appl_gap_adv_data[0].data_param[offset + 1] =
                    HCI_AD_TYPE_RANDOM_TARGET_ADDRESS;
    }

    /* Add Target Address */
    BT_COPY_BD_ADDR
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        g_bd_addr.addr
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + BT_BD_ADDR_SIZE + 2;
}

void appl_set_user_given_target_address_in_adv_data (UCHAR offset)
{
    BT_DEVICE_ADDR scanner_addr;

    appl_gap_get_addr_and_type(&scanner_addr);

    /* Add Target Address AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = BT_BD_ADDR_SIZE + 1;

    /* Add Target Address AD Type. Public/Random */
    if (BT_BD_PUBLIC_ADDRESS_TYPE == scanner_addr.type)
    {
        appl_gap_adv_data[0].data_param[offset + 1] =
                    HCI_AD_TYPE_PUBLIC_TARGET_ADDRESS;
    }
    else
    {
        appl_gap_adv_data[0].data_param[offset + 1] =
                    HCI_AD_TYPE_RANDOM_TARGET_ADDRESS;
    }

    /* Add Target Address */
    BT_COPY_BD_ADDR
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        scanner_addr.addr
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + BT_BD_ADDR_SIZE + 2;
}

void appl_set_user_given_service_solicitation_in_adv_data (UCHAR offset)
{
    /* Service_Data */
    UCHAR ser_uuid[2] = {0x13, 0x18};

    /* Add Service_Data AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 2 + 1;

    /* Add Service_Data Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x14;

    /* Add Service_Data */
    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        ser_uuid,
        2
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 2;
}

void appl_set_user_given_appearence_in_adv_data (UCHAR offset)
{
    /* HID Mouse */
    UCHAR appearence[2] = {0xC2, 0x03};

    /* Add Appearence AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 2 + 1;

    /* Add appearence Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x19;

    /* Add Target Address */
    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        appearence,
        2
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 2;
}
void appl_set_user_given_service_data_in_adv_data (UCHAR offset)
{
    /* Service_Data */
    UCHAR ser_data[1] = {0x00};

    UCHAR ser_uuid[2] = {0x13, 0x18};

    /* Add Service_Data AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 3 + 1;

    /* Add Service_Data Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x16;

    /* Add Service_Data */
    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        ser_uuid,
        2
    );

    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 4,
        ser_data,
        1
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 3 + 2;

}
void appl_set_user_given_slave_ci_in_adv_data (UCHAR offset)
{
    /* Service_Data */
    UCHAR slave_ci[2] = {0x0A, 0x00};

    /* Add Service_Data AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 4 + 1;

    /* Add Service_Data Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x12;

    /* Add Service_Data */
    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        slave_ci,
        2
    );

    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 4,
        slave_ci,
        2
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 4;
}
void appl_set_user_given_le_dev_addr_in_adv_data (UCHAR offset)
{
    /* HID Mouse */
    UCHAR local_dev_addr[BT_BD_ADDR_SIZE];

    BT_hci_get_local_bd_addr(local_dev_addr);

    /* Add Appearence AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = BT_BD_ADDR_SIZE + 1;

    /* Add appearence Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x1B;

    /* Add Target Address */
    BT_COPY_BD_ADDR
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        local_dev_addr
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + BT_BD_ADDR_SIZE;
}

void appl_set_user_given_le_role_in_adv_data (UCHAR offset)
{
    /* HID Mouse */
    UCHAR role = 0x02;

    /* Add Appearence AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 1 + 1;

    /* Add appearence Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x1C;


    appl_gap_adv_data[0].data_param[offset + 2] = role;


    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 1;
}

void appl_set_user_given_oob_in_adv_data (UCHAR offset)
{
    /* HID Mouse */
    UCHAR oob_data_flag = 0x03;

    /* Add Appearence AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 1 + 1;

    /* Add appearence Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x11;


    appl_gap_adv_data[0].data_param[offset + 2] = oob_data_flag;


    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 1;
}

void appl_set_user_given_sm_tk_in_adv_data (UCHAR offset)
{
    /* HID Mouse */
    UCHAR tk[16] = {0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
                    0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E};

    /* Add Appearence AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 16 + 1;

    /* Add appearence Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x10;

    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        tk,
        16
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 16;
}

void appl_set_user_given_adv_int_in_adv_data (UCHAR offset)
{
    /* HID Mouse */
    UCHAR adv_int[2] = {0x64, 0x00};

    /* Add Appearence AD Type Length */
    appl_gap_adv_data[0].data_param[offset] = 2 + 1;

    /* Add appearence Assigned Number */
    appl_gap_adv_data[0].data_param[offset + 1] = 0x1A;

    /* Add Target Address */
    BT_mem_copy
    (
        appl_gap_adv_data[0].data_param + offset + 2,
        adv_int,
        2
    );

    /* Update the actual length of advertising data */
    appl_gap_adv_data[0].len = offset + 2 + 2;
}

/*
 *  Add URI at the end of pre-configured
 *  advertising data associated with specific profile/service.
 *
 *  \param offset:
 *         End index of the pre-configured advertising data.
 *         URI AD Type to be added from this offset.
 *
 *  \note:
 *  This function does not check if adding URI AD type
 *  exceeds specification defined advertising data length.
 */
#ifdef APPL_URI_UTIL
void appl_set_uri_in_adv_data (UCHAR offset)
{
    CHAR appl_uri[] = "http://www.bluetooth.com";
    UINT16 appl_adv_uri_len;
    API_RESULT retval;

    /* Updating the buffer max len for the expected uri encoded output */
    appl_adv_uri_len = (APPL_GAP_MAX_ADV_DATA_LEN - offset - 1 - 1);

    /* Call URI ADV Encode API */
    retval = appl_encode_uri_adv_data
             (
                 appl_uri,
                 BT_str_len(appl_uri),
                 (appl_gap_adv_data[0].data_param + offset + 2),
                 &appl_adv_uri_len
             );

    if (API_SUCCESS == retval)
    {
        /* Add URI AD Type Length */
        appl_gap_adv_data[0].data_param[offset] = appl_adv_uri_len + 1;

        /* Add URI AD Type Identifier i.e. 0x24 assigned number */
        appl_gap_adv_data[0].data_param[offset + 1] = 0x24;

        /* Update the actual length of advertising data */
        appl_gap_adv_data[0].len = offset + appl_adv_uri_len + 1 + 1;
    }
}
#endif /* APPL_URI_UTIL */

#endif /* APPL_GAP_PERIPHERAL_SUPPORT */

void appl_gap_get_addr_and_type(BT_DEVICE_ADDR * bd_addr)
{
    int i, read = 0;

    CONSOLE_OUT("Enter remote BD_ADDR : ");
    // fflush(stdout);
    for (i = 0; i < 6; i ++)
    {
        CONSOLE_IN("%x", &read);
        bd_addr->addr[i] = (UCHAR) read;
    }

    printf("Ether remote BD_ADDR type (0-Public/1-Private) : ");
    scanf("%x", &read);

    bd_addr->type = read;
}

void appl_disconnect_device(void)
{
    UINT16 appl_ble_connection_handle;

    BT_hci_get_connection_handle
    (
        (BT_DEVICE_ADDR *)&peer_bd_addr,
        &appl_ble_connection_handle
    );

    BT_hci_disconnect (appl_ble_connection_handle, 0x13);
}

#ifdef APPL_MENU_OPS
void appl_store_peer_dev_addr(BT_DEVICE_ADDR * peer_div_addr)
{

    BT_COPY_BD_ADDR(peer_bd_addr.addr, peer_div_addr->addr);
    BT_COPY_TYPE(peer_bd_addr.type, peer_div_addr->type);

    appl_power_on_device();
}

void appl_power_on_device(void)
{
    API_RESULT retval;

    retval = API_SUCCESS;

    if (0x00 == appl_profile_bt_on_flag)
    {

        /** Post Power on request to the application fsm */
        retval = fsm_post_event
                 (
                     APPL_FSM_ID,
                     ev_appl_device_init_req,
                     NULL
                 );


        APPL_TRC (
        "[APPL]: FSM Post Event Result for Device Intialization Request = 0x%04X",
        retval);

        if (API_SUCCESS == retval)
        {
            appl_profile_bt_on_flag = 0x01;
        }

    }
    else
    {
        APPL_EVENT_PARAM    fsm_param;
        APPL_EVENT_PARAM_INIT(&fsm_param);

        fsm_param.data_param = (void *) &peer_bd_addr;
        fsm_param.data_len = sizeof (peer_bd_addr);

        /** Call for Scan or ADV when Re-Registration after BT-ON */
        retval = fsm_post_event
                 (
                     APPL_FSM_ID,
                     ev_appl_transport_connect_req,
                     (void*) &fsm_param
                 );
    }

    return;
}

#endif /* APPL_MENU_OPS */
