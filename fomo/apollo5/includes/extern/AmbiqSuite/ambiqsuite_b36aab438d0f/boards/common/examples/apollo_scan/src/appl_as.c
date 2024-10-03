//*****************************************************************************
//
//! @file appl_as.c
//!
//! @brief Mindtree application for apollo scan.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
/* ----------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_att_api.h"
#include "BT_smp_api.h"
#include "smp_pl.h"
#include "l2cap.h"
#include "gatt_defines.h"

#include "appl_as.h"

#define CONSOLE_OUT(...)

/* ----------------------------------------- Configuration Defines */
/* Scan parameters */

/* Scan Type. 0x00 => Passive Scanning. 0x01 => Active Scanning. */
#define APPL_GAP_GET_SCAN_TYPE()               0x00

/* Scan Interval */
#define APPL_GAP_GET_SCAN_INTERVAL()           0x0040

/* Scan Window */
#define APPL_GAP_GET_SCAN_WINDOW()             0x0040

/* Own Address Type. 0x00 => Public. 0x01 => Random */
#define APPL_GAP_GET_OWN_ADDR_TYPE_IN_SCAN()   0x00

/* Scan Filter Policy. 0x00 => Accept All. 0x01 => Use White List. */
#define APPL_GAP_GET_SCAN_FILTER_POLICY()      0x00

#define APPL_GAP_GET_WHITE_LIST_SCAN_FILTER_POLICY()          0x01

#define APPL_GAP_GET_NON_WHITE_LIST_SCAN_FILTER_POLICY()      0x00


/* Connection Paramters */
/* Scan Interval */
#define APPL_GAP_GET_CONN_SCAN_INTERVAL()               0x0040

/* Scan Window */
#define APPL_GAP_GET_CONN_SCAN_WINDOW()                 0x0040

/**
 * Initiator Filter Policy.
 * 0x00 => Do not use White List.
 * 0x01 => Use White List.
 */
#define APPL_GAP_GET_CONN_INITIATOR_FILTER_POLICY()     0x00

/* Own Address Type. 0x00 => Public. 0x01 => Random */
#define APPL_GAP_GET_OWN_ADDR_TYPE_AS_INITIATOR()       0x00

/* Minimum value of connection event interval */
#define APPL_GAP_GET_CONN_INTERVAL_MIN()                0x96

/* Maximum value of connection event interval */
#define APPL_GAP_GET_CONN_INTERVAL_MAX()                0x96

/* Slave Latency */
#define APPL_GAP_GET_CONN_LATENCY()                     0x00

/* Link Supervision Timeout */
#define APPL_GAP_GET_CONN_SUPERVISION_TIMEOUT()         0x03E8

/* Informational Parameter */
#define APPL_GAP_GET_CONN_MIN_CE_LENGTH()               0x0000
#define APPL_GAP_GET_CONN_MAX_CE_LENGTH()               0xFFFF

/* Connection Update Parameters */

/* Minimum value of connection event interval for Connection Update */
#define APPL_GAP_GET_CONN_INTERVAL_MIN_FOR_CU()                0x20

/* Maximum value of connection event interval for Connection Update */
#define APPL_GAP_GET_CONN_INTERVAL_MAX_FOR_CU()                0x40

/* Slave Latency for Connection Update */
#define APPL_GAP_GET_CONN_LATENCY_FOR_CU()                     0x00

/* Link Supervision Timeout for Connection Update */
#define APPL_GAP_GET_CONN_SUPERVISION_TIMEOUT_FOR_CU()         0x0200

/* Informational Parameter for Connection Update */
#define APPL_GAP_GET_CONN_MIN_CE_LENGTH_FOR_CU()               0x0000
#define APPL_GAP_GET_CONN_MAX_CE_LENGTH_FOR_CU()               0xFFFF

#define APPL_CLI_CNFG_VAL_LENGTH    2

#define IAS_NO_ALERT     0x00
#define IAS_MILD_ALERT   0x01
#define IAS_HIGH_ALERT   0x02

#define LLS_NO_ALERT     0x00
#define LLS_MILD_ALERT   0x01
#define LLS_HIGH_ALERT   0x02

/* --------------------------------------------- Macros */

#define APPL_MAX_CHARACTERISTICS          50
#define APPL_MAX_SERVICES                 20
#define GATT_PRIMARY_SERVICE_UUID         0x2800
#define GATT_SECONDARY_SERVICE_UUID       0x2801
#define GATT_CHARACTERISTIC_UUID          0x2803
#define GATT_CLIENT_CNFG_UUID             0x2902


/* GATT Services */
#define GATT_HDI_SERVICE_UUID             0x180A
#define GATT_THERM_SERVICE_UUID           0x1809
#define GATT_HR_SERVICE_UUID              0xF000
#define GATT_BPM_SERVICE_UUID             0xA000

/* GATT Characteristics */
#define GATT_TEMP_CHAR_UUID               0x2A1C
#define GATT_TEMP_TYPE_CHAR_UUID          0x2A1D
#define GATT_INTER_TEMP_CHAR_UUID         0x2A1E
#define GATT_TEMP_INTERVAL_CHAR_UUID      0x2A21

#define GATT_HR_CHAR_UUID                 0xA105
#define GATT_HR_SENSOR_LOC_CHAR_UUID      0xA106
#define GATT_HR_COMMAND_UUID              0xA107

#define GATT_BPM_CHAR_UUID                0xA100
#define GATT_INTER_BPM_UUID               0xA101

#define GATT_DIS_SYSTEM_ID_CHAR_UUID      0x2A23
#define GATT_DIS_MODEL_NUM_CHAR_UUID      0x2A24
#define GATT_DIS_SERIAL_NUM_CHAR_UUID     0x2A25
#define GATT_DIS_FW_VER_CHAR_UUID         0x2A26
#define GATT_DIS_HW_VER_CHAR_UUID         0x2A27
#define GATT_DIS_SW_VER_CHAR_UUID         0x2A28
#define GATT_DIS_MAN_NAME_CHAR_UUID       0x2A29
#define GATT_DIS_REG_CERT_CHAR_UUID       0x2A2A

#define APPL_INVALID_CHARACTERISTICS_INDEX   0xFFFF
#define APPL_INVALID_SERVICE_INDEX           0xFFFF
#define APPL_INVLAID_CHAR_CONFIGURATION      0xFFFF
#define APPL_INVALID_UUID                    0x0000

#define APPL_GATT_CLIENT_CH_NOTIFICATION     0x0001
#define APPL_GATT_CLIENT_CH_INDICATION       0x0002

#define APPL_GATT_CHARACTERISTICS_INDICATION   0x20



/* ----------------------------------------- Macro Defines */
/* Encoding of application PDU parameters */
#define appl_pack_1_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UCHAR *)(src)));

#define appl_pack_2_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UINT16 *)(src))); \
    *((dest) + 1) = (UCHAR)(*((UINT16 *)(src)) >> 8);

#define appl_pack_3_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UINT32 *)(src)));\
    *((dest) + 1) = (UCHAR)(*((UINT32 *)(src)) >> 8);\
    *((dest) + 2) = (UCHAR)(*((UINT32 *)(src)) >> 16);

#define appl_pack_4_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UINT32 *)(src)));\
    *((dest) + 1) = (UCHAR)(*((UINT32 *)(src)) >> 8);\
    *((dest) + 2) = (UCHAR)(*((UINT32 *)(src)) >> 16);\
    *((dest) + 3) = (UCHAR)(*((UINT32 *)(src)) >> 24);

/* Decoding of application PDU parameters */
#define appl_unpack_1_byte_param(dest, src) \
        (dest) = (src)[0];

#define appl_unpack_2_byte_param(dest, src) \
        (dest)  = (src)[1]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[0];

#define appl_unpack_4_byte_param(dest, src) \
        (dest)  = (src)[3]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[2]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[1]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[0];

/* TBD: */
#define DEVICE_CONNECTED
#define CONNECTION_INITIATED
#define APPL_SET_STATE(mask)
#define APPL_GET_STATE(mask) 0x00

#ifndef DONT_USE_STANDARD_IO
#define APPL_TRC printf
#define APPL_ERR printf
#else
extern uint32_t am_util_stdio_printf(const char *pcFmt, ...);
#define APPL_TRC am_util_stdio_printf
#define APPL_ERR am_util_stdio_printf
#endif /* DONT_USE_STANDARD_IO */

typedef struct
{
    UCHAR s_index;
    UCHAR c_property;
    UINT16 value_handle;
    UINT16 uuid;
    UINT16 client_config;
    UINT16 client_config_handle;
}APPL_GATT_CHARACTERISTIC_DESCRIPTOR;

typedef struct
{
    /* Handle at which the Characteristic is defined */
    UINT16 handle;

    /* Characteristic Format Descriptor */
    APPL_GATT_CHARACTERISTIC_DESCRIPTOR  desc;
}APPL_CHARACTERISTIC_INFORMATION;

/**
 * Abstracts handle value information for 16 bit values, this is used to store
 * information related to service and characteristic information and was not
 * designed for generic purposes
 */
typedef struct
{
    /* Handle at which the information is stored */
    ATT_HANDLE_RANGE    range;

    /* Value at the handle */
    UINT16              value;
}APPL_GATT_SERVICE_DESC;

typedef struct
{
    UINT16 char_start_range;

    UINT16 char_end_range;

    APPL_GATT_SERVICE_DESC    service_info;
}APPL_SERVICE_INFORMATION;


/*  Service Discovery State Information */
typedef struct
{
    ATT_ATTR_HANDLE    service_end_handle;

    ATT_UUID16         service_uuid;

    /**
     * APPL_GD_INIT_STATE
     * APPL_GD_IN_SERV_DSCRVY_STATE
     * APPL_GD_IN_CHAR_DSRCVRY_STATE
     * APPL_GD_IN_DSEC_DSCVRY_STATE
     */
    UCHAR              state;

    UCHAR              current_char_index;

    UCHAR              char_count;

}APPL_SERVICE_DISCOVERY_STATE;

typedef struct
{
    /* Contains information of current Service Discovery Proecudure */
    APPL_SERVICE_DISCOVERY_STATE    state;

    /* Primary Service Information */
    UINT16    pri_start_range;

    UINT16    pri_end_range;

    /* Secondary Service Information */
    UINT16    sec_start_range;

    UINT16    sec_end_range;

}APPL_PEER_SERVER_INFO;

/* ----------------------------------------- External Global Variables */
extern volatile uint8_t otaRSPData[5];

void appl_att_read_by_group_type
(
/* IN */ ATT_HANDLE_RANGE * range,
/* IN */ UINT16   uuid
);


void appl_discover_gap_service(void);

void appl_dump_bytes (UCHAR *buffer, UINT16 length);
char * appl_hci_get_command_name (UINT16 opcode);
const char * appl_get_profile_menu (void);
void appl_store_peer_dev_addr(BT_DEVICE_ADDR * peer_div_addr);
void appl_create_connection(void);
/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */
/* Application specific information used to register with ATT */
DECL_STATIC ATT_APPLICATION att_app;

/* GATT Handle */
DECL_STATIC ATT_HANDLE appl_gatt_client_handle;

typedef struct
{
    ATT_ATTR_HANDLE    start_handle;

    ATT_UUID16         uuid;
}APPL_CHARACTERISTIC;

/** PUID Characteristic related information */
typedef struct
{
    /* IAS - Immediate Alert */

    /* Alert Level */
    ATT_ATTR_HANDLE ias_alert_level_hdl;

    /* TPS - Tx Po  wer Service */

    /* Tx Power Level */
    ATT_ATTR_HANDLE tps_tx_power_level_hdl;

    /* Tx Power Level - CCC */
    ATT_ATTR_HANDLE tps_tx_power_level_ccc_hdl;

    /* LLS - Link Loss Service */

    /* Alert Level */
    ATT_ATTR_HANDLE lls_alert_level_hdl;

    /* BAS - Battery Service */

    /* Battery Level */
    ATT_ATTR_HANDLE bas_batt_level_hdl;

} PXM_CHAR_INFO;

//#define APPL_MAX_CHARACTERISTICS         16
#define APPL_UUID_INIT_VAL               0x0000
#define APPL_INVALID_CHAR_INDEX          0xFF
#define APPL_CHAR_COUNT_INIT_VAL         0x00

#define APPL_GD_INIT_STATE               0x00
#define APPL_GD_IN_SERV_DSCRVY_STATE     0x01
#define APPL_GD_IN_CHAR_DSCVRY_STATE     0x02
#define APPL_GD_IN_DESC_DSCVRY_STATE     0x03

