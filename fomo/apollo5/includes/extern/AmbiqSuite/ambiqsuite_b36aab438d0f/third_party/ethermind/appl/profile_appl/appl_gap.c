
/**
 *  \file appl_gap.c
 *
 *  This file contains the ATT client application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_gap.h"

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
#ifdef ENABLE_BACI_INF
static void appl_gap_configure
            (
                UCHAR id,
                UCHAR role
            );
#endif /* ENABLE_BACI_INF */

/* --------------------------------------------- Functions */

#if ((defined APPL_GAP_BROADCASTER_SUPPORT) || (defined APPL_GAP_PERIPHERAL_SUPPORT))
API_RESULT appl_gap_start_advertising
     (
         /* IN */ UCHAR     adv_data_id,
         /* IN */ UCHAR     adv_param_id,
         /* IN */ UCHAR     adv_type
     )
{
    API_RESULT retval;
#ifdef ENABLE_BACI_INF
    BLE_CMD_SET_ADV_DATA cmd_adv_data;
#endif /* ENABLE_BACI_INF */

    retval = API_GAP_INCORRECT_STATE;

    /* If Advertising is already intiated, stop and restart */
    if (BT_FALSE != APPL_GAP_ADV_IS_STATE(APPL_GAP_ADV_ADVERTISING))
    {
        appl_gap_stop_advertising();
    }

    if (BT_FALSE != APPL_GAP_ADV_IS_STATE(APPL_GAP_ADV_IDLE))
    {
#ifndef ENABLE_BACI_INF
        retval = BT_hci_le_set_advertising_data
                 (
                      APPL_GAP_ACCESS_ADV_DATA_LEN(adv_data_id),
                      APPL_GAP_ACCESS_ADV_DATA(adv_data_id)
                 );

        if (API_SUCCESS == retval)
        {
            APPL_GAP_ADV_SET_STATE(APPL_GAP_ADV_UPDATING_DATA);
            APPL_GAP_SET_ADV_PARAM_ID(adv_param_id);
            APPL_GAP_SET_ADV_TYPE(adv_type);
        }
#else /* ENABLE_BACI_INF */

        /* Set Type */
        cmd_adv_data.type = 0x00;

        /* Set Advertising Data */
        cmd_adv_data.adv_data_len = APPL_GAP_ACCESS_ADV_DATA_LEN(adv_data_id);
        BT_mem_copy
        (
            cmd_adv_data.adv_data,
            APPL_GAP_ACCESS_ADV_DATA(adv_data_id),
            cmd_adv_data.adv_data_len
        );

        baci_transport_encode_and_send
        (
            BACI_PACKET_TYPE_HCI_COMMAND,
            M_BLE_CMD_SET_ADV_DATA,
            (UCHAR*) &cmd_adv_data,
            sizeof (cmd_adv_data)
        );

        appl_gap_configure (adv_data_id, 0x01);

        /* TODO: Update state */

#endif /* ENABLE_BACI_INF */
    }

    return retval;
}

API_RESULT appl_gap_stop_advertising (void)
{
#ifndef ENABLE_BACI_INF
    BT_hci_le_set_advertising_enable(0);
#else /* ENABLE_BACI_INF */
    BLE_CMD_SET_ADV_SCAN cmd_scan;

    /* 0 -> Advertise. 1 -> Scan */
    cmd_scan.cmd_mode = 0x00;
    cmd_scan.enable = 0x00;

    baci_transport_encode_and_send
    (
        BACI_PACKET_TYPE_HCI_COMMAND,
        M_BLE_CMD_SET_ADV_SCAN,
        (UCHAR*) &cmd_scan,
        sizeof (cmd_scan)
    );

#endif /* ENABLE_BACI_INF */

    APPL_GAP_ADV_SET_STATE(APPL_GAP_ADV_IDLE);

    return API_SUCCESS;
}
#endif /* APPL_GAP_BROADCASTER || APPL_GAP_PERIPHERAL_SUPPORT */


