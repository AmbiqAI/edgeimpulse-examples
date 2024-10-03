
/**
 *  \file appl_vse8.c
 *
 *  This file contains the Vendor Specific GATT application.
 *  This application depicts how using Indications GATT Server
 *  can transmit vendor specific information.
 *
 *  - Enables a GATT Server Implementation to send Vendor Specific
 *    Information though a
 *    Vendor  Specific Characteristic using UUID
 *    -> 0xB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF
 *    in a Vendor Specific Service using UUID
 *    -> 0xA0A1A2A3A4A5A6A7A8A9AAABACADAEAF
 *    through Attribute Value Notifications.
 *  - Four services are exposed by the Server -
 *        a. GAP
 *        b. GATT
 *        c. Battery
 *        d. Vendor Specific Service.
 *    [NOTE]:See gatt_db.c for complete details of the record.
 *  - The Device once intialized (this includes transport intialization), as a
 *    GAP Peripheral device, starts advertising. Advertismenet Data contains
 *    Short Device Name and Service List, with 2 Services with Battery and
 *    Vendor Specific UUIDs. GAP and GATT are not included as these mandatory.
 *    [NOTE]: See appl.c and appl_fsm_handlers.c for initialization procedures.
 *  - Once connection is established it is expected that the peer GATT Client
 *    Device performs service discovery and configures the Vendor Specific
 *    Characteristics for Indications.
 *  - Once configured for Indications, the application sends Indications with 20
 *    (see APPL_TX_DATA_LENGTH) octets of data every 3 seconds
 *    (see APPL_TX_TRANSMISSION_INTERVAL). Data currently varies
 *    in each Indication  from all zeros to all 0xFF and rolling back to zero.
 * - Application does not stop the transfer unless one of teh following occurs:
 *        a. Peer configures the device to stop sending Indications
 *        b. Link Disconnection
 *        c. Application Exit
 * - On link disconnection, application starts advertising again.
 * - None of the Services are configured to enforce Security.
 * - Please note that it is possible to configure the device for GAP Central
 *   Role by enabling APPL_GAP_CENTRAL_SUPPORT feature flag in place of
 *   APPL_GAP_PERIPHERAL_SUPPORT.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef VSE8

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */
#define APPL_TX_TRANSMISSION_INTERVAL          1

/* --------------------------------------------- Static Global Variables */
static BT_timer_handle timer_handle;
static ATT_ATTR_HANDLE appl_tx_port_hndl;
static UCHAR tx_count;
static GATT_DB_HANDLE appl_vts_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_VENDOR_TX_INST,
    GATT_CHAR_VENDOR_TX_PORT_INST
};

static UCHAR  * tx_data;
static UCHAR  appl_tx_state;

/* --------------------------------------------- Functions */

void appl_vse8_init(void)
{
    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    appl_vse8_server_reinitialize ();
}

void appl_vse8_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    API_RESULT        retval;
    UINT16            cli_cnfg;

    cli_cnfg = 0;

    appl_vts_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_vts_db_handle,
                  &appl_tx_port_hndl
             );
    retval = BT_gatt_db_get_char_cli_cnfg (&appl_vts_db_handle,&value);
    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    APPL_TRC (
    "[APPL]: Fetched Client Configuration (0x%04X) for Device (0x%02X) with "
    "result 0x%04X", cli_cnfg, APPL_GET_DEVICE_HANDLE((*appl_handle)), retval);

    appl_manage_trasnfer(appl_vts_db_handle,cli_cnfg);
}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    APPL_HANDLE    appl_handle;
    API_RESULT     retval;

    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&appl_vts_db_handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&appl_vts_db_handle,&ekey_size);

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
        /* Update the TX state */
        appl_tx_state = BT_TRUE;

        /* Start Transfer Timer to Send measurement every second */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            BT_stop_timer (timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }

        appl_get_handle_from_device_handle (handle.device_id,&appl_handle);
        retval = BT_start_timer
                 (
                     &timer_handle,
                     APPL_TX_TRANSMISSION_INTERVAL,
                     appl_timer_expiry_handler,
                     &appl_handle,
                     sizeof (appl_handle)
                 );
        APPL_TRC (
        "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);

        if (API_SUCCESS != retval)
        {
            /**
             *  Transfer could not be started, take appropriate action. One
             *  option could be to indicate an application error for
             *  configuration request
             */
        }
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        /* Turn Off Transfer Timer if On */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            BT_stop_timer (timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }

        /* Update the TX state */
        appl_tx_state = BT_FALSE;
    }
    else
    {
        /* Incorrect Configuration */
    }
}