#define APPL_CHAR_GET_START_HANDLE(i)\
        appl_char[(i)].start_handle

#define APPL_CHAR_GET_UUID(i)\
        appl_char[(i)].uuid

#define APPL_GET_SER_END_HANDLE()\
        appl_gatt_dscvry_state.service_end_handle

#define APPL_GET_CURRENT_DSCVRY_STATE()\
        appl_gatt_dscvry_state.state

#define APPL_CURRENT_SERVICE_UUID()\
        appl_gatt_dscvry_state.service_uuid

#define APPL_GET_CURRENT_CHAR_INDEX()\
        appl_gatt_dscvry_state.current_char_index

#define APPL_GET_CHAR_COUNT()\
        appl_gatt_dscvry_state.char_count

#define APPL_CHAR_INIT(i)\
        APPL_CHAR_GET_START_HANDLE(i) = ATT_INVALID_ATTR_HANDLE_VAL;\
        APPL_CHAR_GET_UUID(i) = APPL_UUID_INIT_VAL;

#define APPL_INIT_GATT_DSCRY_STATE()\
        APPL_GET_SER_END_HANDLE() = APPL_UUID_INIT_VAL;\
        APPL_GET_CURRENT_DSCVRY_STATE() = APPL_GD_INIT_STATE;\
        APPL_CURRENT_SERVICE_UUID() = APPL_UUID_INIT_VAL;\
        APPL_GET_CURRENT_CHAR_INDEX() = APPL_INVALID_CHAR_INDEX;\
        APPL_GET_CHAR_COUNT() = APPL_CHAR_COUNT_INIT_VAL;

/* IAS - Immediate Alert */

/* Alert Level */
#define IAS_ALERT_LEVEL_HDL (pxm_char_info.ias_alert_level_hdl)

/* TPS - Tx Power Service */

/* Tx Power Level */
#define TPS_TX_POWER_LEVEL_HDL (pxm_char_info.tps_tx_power_level_hdl)

/* Tx Power Level - CCC */
#define TPS_TX_POWER_LEVEL_CCC_HDL (pxm_char_info.tps_tx_power_level_ccc_hdl)

/* LLS - Link Loss Service */

/* Alert Level */
#define LLS_ALERT_LEVEL_HDL (pxm_char_info.lls_alert_level_hdl)

/* Battery Level */
#define BAS_BATTERY_LEVEL_HDL (pxm_char_info.bas_batt_level_hdl)

#define APPL_PXM_INFO_INIT()\
        IAS_ALERT_LEVEL_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        TPS_TX_POWER_LEVEL_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        TPS_TX_POWER_LEVEL_CCC_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        LLS_ALERT_LEVEL_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        BAS_BATTERY_LEVEL_HDL = ATT_INVALID_ATTR_HANDLE_VAL;


DECL_STATIC ATT_ATTR_HANDLE service_start_handle;
DECL_STATIC ATT_ATTR_HANDLE service_end_handle;

DECL_STATIC APPL_CHARACTERISTIC appl_char[APPL_MAX_CHARACTERISTICS];

DECL_STATIC APPL_SERVICE_DISCOVERY_STATE appl_gatt_dscvry_state;

DECL_STATIC PXM_CHAR_INFO pxm_char_info;

/* This holds the descriptor discovery count */
UCHAR appl_char_descptr_discovery_count;

BT_DEVICE_ADDR g_peer_bd_addr;

advData_t g_advData;

extern volatile int bulk_mode;

/* ------------------------------- Functions */

void appl_reset_gatt_dscvry_info (void)
{
    UINT32    index;

    APPL_INIT_GATT_DSCRY_STATE();

    appl_char_descptr_discovery_count = 0;

    index = APPL_MAX_CHARACTERISTICS;
    do
    {
        index--;
        APPL_CHAR_INIT(index);
    } while (index > 0);
}


/**
 *  \brief To poweron the stack.
 *
 *  \par Description:
 *  This routine will turn on the Bluetooth service.
 */
void appl_poweron_stack(void)
{

    /**
     *  Turn on the Bluetooth service.
     *
     *  appl_hci_callback - is the application callback function that is
     *  called when any HCI event is received by the stack
     *
     *  appl_bluetooth_on_complete_callback - is called by the stack when
     *  the Bluetooth service is successfully turned on
     *
     *  Note: After turning on the Bluetooth Service, stack will call
     *  the registered bluetooth on complete callback.
     *  This sample application will initiate Scanning to look for devices
     *  in vicinity [Step 2(a)] from bluetooth on complete callback
     *  'appl_bluetooth_on_complete_callback'
     */
    BT_bluetooth_on
    (
        appl_hci_callback,
        appl_bluetooth_on_complete_callback,
        "EtherMind-PXM"
    );

    return;
}


/**
 *  \brief Entry point of the sample application.
 */
int appl_init(void)
{
    /* Step 1. Initialize and power on the stack */
    BT_ethermind_init ();
    /*
    * Note: Stack implementation is non-blocking.
    * Rest of the control flow will be through various
    * registered callback routines and API calls.
    */

    /** Wait-Loop - to avoid exiting from the process */
    while (1)
    {
        BT_sleep(1);
    }
}


/**
 *  \brief Bluetooth ON callback handler.
 *
 *  \par Description:
 *  This is the callback funtion called by the stack after
 *  completion of Bluetooth ON operation.
 *
 *  Perform following operation
 *  - Register Callback with L2CAP, ATT and SMP.
 *  - Initiate Scanning
 *
 *  \param None
 *
 *  \return API_SUCCESS
 */
API_RESULT appl_bluetooth_on_complete_callback (void)
{
    API_RESULT retval;
    uint8_t cmdRspData[MAX_LEN_PDU - 2];

    cmdRspData[0] = RESET_RSP;
    cmdRspData[1] = 1;
    cmdRspData[2] = 0;
    cmdRspData[3] = 0x00;

    CmdResponse(cmdRspData);
    APPL_TRC (
    "Turned ON Bluetooth.\n");

    /* GATT Discovery State Initialization */
    appl_reset_gatt_dscvry_info ();

    APPL_PXM_INFO_INIT ();


    /* Register application callbacks with stack modules */
    retval = appl_register_callback();

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "Callback Registration Failed. Result = 0x%04X\n", retval);

        return retval;
    }

     return retval;
}


/**
 *  \brief Registers application callback with stack layers.
 *
 *  \par Description:
 *  This routine registers callback with L2CAP, ATT and SMP.
 *
 *  \param None
 *
 *  \return API_SUCCESS
 */
API_RESULT appl_register_callback(void)
{
    API_RESULT retval;

    /* Register callback with L2CAP */
    retval = BT_l2cap_register_le_event_cb(appl_l2cap_callback);

    if (API_SUCCESS != retval)
    {
        return retval;
    }

    /* Register callback with ATT */
    att_app.cb = &appl_att_callback;
    retval = BT_att_register(&att_app);

    if (API_SUCCESS != retval)
    {
        return retval;
    }

    /* Register callback with SMP */
    retval = BT_smp_register_user_interface(appl_smp_callback);

    return retval;
}