#if ((defined APPL_GAP_OBSERVER_SUPPORT) || (defined APPL_GAP_CENTRAL_SUPPORT))
API_RESULT appl_gap_start_scanning
           (
               /* IN */ UCHAR scan_param_id,
               /* IN */ UCHAR scan_type
           )
{
    API_RESULT retval;

    retval = API_GAP_INCORRECT_STATE;

    if (BT_FALSE != APPL_GAP_SCAN_IS_STATE(APPL_GAP_SCAN_SCANNING))
    {
        appl_gap_stop_scanning ();
    }

    if (BT_FALSE != APPL_GAP_SCAN_IS_STATE(APPL_GAP_SCAN_IDLE))
    {
        APPL_GAP_SET_SCAN_TYPE(scan_type);

#ifndef ENABLE_BACI_INF
        retval = BT_hci_le_set_scan_parameters
                 (
                     APPL_GAP_GET_SCAN_TYPE(),
                     APPL_GAP_GET_SCAN_INTERVAL(scan_param_id),
                     APPL_GAP_GET_SCAN_WINDOW(scan_param_id),
                     APPL_OWN_ADDR_TYPE(),
                     APPL_GAP_GET_SCAN_FILTER_POLICY(scan_param_id)
                 );
        if (API_SUCCESS == retval)
        {
            APPL_GAP_SCAN_SET_STATE(APPL_GAP_SCAN_UPDATING_PARAM);
        }
#else /* ENABLE_BACI_INF */
        appl_gap_configure (scan_param_id, 0x00);

        /* TODO: Update state */

#endif /* ENABLE_BACI_INF */
    }
    return retval;
}

API_RESULT appl_gap_stop_scanning (void)
{
#ifndef ENABLE_BACI_INF
    BT_hci_le_set_scan_enable(0, 1);
#else /* ENABLE_BACI_INF */
    BLE_CMD_SET_ADV_SCAN cmd_scan;

    /* 0 -> Advertise. 1 -> Scan */
    cmd_scan.cmd_mode = 0x01;
    cmd_scan.enable = 0x00;

    baci_transport_encode_and_send
    (
        BACI_PACKET_TYPE_HCI_COMMAND,
        M_BLE_CMD_SET_ADV_SCAN,
        (UCHAR*) &cmd_scan,
        sizeof (cmd_scan)
    );

#endif /* ENABLE_BACI_INF */

    APPL_GAP_SCAN_SET_STATE(APPL_GAP_SCAN_IDLE);

    return API_SUCCESS;
}
#endif /* APPL_GAP_OBSERVER_SUPPORT || APPL_GAP_CENTRAL_SUPPORT */

#ifdef APPL_GAP_CENTRAL_SUPPORT
API_RESULT appl_gap_initiate_connection
           (
               /* IN */ BT_DEVICE_ADDR * bd_addr,
               /* IN */ UCHAR            conn_param_id
           )
{
    API_RESULT retval;
#ifdef ENABLE_BACI_INF
    BLE_CMD_CONNECT cmd_connect;
#endif /* ENABLE_BACI_INF */

    retval = API_GAP_INCORRECT_STATE;

#ifndef ENABLE_BACI_INF
    retval = BT_hci_le_create_connection
             (
                 APPL_GAP_GET_CONN_SCAN_INTERVAL(conn_param_id),
                 APPL_GAP_GET_CONN_SCAN_WINDOW(conn_param_id),
                 APPL_GAP_GET_CONN_INITIATOR_FILTER_POLICY(conn_param_id),
                 bd_addr->type,
                 bd_addr->addr,
                 APPL_OWN_ADDR_TYPE(),
                 APPL_GAP_GET_CONN_INTERVAL_MIN(conn_param_id),
                 APPL_GAP_GET_CONN_INTERVAL_MAX(conn_param_id),
                 APPL_GAP_GET_CONN_LATENCY(conn_param_id),
                 APPL_GAP_GET_CONN_SUPERVISION_TIMEOUT(conn_param_id),
                 APPL_GAP_GET_CONN_MIN_CE_LENGTH(conn_param_id),
                 APPL_GAP_GET_CONN_MAX_CE_LENGTH(conn_param_id)
             );

    if (API_SUCCESS == retval)
#else /* ENABLE_BACI_INF */

    /* Get Connection parameters */
    cmd_connect.le_scan_interval =
        (UINT16) APPL_GAP_GET_CONN_SCAN_INTERVAL(conn_param_id);

    cmd_connect.le_scan_window =
        (UINT16) APPL_GAP_GET_CONN_SCAN_WINDOW(conn_param_id);

    cmd_connect.initiator_filter_policy =
        (UCHAR) APPL_GAP_GET_CONN_INITIATOR_FILTER_POLICY(conn_param_id);

    cmd_connect.peer_address_type = (UCHAR) bd_addr->type;

    BT_COPY_BD_ADDR(cmd_connect.peer_address, bd_addr->addr);

    cmd_connect.own_address_type =
        (UCHAR) APPL_CONFIG_CENTRAL_CONN_CONFIG_OWN_ADDR_TYPE();

    cmd_connect.conn_interval_min =
        (UINT16) APPL_GAP_GET_CONN_INTERVAL_MIN(conn_param_id);

    cmd_connect.conn_interval_max =
        (UINT16) APPL_GAP_GET_CONN_INTERVAL_MAX(conn_param_id);

    cmd_connect.conn_latency =
        (UINT16) APPL_GAP_GET_CONN_LATENCY(conn_param_id);

    cmd_connect.supervision_timeout =
        (UINT16) APPL_GAP_GET_CONN_SUPERVISION_TIMEOUT(conn_param_id);

    cmd_connect.maximum_ce_length =
        (UINT16) APPL_GAP_GET_CONN_MAX_CE_LENGTH(conn_param_id);

    cmd_connect.minimum_ce_length =
        (UINT16) APPL_GAP_GET_CONN_MIN_CE_LENGTH(conn_param_id);

    retval = baci_transport_encode_and_send
            (
                BACI_PACKET_TYPE_HCI_COMMAND,
                M_BLE_CMD_CONNECT,
                (UCHAR*) &cmd_connect,
                sizeof(cmd_connect)
            );

    if (BACI_TRANS_ERROR_NONE == retval)
#endif /* ENABLE_BACI_INF */

    {

    }
    return retval;
}

