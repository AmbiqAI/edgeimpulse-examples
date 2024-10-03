
/**
 *  \file appl_hci.c
 *
 *  Source File for HCI Command Line Application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "appl_hci.h"
#include "appl_fsm_defines.h"

#ifdef BT_VENDOR_SPECIFIC_INIT
#include "vendor_specific_init.h"
#endif /* BT_VENDOR_SPECIFIC_INIT */

/* #define APPL_INITIATE_AUTHENTICATION */
#define APPL_AUTO_SCAN_ENABLE
#define APPL_AUTO_ADVERTISE
/* #define APPL_AUTO_DISCOVERY_ON_CONNECTION */

INT8 appl_tx_power_level = 0;

/**
 * TODO: Check why build error with inclusion of appl_att_client.h, despite path
 * being included in Inclusion path.
 */
void appl_att_initiate_service_discovery (void);

void appl_server_reinitialize (void);

/* ----------------------------------------- External Global Variables */


/* ----------------------------------------- Exported Global Variables */
BT_DEVICE_ADDR g_bd_addr;

/* ----------------------------------------- Static Global Variables */
DECL_STATIC const APPL_HCI_OPCODE_NAME opcode_name_table[] =
{
    /* No Operation Opcode */
    {0x0000, "HCI_NO_OPERATION_OPCODE"                                 },

    /* Link Control Commands - OGF : 0x01 */
    {0x0406, "HCI_DISCONNECT_OPCODE"                                   },
    {0x041D, "HCI_READ_REMOTE_VERSION_INFORMATION_OPCODE"              },

    /* Link Policy Commands - OGF : 0x02 */

    /* Host Controller & BaseBand Commands - OGF : 0x03 */
    {0x0C01, "HCI_SET_EVENT_MASK_OPCODE"                               },
    {0x0C03, "HCI_RESET_OPCODE"                                        },
    {0x0C2D, "HCI_READ_TRANSMIT_POWER_LEVEL_OPCODE"                    },
    {0x0C31, "HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL_OPCODE"     },
    {0x0C33, "HCI_HOST_BUFFER_SIZE_OPCODE"                             },
    {0x0C35, "HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_OPCODE"             },

    /* Informational Parameters - OGF : 0x04 */
    {0x1001, "HCI_READ_LOCAL_VERSION_INFORMATION_OPCODE"               },
    {0x1002, "HCI_READ_LOCAL_SUPPORTED_COMMANDS_OPCODE"                },
    {0x1003, "HCI_READ_LOCAL_SUPPORTED_FEATURES_OPCODE"                },
    {0x1005, "HCI_READ_BUFFER_SIZE_OPCODE"                             },
    {0x1007, "HCI_READ_COUNTRY_CODE_OPCODE"                            },
    {0x1009, "HCI_READ_BD_ADDR_OPCODE"                                 },

    /* Status Parameters - OGF : 0x05 */
    {0x1405, "HCI_READ_RSSI_OPCODE"                                    },

    /* Testing Commands - OGF : 0x06 */

    /* Bluetooth Low Energy Commands */
#ifdef BT_LE
    {0x2001, "HCI_LE_SET_EVENT_MASK_OPCODE"                            },
    {0x2002, "HCI_LE_READ_BUFFER_SIZE_OPCODE"                          },
    {0x2003, "HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_OPCODE"             },
    {0x2005, "HCI_LE_SET_RANDOM_ADDRESS_OPCODE"                        },
    {0x2006, "HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE"                },
    {0x2007, "HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_OPCODE"         },
    {0x2008, "HCI_LE_SET_ADVERTISING_DATA_OPCODE"                      },
    {0x2009, "HCI_LE_SET_SCAN_RESPONSE_DATA_OPCODE"                    },
    {0x200A, "HCI_LE_SET_ADVERTISING_ENABLE_OPCODE"                    },
    {0x200B, "HCI_LE_SET_SCAN_PARAMETERS_OPCODE"                       },
    {0x200C, "HCI_LE_SET_SCAN_ENABLE_OPCODE"                           },
    {0x200D, "HCI_LE_CREATE_CONNECTION_OPCODE"                         },
    {0x200E, "HCI_LE_CREATE_CONNECTION_CANCEL_OPCODE"                  },
    {0x200F, "HCI_LE_READ_WHITE_LIST_SIZE_OPCODE"                      },
    {0x2010, "HCI_LE_CLEAR_WHITE_LIST_OPCODE"                          },
    {0x2011, "HCI_LE_ADD_DEVICE_TO_WHITE_LIST_OPCODE"                  },
    {0x2012, "HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_OPCODE"             },
    {0x2013, "HCI_LE_CONNECTION_UPDATE_OPCODE"                         },
    {0x2014, "HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_OPCODE"           },
    {0x2015, "HCI_LE_READ_CHANNEL_CLASSIFICATION_OPCODE"               },
    {0x2016, "HCI_LE_READ_REMOTE_USED_FEATURES_OPCODE"                 },
    {0x2017, "HCI_LE_ENCRYPT_OPCODE"                                   },
    {0x2018, "HCI_LE_RAND_OPCODE"                                      },
    {0x2019, "HCI_LE_START_ENCRYPTION_OPCODE"                          },
    {0x201A, "HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_OPCODE"             },
    {0x201B, "HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_OPCODE"    },
    {0x201C, "HCI_LE_READ_SUPPORTED_STATES_OPCODE"                     },
    {0x201D, "HCI_LE_RECEIVER_TEST_COMMAND_OPCODE"                     },
    {0x201E, "HCI_LE_TRANSMITTER_TEST_COMMAND_OPCODE"                  },
    {0x201F, "HCI_LE_TEST_END_OPCODE"                                  }
#endif /* BT_LE */
};

