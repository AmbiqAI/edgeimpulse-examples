
/**
 *  \file appl_hci_api.c
 *
 *  Source File for HCI Command Line Application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "appl_hci.h"


/* ----------------------------------------- External Global Variables */


/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */


/* ----------------------------------------- Functions */
void appl_hci_disconnect ( void )
{
    unsigned int read;
    API_RESULT retval;
    UCHAR      reason;
    UINT16     handle;

    printf("Disconnect ACL/SCO Connection. Enter Required Information ...\n");

    printf("ACL/SCO Connection Handle (in Hex) = "); fflush(stdout);
    scanf("%x", &read);
    handle = (UINT16)read;

    printf("Reason for Disconnection (in Hex) = "); fflush(stdout);
    scanf("%x", &read);
    reason = (UCHAR)read;

    printf("\n");
    printf("Starting ACL/SCO Disconnection ...\n"); fflush(stdout);
    retval = BT_hci_disconnect ( handle, reason );
    if ( retval != API_SUCCESS )
    {
        printf("FAILED !! Error Code = 0x%04x\n", retval);
    }
    else
    {
        printf("Successfully started.\n");
        printf("Please wait for Disconnect Complete.\n");
    }
}

void appl_hci_get_connection_details ( void )
{
    API_RESULT retval;
    UCHAR i, num_list;

    HCI_CONNECTION_LIST hci_conn_list[BT_MAX_REMOTE_DEVICES];

    printf("Getting Connection Details from HCI ...\n"); fflush(stdout);
    retval = BT_hci_get_connection_details
             (hci_conn_list, BT_MAX_REMOTE_DEVICES, &num_list);

    if (retval != API_SUCCESS)
    {
         printf("FAILED !! Error Code = 0x%04X\n", retval);
    }
    else
    {
        printf("Succeeded.\n");
        for (i = 0; i < num_list; i ++)
        {
            printf("Connection Detail %d:\n", i);
            printf("  BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X. Type 0x%02X\n",
            BT_BD_ADDR(&hci_conn_list[i].bd_addr)[0],
            BT_BD_ADDR(&hci_conn_list[i].bd_addr)[1],
            BT_BD_ADDR(&hci_conn_list[i].bd_addr)[2],
            BT_BD_ADDR(&hci_conn_list[i].bd_addr)[3],
            BT_BD_ADDR(&hci_conn_list[i].bd_addr)[4],
            BT_BD_ADDR(&hci_conn_list[i].bd_addr)[5],
            BT_BD_ADDR_TYPE(&hci_conn_list[i].bd_addr));
            printf("  Conn Handle = 0x%04X\n", hci_conn_list[i].acl_handle);
        }
    }
}