#ifdef ENABLE_BACI_INF
static void appl_gap_configure
            (
                UCHAR id,
                UCHAR role
            )
{
    UINT32 read;
    INT32 index;
    BLE_CMD_CONFIGURE cmd_config;

    cmd_config.role = role;

    /* If Peripheral, get advertisement parameter */
    if (0x01 == cmd_config.role)
    {
        /* appl_set_random_address(); */

        /* Get Advertising Parameters */
        cmd_config.advertising_interval_min =
            (UINT16) APPL_GAP_GET_ADV_MIN_INTERVAL(id);

        cmd_config.advertising_interval_max =
            (UINT16) APPL_GAP_GET_ADV_MAX_INTERVAL(id);

        cmd_config.advertising_type =
            (UCHAR) APPL_GAP_GET_ADV_TYPE();

        cmd_config.adv_own_addr_type =
            (UCHAR) APPL_OWN_ADDR_TYPE();

        cmd_config.direct_addr_type =
            (UCHAR) APPL_DIRECT_ADDR_TYPE();

        /* Setting dummy BD_ADDR */
        /* APPL_CONFIG_PEER_DEVICE_ADDR() */
        BT_mem_set (cmd_config.direct_addr, 0x00, BT_BD_ADDR_SIZE);

        cmd_config.advertising_channel_map =
            (UCHAR) APPL_GAP_GET_ADV_CHANNEL_MAP(id);

        cmd_config.advertising_filter_policy =
            (UCHAR) APPL_GAP_GET_ADV_FILTER_POLICY(id);
    }
    else
    {
        /* Set Scan Parameters */
        cmd_config.le_scan_type =
            (UINT16) APPL_GAP_GET_SCAN_TYPE();

        cmd_config.le_scan_interval =
            (UINT16) APPL_GAP_GET_SCAN_INTERVAL(id);

        cmd_config.le_scan_window =
            (UCHAR) APPL_GAP_GET_SCAN_WINDOW(id);

        cmd_config.scan_own_addr_type =
            (UCHAR) APPL_CONFIG_CENTRAL_UNBONDED_SCAN_OWN_ADDR_TYPE();

        cmd_config.scan_filter_policy =
            (UCHAR) APPL_GAP_GET_SCAN_FILTER_POLICY(id);
    }

    baci_transport_encode_and_send
    (
        BACI_PACKET_TYPE_HCI_COMMAND,
        M_BLE_CMD_CONFIGURE,
        (UCHAR*) &cmd_config,
        sizeof (cmd_config)
    );
}
#endif /* ENABLE_BACI_INF */

#endif /* APPL_GAP_CENTRAL_SUPPORT */