void appl_send_vse8_measurement (APPL_HANDLE   * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;
    UINT16                data_len;

    if (BT_TRUE != appl_tx_state)
    {
        /* Do not initiate NTFs if CCCD is not configured */
        return;
    }

    APPL_TRC ("Sending measurement On Handle 0x%04X\n",appl_tx_port_hndl);

    /* Total data_len available for NTF transmission is ATT_MTU - 1Byte Opcoe - 2Bytes Attr Handle */
    data_len = APPL_GET_MTU(*handle) - 3;

    tx_data   = BT_alloc_mem(data_len);

    if (NULL == tx_data)
    {
        CONSOLE_OUT("\n[ERROR]: Mem Alloc Failure for Ntf!!!\n");
    }

    /* Set the Pattern */
    BT_mem_set
    (
        tx_data,
        tx_count,
        data_len
    );

    hndl_val_param.handle    =  appl_tx_port_hndl;
    hndl_val_param.value.val = tx_data;
    hndl_val_param.value.len = data_len;

    retval = BT_att_send_hndl_val_ntf
             (
                 &APPL_GET_ATT_INSTANCE(*handle),
                 &hndl_val_param
              );

    /* Update the Pattern */
    tx_count++;

    if (API_SUCCESS != retval)
    {
        APPL_EVENT_PARAM    fsm_param;
        fsm_param.handle = *handle;

        APPL_TRC ("[** ERR **]: Failed to send measurement, reason 0x%04X",
        retval);

        /* Free the Allocated Memory */
        BT_free_mem(tx_data);

        /**
         * Retry if there is ATT_INVALID_PARAM_LEN error.
         * This scenario can occur, if Application is trying to send a
         * Notification with larger MTU, but during the process the remote
         * has negotiated and configured with a lesser MTU.
         */
        if (ATT_INVALID_PARAM_LEN == retval)
        {
            fsm_post_event
            (
                APPL_FSM_ID,
                ev_appl_measurement_ind,
                &fsm_param
            );
        }
    }
}

void appl_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM    fsm_param;
    API_RESULT          retval;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    retval = API_SUCCESS;

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

void appl_vse8_server_reinitialize (void)
{
    /* Turn Off Transfer Timer if On.  */
    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        BT_stop_timer (timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    tx_count = 0;

    appl_tx_state = BT_FALSE;
}

void appl_vse8_handle_ind_complete
     (
         APPL_HANDLE * handle,
         UCHAR       * event_data,
         UINT16        datalen
     )
{
    /* Currently Empty */
}

void appl_vse8_handle_ntf_complete
     (
         APPL_HANDLE * handle,
         UCHAR       * event_data,
         UINT16        datalen
     )
{
    APPL_EVENT_PARAM    fsm_param;
    fsm_param.handle = *handle;

    CONSOLE_OUT("\n>> appl_vse8_handle_ntf_complete\n");
    CONSOLE_OUT("\n NTF Sent for Appl Handle 0x%02X", *handle);
    appl_dump_bytes(event_data, datalen);
    CONSOLE_OUT("\n<< appl_vse8_handle_ntf_complete\n");

    /**
     * If more than one Characterisitics have notification
     * capabilities, extract the "Attribute Handle" of notification
     * complete from the 0th and 1st Byte of "event_data".
     * Based on that, the next set of Notifications of the desired,
     * "Attribute Handle" can be triggered.
     */

    /* Free the allocated Memory for Ntf Transmissions */
    BT_free_mem(tx_data);

    /* Start Chaining of next set of Notifications */
    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_measurement_ind,
        &fsm_param
    );
}

void appl_vse8_handle_mtu_update_complete
     (
         APPL_HANDLE * handle,
         UINT16 mtu
     )
{
    CONSOLE_OUT("\n>> appl_vse8_handle_mtu_update_complete\n");
    CONSOLE_OUT("\n UPDATED MTU is %d for Appl Handle 0x%02X", mtu, *handle);
    CONSOLE_OUT("\n<< appl_vse8_handle_mtu_update_complete\n");
}

#endif /* VSE8 */