void appl_handle_conn_complete(UCHAR* peer_addr,
                               UCHAR peer_addr_type)
{
    SMP_AUTH_INFO auth;
    SMP_BD_ADDR   smp_peer_bd_addr;
    SMP_BD_HANDLE smp_bd_handle;
    API_RESULT retval;

    APPL_SET_STATE(DEVICE_CONNECTED);

    auth.param = 1;
    auth.bonding = 1;
    auth.ekey_size = 12;
    auth.security = SMP_SEC_LEVEL_1;
#ifdef SMP_LESC
    /* TODO: Have a Mechanism to choose SMP_LESC_MODE or SMP_LEGACY_MODE here */
    auth.pair_mode = SMP_LESC_MODE;
#endif /* SMP_LESC */

    BT_COPY_BD_ADDR(smp_peer_bd_addr.addr, peer_addr);
    BT_COPY_TYPE(smp_peer_bd_addr.type, peer_addr_type);

    retval = device_queue_search_remote_addr
            (
                &smp_bd_handle,
                &smp_peer_bd_addr
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
}
/* ------------------------------- HCI Callback Function */

/*
 *  This is a callback function registered with the HCI layer during
 *  bluetooth ON.
 *
 *  \param event_type: Type of HCI event.
 *  \param event_data: Parameters for the event 'event_type'.
 *  \param event_datalen: Length of the parameters for the event 'event_type'.
 *
 *  \return   : API_SUCEESS on successful processing of the event.
 *              API_FAILURE otherwise
 */
API_RESULT appl_hci_callback
           (
               UCHAR    event_type,
               UCHAR  * event_data,
               UCHAR    event_datalen
           )
{
    UINT32              count;
    UINT16 connection_handle, value_2;
    UCHAR  status, value_1;

    UCHAR    sub_event_code;
    UCHAR    num_reports;
    UCHAR    address_type;
    UCHAR    clock_accuracy;
    UCHAR    length_data;
    UCHAR    peer_addr_type;
    UCHAR    role;
    UCHAR    rssi;
    UCHAR  * address;
    UCHAR  * data;
    UCHAR  * le_feature;
    UCHAR  * peer_addr, * local_rpa_addr, * peer_rpa_addr;
    UCHAR  * random_number;
    UINT16   conn_interval;
    UINT16   conn_latency;
    UINT16   encripted_diversifier;
    UINT16   supervision_timeout;
    UINT8    cmdRspData[MAX_LEN_PDU - 2];
    UCHAR *pRspData;

    /* Num completed packets event, can be ignored by the application */
    if (HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT == event_type)
    {
        return API_SUCCESS;
    }

    // APPL_TRC ("\n");
    // APPL_TRC("\nhci callback: event:0x%x\n", event_type);
    /* Switch based on the Event Code */
    switch (event_type)
    {
        case HCI_DISCONNECTION_COMPLETE_EVENT:
            APPL_TRC (
            "Received HCI_DISCONNECTION_COMPLETE_EVENT.\n");

            pRspData = event_data;

            /* Status */
            hci_unpack_1_byte_param(&status, event_data);
            APPL_TRC (
            "\tStatus: 0x%02X\n", status);
            event_data += 1;

            /* Connection Handle */
            hci_unpack_2_byte_param(&connection_handle, event_data);
            APPL_TRC (
            "\tConnection Handle: 0x%04X\n", connection_handle);
            event_data += 2;

            /* Reason */
            hci_unpack_1_byte_param(&value_1, event_data);
            APPL_TRC (
            "\tReason: 0x%02X\n", value_1);
            event_data += 1;

            if ((UCHAR)HC_CONNECTION_TIMEOUT == value_1)
            {
                APPL_TRC("\nDevice Disconnected : Connection Timeout ");
            }

            APPL_TRC("\nconnection Complete. Handle: 0x%04X. Reason: 0x%04X\n",
            connection_handle, value_1);

            cmdRspData[0] = DIS_CONNECT_RSP;
            cmdRspData[1] = 1 + 4;
            cmdRspData[2] = 0;
            cmdRspData[3] = 0;
            memcpy (&cmdRspData[4], pRspData, 4);
            CmdResponse(cmdRspData);
             break;

        case HCI_ENCRYPTION_CHANGE_EVENT:
            APPL_TRC (
            "Received HCI_ENCRYPTION_CHANGE_EVENT.\n");

            /* Status */
            hci_unpack_1_byte_param(&status, event_data);
            APPL_TRC (
            "\tStatus: 0x%02X\n", status);
            event_data += 1;

            /* Connection Handle */
            hci_unpack_2_byte_param(&connection_handle, event_data);
            APPL_TRC (
            "\tConnection Handle: 0x%04X\n", connection_handle);
            event_data += 2;

            /* Encryption Enable */
            hci_unpack_1_byte_param(&value_1, event_data);
            APPL_TRC (
            "\tEcnryption Enable: 0x%02X", value_1);
            switch (value_1)
            {
                case 0x00:
                    APPL_TRC (
                    " -> Encryption OFF\n");
                    break;
                case 0x01:
                    APPL_TRC (
                    " -> Encryption ON\n");
                    break;
                default:
                    APPL_TRC (
                    " -> ???\n");
                    break;
            }
            event_data += 1;

            break;

        case HCI_COMMAND_COMPLETE_EVENT:
            APPL_TRC (
            "Received HCI_COMMAND_COMPLETE_EVENT.\n");

            /* Number of Command Packets */
            hci_unpack_1_byte_param(&value_1, event_data);
            APPL_TRC (
            "\tNum Command Packets: %d (0x%02X)\n", value_1, value_1);
            event_data += 1;

            /* Command Opcode */
            hci_unpack_2_byte_param(&value_2, event_data);
            APPL_TRC (
            "\tCommand Opcode: 0x%04X -> %s\n",
            value_2, appl_hci_get_command_name(value_2));
            event_data += 2;

            /* Command Status */
            hci_unpack_1_byte_param(&status, event_data);
            APPL_TRC (
            "\tCommand Status: 0x%02X\n", status);
            event_data += 1;

            /* Check for HCI_LE_SET_ADVERTISING_DATA_OPCODE */
            if (HCI_LE_SET_ADVERTISING_DATA_OPCODE == value_2)
            {
                APPL_TRC (
                "Set Advertising Parameters.\n");
                APPL_TRC (
                "Status = 0x%02X\n", status);

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
                APPL_TRC (
                "Enabling Advertising\n");
                APPL_TRC (
                "Status = 0x%02X\n", status);

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
                    APPL_TRC (
                    "Enabled Advertising...\n");
                    APPL_TRC (
                    "EtherMind Server is now Ready\n");
                }
            }
            /* Check for HCI_LE_SET_SCAN_PARAMETERS_OPCODE */
            else if (HCI_LE_SET_SCAN_PARAMETERS_OPCODE == value_2)
            {
                APPL_TRC (
                "Set Scan Parameters Complete.\n");
                APPL_TRC (
                "Status = 0x%02X\n", status);

                if (API_SUCCESS == status)
                {

                    /* Enable Scanning [Step 2(a)] */
                    BT_hci_le_set_scan_enable (1, 1);

                }
            }
            else if (HCI_LE_SET_SCAN_ENABLE_OPCODE == value_2)
            {
                APPL_TRC("\nSet Scan Enable Complete. Status: 0x%02X\n", status);
                if (API_SUCCESS == status)
                {
                    APPL_TRC (
                    "Enabled Scanning ...\n");

                    /**
                     *  Once peer device starts advertising
                     *  local device will receive
                     *  'HCI_LE_ADVERTISING_REPORT_SUBEVENT'
                     *  and initiate connection.
                     */
                    APPL_TRC (
                    "Wait for the Advertising Events\n");
                }

                /*response stop scan command*/
                if (g_stopScanFlag)
                {
                    APPL_TRC ("Response stop scan command.\n");
                    cmdRspData[0] = STOP_SCAN_RSP;
                    cmdRspData[1] = 2;
                    cmdRspData[2] = 0;
                    cmdRspData[3] = 0;
                    cmdRspData[4] = status;
                    CmdResponse(cmdRspData);
                }
            }

            /* Command Return Parameters */
            if (event_datalen > 4)
            {
                APPL_TRC (
                "\tReturn Parameters: ");
                for (count = 4; count < event_datalen; count ++)
                {
                    APPL_TRC (
                    "%02X ", *event_data);
                    event_data += 1;
                }
                APPL_TRC ("\n");
            }

            break;

        case HCI_COMMAND_STATUS_EVENT:
            APPL_TRC (
            "Received HCI_COMMAND_STATUS_EVENT.\n");

            /* Status */
            hci_unpack_1_byte_param(&status, event_data);
            APPL_TRC (
            "\tCommand Status: 0x%02X\n", status);
            event_data += 1;

            /* Number of Command Packets */
            hci_unpack_1_byte_param(&value_1, event_data);
            APPL_TRC (
            "\tNum Command Packets: %d (0x%02X)\n", value_1, value_1);
            event_data += 1;

            /* Command Opcode */
            hci_unpack_2_byte_param(&value_2, event_data);
            APPL_TRC (
            "\tCommand Opcode: 0x%04X (%s)\n",
            value_2, appl_hci_get_command_name(value_2));
            event_data += 2;

            break;

        case HCI_HARDWARE_ERROR_EVENT:
            APPL_TRC (
            "Received HCI_HARDWARE_ERROR_EVENT.\n");

            APPL_TRC ("\r\nReceived HCI_HARDWARE_ERROR_EVENT.\r\n");

            /* Hardware Code */
            hci_unpack_1_byte_param(&value_1, event_data);
            APPL_TRC (
            "\tHardware Code: 0x%02X\n", value_1);
            event_data += 1;

            break;

        case HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT:
            APPL_TRC (
            "Received HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT.\n");
            break;

        case HCI_DATA_BUFFER_OVERFLOW_EVENT:
            APPL_TRC (
            "Received HCI_DATA_BUFFER_OVERFLOW_EVENT.\n");

            /* Link Type */
            hci_unpack_1_byte_param(&value_1, event_data);
            APPL_TRC (
            "\tLink Type: 0x%02X", value_1);
            switch (value_1)
            {
                case 0x00:
                    APPL_TRC (
                    " -> Synchronous Buffer Overflow\n");
                    break;
                case 0x01:
                    APPL_TRC (
                    " -> ACL Buffer Overflow\n");
                    break;
                default:
                    APPL_TRC (
                    " -> ???\n");
                    break;
            }
            event_data += 1;

            break;

        case HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT:
            APPL_TRC (
            "Received HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT\n");

            /* Status */
            hci_unpack_1_byte_param(&status, event_data);
            APPL_TRC (
            "Status: 0x%02X\n", status);
            event_data ++;

            /* Connection Handle */
            hci_unpack_2_byte_param(&value_2, event_data);
            APPL_TRC (
            "\tConnection Handle: 0x%04X\n", value_2);

            break;

        case HCI_LE_META_EVENT:
            APPL_TRC (
            "Received HCI_LE_META_EVENT.\n");
            hci_unpack_1_byte_param(&sub_event_code, event_data);
            event_data = event_data + 1;
            switch(sub_event_code)
            {
                case HCI_LE_CONNECTION_COMPLETE_SUBEVENT:
                    pRspData = event_data;
                    APPL_TRC (
                    "Subevent : HCI_LE_CONNECTION_COMPLETE_SUBEVENT.\n");
                    hci_unpack_1_byte_param(&status, event_data + 0);
                    hci_unpack_2_byte_param(&connection_handle, event_data + 1);
                    hci_unpack_1_byte_param(&role, event_data + 3);
                    hci_unpack_1_byte_param(&peer_addr_type, event_data + 4);
                    peer_addr = 5 + event_data;
                    hci_unpack_2_byte_param(&conn_interval, event_data + 11);
                    hci_unpack_2_byte_param(&conn_latency, event_data + 13);
                    hci_unpack_2_byte_param(&supervision_timeout, event_data + 15);
                    hci_unpack_1_byte_param(&clock_accuracy, event_data + 17);


                    /* Print the parameters */
                    APPL_TRC (
                    "status = 0x%02X\n", status);
                    APPL_TRC (
                    "connection_handle = 0x%04X\n", connection_handle);
                    APPL_TRC (
                    "role = 0x%02X\n", role);
                    APPL_TRC (
                    "peer_addr_type = 0x%02X\n", peer_addr_type);
                    APPL_TRC (
                    "peer_addr = \n");
                    appl_dump_bytes(peer_addr, 6);
                    APPL_TRC (
                    "conn_interval = 0x%04X\n", conn_interval);
                    APPL_TRC (
                    "conn_latency = 0x%04X\n", conn_latency);
                    APPL_TRC (
                    "supervision_timeout = 0x%04X\n", supervision_timeout);
                    APPL_TRC (
                    "clock_accuracy = 0x%02X\n", clock_accuracy);
                    APPL_TRC(
                    "Connection Complete.\r\nStatus: 0x%02X.\r\nBD Addr: "
                    "%02X %02X %02X %02X %02X %02X.\r\nBD Addr Type: 0x%02X.\r\n"
                    "Connection Handle: 0x%04X\n", status, peer_addr[0],
                    peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4],
                    peer_addr[5], peer_addr_type, connection_handle);

                    cmdRspData[0] = CONNECT_RSP;
                    cmdRspData[1] = 1 + 18;
                    cmdRspData[2] = 0;
                    cmdRspData[3] = 0;
                    memcpy (&cmdRspData[4], pRspData, 18);
                    CmdResponse(cmdRspData);

                    break;
#ifdef HCI_LE_PRIVACY_1_2_SUPPORT
                case HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT:
                    APPL_TRC (
                    "Subevent : HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT.\n");
                    pRspData = event_data;
                    hci_unpack_1_byte_param(&status, event_data + 0);
                    hci_unpack_2_byte_param(&connection_handle, event_data + 1);
                    hci_unpack_1_byte_param(&role, event_data + 3);
                    hci_unpack_1_byte_param(&peer_addr_type, event_data + 4);
                    /* Check the Identity Address Type */
                    peer_addr_type = ((0x01 == peer_addr_type) || (0x03 == peer_addr_type)) ? \
                        BT_BD_RANDOM_ADDRESS_TYPE : BT_BD_PUBLIC_ADDRESS_TYPE;

                    peer_addr = 5 + event_data;
                    local_rpa_addr = 11 + event_data;
                    peer_rpa_addr = 17 + event_data;

                    (void)local_rpa_addr;
                    hci_unpack_2_byte_param(&conn_interval, event_data + 23);
                    hci_unpack_2_byte_param(&conn_latency, event_data + 25);
                    hci_unpack_2_byte_param(&supervision_timeout, event_data + 27);
                    hci_unpack_1_byte_param(&clock_accuracy, event_data + 29);

                    /**
                     * If incoming peer rpa address is valid i.e. Non zero,
                     * then use that address as peer address.
                     * Else, the peer identity address is the original address
                     * used by the Peer Device.
                     */
                    if (BT_BD_ADDR_IS_NON_ZERO(peer_rpa_addr))
                    {
                        peer_addr_type = BT_BD_RANDOM_ADDRESS_TYPE;
                        peer_addr      = peer_rpa_addr;
                    }

                    /* Print the parameters */
                    APPL_TRC (
                    "status = 0x%02X\n", status);
                    APPL_TRC (
                    "connection_handle = 0x%04X\n", connection_handle);
                    APPL_TRC (
                    "role = 0x%02X\n", role);
                    APPL_TRC (
                    "peer_addr_type = 0x%02X\n", peer_addr_type);
                    APPL_TRC (
                    "peer_addr = \n");
                    appl_dump_bytes(peer_addr, 6);
                    APPL_TRC (
                    "conn_interval = 0x%04X\n", conn_interval);
                    APPL_TRC (
                    "conn_latency = 0x%04X\n", conn_latency);
                    APPL_TRC (
                    "supervision_timeout = 0x%04X\n", supervision_timeout);
                    APPL_TRC (
                    "clock_accuracy = 0x%02X\n", clock_accuracy);
                    APPL_TRC(
                    "Connection Complete.\r\nStatus: 0x%02X.\r\nBD Addr: "
                    "%02X %02X %02X %02X %02X %02X.\r\nBD Addr Type: 0x%02X.\r\n"
                    "Connection Handle: 0x%04X\n", status, peer_addr[0],
                    peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4],
                    peer_addr[5], peer_addr_type, connection_handle);

                    cmdRspData[0] = CONNECT_RSP;
                    cmdRspData[1] = 1 + 30;
                    cmdRspData[2] = 0;
                    cmdRspData[3] = 0;
                    memcpy (&cmdRspData[4], pRspData, 30);
                    CmdResponse(cmdRspData);
                    break;
#endif /* HCI_LE_PRIVACY_1_2_SUPPORT */
                case HCI_LE_ADVERTISING_REPORT_SUBEVENT:
                    APPL_TRC("Subevent : HCI_LE_ADVERTISING_REPORT_SUBEVENT.\n");
                    pRspData = event_data;
                    /* Number of Responses */
                    hci_unpack_1_byte_param(&num_reports, event_data);
                    APPL_TRC("num_reports = 0x%02X\n", num_reports);
                    event_data += 1;

                    /* For each Response, Print the Inquiry Result */
                    for (count = 0; count < num_reports; count ++)
                    {
                        hci_unpack_1_byte_param(&event_type, event_data);
                        event_data += 1;
                        hci_unpack_1_byte_param(&address_type, event_data);
                        event_data += 1;
                        address = event_data;
                        event_data += 6;
                        hci_unpack_1_byte_param(&length_data, event_data);
                        event_data += 1;
                        data = event_data;
                        event_data += length_data;
                        hci_unpack_1_byte_param(&rssi, event_data);
                        event_data += 1;

                        /* Print the parameters */
                        APPL_TRC("event_type = 0x%02X\n", event_type);
                        APPL_TRC("address_type = 0x%02X\n", address_type);
                        APPL_TRC("address = \n");
                        appl_dump_bytes(address, 6);
                        APPL_TRC("length_data = 0x%02X\n", length_data);
                        APPL_TRC("data = \n");
                        appl_dump_bytes(data, length_data);
                        APPL_TRC("rssi = 0x%02X\n", rssi);
                        cmdRspData[0] = SCAN_RSP;
                        cmdRspData[1] = length_data + 12;
                        cmdRspData[2] = 0;
                        cmdRspData[3] = 0;
                        memcpy (&cmdRspData[4], pRspData, length_data + 11);
                              //  am_util_delay_ms(200);
                        CmdResponse(cmdRspData);
                    }

                    /* If already connection initiated, do not try to initiate again */
                    if (0 != APPL_GET_STATE(CONNECTION_INITIATED))
                    {
                        break;
                    }

#ifndef BT_SW_TXP
                    BT_COPY_BD_ADDR(g_peer_bd_addr.addr, address);
                    BT_COPY_TYPE(g_peer_bd_addr.type, address_type);
#else
                    if (0 != BT_mem_cmp(g_peer_bd_addr.addr,\
                                        address, BT_BD_ADDR_SIZE))
                    {
                        break;
                    }

                    /* Initiate connection */
                    CONSOLE_OUT("Initiating Connection ...\n");

                    appl_create_connection();

#endif /* BT_SW_TXP */
                    break;

                case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT:
                    APPL_TRC (
                    "Subevent : HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT.\n");
                    hci_unpack_1_byte_param(&status, event_data + 0);
                    hci_unpack_2_byte_param(&connection_handle, event_data + 1);
                    hci_unpack_2_byte_param(&conn_interval, event_data + 3);
                    hci_unpack_2_byte_param(&conn_latency, event_data + 5);
                    hci_unpack_2_byte_param(&supervision_timeout, event_data + 7);

                    /* Print the parameters */
                    APPL_TRC (
                    "status = 0x%02X\n", status);
                    APPL_TRC (
                    "connection_handle = 0x%04X\n", connection_handle);
                    APPL_TRC (
                    "conn_interval = 0x%04X\n", conn_interval);
                    APPL_TRC (
                    "conn_latency = 0x%04X\n", conn_latency);
                    APPL_TRC (
                    "supervision_timeout = 0x%04X\n", supervision_timeout);

                    break;

                case HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT:
                    APPL_TRC (
                    "Subevent: HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT.\n");

                    hci_unpack_1_byte_param(&status, event_data + 0);
                    hci_unpack_2_byte_param(&connection_handle, event_data + 1);
                    le_feature = 3 + event_data;

                    /* Print the parameters */
                    APPL_TRC (
                    "status = 0x%02X\n", status);
                    APPL_TRC (
                    "connection_handle = 0x%04X\n", connection_handle);
                    APPL_TRC (
                    "le_feature = \n");
                    appl_dump_bytes(le_feature, 8);
                    break;

                case HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT:
                    APPL_TRC("Subevent : HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT.\n");
                    hci_unpack_2_byte_param(&connection_handle, event_data + 0);
                    random_number = 2 + event_data;
                    hci_unpack_2_byte_param(&encripted_diversifier, event_data + 10);

                    /* Print the parameters */
                    APPL_TRC (
                    "connection_handle = 0x%04X\n", connection_handle);
                    APPL_TRC (
                    "random_number = \n");
                    appl_dump_bytes(random_number, 8);
                    APPL_TRC (
                    "encrypted_diversifier = 0x%04X\n", encripted_diversifier);

                    break;

                default:
                    APPL_ERR (
                    "Unknown/Unhandled LE SubEvent Code 0x%02X Received.\n",
                    sub_event_code);
                    break;
            }
            break;

        default:
            APPL_ERR (
            "Unknown/Unhandled Event Code 0x%02X Received.\n", event_type);
            break;
    }

    return API_SUCCESS;
}