#ifdef BT_LE
/* Send HCI_LE_SET_EVENT_MASK */
void appl_hci_le_set_event_mask( void )
{
    API_RESULT retval;
    UCHAR   le_event_mask[8];
    UINT32   index;
    unsigned int read;

    printf("Enter le_event_mask : \n");
    for(index = 0; index<8; index++)
    {
        scanf("%X",&read);
        le_event_mask[index] = (UCHAR) read;
    }
    retval = BT_hci_le_set_event_mask
                  (
                      le_event_mask
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_BUFFER_SIZE */
void appl_hci_le_read_buffer_size( void )
{
    API_RESULT retval;
    retval = BT_hci_le_read_buffer_size();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_LOCAL_SUPPORTED_FEATURES */
void appl_hci_le_read_local_supported_features( void )
{
    API_RESULT retval;
    retval = BT_hci_le_read_local_supported_features();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_RANDOM_ADDRESS */
void appl_hci_le_set_random_address( void )
{
    API_RESULT retval;
    UCHAR   random_address[6];
    UINT32   index;
    unsigned int read;

    printf("Enter random_address : \n");
    for(index = 0; index<6; index++)
    {
        scanf("%X",&read);
        random_address[index] = (UCHAR) read;
    }
    retval = BT_hci_le_set_random_address
                  (
                      random_address
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_ADVERTISING_PARAMETERS */
void appl_hci_le_set_advertising_parameters( void )
{
    API_RESULT retval;
    UCHAR   advertising_channel_map;
    UCHAR   advertising_filter_policy;
    UCHAR   advertising_type;
    UCHAR   direct_addr[BT_BD_ADDR_SIZE];
    UCHAR   direct_addr_type;
    UCHAR   own_addr_type;
    UINT16   advertising_interval_max;
    UINT16   advertising_interval_min;
    UINT32   index;
    unsigned int read;

    BT_mem_set(direct_addr, 0, BT_BD_ADDR_SIZE);
    direct_addr_type = 0;

    printf("Enter advertising_interval_min : \n");
    scanf("%X",&read);
    advertising_interval_min = (UINT16) read;
    printf("Enter advertising_interval_max : \n");
    scanf("%X",&read);
    advertising_interval_max = (UINT16) read;
    printf("Enter advertising_type : \n");
    scanf("%X",&read);
    advertising_type = (UCHAR) read;
    printf("Enter own_addr_type : \n");
    scanf("%X",&read);
    own_addr_type = (UCHAR) read;

    printf("Enter direct_addr_type : \n");
    scanf("%X",&read);
    direct_addr_type = (UCHAR) read;
    printf("Enter direct_addr : \n");
    for(index = 0; index<6; index++)
    {
        scanf("%X",&read);
        direct_addr[index] = (UCHAR) read;
    }

    printf("Enter advertising_channel_map : \n");
    scanf("%X",&read);
    advertising_channel_map = (UCHAR) read;
    printf("Enter advertising_filter_policy : \n");
    scanf("%X",&read);
    advertising_filter_policy = (UCHAR) read;
    retval = BT_hci_le_set_advertising_parameters
                  (
                       advertising_interval_min,
                       advertising_interval_max,
                       advertising_type,
                       own_addr_type,
                       direct_addr_type,
                      direct_addr,
                       advertising_channel_map,
                       advertising_filter_policy
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER */
void appl_hci_le_read_advertising_channel_tx_power( void )
{
    API_RESULT retval;
    retval = BT_hci_le_read_advertising_channel_tx_power();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_ADVERTISING_DATA */
void appl_hci_le_set_advertising_data( void )
{
    API_RESULT retval;
    UCHAR   advertising_data[31];
    UCHAR   advertising_data_len;
    UINT32   index;
    unsigned int read;

    BT_mem_set(advertising_data, 0, 31);

    printf("Enter advertising_data_len : \n");
    scanf("%X",&read);
    advertising_data_len = (UCHAR) read;
    printf("Enter advertising_data : \n");
    for(index = 0; index<advertising_data_len; index++)
    {
        scanf("%X",&read);
        advertising_data[index] = (UCHAR) read;
    }
    retval = BT_hci_le_set_advertising_data
                  (
                       advertising_data_len,
                      advertising_data
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_SCAN_RESPONSE_DATA */
void appl_hci_le_set_scan_response_data( void )
{
    API_RESULT retval;
    UCHAR   scan_data[31];
    UCHAR   scan_data_len;
    UINT32   index;
    unsigned int read;

    BT_mem_set(scan_data, 0, 31);

    printf("Enter scan_data_len : \n");
    scanf("%X",&read);
    scan_data_len = (UCHAR) read;
    printf("Enter scan_data : \n");
    for(index = 0; index<scan_data_len; index++)
    {
        scanf("%X",&read);
        scan_data[index] = (UCHAR) read;
    }
    retval = BT_hci_le_set_scan_response_data
                  (
                       scan_data_len,
                      scan_data
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_ADVERTISING_ENABLE */
void appl_hci_le_set_advertising_enable( void )
{
    API_RESULT retval;
    UCHAR   advertising_enable;
    unsigned int read;

    printf("Enter advertising_enable : \n");
    scanf("%X",&read);
    advertising_enable = (UCHAR) read;
    retval = BT_hci_le_set_advertising_enable
                  (
                       advertising_enable
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_SCAN_PARAMETERS */
void appl_hci_le_set_scan_parameters( void )
{
    API_RESULT retval;
    UCHAR   le_scan_type;
    UCHAR   own_addr_type;
    UCHAR   scan_filter_policy;
    UINT16   le_scan_interval;
    UINT16   le_scan_window;
    unsigned int read;

    printf("Enter le_scan_type : \n");
    scanf("%X",&read);
    le_scan_type = (UCHAR) read;
    printf("Enter le_scan_interval : \n");
    scanf("%X",&read);
    le_scan_interval = (UINT16) read;
    printf("Enter le_scan_window : \n");
    scanf("%X",&read);
    le_scan_window = (UINT16) read;
    printf("Enter own_addr_type : \n");
    scanf("%X",&read);
    own_addr_type = (UCHAR) read;
    printf("Enter scan_filter_policy : \n");
    scanf("%X",&read);
    scan_filter_policy = (UCHAR) read;
    retval = BT_hci_le_set_scan_parameters
                  (
                       le_scan_type,
                       le_scan_interval,
                       le_scan_window,
                       own_addr_type,
                       scan_filter_policy
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_SCAN_ENABLE */
void appl_hci_le_set_scan_enable( void )
{
    API_RESULT retval;
    UCHAR   filter_dups;
    UCHAR   le_scan_enable;
    unsigned int read;

    printf("Enter le_scan_enable : \n");
    scanf("%X",&read);
    le_scan_enable = (UCHAR) read;
    printf("Enter filter_dups : \n");
    scanf("%X",&read);
    filter_dups = (UCHAR) read;
    retval = BT_hci_le_set_scan_enable
                  (
                       le_scan_enable,
                       filter_dups
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_CREATE_CONNECTION */
void appl_hci_le_create_connection( void )
{
    API_RESULT retval;
    UCHAR   initiator_filter_policy;
    UCHAR   own_address_type;
    UCHAR   peer_address[6];
    UCHAR   peer_address_type;
    UINT16   conn_interval_max;
    UINT16   conn_interval_min;
    UINT16   conn_latency;
    UINT16   le_scan_interval;
    UINT16   le_scan_window;
    UINT16   maximum_ce_length;
    UINT16   minimum_ce_length;
    UINT16   supervision_timeout;
    UINT32   index;
    unsigned int read;

    printf("Enter le_scan_interval : \n");
    scanf("%X",&read);
    le_scan_interval = (UINT16) read;
    printf("Enter le_scan_window : \n");
    scanf("%X",&read);
    le_scan_window = (UINT16) read;
    printf("Enter initiator_filter_policy : \n");
    scanf("%X",&read);
    initiator_filter_policy = (UCHAR) read;
    printf("Enter peer_address_type : \n");
    scanf("%X",&read);
    peer_address_type = (UCHAR) read;
    printf("Enter peer_address : \n");
    for(index = 0; index<6; index++)
    {
        scanf("%X",&read);
        peer_address[index] = (UCHAR) read;
    }
    printf("Enter own_address_type : \n");
    scanf("%X",&read);
    own_address_type = (UCHAR) read;
    printf("Enter conn_interval_min : \n");
    scanf("%X",&read);
    conn_interval_min = (UINT16) read;
    printf("Enter conn_interval_max : \n");
    scanf("%X",&read);
    conn_interval_max = (UINT16) read;
    printf("Enter conn_latency : \n");
    scanf("%X",&read);
    conn_latency = (UINT16) read;
    printf("Enter supervision_timeout : \n");
    scanf("%X",&read);
    supervision_timeout = (UINT16) read;
    printf("Enter maximum_ce_length : \n");
    scanf("%X",&read);
    maximum_ce_length = (UINT16) read;
    printf("Enter minimum_ce_length : \n");
    scanf("%X",&read);
    minimum_ce_length = (UINT16) read;
    retval = BT_hci_le_create_connection
              (
                   le_scan_interval,
                   le_scan_window,
                   initiator_filter_policy,
                   peer_address_type,
                  peer_address,
                   own_address_type,
                   conn_interval_min,
                   conn_interval_max,
                   conn_latency,
                   supervision_timeout,
                   maximum_ce_length,
                   minimum_ce_length
              );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_CREATE_CONNECTION_CANCEL */
void appl_hci_le_create_connection_cancel( void )
{
    API_RESULT retval;
    retval = BT_hci_le_create_connection_cancel();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_WHITE_LIST_SIZE */
void appl_hci_le_read_white_list_size( void )
{
    API_RESULT retval;
    retval = BT_hci_le_read_white_list_size();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_CLEAR_WHITE_LIST */
void appl_hci_le_clear_white_list( void )
{
    API_RESULT retval;
    retval = BT_hci_le_clear_white_list();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_ADD_DEVICE_TO_WHITE_LIST */
void appl_hci_le_add_device_to_white_list( void )
{
    API_RESULT retval;
    UCHAR   addr[6];
    UCHAR   addr_type;
    UINT32   index;
    unsigned int read;

    printf("Enter addr_type : \n");
    scanf("%X",&read);
    addr_type = (UCHAR) read;
    printf("Enter addr : \n");
    for(index = 0; index<6; index++)
    {
        scanf("%X",&read);
        addr[index] = (UCHAR) read;
    }
    retval = BT_hci_le_add_device_to_white_list
                  (
                       addr_type,
                      addr
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST */
void appl_hci_le_remove_device_from_white_list( void )
{
    API_RESULT retval;
    UCHAR   addr[6];
    UCHAR   addr_type;
    UINT32   index;
    unsigned int read;

    printf("Enter addr_type : \n");
    scanf("%X",&read);
    addr_type = (UCHAR) read;
    printf("Enter addr : \n");
    for(index = 0; index<6; index++)
    {
        scanf("%X",&read);
        addr[index] = (UCHAR) read;
    }
    retval = BT_hci_le_remove_device_from_white_list
                  (
                       addr_type,
                      addr
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_CONNECTION_UPDATE */
void appl_hci_le_connection_update( void )
{
    API_RESULT retval;
    UINT16   conn_interval_max;
    UINT16   conn_interval_min;
    UINT16   conn_latency;
    UINT16   connection_handle;
    UINT16   maximum_ce_length;
    UINT16   minimum_ce_length;
    UINT16   supervision_timeout;
    unsigned int read;

    printf("Enter connection_handle : \n");
    scanf("%X",&read);
    connection_handle = (UINT16) read;
    printf("Enter conn_interval_min : \n");
    scanf("%X",&read);
    conn_interval_min = (UINT16) read;
    printf("Enter conn_interval_max : \n");
    scanf("%X",&read);
    conn_interval_max = (UINT16) read;
    printf("Enter conn_latency : \n");
    scanf("%X",&read);
    conn_latency = (UINT16) read;
    printf("Enter supervision_timeout : \n");
    scanf("%X",&read);
    supervision_timeout = (UINT16) read;
    printf("Enter minimum_ce_length : \n");
    scanf("%X",&read);
    minimum_ce_length = (UINT16) read;
    printf("Enter maximum_ce_length : \n");
    scanf("%X",&read);
    maximum_ce_length = (UINT16) read;
    retval = BT_hci_le_connection_update
                  (
                       connection_handle,
                       conn_interval_min,
                       conn_interval_max,
                       conn_latency,
                       supervision_timeout,
                       minimum_ce_length,
                       maximum_ce_length
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION */
void appl_hci_le_set_host_channel_classification( void )
{
    API_RESULT retval;
    UCHAR   le_channel_map[6];
    UINT32   index;
    unsigned int read;

    printf("Enter le_channel_map : \n");
    for(index = 0; index<6; index++)
    {
        scanf("%X",&read);
        le_channel_map[index] = (UCHAR) read;
    }
    retval = BT_hci_le_set_host_channel_classification
                  (
                      le_channel_map
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_CHANNEL_MAP */
void appl_hci_le_read_channel_map( void )
{
    API_RESULT retval;
    UINT16   connection_handle;
    unsigned int read;

    printf("Enter connection_handle : \n");
    scanf("%X",&read);
    connection_handle = (UINT16) read;
    retval = BT_hci_le_read_channel_map
                  (
                       connection_handle
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_REMOTE_USED_FEATURES */
void appl_hci_le_read_remote_used_features( void )
{
    API_RESULT retval;
    UINT16   connection_handle;
    unsigned int read;

    printf("Enter connection_handle : \n");
    scanf("%X",&read);
    connection_handle = (UINT16) read;
    retval = BT_hci_le_read_remote_used_features
                  (
                       connection_handle
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_ENCRYPT */
void appl_hci_le_encrypt( void )
{
    API_RESULT retval;
    UCHAR   key[16];
    UCHAR   plain_text[16];
    UINT32   index;
    unsigned int read;

    printf("Enter key : \n");
    for(index = 0; index<16; index++)
    {
        scanf("%X",&read);
        key[index] = (UCHAR) read;
    }
    printf("Enter plain_text : \n");
    for(index = 0; index<16; index++)
    {
        scanf("%X",&read);
        plain_text[index] = (UCHAR) read;
    }
    retval = BT_hci_le_encrypt
                  (
                      key,
                      plain_text,
                      NULL
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_RAND */
void appl_hci_le_rand( void )
{
    API_RESULT retval;
    retval = BT_hci_le_rand();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_START_ENCRYPTION */
void appl_hci_le_start_encryption( void )
{
    API_RESULT retval;
    UCHAR   long_term_key[16];
    UCHAR   random_number[8];
    UINT16   conn_handle;
    UINT16   encrypted_diversifier;
    UINT32   index;
    unsigned int read;

    printf("Enter conn_handle : \n");
    scanf("%X",&read);
    conn_handle = (UINT16) read;
    printf("Enter random_number : \n");
    for(index = 0; index<8; index++)
    {
        scanf("%X",&read);
        random_number[index] = (UCHAR) read;
    }
    printf("Enter encrypted_diversifier : \n");
    scanf("%X",&read);
    encrypted_diversifier = (UINT16) read;
    printf("Enter long_term_key : \n");
    for(index = 0; index<16; index++)
    {
        scanf("%X",&read);
        long_term_key[index] = (UCHAR) read;
    }
    retval = BT_hci_le_start_encryption
                  (
                       conn_handle,
                      random_number,
                       encrypted_diversifier,
                      long_term_key
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY */
void appl_hci_le_long_term_key_requested_reply( void )
{
    API_RESULT retval;
    UCHAR   long_term_key[16];
    UINT16   connection_handle;
    UINT32   index;
    unsigned int read;

    printf("Enter connection_handle : \n");
    scanf("%X",&read);
    connection_handle = (UINT16) read;
    printf("Enter long_term_key : \n");
    for(index = 0; index<16; index++)
    {
        scanf("%X",&read);
        long_term_key[index] = (UCHAR) read;
    }
    retval = BT_hci_le_long_term_key_requested_reply
                  (
                       connection_handle,
                      long_term_key
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY */
void appl_hci_le_long_term_key_requested_negative_reply( void )
{
    API_RESULT retval;
    UINT16   connection_handle;
    unsigned int read;

    printf("Enter connection_handle : \n");
    scanf("%X",&read);
    connection_handle = (UINT16) read;
    retval = BT_hci_le_long_term_key_requested_negative_reply
                  (
                       connection_handle
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_READ_SUPPORTED_STATES */
void appl_hci_le_read_supported_states( void )
{
    API_RESULT retval;
    retval = BT_hci_le_read_supported_states();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}


/* Send HCI_READ_TRANSMIT_POWER_LEVEL */
void appl_hci_read_transmit_power_level(void)
{
    unsigned int read;
    API_RESULT retval;
    UCHAR      power_type;
    UINT16     handle;

    printf("Read Transmit Power Leve. Enter Required Information ...\n");

    printf("Connection Handle (in Hex) = "); fflush(stdout);
    scanf("%x", &read);
    handle = (UINT16)read;

    printf("[Transmit Power Level: 0x00->Current, 0x01->Maximum]");
    printf("Power Type (in Hex) = "); fflush(stdout);

    scanf("%x", &read);
    power_type = (UCHAR)read;

    printf("\n");
    printf("Starting Reat Transmit Power Level ...\n"); fflush(stdout);
    retval = BT_hci_read_transmit_power_level (handle, power_type);
    if ( retval != API_SUCCESS )
    {
        printf("FAILED !! Error Code = 0x%04x\n", retval);
    }
    else
    {
        printf("Successfully started.\n");
    }
}

/* Send HCI_LE_RECEIVER_TEST_COMMAND */
void appl_hci_le_receiver_test_command( void )
{
    API_RESULT retval;
    UCHAR   rx_frequency;
    unsigned int read;

    printf("Enter rx_frequency : \n");
    scanf("%X",&read);
    rx_frequency = (UCHAR) read;
    retval = BT_hci_le_receiver_test_command
                  (
                       rx_frequency
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

/* Send HCI_LE_TRANSMITTER_TEST_COMMAND */
void appl_hci_le_transmitter_test_command( void )
{
#ifdef HCI_TESTING_COMMANDS
    API_RESULT retval;
    UCHAR   packet_payload;
    UCHAR   rx_frequency;
    UCHAR   test_data_length;
    unsigned int read;

    printf("Enter rx_frequency : \n");
    scanf("%X",&read);
    rx_frequency = (UCHAR) read;
    printf("Enter test_data_length : \n");
    scanf("%X",&read);
    test_data_length = (UCHAR) read;
    printf("Enter packet_payload : \n");
    scanf("%X",&read);
    packet_payload = (UCHAR) read;
    retval = BT_hci_le_transmitter_test_command
                  (
                       rx_frequency,
                       test_data_length,
                       packet_payload
                  );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
#else /* HCI_TESTING_COMMANDS */
    printf("Defien HCI_TESTING_COMMANDS for testing commands.\n");
#endif /* HCI_TESTING_COMMANDS */
}

/* Send HCI_LE_TEST_END */
void appl_hci_le_test_end( void )
{
#ifdef HCI_TESTING_COMMANDS
    API_RESULT retval;
    retval = BT_hci_le_test_end();

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
#else /* HCI_TESTING_COMMANDS */
    printf("Defien HCI_TESTING_COMMANDS for testing commands.\n");
#endif /* HCI_TESTING_COMMANDS */
}

void appl_send_data()
{
    API_RESULT retval;
    BT_DEVICE_ADDR   bd_addr;
    DEVICE_HANDLE    handle;
    UCHAR   data[100];
    unsigned int read;
    UINT16  len;
    UINT16  lcid;
    UINT32  index;

    printf("Enter Bluetooth address : \n");
    for(index = 0; index<BT_BD_ADDR_SIZE; index++)
    {
        scanf("%X",&read);
        BT_BD_ADDR(&bd_addr)[index] = (UCHAR) read;
    }

    printf("Enter bd_addr_type ");
    scanf("%X", &read);
    BT_BD_ADDR_TYPE(&bd_addr) = (UCHAR) read;

    printf("On which lcid? ");
    scanf("%X",&read);
    lcid = (UINT16) read;

    printf("Enter data length : ");
    scanf("%X",&read);
    len = (UINT16) read;

    printf("Enter data : \n");
    for(index = 0; index<len; index++)
    {
        scanf("%X",&read);
        data[index + L2CAP_HDR_LEN] = (UCHAR) read;
    }
    len += L2CAP_HDR_LEN;

    retval = device_queue_search_remote_addr (&handle,&bd_addr);

    retval = l2cap_fixed_channel_data_write
             (
                  &handle,
                  lcid,
                  data,
                  len
             );

    if(API_SUCCESS != retval)
    {
        printf("FAILED !!! Error code = %04X\n", retval);
    }
    else
    {
        printf("API returned success...\n");
    }
}

#endif /* BT_LE */