/* ----------------------------------------- Functions */

API_RESULT appl_hci_event_indication_callback
           (
               UCHAR  event_type,
               UCHAR  * event_data,
               UCHAR  event_datalen
           )
{
    APPL_EVENT_PARAM    fsm_param;
    UINT32              count;

#ifdef APPL_GAP_CENTRAL_SUPPORT
    API_RESULT          retval;
#endif /* APPL_GAP_CENTRAL_SUPPORT */

    UINT16 connection_handle, value_2;
    UCHAR  status, value_1;

#ifdef  BT_LE
    UCHAR    sub_event_code;
    UCHAR    num_reports;
    UCHAR    length_data;
    UCHAR    peer_addr_type;
#ifndef DONT_USE_STANDARD_IO
    UCHAR    clock_accuracy;
    UCHAR    role;
    UCHAR    rssi;
    UINT16   conn_latency;
    UINT16   conn_interval;
    UINT16   supervision_timeout;
#endif /* DONT_USE_STANDARD_IO */
    UCHAR  * data_param;
    UCHAR  * le_feature;
    UCHAR  * peer_addr;
#ifdef HCI_LE_PRIVACY_1_2_SUPPORT
    UCHAR  * local_rpa_addr, * peer_rpa_addr, * peer_id_addr;
    UCHAR  offset_len;
#endif /* HCI_LE_PRIVACY_1_2_SUPPORT */
    UCHAR  * random_number;
    UINT16   encripted_diversifier;
    BT_DEVICE_ADDR bd_addr;
#endif /* BT_LE */

    if (HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT == event_type)
    {
        return API_SUCCESS;
    }

    APPL_EVENT_PARAM_INIT(&fsm_param);

    APPL_TRC("\n");

    /* Switch on the Event Code */
    switch (event_type)
    {
    case HCI_DISCONNECTION_COMPLETE_EVENT:
        APPL_TRC("Received HCI_DISCONNECTION_COMPLETE_EVENT.\n");

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        APPL_TRC("\tStatus: 0x%02X\n", status);
        event_data += 1;

        /* Connection Handle */
        hci_unpack_2_byte_param(&connection_handle, event_data);
        APPL_TRC("\tConnection Handle: 0x%04X\n", connection_handle);
        event_data += 2;

        /* Reason */
        hci_unpack_1_byte_param(&value_1, event_data);
        APPL_TRC("\tReason: 0x%02X\n", value_1);
        event_data += 1;

#ifdef APPL_MENU_OPS
        CONSOLE_OUT("\nDIS-CONN:\r\n"
        "* Conn Hndl        - 0x%04X\r\n"
        "* Reason           - 0x%02X\r\n\n",
        connection_handle,
        value_1);
#endif /* APPL_MENU_OPS */

        /* Get Device Handle of peer which is now disconnected */
        /* Disconnection is accompanied with reason for disconnection as event param */
        fsm_param.data_param = &value_1;
        appl_get_handle_from_cxn_handle(connection_handle,&fsm_param.handle);

        /* Post Disconnection Event */
        fsm_post_event (APPL_FSM_ID,ev_appl_transport_disconnected_ind,&fsm_param);
        appl_delete_device (connection_handle);
        break;

    case HCI_ENCRYPTION_CHANGE_EVENT:
        APPL_TRC("Received HCI_ENCRYPTION_CHANGE_EVENT.\n");

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        APPL_TRC("\tStatus: 0x%02X\n", status);
        event_data += 1;

        /* Connection Handle */
        hci_unpack_2_byte_param(&connection_handle, event_data);
        APPL_TRC("\tConnection Handle: 0x%04X\n", connection_handle);
        event_data += 2;

        /* Encryption Enable */
        hci_unpack_1_byte_param(&value_1, event_data);
        APPL_TRC("\tEcnryption Enable: 0x%02X", value_1);
        switch (value_1)
        {
        case 0x00:
            APPL_TRC(" -> Encryption OFF\n");
            break;
        case 0x01:
            APPL_TRC(" -> Encryption ON\n");
            break;
        default:
            APPL_TRC(" -> ???\n");
            break;
        }
        event_data += 1;

        break;

    case HCI_COMMAND_COMPLETE_EVENT:
        APPL_TRC("Received HCI_COMMAND_COMPLETE_EVENT.\n");

        /* Number of Command Packets */
        hci_unpack_1_byte_param(&value_1, event_data);
        APPL_TRC("\tNum Command Packets: %d (0x%02X)\n", value_1, value_1);
        event_data += 1;

        /* Command Opcode */
        hci_unpack_2_byte_param(&value_2, event_data);
        APPL_TRC("\tCommand Opcode: 0x%04X -> %s\n",
        value_2, appl_hci_get_command_name(value_2));
        event_data += 2;

        /* Command Status */
        hci_unpack_1_byte_param(&status, event_data);
        APPL_TRC("\tCommand Status: 0x%02X\n", status);
        event_data += 1;

#ifdef BT_LE
        /* Check for HCI_LE_SET_ADVERTISING_DATA_OPCODE */
        if (HCI_LE_SET_ADVERTISING_DATA_OPCODE == value_2)
        {
            APPL_TRC("Set Advertising Parameters.\n");
            APPL_TRC("Status = 0x%02X\n", status);

            if (API_SUCCESS == status)
            {
#ifdef APPL_AUTO_ADVERTISE
#if ((defined APPL_GAP_BROADCASTER_SUPPORT) || (defined APPL_GAP_PERIPHERAL_SUPPORT))
                appl_gap_set_adv_data_complete (APPL_GAP_GET_ADV_PARAM_ID);
#endif /* APPL_GAP_BROADCASTER_SUPPORT || APPL_GAP_PERIPHERAL_SUPPORT */
#endif /* APPL_AUTO_ADVERTISE */
            }
        }
        /* Check for HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE */
        else if (HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE == value_2)
        {
            APPL_TRC("Enabling Advertising\n");
            APPL_TRC("Status = 0x%02X\n", status);

            if (API_SUCCESS == status)
            {
#ifdef APPL_AUTO_ADVERTISE
#if ((defined APPL_GAP_BROADCASTER_SUPPORT) || (defined APPL_GAP_PERIPHERAL_SUPPORT))
                appl_gap_set_adv_param_complete ();
#endif /* APPL_GAP_BROADCASTER_SUPPORT || APPL_GAP_PERIPHERAL_SUPPORT */
#endif /* APPL_AUTO_ADVERTISE */
            }
        }
        else if (HCI_LE_SET_ADVERTISING_ENABLE_OPCODE == value_2)
        {
            if (API_SUCCESS == status)
            {
                APPL_TRC("Enabled Advertising...\n");
                APPL_TRC("EtherMind Server is now Ready\n");
            }
        }
        /* Check for HCI_LE_SET_SCAN_PARAMETERS_OPCODE */
        else if (HCI_LE_SET_SCAN_PARAMETERS_OPCODE == value_2)
        {
            APPL_TRC("Set Scan Parameters.\n");
            APPL_TRC("Status = 0x%02X\n", status);

            if (API_SUCCESS == status)
            {
#ifdef APPL_AUTO_SCAN_ENABLE
#if ((defined APPL_GAP_OBSERVER_SUPPORT) || (defined APPL_GAP_CENTRAL_SUPPORT))

#ifndef OTC
                /*
                 * Currently, Calling Scan Enable only for Profiles other than
                 * OTC.
                 * For OTC, Scan Enable Disable can be done from MENU.
                 * TODO:
                 * Have Cleaner Interface for handling GAP events based on
                 * Profiles.
                 */
                appl_gap_set_scan_param_complete();
#endif /* OTC */
#endif /* APPL_GAP_OBSERVER_SUPPORT || APPL_GAP_CENTRAL_SUPPORT */
#endif /* APPL_AUTO_SCAN_ENABLE */
            }
        }
        else if (HCI_LE_SET_SCAN_ENABLE_OPCODE == value_2)
        {
            if (API_SUCCESS == status)
            {
                APPL_TRC("Enabled Scanning ...\n");
            }
        }

#ifdef SMP_PL_ENABLE_KEY_GENERATION
        else if (HCI_LE_RAND_OPCODE == value_2)
        {
            smp_rand_complete_pl (event_data, (event_datalen - 4));
        }
        else if (HCI_LE_ENCRYPT_OPCODE == value_2)
        {
            smp_encryption_complete_pl (event_data, (event_datalen - 4));
        }
#endif /* SMP_PL_ENABLE_KEY_GENERATION */

        /* Tx Power Level for Proximity */
#if defined(PXR) || defined (CUSTOM_PROFILE)
        else if (HCI_READ_TRANSMIT_POWER_LEVEL_OPCODE == value_2)
        {
            /* Extract Connection Handle */
            hci_unpack_2_byte_param(&connection_handle, event_data);
            event_data += 2;

            /* Extract Tx Power Level */
            hci_unpack_1_byte_param(&value_1, event_data);

            appl_pxr_get_tx_power_level_complete
            (
                status,
                connection_handle,
                value_1
            );

            /**
             * Re-adjusting the 'event_data' to print
             * return parameters below correctly.
             */
            event_data -= 2;
        }
#endif /* PXR */
        appl_tx_power_level = value_1;

#endif /* BT_LE */

        /* Command Return Parameters */
        if (event_datalen > 4)
        {
            APPL_TRC("\tReturn Parameters: ");
            for (count = 4; count < event_datalen; count ++)
            {
                APPL_TRC("%02X ", *event_data);
                event_data += 1;
            }
            APPL_TRC("\n");
        }

    #ifdef BT_VENDOR_SPECIFIC_INIT
        app_vendor_specific_init(NULL);
    #endif /* BT_VENDOR_SPECIFIC_INIT */
        break;

    case HCI_COMMAND_STATUS_EVENT:
        APPL_TRC("Received HCI_COMMAND_STATUS_EVENT.\n");

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        APPL_TRC("\tCommand Status: 0x%02X\n", status);
        event_data += 1;

        /* Number of Command Packets */
        hci_unpack_1_byte_param(&value_1, event_data);
        APPL_TRC("\tNum Command Packets: %d (0x%02X)\n", value_1, value_1);
        event_data += 1;

        /* Command Opcode */
        hci_unpack_2_byte_param(&value_2, event_data);
        APPL_TRC("\tCommand Opcode: 0x%04X (%s)\n",
        value_2, appl_hci_get_command_name(value_2));
        event_data += 2;

        break;

    case HCI_HARDWARE_ERROR_EVENT:
        APPL_TRC("Received HCI_HARDWARE_ERROR_EVENT.\n");

        /* Hardware Code */
        hci_unpack_1_byte_param(&value_1, event_data);
        APPL_TRC("\tHardware Code: 0x%02X\n", value_1);
        event_data += 1;

        break;

#if 0
    case HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT:
        APPL_TRC("Received HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT.\n");
        break;
#endif /* 0 */

    case HCI_DATA_BUFFER_OVERFLOW_EVENT:
        APPL_TRC("Received HCI_DATA_BUFFER_OVERFLOW_EVENT.\n");

        /* Link Type */
        hci_unpack_1_byte_param(&value_1, event_data);
        APPL_TRC("\tLink Type: 0x%02X", value_1);
        switch (value_1)
        {
        case 0x00:
            APPL_TRC(" -> Synchronous Buffer Overflow\n");
            break;
        case 0x01:
            APPL_TRC(" -> ACL Buffer Overflow\n");
            break;
        default:
            APPL_TRC(" -> ???\n");
            break;
        }
        event_data += 1;

        break;

    case HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT:
        APPL_TRC ("Received HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT\n");

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        APPL_TRC("Status: 0x%02X\n", status);
        event_data ++;

        /* Connection Handle */
        hci_unpack_2_byte_param(&value_2, event_data);
        APPL_TRC("\tConnection Handle: 0x%04X\n", value_2);

        break;

#ifdef BT_LE
    case HCI_LE_META_EVENT:
        APPL_TRC("Receaved HCI_LE_META_EVENT.\n");
        hci_unpack_1_byte_param(&sub_event_code, event_data);
        event_data = event_data + 1;
        switch(sub_event_code)
        {
     case HCI_LE_CONNECTION_COMPLETE_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_CONNECTION_COMPLETE_SUBEVENT.\n");
            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
#ifndef DONT_USE_STANDARD_IO
            hci_unpack_1_byte_param(&role, event_data +3);
#endif /* DONT_USE_STANDARD_IO */
            hci_unpack_1_byte_param(&peer_addr_type, event_data + 4);
            peer_addr = 5 + event_data;
#ifndef DONT_USE_STANDARD_IO
            hci_unpack_2_byte_param(&conn_interval, event_data+11);
            hci_unpack_2_byte_param(&conn_latency, event_data+13);
            hci_unpack_2_byte_param(&supervision_timeout, event_data+15);
            hci_unpack_1_byte_param(&clock_accuracy, event_data + 17);
#endif /* DONT_USE_STANDARD_IO */

            /* Print the parameters */
            APPL_TRC("status = 0x%02X\n",status);
            APPL_TRC("connection_handle = 0x%04X\n",connection_handle);
#ifndef DONT_USE_STANDARD_IO
            APPL_TRC("role = 0x%02X\n",role);
            APPL_TRC("peer_addr_type = 0x%02X\n",peer_addr_type);
            APPL_TRC("peer_addr = \n");
            appl_dump_bytes(peer_addr, 6);
            APPL_TRC("conn_interval = 0x%04X\n",conn_interval);
            APPL_TRC("conn_latency = 0x%04X\n",conn_latency);
            APPL_TRC("supervision_timeout = 0x%04X\n",supervision_timeout);
            APPL_TRC("clock_accuracy = 0x%02X\n",clock_accuracy);
#endif /*  DONT_USE_STANDARD_IO */
            appl_handle_connection_complete
            (
                peer_addr,
                peer_addr_type,
                connection_handle
            );
            break;
#ifdef HCI_LE_PRIVACY_1_2_SUPPORT
     case HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT.\n");
            offset_len = 0;
            hci_unpack_1_byte_param(&status, event_data + offset_len);
            offset_len += 1;

            hci_unpack_2_byte_param(&connection_handle, event_data + offset_len);
            offset_len += 2;

#ifndef DONT_USE_STANDARD_IO
            hci_unpack_1_byte_param(&role, event_data + offset_len);
#endif /* DONT_USE_STANDARD_IO */
            offset_len += 1;

            hci_unpack_1_byte_param(&peer_addr_type, event_data + offset_len);
            offset_len += 1;
            /* Check the Identity Address Type */
            peer_addr_type = ((0x01 == peer_addr_type) || (0x03 == peer_addr_type)) ? \
                BT_BD_RANDOM_ADDRESS_TYPE : BT_BD_PUBLIC_ADDRESS_TYPE;

            peer_id_addr = event_data + offset_len;
            offset_len += 6;

            local_rpa_addr = event_data + offset_len;
            offset_len += 6;

            peer_rpa_addr  = event_data + offset_len;
            offset_len += 6;

            /**
             * If incoming peer rpa address is valid i.e. Non zero, then use that address as peer address.
             * Else, the peer identity address is the original address used by the Peer Device.
             */
            if (BT_BD_ADDR_IS_NON_ZERO(peer_rpa_addr))
            {
                peer_addr      = peer_rpa_addr;
                peer_addr_type = BT_BD_RANDOM_ADDRESS_TYPE;
            }
            else
            {
                peer_addr = peer_id_addr;
            }

#ifndef DONT_USE_STANDARD_IO
            hci_unpack_2_byte_param(&conn_interval, event_data + offset_len);
            offset_len += 2;
            hci_unpack_2_byte_param(&conn_latency, event_data + offset_len);
            offset_len += 2;
            hci_unpack_2_byte_param(&supervision_timeout, event_data + offset_len);
            offset_len += 2;
            hci_unpack_1_byte_param(&clock_accuracy, event_data + offset_len);
            offset_len += 1;
#endif /* DONT_USE_STANDARD_IO */

            /* Print the parameters */
            APPL_TRC("status = 0x%02X\n",status);
            APPL_TRC("connection_handle = 0x%04X\n",connection_handle);
#ifndef DONT_USE_STANDARD_IO
            APPL_TRC("role = 0x%02X\n",role);
            APPL_TRC("peer_addr_type = 0x%02X\n",peer_addr_type);
            APPL_TRC("peer_addr = \n");
            appl_dump_bytes(peer_addr, 6);
            APPL_TRC("peer_id_addr_type = 0x%02X\n", peer_addr_type);
            APPL_TRC("peer_id_addr = \n");
            appl_dump_bytes(peer_id_addr, 6);
            APPL_TRC("local_rpa_addr_type = 0x%02X\n", BT_BD_RANDOM_ADDRESS_TYPE);
            APPL_TRC("local_rpa_addr = \n");
            appl_dump_bytes(local_rpa_addr, 6);
            APPL_TRC("peer_rpa_addr_type = 0x%02X\n", BT_BD_RANDOM_ADDRESS_TYPE);
            APPL_TRC("peer_rpa_addr = \n");
            appl_dump_bytes(peer_rpa_addr, 6);
            APPL_TRC("conn_interval = 0x%04X\n",conn_interval);
            APPL_TRC("conn_latency = 0x%04X\n",conn_latency);
            APPL_TRC("supervision_timeout = 0x%04X\n",supervision_timeout);
            APPL_TRC("clock_accuracy = 0x%02X\n",clock_accuracy);
#else
            (void)local_rpa_addr;
#endif /*  DONT_USE_STANDARD_IO */

            appl_handle_connection_complete
            (
                peer_addr,
                peer_addr_type,
                connection_handle
            );
            break;
#endif /* HCI_LE_PRIVACY_1_2_SUPPORT */

        case HCI_LE_ADVERTISING_REPORT_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_ADVERTISING_REPORT_SUBEVENT.\n");
            /* Number of Responses */
            hci_unpack_1_byte_param(&num_reports, event_data);
            APPL_TRC("num_reports = 0x%02X\n",num_reports);
            event_data += 1;

            /* For each Response, Print the Inquiry Result */
            for (count = 0; count < num_reports; count ++)
            {
                hci_unpack_1_byte_param(&event_type, event_data);
                event_data += 1;

                hci_unpack_1_byte_param(&bd_addr.type, event_data);
                event_data += 1;

                BT_COPY_BD_ADDR
                (
                   bd_addr.addr,
                   event_data
                );

                event_data += BT_BD_ADDR_SIZE;
                hci_unpack_1_byte_param(&length_data, event_data);
                event_data += 1;
                data_param = event_data;
                event_data += length_data;
#ifndef DONT_USE_STANDARD_IO
                hci_unpack_1_byte_param(&rssi, event_data);
#endif /* DONT_USE_STANDARD_IO */
                event_data += 1;

                /* Print the parameters */
                APPL_TRC("event_type = 0x%02X\n",event_type);
                APPL_TRC("address_type = 0x%02X\n",bd_addr.type);
                APPL_TRC("address = \n");
                appl_dump_bytes(bd_addr.addr, BT_BD_ADDR_SIZE);
                APPL_TRC("length_data = 0x%02X\n",length_data);
                APPL_TRC("data = \n");
                appl_dump_bytes(data_param, length_data);
#ifndef DONT_USE_STANDARD_IO
                APPL_TRC("rssi = 0x%02X\n",rssi);
#endif /* DONT_USE_STANDARD_IO */
            }
#ifdef APPL_GAP_CENTRAL_SUPPORT

            fsm_param.data_param = (void *) &bd_addr;
            fsm_param.data_len = sizeof (bd_addr);

            if (0 != BT_mem_cmp(APPL_CONFIG_PEER_DEVICE_ADDR(),\
                                bd_addr.addr, BT_BD_ADDR_SIZE))
            {
                break;
            }

            /** Post Transport Connect request to the application fsm */
            retval = fsm_post_event
                     (
                         APPL_FSM_ID,
                         ev_appl_transport_connect_req,
                         (void *) &fsm_param
                     );

            APPL_TRC (
            "[APPL]: FSM Post Event Result for Transport Connect Request = 0x%04X",
            retval);

#endif /* APPL_GAP_CENTRAL_SUPPORT */
            break;

        case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT.\n");
            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
#ifndef DONT_USE_STANDARD_IO
            hci_unpack_2_byte_param(&conn_interval, event_data+3);
            hci_unpack_2_byte_param(&conn_latency, event_data+5);
            hci_unpack_2_byte_param(&supervision_timeout, event_data+7);
#endif /* DONT_USE_STANDARD_IO */

            /* Print the parameters */
            APPL_TRC("status = 0x%02X\n",status);
            APPL_TRC("connection_handle = 0x%04X\n",connection_handle);
#ifndef DONT_USE_STANDARD_IO
            APPL_TRC("conn_interval = 0x%04X\n",conn_interval);
            APPL_TRC("conn_latency = 0x%04X\n",conn_latency);
            APPL_TRC("supervision_timeout = 0x%04X\n",supervision_timeout);
#endif /* DONT_USE_STANDARD_IO */
            break;

        case HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT:
            APPL_TRC
            ("Subevent: HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT.\n");

            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
            le_feature = 3 + event_data;

            /* Print the parameters */
            APPL_TRC("status = 0x%02X\n",status);
            APPL_TRC("connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC("le_feature = \n");
            appl_dump_bytes(le_feature, 8);
            break;

        case HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT.\n");
            hci_unpack_2_byte_param(&connection_handle, event_data + 0);
            random_number = 2 + event_data;
            hci_unpack_2_byte_param(&encripted_diversifier, event_data+10);

            /* Print the parameters */
            APPL_TRC("connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC("random_number = \n");
            appl_dump_bytes(random_number, 8);
            APPL_TRC("encripted_diversifier = 0x%04X\n",encripted_diversifier);
            break;

        case HCI_LE_REMOTE_CONN_PARAM_REQ_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_REMOTE_CONN_PARAM_REQ_SUBEVENT.\n");

            appl_handle_conn_param_req(event_data);
            break;

        default:
            APPL_TRC("Unknown LE SubEvent Code 0x%02X Received.\n",
                sub_event_code);
            break;
        }
        break;
#endif /* BT_LE */

    default:
        APPL_TRC("Unknown Event Code 0x%02X Received.\n", event_type);
        break;
    }

    return API_SUCCESS;
}

void appl_handle_connection_complete
     (
         UCHAR   * peer_addr,
         UCHAR   peer_addr_type,
         UINT16  connection_handle
     )
{
    /* Local Varaibles */
    APPL_EVENT_PARAM    fsm_param;
    BT_DEVICE_ADDR      device_addr;
    DEVICE_HANDLE dq_handle;
    API_RESULT retval;

#ifdef SMP_HAVE_OOB_SUPPORT
#ifndef SMP_LESC
    UCHAR key[SMP_OOB_TEMP_KEY_SIZE] =
          {
              0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
              0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
          };
#endif /* SMP_LESC */
#endif /* SMP_HAVE_OOB_SUPPORT */

    APPL_EVENT_PARAM_INIT(&fsm_param);
    retval = API_FAILURE;

    /* Copy the incoming device address */
    BT_COPY_BD_ADDR(device_addr.addr,peer_addr);
    BT_COPY_TYPE(device_addr.type,peer_addr_type);

    /* Copy the Peer Address to global */
    BT_COPY_BD_ADDR_AND_TYPE(&g_bd_addr,&device_addr);

    /* Search for the remote address */
    retval = device_queue_search_remote_addr
             (
                  &dq_handle,
                  &device_addr
             );

    if (API_SUCCESS == retval)
    {
#ifdef SMP_HAVE_OOB_SUPPORT
        BT_smp_add_device_pl(&device_addr);
#ifndef SMP_LESC
        BT_smp_set_oob_data_pl (&device_addr, 1, key, NULL, 0);
#endif /* SMP_LESC */
#endif /* SMP_HAVE_OOB_SUPPORT */
        appl_set_cxn_handle
        (
             connection_handle,
             dq_handle,
             &fsm_param.handle
         );

#ifdef APPL_MENU_OPS
        CONSOLE_OUT("\nCONN:\r\n"
                    "* BD Addr[MSB-LSB] - 0x%02X%02X%02X%02X%02X%02X : %02X \r\n"
                    "* DQ Hndl          - 0x%02X\r\n"
                    "* Conn Hndl        - 0x%04X\r\n\n",
                    peer_addr[5],
                    peer_addr[4],
                    peer_addr[3],
                    peer_addr[2],
                    peer_addr[1],
                    peer_addr[0],
                    peer_addr_type,
                    dq_handle,
                    connection_handle);
#endif /* APPL_MENU_OPS */
    }
    else
    {
        /*
         * Connection but not available with stack device queue.
         * Assign an invalid handle, and wait for stack to disconnect.
         */
        dq_handle = 0xFF;

        appl_set_cxn_handle
        (
             connection_handle,
             dq_handle,
             &fsm_param.handle
        );

        /* Post Connection Event */
        fsm_post_event
        (
             APPL_FSM_ID,
             ev_appl_transport_connected_ind,
             (void *)(&fsm_param)
        );
    }

#ifdef SMP_PL_ENABLE_KEY_GENERATION
    BT_smp_generate_keys_pl (0x01);
#endif /* SMP_PL_ENABLE_KEY_GENERATION */

#ifdef APPL_ATT_CLIENT
#ifdef APPL_AUTO_DISCOVERY_ON_CONNECTION
    appl_att_initiate_service_discovery ();
#endif /* APPL_AUTO_DISCOVERY_ON_CONNECTION */
#endif  /* APPL_ATT_CLIENT */

#ifdef APPL_ATT_SERVER
#ifdef APPL_INITIATE_AUTHENTICATION
    {
        SMP_AUTH_INFO auth;

        auth.param = 1;
        auth.bonding = 1;
        auth.ekey_size = 12;
        auth.security = SMP_SEC_LEVEL_1;
#ifdef SMP_LESC
    /* TODO: Have a Mechanism to choose SMP_LESC_MODE or SMP_LEGACY_MODE here */
    auth.pair_mode = SMP_LESC_MODE;
#endif /* SMP_LESC */

        retval = BT_smp_authenticate
                 (
                     (SMP_BD_HANDLE *)&dq_handle,
                     &auth
                 );
    }
#endif /* APPL_INITIATE_AUTHENTICATION */
#endif /* APPL_ATT_SERVER */
}

void appl_handle_conn_param_req(UCHAR *event_data)
{
    UINT16   conn_interval_max;
    UINT16   conn_interval_min;
    UINT16   conn_latency;
    UINT16   connection_handle;
    UINT16   maximum_ce_length;
    UINT16   minimum_ce_length;
    UINT16   supervision_timeout;

    hci_unpack_2_byte_param(&connection_handle, event_data+0);
    hci_unpack_2_byte_param(&conn_interval_min, event_data+2);
    hci_unpack_2_byte_param(&conn_interval_max, event_data+4);
    hci_unpack_2_byte_param(&conn_latency, event_data+6);
    hci_unpack_2_byte_param(&supervision_timeout, event_data+8);
    minimum_ce_length = 0xFF;
    maximum_ce_length = 0xFF;

    /**
     * TODO: Ideally, the application might not be aware of how many concurrent
     *       connections are present, and if the parameters requested by remote
     *       device are suitable or not.
     *       The application should choose to have better parameters for connection
     *       instead of replying with the incoming parameters from peer.
     */

    BT_hci_le_remote_conn_param_req_reply
    (
        connection_handle,
        conn_interval_min,
        conn_interval_max,
        conn_latency,
        supervision_timeout,
        minimum_ce_length,
        maximum_ce_length
    );
}

char *appl_hci_get_command_name (UINT16 opcode)
{
    int i, size;
    char *command_name;

    command_name = "UNKNOWN";
    size = sizeof(opcode_name_table)/sizeof(APPL_HCI_OPCODE_NAME);

    for (i = 0; i < size; i++)
    {
        if (opcode_name_table[i].opcode == opcode)
        {
            command_name = opcode_name_table[i].command_name;
            break;
        }
    }

    return command_name;
}