/* ------------------------------- L2CAP Callback Function */
/**
 *  This is a callback function registered with the L2CAP layer
 *  after successfully powering on the stack.
 *
 *  \param handle: Device Identifier.
 *  \param event_type: L2CAP specific Event Identifier.
 *  \param event_data: Parameters for the 'event_type'.
 *  \param event_datalen: Length of the parameters for the 'event_type'.
 *
 *  \return   : API_SUCEESS on successful processing of the event.
 *              API_FAILURE otherwise
 */
void appl_l2cap_callback
     (
         DEVICE_HANDLE * handle,
         UCHAR           event_type,
         UCHAR         * event_data,
         UINT16          event_datalen
     )
{
    UINT16 length, min_int, max_int, conn_lat, sup_time_out, reason, result;
    API_RESULT retval;
    UINT16   connection_handle;
    BT_DEVICE_ADDR peer_bd_addr;
    UCHAR   * bd_addr;
    UCHAR     bd_addr_type;

    device_queue_get_remote_addr (handle, &peer_bd_addr);
    bd_addr = BT_BD_ADDR(&peer_bd_addr);
    bd_addr_type = BT_BD_ADDR_TYPE (&peer_bd_addr);

    if (L2CAP_CONNECTION_UPDATE_REQUEST_EVENT == event_type)
    {
        appl_unpack_2_byte_param(length, &event_data[0]);
        appl_unpack_2_byte_param(min_int, &event_data[2]);
        appl_unpack_2_byte_param(max_int, &event_data[4]);
        appl_unpack_2_byte_param(conn_lat, &event_data[6]);
        appl_unpack_2_byte_param(sup_time_out, &event_data[8]);

#ifdef APPL_MENU_OPS
        CONSOLE_OUT("L2CAP Connection Update Request\n");
#endif /* APPL_MENU_OPS */

        APPL_TRC (
        "Received : L2CAP_CONNECTION_UPDATE\n");
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
        bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        APPL_TRC (
        "\tbd_addr_type : %02X\n", bd_addr_type);
        APPL_TRC (
        "\tLength : 0x%04X\n", length);
        APPL_TRC (
        "\tmin interval : 0x%04X\n", min_int);
        APPL_TRC (
        "\tmax interval : 0x%04X\n", max_int);
        APPL_TRC (
        "\tconn latency : 0x%04X\n", conn_lat);
        APPL_TRC (
        "\tsupervision timeout : 0x%04X\n", sup_time_out);

        /* TBD: Check if local device is master */

        /* Acknowledge with Success */
        retval = BT_l2cap_le_connection_param_update_response
                 (
                     handle,
                     0x00
                 );

        (void)retval;
        /* Send command to HCI */
        BT_hci_get_le_connection_handle(&peer_bd_addr, &connection_handle);

        BT_hci_le_connection_update
        (
             connection_handle,
             min_int,
             max_int,
             conn_lat,
             sup_time_out,
             0x0000, /* minimum_ce_length, */
             0xFFFF  /* maximum_ce_length */
        );
    }
    else if (L2CAP_CONNECTION_UPDATE_RESPONSE_EVENT == event_type)
    {
        appl_unpack_2_byte_param(length, &event_data[0]);
        appl_unpack_2_byte_param(result, &event_data[2]);

        CONSOLE_OUT("L2CAP Connection Update Response. Result: 0x%04X\n",
            result);

        APPL_TRC (
        "Received : L2CAP_CONNECTION_UPDATE\n");
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
        bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        APPL_TRC (
        "\tbd_addr_type : %02X\n", bd_addr_type);
        APPL_TRC (
        "\tLength       : %04X\n", length);
        APPL_TRC (
        "\tResult       : %04X\n", result);
    }
    else if (L2CAP_COMMAND_REJECTED_EVENT == event_type)
    {
        appl_unpack_2_byte_param(reason, &event_data[0]);

        CONSOLE_OUT("L2CAP Command Rejected. Reason: 0x%04X\n", reason);

        APPL_TRC (
        "Received : L2CAP_COMMAND_REJ\n");
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
        bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        APPL_TRC (
        "\tbd_addr_type : %02X\n", bd_addr_type);
        APPL_TRC (
        "\tReason       : %04X\n", reason);
    }
    else
    {
        APPL_ERR (
        "Received Invalid Event. Event = 0x%02X\n", event_type);
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
        bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        APPL_TRC (
        "\tbd_addr_type : %02X\n", bd_addr_type);
    }
}

void appl_print_char_property (UCHAR property)
{
    APPL_TRC ("Characteristic Property 0x%02X\n", property);

    if (GATT_CH_PROP_BIT_BROADCAST == (GATT_CH_PROP_BIT_BROADCAST & property))
    {
        APPL_TRC ("Broadcast Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Broadcast Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_READ == (GATT_CH_PROP_BIT_READ & property))
    {
        APPL_TRC ("Read Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Read Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_WRITE_WO_RSP == (GATT_CH_PROP_BIT_WRITE_WO_RSP & property))
    {
        APPL_TRC ("Write Without Response Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Write Without Response Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_WRITE == (GATT_CH_PROP_BIT_WRITE & property))
    {
        APPL_TRC ("Write Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Write Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_NOTIFY == (GATT_CH_PROP_BIT_NOTIFY & property))
    {
        APPL_TRC ("Notify Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Notify Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_INDICATE == (GATT_CH_PROP_BIT_INDICATE & property))
    {
        APPL_TRC ("Indicate Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Indicate Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_SIGN_WRITE == (GATT_CH_PROP_BIT_SIGN_WRITE & property))
    {
        APPL_TRC ("Authenticated Signed Write Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Authenticated Signed Write Permitted: No\n");
    }

    if (GATT_CH_PROP_BIT_EXT_PROPERTY == (GATT_CH_PROP_BIT_EXT_PROPERTY & property))
    {
        APPL_TRC ("Extended Properties Permitted: Yes\n");
    }
    else
    {
        APPL_TRC ("Extended Properties Permitted: No\n");
    }
}

API_RESULT appl_read_req
           (
                /* IN */ ATT_ATTR_HANDLE    handle
           )
{
    API_RESULT retval;

    retval = BT_att_send_read_req
             (
                 &appl_gatt_client_handle,
                 &handle
             );

    APPL_TRC (
    "[APPL]: Initiated Read Request with result 0x%04X\n", retval);

    return retval;
}

API_RESULT appl_write_req
           (
                /* IN */ ATT_ATTR_HANDLE     handle,
                /* IN */ ATT_VALUE           * value
           )
{
    ATT_WRITE_REQ_PARAM write_req_param;
    API_RESULT retval;

    write_req_param.handle = handle;
    write_req_param.value = (*value);

    retval = BT_att_send_write_req
             (
                 &appl_gatt_client_handle,
                 &write_req_param
             );

    APPL_TRC (
    "[APPL]: ATT Write Request Initiated with retval 0x%04X\n",
    retval);

    return retval;
}

API_RESULT appl_write_cmd
           (
                /* IN */ ATT_ATTR_HANDLE     handle,
                /* IN */ ATT_VALUE           * value
           )
{
    ATT_WRITE_CMD_PARAM write_cmd_param;
    API_RESULT retval;

    write_cmd_param.handle = handle;
    write_cmd_param.value = (*value);

    retval = BT_att_send_write_cmd
             (
                 &appl_gatt_client_handle,
                 &write_cmd_param
             );

    APPL_TRC (
    "[APPL]: ATT Write Command Initiated with retval 0x%04X\n",
    retval);

    return retval;
}

API_RESULT appl_find_info_req
           (
                ATT_HANDLE_RANGE    * range
           )
{
    API_RESULT    retval;

    APPL_TRC (
    "Initiating Descriptor Discovery from 0x%04X to 0x%04X\n",
    range->start_handle, range->end_handle);

    retval = BT_att_send_find_info_req
             (&appl_gatt_client_handle, range);

    if (API_SUCCESS != retval)
    {
        APPL_ERR ("[***ERR***]: Failed to to initiate Find Information Request,"
        "reason 0x%04X\n", retval);
    }
    else
    {
        APPL_TRC ("Successful sent Find Information Request\n");
    }

    return retval;
}

void appl_initiate_descriptor_discovery (void)
{
    ATT_HANDLE_RANGE   range;
    API_RESULT         retval;

    if (0 != APPL_GET_CHAR_COUNT())
    {
        do
        {
            range.start_handle =
                APPL_CHAR_GET_START_HANDLE(APPL_GET_CURRENT_CHAR_INDEX()) + 1;
            if ((APPL_GET_CURRENT_CHAR_INDEX() + 1) == APPL_GET_CHAR_COUNT())
            {
                range.end_handle = APPL_GET_SER_END_HANDLE();
            }
            else
            {
                range.end_handle =
                   (APPL_CHAR_GET_START_HANDLE(APPL_GET_CURRENT_CHAR_INDEX() + 1) - 1);
            }

            /**
             *  Characteristic array maintained by the application has the start
             *  handle initialized to Characteristic Value handle and not to
             *  Characteristic definition. Therefore, if between start handle
             *  and end handle (dervied from next characteristic's value
             *  handle), there exists only one attribute indicated by
             *  start_handle = end_handle, that one attribute can be only the
             *  Characteristic definition. However, the only one place where
             *  this funda does not apply is when it is the last characteristic,
             *  and end handle has been initialized to service end handle. Here
             *  start_handle = end_handle implies a definite descriptor.
             */
            if ((range.start_handle < range.end_handle) ||
               ((range.start_handle == range.end_handle) &&
               (range.end_handle == APPL_GET_SER_END_HANDLE())))
            {
                retval = appl_find_info_req (&range);
                if ( API_SUCCESS == retval )
                {
                    APPL_GET_CURRENT_DSCVRY_STATE() = APPL_GD_IN_DESC_DSCVRY_STATE;
                    break;
                }
            }
            APPL_GET_CURRENT_CHAR_INDEX() ++;
        }while (APPL_GET_CURRENT_CHAR_INDEX() < APPL_GET_CHAR_COUNT());

        if (APPL_GET_CURRENT_CHAR_INDEX() == APPL_GET_CHAR_COUNT())
        {
            if (0 == appl_char_descptr_discovery_count)
            {
                APPL_TRC (
                "Search Complete, Service, Characteristic Discovered, "
                "No Descriptors Found!\n");
            }
            else
            {
                APPL_TRC (
                "Search Complete, Service, Characteristic & "
                "Descriptors Discovered!\n");
            }
            /* Reset GATT Discovery infomation */
            appl_reset_gatt_dscvry_info ();
        }
    }
    else
    {
        APPL_TRC (
        "Search Complete, No Characteristic Found!\n");
        /* Reset GATT Discovery infomation */
        appl_reset_gatt_dscvry_info ();
    }
}

void appl_handle_error_response
     (
         /* IN */ UCHAR response_code
     )
{
    if ((ATT_ATTRIBUTE_NOT_FOUND == response_code) ||
        (ATT_UNSUPPORTED_GROUP_TYPE == response_code))
    {
        /**
         * Check the UUID Being serached.
         * - If Received for primary service, initiate discovery of Primary
         * Services' characteristics. If no primary services present initiate
         * discovery for secondary services.
         * - If Received for Characteristics, initiate discovery of
         * Characteristics in the next service or initiate discovery for
         * Secondary service, or discovery complete.
         * - If received for Secondary Service, Discovery Complete
         */
        if (APPL_GD_IN_SERV_DSCRVY_STATE == APPL_GET_CURRENT_DSCVRY_STATE())
        {
            APPL_TRC (
            "Service Not found!");
            appl_reset_gatt_dscvry_info ();
        }
        else if (APPL_GD_IN_CHAR_DSCVRY_STATE == APPL_GET_CURRENT_DSCVRY_STATE())
        {
            /* Initiate Descriptor Search */
            APPL_GET_CURRENT_CHAR_INDEX() = 0;
            appl_initiate_descriptor_discovery ();
        }
        else if (APPL_GD_IN_DESC_DSCVRY_STATE == APPL_GET_CURRENT_DSCVRY_STATE())
        {
            if ((APPL_GET_CURRENT_CHAR_INDEX() + 1) == APPL_GET_CHAR_COUNT())
            {
                if (0 == appl_char_descptr_discovery_count)
                {
                    APPL_TRC (
                    "Search Complete, Service, Characteristic Discovered, "
                    "No Descriptors Found!\n");
                }
                else
                {
                    APPL_TRC (
                    "Search Complete, Service, Characteristic & "
                    "Descriptors Discovered!\n");
                }
                /* Reset GATT Discovery infomation */
                appl_reset_gatt_dscvry_info ();
            }
            else
            {
                APPL_GET_CURRENT_CHAR_INDEX() ++;
                appl_initiate_descriptor_discovery ();
            }
        }
    }
    else if (ATT_INSUFFICIENT_AUTHORIZATION == response_code)
    {
        /**
         * Add initiation of necessary Security Procedures, Reinitiate Search on
         * successful completion
         */
    }
    else
    {
        /* TODO: Determind the behavior */
    }
}

void appl_att_read_by_type
     (
         /* IN */ ATT_HANDLE_RANGE * range,
         /* IN */ UINT16 uuid
     )
{
    ATT_READ_BY_TYPE_REQ_PARAM    read_by_type_req_param;
    API_RESULT   retval;

    APPL_TRC (
    "Searching for UUID 0x%04X, from handle 0x%04X to 0x%04X\n",
    uuid, range->start_handle, range->end_handle);

    read_by_type_req_param.range = *range;
    ATT_SET_16_BIT_UUID(&read_by_type_req_param.uuid, uuid);
    read_by_type_req_param.uuid_format = ATT_16_BIT_UUID_FORMAT;

    if (GATT_CHARACTERISTIC == uuid)
    {
        APPL_GET_CURRENT_DSCVRY_STATE() = APPL_GD_IN_CHAR_DSCVRY_STATE;
    }

    retval = BT_att_send_read_by_type_req
             (
                 &appl_gatt_client_handle,
                 &read_by_type_req_param
             );

    APPL_TRC(
    "[ATT]: Read by Type Request Initiated with result 0x%04X", retval);
}

void appl_handle_find_by_type_val_rsp (UCHAR * data, UINT16 datalen)
{
    ATT_HANDLE_RANGE range;

    APPL_TRC ("Received Find By Type Value Response\n");
    appl_dump_bytes (data, datalen);

    /* For DIS, there should be only one Service */
    if (4 == datalen)
    {
        APPL_TRC ("Start Handle        End Handle\n");

        BT_UNPACK_LE_2_BYTE(&service_start_handle, data);
        APPL_TRC ("0x%04X", service_start_handle);
        BT_UNPACK_LE_2_BYTE(&service_end_handle, (data + 2));
        APPL_TRC ("              0x%04X\n\n", service_end_handle);

        range.start_handle = service_start_handle;
        range.end_handle = service_end_handle;
        APPL_GET_SER_END_HANDLE() = service_end_handle;
        APPL_GET_CHAR_COUNT() = 0;

        appl_att_read_by_type
        (
            &range,
            GATT_CHARACTERISTIC
        );
    }
    else
    {
        APPL_TRC ("**** Service Found More Than Once. FAILED ****\n");
    }
}



void appl_handle_find_info_response (UCHAR * list, UINT16 length, UCHAR type)
{
    UCHAR               uuid_arr[16];
    ATT_HANDLE_RANGE    range;
    UINT32              index;
    UINT16              handle;
    UINT16              uuid;
    API_RESULT          retval;

    if ( 0 < length )
    {
        if ( 0x01 == type )
        {
            APPL_TRC ("\n\n16 bit UUIDs : \n");
            for ( index = 0; index < length; index = index + 4 )
            {
                BT_UNPACK_LE_2_BYTE(&handle, (list + index));
                BT_UNPACK_LE_2_BYTE(&uuid, (list + index + 2));
                APPL_TRC ("Handle : %04X -> ", handle);
                APPL_TRC ("UUID   : %04X \n", uuid);

                /* Populate Needed CCCDs here */
                if (GATT_CLIENT_CONFIG == uuid)
                {
                    if (GATT_TX_POWER_LEVEL_CHARACTERISTIC == APPL_CHAR_GET_UUID(APPL_GET_CURRENT_CHAR_INDEX()))
                    {
                        TPS_TX_POWER_LEVEL_CCC_HDL = handle;
                    }
                }
            }
        }
        else if ( 0x02 == type )
        {
            for ( index = 0; index < length; index = index + 18 )
            {
                APPL_TRC("\n\n128 bit UUIDs : \n");
                BT_UNPACK_LE_2_BYTE(&handle, (list + index));
                ATT_UNPACK_UUID(uuid_arr, (list + index + 2), 16);
                APPL_TRC ("Handle : %04X\n", handle);
                APPL_TRC ("UUID   : ");

                appl_dump_bytes(uuid_arr, 16);
            }
        }
        else
        {
            /* TODO: Check if this is correct interpretaion */
            APPL_TRC ("List of handles corrosponding to the Req UUID:\n");

            for (index = 0; index < length; index += index)
            {

                BT_UNPACK_LE_2_BYTE(&handle, list + index);
                APPL_TRC ("Handle : %04X -> \n", handle);
            }
        }
    }

    /**
     * Continue Descriptor Discovery for Existig/Next Characteristic.
     */
    if ((APPL_GET_CURRENT_CHAR_INDEX() + 1) == APPL_GET_CHAR_COUNT())
    {
        range.end_handle = APPL_GET_SER_END_HANDLE();
    }
    else
    {
        range.end_handle =
           (APPL_CHAR_GET_START_HANDLE(APPL_GET_CURRENT_CHAR_INDEX() + 1) - 1);
    }

    if (handle < range.end_handle)
    {
        /* More descriptors to be discovered */
        range.start_handle = handle + 1;
        retval = appl_find_info_req (&range);
        if (API_SUCCESS == retval)
        {
            APPL_TRC (
            "[APPL]: Follow-up Find Info Request to discover more "
            "characteristics\n");
        }
        else
        {
            APPL_ERR (
            "[APPL]: Failed to initiate follow request, reason 0x%04X\n", retval);
        }
    }
    else
    {
        if ((APPL_GET_CURRENT_CHAR_INDEX() + 1) == APPL_GET_CHAR_COUNT())
        {
            /* Discovery Complete */
            APPL_TRC (
            "[APPL]: Search Complete, Service, Characteristic & Descriptors Discovered!");
            /* Reset GATT Discovery infomation */
            appl_reset_gatt_dscvry_info ();
        }
        else
        {
            appl_char_descptr_discovery_count ++;
            APPL_GET_CURRENT_CHAR_INDEX() ++;
            appl_initiate_descriptor_discovery ();
        }
    }
}

void appl_handle_read_by_type_response
     (
         /* IN */ UINT16    data_elem_size,
         /* IN */ UCHAR   * data,
         /* IN */ UINT16    data_len
     )
{
    ATT_HANDLE_RANGE range;
    UINT32 index;
    UINT16 handle;
    UINT16 diff_in_size;
    UCHAR  * value;
    UCHAR  search_again;

    UINT16 value_handle;
    UCHAR  c_property;
    UINT16 uuid;

    search_again = 0;

    /**
     * Check if the response received was smaller than max MTU, then no further
     * searching is needed
     */
    diff_in_size = ATT_DEFAULT_MTU - data_len -1;
    if (0 == diff_in_size || ((0 != diff_in_size)
       && (diff_in_size < data_elem_size)))
    {
        /**
         * There could be more requests, as the MTU has been optimally utilized
         */
        search_again = 1;
    }

    for (index = 0; index < (unsigned)(data_len / (data_elem_size)); index++ )
    {
        BT_UNPACK_LE_2_BYTE(&handle, (data + (index * data_elem_size)));
        APPL_TRC ("Handle - 0x%04X\n", handle);
        APPL_TRC ("Handle Value Received - \n");
        value = data + (2 + index * data_elem_size);
        appl_dump_bytes(value, data_elem_size-2);

        if (APPL_GD_IN_CHAR_DSCVRY_STATE == APPL_GET_CURRENT_DSCVRY_STATE())
        {
            /* Extract Property */
            c_property = value[0];

            BT_UNPACK_LE_2_BYTE
            (
                &value_handle,
                value + 1
            );

            BT_UNPACK_LE_2_BYTE
            (
                &uuid,
                value + 3
            );

            APPL_TRC ("----\n");
            APPL_TRC ("Value Handle 0x%04X, Property 0x%02X, UUID 0x%04X\n",
            value_handle, c_property, uuid);

            appl_print_char_property (c_property);

            APPL_TRC ("UUID 0x%04X ", uuid);

            switch (uuid)
            {
                case GATT_ALERT_LEVEL_CHARACTERISTIC:
                    if (GATT_LINK_LOSS_SERVICE == APPL_CURRENT_SERVICE_UUID())
                    {
                        LLS_ALERT_LEVEL_HDL = value_handle;
                    }
                    else if (GATT_IMMEDIATE_ALERT_SERVICE == APPL_CURRENT_SERVICE_UUID())
                    {
                        if (IAS_ALERT_LEVEL_HDL == ATT_INVALID_ATTR_HANDLE_VAL)
                        {
                            IAS_ALERT_LEVEL_HDL = value_handle;
                        }
                    }
                    break;

                case GATT_BATTERY_LEVEL_CHARACTERISTIC:
                    BAS_BATTERY_LEVEL_HDL = value_handle;
                    break;

               case GATT_TX_POWER_LEVEL_CHARACTERISTIC:
                    TPS_TX_POWER_LEVEL_HDL = value_handle;
                    break;

                default:
                    APPL_TRC ("Unknown\n");
                    break;
            }
            APPL_TRC ("----\n\n");
        }
        APPL_CHAR_GET_START_HANDLE (APPL_GET_CHAR_COUNT()) = value_handle;
        APPL_CHAR_GET_UUID (APPL_GET_CHAR_COUNT()) = uuid;
        APPL_GET_CHAR_COUNT()++;
    }

    if (1 == search_again)
    {
        if (value_handle < service_end_handle)
        {
            range.start_handle = value_handle + 1;
            range.end_handle = service_end_handle;

            appl_att_read_by_type
            (
                &range,
                GATT_CHARACTERISTIC
            );
        }
        else
        {
            search_again = 0;
        }
    }
    if (0 == search_again)
    {
        /* Initiate Descriptor Discovery */
        APPL_GET_CURRENT_CHAR_INDEX() = 0;
        appl_initiate_descriptor_discovery ();
    }
}

void appl_handle_read_by_group_response(UINT16 data_elem_size,
                                        UCHAR * data,
                                        UINT16 data_len)
{
    ATT_HANDLE_RANGE range;
    UINT32 index;
    UCHAR  * value;

    for (index = 0; index < (unsigned)(data_len / (data_elem_size)); index++ )
    {
        BT_UNPACK_LE_2_BYTE
        (
            &range.start_handle,
            (data + (index * data_elem_size))
        );

        BT_UNPACK_LE_2_BYTE
        (
            &range.end_handle,
            (data + ((index * data_elem_size) + 2))
        );
        CONSOLE_OUT(
        "Handle Range 0x%04X - 0x%04X\n", range.start_handle, range.end_handle);
//      printf (
//        "Handle Range 0x%04X - 0x%04X\n", range.start_handle, range.end_handle);
        CONSOLE_OUT("Handle Value Received - \n");
//      printf ("Handle Value Received - \n");
        value = data + (4 + index * data_elem_size);
        appl_dump_bytes(value, data_elem_size - 4);
    }
}


extern volatile int write_tx_complete_flag;
extern volatile UINT16 ota_mtu;

/* ------------------------------- ATT Callback Function */
/**
 *  This is a callback function registered with the ATT layer
 *  after successfully powering on the stack.
 *
 *  \param handle: Device Identifier.
 *  \param event_type: ATT specific Event Identifier.
 *  \param event_result: Result associated with the 'event_type'.
 *  \param event_data: Parameters for the 'event_type'.
 *  \param event_datalen: Length of the parameters for the 'event_type'.
 *
 *  \return   : API_SUCEESS on successful processing of the event.
 *              API_FAILURE otherwise
 */
API_RESULT appl_att_callback
           (
               ATT_HANDLE    * att_handle,
               UCHAR           event_type,
               API_RESULT      event_result,
               UCHAR         * event_data,
               UINT16          event_datalen
           )
{
    UINT16 attr_handle;
    UINT16 mtu;
    UCHAR op_code;
    UCHAR rsp_code;
    API_RESULT retval;
    UINT8    cmdRspData[MAX_LEN_PDU - 2];
    UINT16 arg_len;
    /*
    APPL_TRC (
    "[ATT]:[0x%02X]: Received ATT Event 0x%02X with result 0x%04X\n",
    *att_handle, event_type, event_result);
*/
    cmdRspData[0] = DIS_SVC_RSP;
    cmdRspData[1] = event_datalen + 8;
    cmdRspData[2] = 0;
    cmdRspData[3] = 0;  // command excutes status
    cmdRspData[4] = att_handle->device_id;
    cmdRspData[5] = att_handle->att_id;
    cmdRspData[6] = event_type;
    cmdRspData[7] = event_result & 0xFF;
    cmdRspData[8] = (event_result & 0xFF00) >> 8;
    cmdRspData[9] = event_datalen & 0xFF;
    cmdRspData[10] = (event_datalen & 0xFF00) >> 8;
    // this is the write response, do not send back all the data
    if ((cmdRspData[6] == 0x53) ||
        (cmdRspData[6] == 0x13))
    {
        cmdRspData[1] = 8;
    }
    else if (cmdRspData[6] == 0x1b)
    {
#if 1
        // this is notification, trim for bulk mode
        // only sending limited bytes (8 byte headers + 10 bytes data)
        UINT8 ll = 18;
        if (/*(bulk_mode == 1) && */(event_datalen > ll))
        {
            cmdRspData[1] = ll;
            memcpy (&cmdRspData[11], event_data, ll - 8);
        }
        else
#endif
        {
            memcpy (&cmdRspData[11], event_data, event_datalen);
        }
    }
    else
    {
        memcpy (&cmdRspData[11], event_data, event_datalen);
    }

    if ((bulk_mode == 1) && ((cmdRspData[6] == 0x53) || (cmdRspData[6] == 0x13)))
    {
        // no write response for bulk mode
    }
    else
    {
        CmdResponse(cmdRspData);
    }

//   appl_dump_bytes (event_data, event_datalen);

    switch(event_type)
    {
        case ATT_CONNECTION_IND:
            APPL_TRC (
            "[0x%02X]:Received Connection Indtication, Result 0x%04X!\n",
            *att_handle, event_result);
            appl_gatt_client_handle = *att_handle;
            /* appl_att_initiate_service_discovery (); */
            break;

        case ATT_DISCONNECTION_IND:

            appl_reset_gatt_dscvry_info ();

            APPL_PXM_INFO_INIT ();
            APPL_TRC (
            "[0x%02X]:Received Disconnection Indtication, Result 0x%04X!\n",
            *att_handle, event_result);
            break;

        case ATT_ERROR_RSP:
            BT_UNPACK_LE_1_BYTE(&op_code, event_data);
            BT_UNPACK_LE_2_BYTE(&attr_handle, event_data + 1);
            BT_UNPACK_LE_1_BYTE(&rsp_code, event_data + 3);
            APPL_TRC (
                "Received Error Response, for Op-Code 0x%02X for Handle 0x%04X, Rsp "
                "Code 0x%02X!\n", op_code, attr_handle, rsp_code);
            if ((ATT_READ_BY_TYPE_REQ == op_code) ||
               (ATT_READ_BY_GROUP_REQ == op_code))
            {
                appl_handle_error_response (event_data[3]);
            }
            break;

        case ATT_XCNHG_MTU_REQ:
            BT_UNPACK_LE_2_BYTE(&mtu, event_data);
            APPL_TRC (
            "Received Exchange MTU Request with Result 0x%04X. MTU Size "
            "= 0x%04X!\n", event_result, mtu);

            break;

        case ATT_XCHNG_MTU_RSP:
            if (NULL == event_data)
            {
                break;
            }

            BT_UNPACK_LE_2_BYTE(&mtu, event_data);
        ota_mtu = mtu >= 237 ? 237 : mtu;
            APPL_TRC (
            "Received Exchange MTU Response with Result 0x%04X. MTU Size "
            "= 0x%04X!\n", event_result, mtu);
            break;


        case ATT_READ_BY_TYPE_RSP:
            APPL_TRC (
            "Received Read BY Type Response with Result 0x%04X!\n",
            event_result);
            if (NULL == event_data)
            {
                break;
            }
            appl_handle_read_by_type_response
            (
                event_data[0],
                &event_data[1],
                event_datalen--
            );
            break;
        case ATT_FIND_INFO_RSP:
            APPL_TRC(
            "Received Find Information Response Opcode!\n");

            if (NULL == event_data)
            {
                break;
            }

            appl_handle_find_info_response
            (
                &event_data[1],
                event_datalen-1,
                event_data[0]
            );
            break;
        case ATT_FIND_BY_TYPE_VAL_RSP:
            APPL_TRC ("Received Find by Type Value Response Opcode!\n");
            if (NULL == event_data)
            {
                break;
            }
            /**
             * Received response contains less than the maximum possible
             * number of handle range that can be packed in the response.
             *
             * This is observed, as a BLE device will have only one GAP service.
             */
            appl_handle_find_by_type_val_rsp (event_data, event_datalen);

            break;
        case ATT_HANDLE_VALUE_NTF:
            // APPL_TRC ("Received HVN\n");
            BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
            // APPL_TRC ("Handle - 0x%04X\n", attr_handle);
            // APPL_TRC ("Handle Value Received - \n");
            appl_dump_bytes(event_data + 2, (event_datalen - 2));
            // APPL_TRC ("\n");

            BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
            CONSOLE_OUT("Handle - 0x%04X\r\n", attr_handle);
            CONSOLE_OUT("Handle Value Received - 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
            , event_data[2], event_data[3], event_data[4], event_data[5]);
            break;
        case ATT_HANDLE_VALUE_IND:
            APPL_TRC ("Received HVI\n");
            BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
            APPL_TRC ("Handle - 0x%04X\n", attr_handle);
            APPL_TRC ("Handle Value Received - \n");
            appl_dump_bytes(event_data + 2, (event_datalen-2));

            retval = BT_att_send_hndl_val_cnf (&appl_gatt_client_handle);
            (void)retval;
        case ATT_READ_BY_GROUP_RSP:
            if (NULL == event_data)
            {
                break;
            }
            APPL_TRC ("Received Read Group Type Response Opcode!\n");
            arg_len = event_data[0];

            event_datalen --;
            event_data++;

            appl_handle_read_by_group_response(arg_len, event_data, event_datalen);
//          printf ("\n");
            break;
        case ATT_WRITE_CMD_TX_COMPLETE:
//          APPL_TRC ("Received Write Command Tx Complete (Locally generated)\n");
//          printf("Received Write Command Tx Complete (Locally generated)\n");
            write_tx_complete_flag = 1;
//          appl_dump_bytes(event_data, event_datalen);
            break;
        default:
            break;
    }

    APPL_TRC("\n\n");

    return API_SUCCESS;
}


/* ------------------------------- SMP Callback Function */
/**
 *  This is a callback function registered with the SMP layer
 *  after successfully powering on the stack.
 *
 *  \param bd_handle: Device Identifier.
 *  \param event_type: SMP specific Event Identifier.
 *  \param event_result: Result associated with the 'event_type'.
 *  \param event_data: Parameters for the 'event_type'.
 *  \param event_datalen: Length of the parameters for the 'event_type'.
 *
 *  \return   : API_SUCEESS on successful processing of the event.
 *              API_FAILURE otherwise
 */
API_RESULT appl_smp_callback
           (
               /* IN */ SMP_BD_HANDLE   * bd_handle,
               /* IN */ UCHAR             event_type,
               /* IN */ UINT16            event_result,
               /* IN */ void            * event_data,
               /* IN */ UINT16            event_datalen
           )
{
    API_RESULT retval;

#ifdef SMP_SLAVE
    UINT16   ediv;
    UCHAR  * peer_rand;
    UCHAR    ltk[SMP_LTK_SIZE];
    UCHAR * data_param;
#endif /* SMP_SLAVE */

    SMP_KEY_DIST * key_info;
    SMP_AUTH_INFO * auth;
    SMP_BD_ADDR bdaddr;

    SMP_KEY_XCHG_PARAM * kx_param;

    UCHAR * bd_addr;
    UCHAR   bd_addr_type;
    UINT8    cmdRspData[MAX_LEN_PDU - 2];

    /* Get the BD Address from handle */
    BT_smp_get_bd_addr (bd_handle, &bdaddr);

    bd_addr = bdaddr.addr;
    bd_addr_type = bdaddr.type;
    // APPL_TRC("\n smp callback, event:0x%x\n", event_type);
    switch(event_type)
    {
        case SMP_AUTHENTICATION_COMPLETE:
            APPL_TRC (
            "\nRecvd SMP_AUTHENTICATION_COMPLETE\n");
            CONSOLE_OUT("\r\nRecvd SMP_AUTHENTICATION_COMPLETE.\r\n"
            "Status : %04X\r\n", event_result );
            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
            bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            APPL_TRC (
            "BD addr type : %s\n",
            (0 == bd_addr_type)? "Public Address": "Random Address");
            APPL_TRC (
            "Status : %04X\n", event_result);

            if (NULL != event_data)
            {
                if (API_SUCCESS == event_result)
                {
                    auth = (SMP_AUTH_INFO *)event_data;

                    APPL_TRC (
                    "Authentication type : %s\n",
                    (SMP_SEC_LEVEL_2 == (auth->security & 0x0F))?  "With MITM":
                    "Encryption Only (without MITM)");

                    APPL_TRC (
                    "Bonding type : %s\n",
                    (auth->bonding)? "Bonding": "Non-Bonding");

                    APPL_TRC (
                    "Encryption Key size : %d\n", auth->ekey_size);

                }
            }
            else
            {
                if (API_SUCCESS == event_result)
                {
                    APPL_TRC("\nConfirmed Authentication using Encryption\n");

                }
            }

            cmdRspData[0] = BOND_RSP;
            cmdRspData[1] = 2; // length
            cmdRspData[2] = 0;
            memcpy(&cmdRspData[3], &event_result, sizeof(event_result));
            CmdResponse(cmdRspData);
            break;

        case SMP_AUTHENTICATION_REQUEST:
            APPL_TRC (
            "\nRecvd SMP_AUTHENTICATION_REQUEST\n");
            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
            bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            APPL_TRC (
            "BD addr type : %s\n",
            (0 == bd_addr_type)? "Public Address": "Random Address");

            auth = (SMP_AUTH_INFO *)event_data;

            APPL_TRC (
            "Authentication type : %s\n",
            (SMP_SEC_LEVEL_2 == (auth->security & 0x0F))?  "With MITM":
            "Encryption Only (without MITM)");

            APPL_TRC (
            "Bonding type : %s\n",
            (auth->bonding)? "Bonding": "Non-Bonding");

            APPL_TRC (
            "Encryption Key size : %d\n", auth->ekey_size);

            /**
             * Performing automatic action, in the sample application.
             *
             * In devices with appropriate MMI, user confirmation
             * can be requested.
             */
            {
                /* Accept Authentication */
                auth->param = SMP_ERROR_NONE;

                APPL_TRC (
                "\n\nSending +ve Authentication request reply.\n");
                retval = BT_smp_authentication_request_reply
                         (
                             bd_handle,
                             auth
                         );
            }
            break;

        case SMP_PASSKEY_ENTRY_REQUEST:
            APPL_TRC (
            "\nEvent   : SMP_PASSKEY_ENTRY_REQUEST\n");
            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
            bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            APPL_TRC (
            "BD addr type : %s\n",
            (0 == bd_addr_type)? "Public Address": "Random Address");
            break;

        case SMP_PASSKEY_DISPLAY_REQUEST:
            APPL_TRC (
            "\nEvent   : SMP_PASSKEY_DISPLAY_REQUEST\n");
            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
            bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            APPL_TRC (
            "BD addr type : %s\n",
            (0 == bd_addr_type)? "Public Address": "Random Address");

            APPL_TRC (
            "Passkey : %06u", *((UINT32 *)event_data));

            break;

        case SMP_KEY_EXCHANGE_INFO_REQUEST:
            APPL_TRC (
            "\nEvent   : SMP_KEY_EXCHANGE_INFO_REQUEST\n");
            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
            bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            APPL_TRC (
            "BD addr type : %s\n",
            (0 == bd_addr_type)? "Public Address": "Random Address");

            /* Reference the event data */
            kx_param = (SMP_KEY_XCHG_PARAM *) event_data;

            APPL_TRC("Local keys negotiated - 0x%02X\n", kx_param->keys);
            APPL_TRC("Encryption Key Size negotiated - 0x%02X\n",
                    kx_param->ekey_size);

            /**
             * Performing automatic action, in the sample application.
             *
             * In devices with appropriate MMI, user confirmation
             * can be requested.
             */
            {
                /* Get platform data of key informations */
                BT_smp_get_key_exchange_info_pl (&key_info);
                BT_smp_key_exchange_info_request_reply (bd_handle, key_info);
            }
            break;

#ifdef SMP_SLAVE
        case SMP_LONG_TERM_KEY_REQUEST:

            /* Copy parameters to local variables */
            data_param = (UCHAR *)event_data;
            smp_unpack_2_byte_param(&ediv, &data_param[8]);
            peer_rand = event_data;

            APPL_TRC (
            "\nEvent   : SMP_LONG_TERM_KEY_REQUEST\n");
            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
            bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            APPL_TRC (
            "BD addr type : %s\n",
            (0 == bd_addr_type)? "Public Address": "Random Address");
            APPL_TRC (
            "Div  : 0x%04X\n", ediv);
            APPL_TRC (
            "Rand : %02X %02X %02X %02X %02X %02X %02X %02X\n",
            peer_rand[0], peer_rand[1], peer_rand[2], peer_rand[3],
            peer_rand[4], peer_rand[5], peer_rand[6], peer_rand[7]);

            /**
             * Performing automatic action, in the sample application.
             *
             * In devices with appropriate MMI, user confirmation
             * can be requested.
             */
            {
                /* Get LTK for remote device */
                retval = BT_smp_get_long_term_key_pl
                            (
                                peer_rand,
                                ediv,
                                ltk
                             );

                if ( API_SUCCESS == retval )
                {
                    APPL_TRC("\n\nSending +ve LTK request reply.\n");
                    retval = BT_smp_long_term_key_request_reply
                             (
                                 bd_handle,
                                 ltk,
                                 SMP_TRUE
                             );
                }
                else
                {
                    APPL_TRC("\n\nSending -ve LTK request reply.\n");
                    retval = BT_smp_long_term_key_request_reply
                             (
                                 bd_handle,
                                 NULL,
                                 SMP_FALSE
                             );
                }
            }
            break;
#endif /* SMP_SLAVE */

        case SMP_RESOLVABLE_PVT_ADDR_CREATE_CNF:
            APPL_TRC (
            "\nEvent   : SMP_RESOLVABLE_PVT_ADDR_CREATE_CNF\n");
            APPL_TRC (
            "Status : %04X\n", event_result);
            if ( API_SUCCESS == event_result );
            {
                bd_addr = (UCHAR *) event_data;

                APPL_TRC (
                "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            }
            break;

        case SMP_RESOLVABLE_PVT_ADDR_VERIFY_CNF:
            APPL_TRC (
            "\nEvent   : SMP_RESOLVABLE_PVT_ADDR_VERIFY_CNF\n");
            APPL_TRC (
            "Status : %04X\n", event_result);
            if ( API_SUCCESS == event_result );
            {
                bd_addr = (UCHAR *) event_data;

                APPL_TRC (
                "BD Address : %02X %02X %02X %02X %02X %02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            }
            break;

#ifdef SMP_DATA_SIGNING
        case SMP_DATA_SIGNING_COMPLETE:
            APPL_TRC("\nEvent   : SMP_SIGNING_DATA_COMPLETE\n");
            APPL_TRC("Status : %04X\n", event_result);
            if ( API_SUCCESS == event_result )
            {
                APPL_TRC("Message Authentication Code : ");
                appl_dump_bytes
                (
                    event_data,
                    event_datalen
                );
            }
            break;

        case SMP_SIGN_DATA_VERIFICATION_COMPLETE:
            APPL_TRC("\nEvent   : SMP_SIGN_DATA_VERIFICATION_COMPLETE\n");
            APPL_TRC("Status : %04X\n", event_result);

            APPL_TRC("Message Authentication Code : ");
            appl_dump_bytes
            (
                event_data,
                event_datalen
            );
            break;
#endif /* SMP_DATA_SIGNING */

        case SMP_KEY_EXCHANGE_INFO:
            APPL_TRC (
            "\nRecvd SMP_KEY_EXCHANGE_INFO\n");
            APPL_TRC (
            "Status - 0x%04X\n", event_result);

            /* Reference the event data */
            kx_param = (SMP_KEY_XCHG_PARAM *) event_data;

            APPL_TRC("Remote keys negotiated - 0x%02X\n", kx_param->keys);
            APPL_TRC("Encryption Key Size negotiated - 0x%02X\n",
                    kx_param->ekey_size);

            /* Reference the key information */
            key_info = kx_param->keys_info;

            APPL_TRC (
            "Encryption Info:\n");
            appl_dump_bytes(key_info->enc_info, sizeof (key_info->enc_info));
            APPL_TRC (
            "Master Identification Info:\n");
            appl_dump_bytes(key_info->mid_info, sizeof (key_info->mid_info));
            APPL_TRC (
            "Identity Info:\n");
            appl_dump_bytes(key_info->id_info, sizeof (key_info->id_info));
            APPL_TRC (
            "Identity Address Info:\n");
            appl_dump_bytes
            (
                key_info->id_addr_info,
                sizeof (key_info->id_addr_info)
            );

            APPL_TRC (
            "Signature Info:\n");
            appl_dump_bytes(key_info->sign_info, sizeof (key_info->sign_info));

            break;

        default:
            APPL_ERR (
            "\nERROR!!! Received unknown event. event = 0x%02X\n", event_type);
    }

    (void)retval;

    return API_SUCCESS;
}


/* ------------------------------- Utility Functions */
void appl_dump_bytes (UCHAR *buffer, UINT16 length)
{

#if 0
    char hex_stream[49];
    char char_stream[17];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;

    APPL_TRC ("\n");
    APPL_TRC (
    "-- Dumping %d Bytes --\n",
    (int)length);

    APPL_TRC (
    "-------------------------------------------------------------------\n");

    count = 0;
    offset = 0;
    for ( i = 0; i < length; i++ )
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

        if ( 16 == count )
        {
            char_stream[count] = '\0';
            count = 0;
            offset = 0;

            APPL_TRC("%s   %s\n",
            hex_stream, char_stream);

            BT_mem_set(hex_stream, 0, 49);
            BT_mem_set(char_stream, 0, 17);
        }
    }

    if ( offset != 0 )
    {
        char_stream[count] = '\0';

        /* Maintain the alignment */
        APPL_TRC (
        "%-48s   %s\n",
        hex_stream, char_stream);
    }

    APPL_TRC (
    "-------------------------------------------------------------------\n");

    APPL_TRC ("\n");
#endif
    return;
}

/* ------------------------------- ATT related Functions */

/**
 *  Function to initiate Service Discovery for a specific service
 *  represented by the 'uuid'.
 *
 *  \param uuid: Universally Unique Identifier (16 or 128 bit).
 *  \param uuid_frmt: Identifier as 16 or 128 bit uuid.
 *
 *  \return   : API_SUCEESS on successful processing of the event.
 *              Appropriate error code otherwise.
 */

#define APPL_INVALID_UUID_FORMAT   (0x00CD | APPL_ERR_ID)

API_RESULT appl_discover_service
           (
               /* IN */ ATT_UUID           uuid,
               /* IN */ UCHAR              uuid_frmt
           )
{
    ATT_HANDLE_RANGE hdl;
    hdl.start_handle = 0x0001;
      hdl.end_handle = 0xffff;
    UINT16 uuid16 = uuid.uuid_16;
      appl_att_read_by_group_type(&hdl, uuid16);
    return 0;
}

API_RESULT appl_discover_all_service
           (
               /* IN */ UINT16            start_handle,
               /* IN */ UINT16            end_handle,
                        UINT16                    uuid,
               /* IN */ UCHAR             uuid_frmt
           )
{
    ATT_HANDLE_RANGE hdl;
    hdl.start_handle = start_handle;
      hdl.end_handle = end_handle;
    UINT16 uuid16 = uuid;
      appl_att_read_by_group_type(&hdl, uuid16);
    return 0;
}

API_RESULT appl_discover_characteristics
           (
               /* IN */ ATT_UUID           uuid,
               /* IN */ UCHAR              uuid_frmt
           )
{
    ATT_FIND_BY_TYPE_VAL_REQ_PARAM    find_by_type_val_pram;
    UCHAR                             value[16];
    API_RESULT                        retval;

    find_by_type_val_pram.range.start_handle = ATT_ATTR_HANDLE_START_RANGE;
    find_by_type_val_pram.range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
    find_by_type_val_pram.uuid = GATT_PRIMARY_SERVICE;

    if ( ATT_128_BIT_UUID_FORMAT == uuid_frmt )
    {
        BT_PACK_LE_16_BYTE (value, (const void*)(&(uuid.uuid_128)));

        find_by_type_val_pram.value.val = value;
        find_by_type_val_pram.value.len = 16;
    }
    else
    {
        BT_PACK_LE_2_BYTE (value, &(uuid.uuid_16));

        find_by_type_val_pram.value.val = value;
        find_by_type_val_pram.value.len = 2;
    }



    retval = BT_att_send_find_by_type_val_req
             (
                  &appl_gatt_client_handle,
                  &find_by_type_val_pram
             );
    return retval;
}

API_RESULT appl_discover_all_characteristics
           (
               /* IN */ UINT16            start_handle,
               /* IN */ UINT16            end_handle,
                        UINT16                    uuid,
               /* IN */ UCHAR             uuid_frmt
           )
{
    ATT_HANDLE_RANGE hdl;
    hdl.start_handle = start_handle;
      hdl.end_handle = end_handle;
    UINT16 uuid16 = uuid;
      appl_att_read_by_type(&hdl, uuid16);
    return 0;
}


/**
 *  Function to initiate Service Discovery for GAP.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_gap_service(void)
{
    ATT_UUID             uuid;

    uuid.uuid_16 = GATT_GAP_SERVICE;

    /* Discover GAP Service */
    appl_discover_service
    (
        uuid,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/**
 *  Function to initiate Service Discovery for Link Loss.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_link_loss_service(void)
{
    ATT_UUID             uuid;

    uuid.uuid_16 = GATT_LINK_LOSS_SERVICE;

    /* Discover Link Loss Service */
    appl_discover_service
    (
        uuid,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/**
 *  Function to initiate Service Discovery for Immediate Alert.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_immdt_alert_service(void)
{
    ATT_UUID             uuid;

    uuid.uuid_16 = GATT_IMMEDIATE_ALERT_SERVICE;

    /* Discover IA Service */
    appl_discover_service
    (
        uuid,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/**
 *  Function to initiate Service Discovery for Tx Power.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_tx_power_service(void)
{
    // ATT_UUID             uuid;
    DECL_STATIC ATT_UUID amota_service_uuid128 = {.uuid_128.value = {0x01, 0x10, 0x2E, 0xC7, 0x8A, 0x0E, 0x73, 0x90, 0xE1, 0x11, 0xC2, 0x08, 0x60, 0x27, 0x00, 0x00}};
    // uuid.uuid_16 = GATT_TX_POWER_SERVICE;

    /* Discover Tx Power Service */
    appl_discover_service
    (
        amota_service_uuid128,
        ATT_128_BIT_UUID_FORMAT
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
    ATT_UUID             uuid;

    uuid.uuid_16 = GATT_BATTERY_SERVICE;

    /* Discover Battery Service */
    appl_discover_service
    (
        uuid,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/* ------------------------------- Dummy Functions */
#include "fsm_defines.h"
DECL_CONST DECL_CONST_QUALIFIER STATE_EVENT_TABLE appl_state_event_table[] =
{
    {
        /*0*/ 0x00,
        0,
        0
    }
};

DECL_CONST DECL_CONST_QUALIFIER EVENT_HANDLER_TABLE appl_event_handler_table[] =
{
    {
        /*0*/ 0x0000,
        NULL
    }
};

API_RESULT appl_access_state_handler
           (
                void       * param,
                STATE_T    * state
           )
{
    return API_SUCCESS;
}

char * appl_hci_get_command_name (UINT16 opcode)
{
    return "DUMMY";
}

#ifdef BT_SW_TXP
void appl_message_handler(BT_MSG_T *btmsg)
{
    host_task_handler(btmsg);

    return;
}
#endif /* BT_SW_TXP */

#ifndef BT_SW_TXP

void appl_att_read_by_group_type
    (
/* IN */ ATT_HANDLE_RANGE * range,
/* IN */ UINT16   uuid
  )
{
     ATT_READ_BY_GROUP_TYPE_REQ_PARAM    find_info_param;
     API_RESULT   retval;
    // printf ("Searching for UUID 0x%04X, from handle 0x%04X to 0x%04X\n",    uuid, range->start_handle, range->end_handle);
     find_info_param.range = *range;
     find_info_param.group_type.uuid_16 = uuid;
     find_info_param.uuid_format = ATT_16_BIT_UUID_FORMAT;
    #if 0
     peer_info.state.handle = range->end_handle;
     peer_info.state.uuid = uuid;
    #endif
     retval = BT_att_send_read_by_group_req             (
        &appl_gatt_client_handle,
        &find_info_param
        );
     APPL_TRC(    "[ATT]: Read by Type Request Initiated with result 0x%04X", retval);
}


void appl_read_by_group()
{
#if 0
    ATT_READ_BY_GROUP_TYPE_REQ_PARAM read_by_group_param;

    int data;
    int index;
    API_RESULT retval;


    CONSOLE_OUT("Enter start handle[HEX] : ");
    CONSOLE_IN("%x", &data);
//  scanf("%x", &data);
    read_by_group_param.range.start_handle = (ATT_ATTR_HANDLE) data;
    CONSOLE_OUT("Enter end handle[HEX] : ");
    CONSOLE_IN("%x", &data);
//  scanf("%x", &data);
    read_by_group_param.range.end_handle = (ATT_ATTR_HANDLE) data;
#ifdef ATT_SUPPORT_128_BIT_UUID
    CONSOLE_OUT("Enter UUID Format type\n\t 1 - 16\n\t 2 - 128 Bit\n");
    CONSOLE_IN("%x", &data);
//  scanf("%x", &data);
    read_by_group_param.uuid_format = (UCHAR) data;
    if (ATT_16_BIT_UUID_FORMAT == read_by_group_param.uuid_format)
    {
        CONSOLE_OUT("Enter 16 Bit UUID[HEX] : ");
        CONSOLE_IN("%x", &data);
//      scanf("%x", &data);
        read_by_group_param.group_type = (UINT16) data;
    }
    else if (ATT_128_BIT_UUID_FORMAT == read_by_group_param.uuid_format)
    {
        CONSOLE_OUT("Enter 128 Bit UUID[HEX] : ");
        for (index =0; index < ((0 == uuid_type)? 2 : 16); index ++)
        {
//          scanf("%x", &data);
            CONSOLE_IN("%x", &data);
            uuid[index] = (UCHAR) data;
        }
    }
#else /* ATT_SUPPORT_128_BIT_UUID */
//  scanf("%x", &data);
    CONSOLE_OUT("Enter 16 Bit UUID[HEX] : ");
    CONSOLE_IN("%x", &data);
    read_by_group_param.group_type.uuid_16= (UINT16) data;
#endif /* ATT_SUPPORT_128_BIT_UUID */
    retval = BT_att_send_read_by_group_req
             (
                 &appl_att_client_handle,
                 &read_by_group_param
             );

    APPL_TRC
    ("ATT Read By Group Type Request sent with result 0x%04X\n",
    retval);
#else
    ATT_HANDLE_RANGE hdl;
    hdl.start_handle = 0x0001;
      hdl.end_handle = 0xffff;
      ATT_UUID uuid;
      uuid.uuid_16 = 0x2800;
    appl_att_read_by_group_type(&hdl, uuid.uuid_16);
#endif
}

#else
void appl_bluetooth_on(void)
{
     /**
     *  Turn on the Bluetooth service.
     *
     *  appl_hci_callback - is the application callback function that is
     *  called when any HCI event is received by the stack
     *
     *  appl_bluetooth_on_complete_callback - is called by the stack when
     *  the Bluetooth service is successfully turned on
     *
     *  Note: After turning on the Bluetooth Service, stack will call
     *  the registered bluetooth on complete callback.
     *  This sample application will initiate Scanning to look for devices
     *  in vicinity [Step 2(a)] from bluetooth on complete callback
     *  'appl_bluetooth_on_complete_callback'
     */
    BT_bluetooth_on
    (
        appl_hci_callback,
        appl_bluetooth_on_complete_callback,
        "EtherMind-PXM"
    );

}

void appl_bluetooth_off(void)
{
    BT_bluetooth_off ();
}

void appl_set_scan_parameters(UCHAR read)
{
    UCHAR scan_filter_policy;
    API_RESULT retval;

    scan_filter_policy = (UCHAR)((read)?
                         APPL_GAP_GET_WHITE_LIST_SCAN_FILTER_POLICY():
                         APPL_GAP_GET_NON_WHITE_LIST_SCAN_FILTER_POLICY());

    /**
    *  Step 2(a). Initiate Scanning.
    *
    *  The operation is performed in two parts.
    *  First set the scan parameters.
    *  On completion of set scan parameter, enable scanning.
    *
    *  Completion of set scan parameter is informed through
    *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback 'appl_hci_callback'.
    *  Look for the handling of opcode
    *  'HCI_LE_SET_SCAN_PARAMETERS_OPCODE'.
    */
    retval = BT_hci_le_set_scan_parameters
             (
                 APPL_GAP_GET_SCAN_TYPE(),
                 APPL_GAP_GET_SCAN_INTERVAL(),
                 APPL_GAP_GET_SCAN_WINDOW(),
                 APPL_GAP_GET_OWN_ADDR_TYPE_IN_SCAN(),
                 scan_filter_policy
             );

    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "Failed to set scan parameters. Result = 0x%04X\n", retval);
    }
}

void appl_set_scan_mode(UCHAR mode)
{
    BT_hci_le_set_scan_enable (mode, 1);
}


void appl_create_connection(void)
{
    API_RESULT retval;

    /* Stop Scanning */
    BT_hci_le_set_scan_enable (0, 1);

    retval = BT_hci_le_create_connection
             (
                 APPL_GAP_GET_CONN_SCAN_INTERVAL(),
                 APPL_GAP_GET_CONN_SCAN_WINDOW(),
                 APPL_GAP_GET_CONN_INITIATOR_FILTER_POLICY(),
                 g_peer_bd_addr.type,
                 g_peer_bd_addr.addr,
                 APPL_GAP_GET_OWN_ADDR_TYPE_AS_INITIATOR(),
                 APPL_GAP_GET_CONN_INTERVAL_MIN(),
                 APPL_GAP_GET_CONN_INTERVAL_MAX(),
                 APPL_GAP_GET_CONN_LATENCY(),
                 APPL_GAP_GET_CONN_SUPERVISION_TIMEOUT(),
                 APPL_GAP_GET_CONN_MIN_CE_LENGTH(),
                 APPL_GAP_GET_CONN_MAX_CE_LENGTH()
             );

    /* If fails, initiate scanning again */
    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "Failed to Create LE connection with %02X:%02X:%02X:%02X::%02X"
        ":%02X, reason 0x%04X\n", g_peer_bd_addr.addr[5],
        g_peer_bd_addr.addr[4], g_peer_bd_addr.addr[3], g_peer_bd_addr.addr[2],
        g_peer_bd_addr.addr[1], g_peer_bd_addr.addr[0], retval);

        BT_hci_le_set_scan_enable (1, 1);
    }
}

void appl_bond_with_peer(void)
{
    /* If connection is successful, initiate bonding [Step 2(c)] */
    API_RESULT retval;
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
    UINT16 connection_handle;
    API_RESULT retval;

    /*  Initiate Connection Update from Master. [Step 2(d)] */
    retval = BT_hci_get_le_connection_handle
             (
                 &g_peer_bd_addr,
                 &connection_handle
             );

    /**
     *  Completion of connection update is informed through
     *  'HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT' in HCI Callback
     *  'appl_hci_callback'.
     */
    retval = BT_hci_le_connection_update
             (
                  connection_handle,
                  APPL_GAP_GET_CONN_INTERVAL_MIN_FOR_CU(),
                  APPL_GAP_GET_CONN_INTERVAL_MAX_FOR_CU(),
                  APPL_GAP_GET_CONN_LATENCY_FOR_CU(),
                  APPL_GAP_GET_CONN_SUPERVISION_TIMEOUT_FOR_CU(),
                  APPL_GAP_GET_CONN_MIN_CE_LENGTH_FOR_CU(),
                  APPL_GAP_GET_CONN_MAX_CE_LENGTH_FOR_CU()
             );

    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "Failed to initiate Connection Update. Reason 0x%04X\n",
        retval);
    }
}

void appl_add_dev_to_white_list(BT_DEVICE_ADDR * peer_div_addr)
{
    API_RESULT retval;

    retval = BT_hci_le_add_device_to_white_list
             (
                 peer_div_addr->type,
                 peer_div_addr->addr
             );
    /* Setting the global Peer Dev Address */

    BT_COPY_BD_ADDR(g_peer_bd_addr.addr, peer_div_addr->addr);
    BT_COPY_TYPE(g_peer_bd_addr.type, peer_div_addr->type);

    if (API_SUCCESS != retval)
    {
        APPL_TRC ("Failed to Add in White List. Error 0x%04X\n",
        retval);
    }
}

void appl_disconnect_device(void)
{
    BT_hci_disconnect (appl_ble_connection_handle, 0x13);
}

void appl_set_adv_data(void)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_set_adv_params(void)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}

void appl_set_adv_mode(UCHAR mode)
{
    CONSOLE_OUT("Function Not Supported in this Role\n");
}
#endif /* BT_SW_TXP */






