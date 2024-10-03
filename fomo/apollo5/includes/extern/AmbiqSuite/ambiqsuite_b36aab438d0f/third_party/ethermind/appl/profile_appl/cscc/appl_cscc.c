
/**
 *  \file appl_cscc.c
 *
 *  This is a stand-alone Cycling Speed & Cadence Collector application.
 *
 *  The application illustrates the usage of host stack APIs
 *  for a BLE master (initiator of connection) implementation through
 *  operations like, stack initialization, device discovery,
 *  connection management, bonding and service discovery.
 *
 *  Control flow of this application is listed below.
 *  Step 1. Initialize and power on the stack
 *
 *  Step 2. Play the master role
 *          (a) Start Scanning (look for device in vicinity)
 *          (b) Initiate Connection with peer device
 *          (c) On connection complete, initiate bonding
 *          (d) On bonding complete, initiate connection update
 *          (e) On connection update complete,
 *              initiate service discovery for GAP service
 *          (f) Disconnect on service discovery completion
 *          (g) On disconnection complete, start scanning again, with whitelist
 *          (h) On connection complete, initiate encryption
 *              (to ensure device is still bonded)
 *          (i) On encryption complete (succeeds or fails), disconnect (TBD)
 *
 *  Step 3. Power off the stack
 *          # End of demonstration application
 *
 *  Note: main() is the entry point of this sample application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_att_api.h"
#include "BT_smp_api.h"
#include "smp_pl.h"
#include "l2cap.h"
#include "gatt_defines.h"

#ifdef BT_ANALYZER
#include "analyzer_interface.h"
#endif /* BT_ANALYZER */

#ifdef HCI_TX_RUN_TIME_SELECTION
#ifdef BT_USB
#include "hci_usb.h"
#endif /* BT_USB */

#ifdef BT_UART
#include "hci_uart.h"
#endif /* BT_UART */

#ifdef BT_SOCKET
#include "hci_socket.h"
#endif /* BT_SOCKET */
#endif /* HCI_TX_RUN_TIME_SELECTION */

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
#include "host_interface.h"
#endif /* BT_SW_TXP */

#ifndef DONT_USE_STANDARD_IO
#define CONSOLE_OUT  APPL_TRC
#define CONSOLE_IN   scanf
#else
#define CONSOLE_OUT  console_printf
#define CONSOLE_IN
#endif /* DONT_USE_STANDARD_IO */
#endif /* APPL_MENU_OPS */

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
#define DEVICE_CONNECTED     0x00
#define CONNECTION_INITIATED 0x00
#define APPL_SET_STATE(mask) 0x00
#define APPL_GET_STATE(mask) 0x00

#ifndef DONT_USE_STANDARD_IO
#define APPL_TRC printf
#define APPL_ERR printf
#else
#define APPL_TRC(...)
#define APPL_ERR(...)
#endif /* DONT_USE_STANDARD_IO */

/* ----------------------------------------- External Global Variables */

/* TBD: Re-arrange function definition. Avoid explicit function declaration. */
void appl_poweron_stack(void);
void appl_master_menu_handler(void);
API_RESULT appl_bluetooth_on_complete_callback (void);
API_RESULT appl_register_callback(void);
API_RESULT appl_hci_callback
           (
               UCHAR    event_type,
               UCHAR  * event_data,
               UCHAR    event_datalen
           );
void appl_l2cap_callback
     (
         DEVICE_HANDLE * handle,
         UCHAR           event_type,
         UCHAR         * event_data,
         UINT16          event_datalen
     );
API_RESULT appl_att_callback
           (
               ATT_HANDLE    * handle,
               UCHAR           event_type,
               API_RESULT      event_result,
               UCHAR         * event_data,
               UINT16          event_datalen
           );
API_RESULT appl_smp_callback
           (
               /* IN */ SMP_BD_HANDLE   * bd_handle,
               /* IN */ UCHAR             event_type,
               /* IN */ UINT16            event_result,
               /* IN */ void            * event_data,
               /* IN */ UINT16            event_datalen
           );

API_RESULT appl_discover_service
           (
               /* IN */ ATT_UUID16          * uuid,
               /* IN */ UCHAR                 uuid_frmt
           );
void appl_discover_gap_service(void);

void appl_dump_bytes (UCHAR *buffer, UINT16 length);
char * appl_hci_get_command_name (UINT16 opcode);
const char * appl_get_profile_menu (void);

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */
/* Application specific information used to register with ATT */
DECL_STATIC ATT_APPLICATION att_app;

/* GATT Handle */
DECL_STATIC ATT_HANDLE appl_gatt_client_handle;

/* BLE Connection Handle */
DECL_STATIC UINT16 appl_ble_connection_handle;

typedef struct
{
    ATT_ATTR_HANDLE    start_handle;

    ATT_UUID16         uuid;
}APPL_CHARACTERISTIC;

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

/** CSCC Characteristic related information */
typedef struct
{
    /* DIS - Device Information Service */

    /* Manufacturer Name */
    ATT_ATTR_HANDLE dis_manufacturer_name_hdl;

    /* Model Number */
    ATT_ATTR_HANDLE dis_model_number_hdl;

    /* Serial Number */
    ATT_ATTR_HANDLE dis_serial_number_hdl;

    /* Hardware Revision */
    ATT_ATTR_HANDLE dis_hardware_revision_hdl;

    /* Firmware Revision */
    ATT_ATTR_HANDLE dis_firmware_revision_hdl;

    /* Software Revision */
    ATT_ATTR_HANDLE dis_software_revision_hdl;

    /* System Id */
    ATT_ATTR_HANDLE dis_sys_id_hdl;

    /* RegCertDataList */
    ATT_ATTR_HANDLE dis_reg_cert_data_list_hdl;

    /* CSCS - Runnig Speed and Cadence Service */

    /* CSC Measurement */
    ATT_ATTR_HANDLE cscs_csc_msrmnt_hdl;

    /* CSC Measurement - CCC Handle */
    ATT_ATTR_HANDLE cscs_csc_msrmnt_ccc_hdl;

    /* CSC Feature */
    ATT_ATTR_HANDLE cscs_csc_feature_hdl;

    /* Sensor Location */
    ATT_ATTR_HANDLE cscs_snsr_loc_hdl;

    /* Speed and Cadence Control Point */
    ATT_ATTR_HANDLE cscs_sc_cp_hdl;

    /* Speed and Cadence Control Point - CCC Handle */
    ATT_ATTR_HANDLE cscs_sc_cp_ccc_hdl;

    /* BAS - Battery Service */

    /* Battery Level */
    ATT_ATTR_HANDLE bas_batt_level_hdl;
} CSCC_CHAR_INFO;

#define APPL_MAX_CHARACTERISTICS         16
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

/* DIS - Device Information Service */

/* Manufacturer Name */
#define DIS_MANUFACTURER_NAME_HDL     (cscc_char_info.dis_manufacturer_name_hdl)

/* Model Number */
#define DIS_MODEL_NUMBER_HDL          (cscc_char_info.dis_model_number_hdl)

/* Serial Number */
#define DIS_SERIAL_NUMBER_HDL         (cscc_char_info.dis_serial_number_hdl)

/* Hardware Revision */
#define DIS_HARDWARE_REVISION_HDL     (cscc_char_info.dis_hardware_revision_hdl)

/* Firmware Revision */
#define DIS_FIRMWARE_REVISION_HDL     (cscc_char_info.dis_firmware_revision_hdl)

/* Software Revision */
#define DIS_SOFTWARE_REVISION_HDL     (cscc_char_info.dis_software_revision_hdl)

/* System Id */
#define DIS_SYSTEM_ID_HDL             (cscc_char_info.dis_sys_id_hdl)

/* RegCertDataList */
#define DIS_REG_CERT_DATA_LIST_HDL    (cscc_char_info.dis_reg_cert_data_list_hdl)

/* CSCS - Cycling Speed and Cadence Service */

/* CSC Measurement */
#define CSCS_CSC_MSRMNT_HDL           (cscc_char_info.cscs_csc_msrmnt_hdl)

/* CSC Measurement - CCC Handle */
#define CSCS_CSC_MSRMNT_CCC_HDL       (cscc_char_info.cscs_csc_msrmnt_ccc_hdl)

/* CSC Feature */
#define CSCS_CSC_FEATURE_HDL          (cscc_char_info.cscs_csc_feature_hdl)

/* Sensor Location */
#define CSCS_SNSR_LOC_HDL             (cscc_char_info.cscs_snsr_loc_hdl)

/* Speed and Cadence Control Point */
#define CSCS_SC_CP_HDL                (cscc_char_info.cscs_sc_cp_hdl)

/* Record Access Control Point - CCC Handle */
#define CSCS_SC_CP_CCC_HDL            (cscc_char_info.cscs_sc_cp_ccc_hdl)

/* BAS - Battery Service */

/* Battery Level */
#define BAS_BATTERY_LEVEL_HDL        (cscc_char_info.bas_batt_level_hdl)

#define APPL_CLI_CNFG_VAL_LENGTH    2

#define APPL_CSCC_INFO_INIT()\
        DIS_MANUFACTURER_NAME_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_MODEL_NUMBER_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_SERIAL_NUMBER_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_HARDWARE_REVISION_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_FIRMWARE_REVISION_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_SOFTWARE_REVISION_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_SYSTEM_ID_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        DIS_REG_CERT_DATA_LIST_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        CSCS_CSC_MSRMNT_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        CSCS_CSC_MSRMNT_CCC_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        CSCS_CSC_FEATURE_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        CSCS_SNSR_LOC_HDL  = ATT_INVALID_ATTR_HANDLE_VAL;\
        CSCS_SC_CP_HDL = ATT_INVALID_ATTR_HANDLE_VAL;\
        CSCS_SC_CP_CCC_HDL = ATT_INVALID_ATTR_HANDLE_VAL;

/** Speed and Cadence Control Point Commands */
#define SET_CUMMULATIVE_VALUE_OPCODE             1
#define START_SENSOR_CALIBRATION_OPCODE          2
#define UPDATE_SENSOR_LOCATION_OPCODE            3
#define REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE 4

#define SC_CP_SET_CUMMULATIVE_VALUE_REQ_LEN         5
#define SC_CP_START_SENSOR_CALIBRATION_REQ_LEN      1
#define SC_CP_UPDATE_SENSOR_LOCATION_REQ_LEN        2
#define SC_CP_REQUEST_SUPPORTED_SENSOR_LOCATION_LEN 1

#ifndef APPL_MENU_OPS
/*
 * Connection Complete Count.
 * Used to control the application scenario.
 */
DECL_STATIC UCHAR appl_connection_complete_count;
#endif /* APPL_MENU_OPS */

DECL_STATIC ATT_ATTR_HANDLE service_start_handle;
DECL_STATIC ATT_ATTR_HANDLE service_end_handle;

DECL_STATIC APPL_CHARACTERISTIC appl_char[APPL_MAX_CHARACTERISTICS];

DECL_STATIC APPL_SERVICE_DISCOVERY_STATE appl_gatt_dscvry_state;

DECL_STATIC CSCC_CHAR_INFO cscc_char_info;

/* This holds the descriptor discovery count */
UCHAR appl_char_descptr_discovery_count;

#ifdef APPL_MENU_OPS

BT_DEVICE_ADDR g_peer_bd_addr;

#ifndef BT_SW_TXP
static const char main_options[] = " \r\n\
======== M A I N   M E N U ======== \r\n\
    0.  Exit \r\n\
    1.  Refresh \r\n\
 \r\n\
    2.  Power on \r\n\
    3.  Power off \r\n\
 \r\n\
   11.  Set Scan Params \r\n\
   12.  Scan Enable \r\n\
   13.  Scan Disable \r\n\
 \r\n\
   21.  Connect \r\n\
   22.  Bond \r\n\
   23.  Connection Update \r\n\
 \r\n\
   31.  Add to white List \r\n\
 \r\n\
   41.  Profile Menu \r\n\
 \r\n\
   51.  Disconnect \r\n\
 \r\n\
   Your Option ? \0";
static UCHAR cscc_client_menu[] =" \r\n\
======== C L I E N T   M E N U ======== \r\n\
    0.  Exit \r\n\
    1.  Refresh \r\n\
\r\n\n\
       === Cycling Speed and Cadence Service ===\r\n\n\
   10.  Discover Cycling Speed and Cadence Service \r\n\
\r\n\
        -- CSC Measurement -- \r\n\
   11.  Configure CSC Measurement for Notification \r\n\
   12.  Stop CSC Measurement Notification \r\n\
\r\n\
        -- CSC Feature -- \r\n\
   13.  Read CSC Feature \r\n\
\r\n\
        -- Sensor Location -- \r\n\
   14.  Read Sensor Location \r\n\
\r\n\
        -- Speed and Cadence Control Point -- \r\n\
   15.  Configure SC Control Point for Indication \r\n\
   16.  Stop SC Control Point Indication \r\n\
   17.  Set Cummulative Value through Control Point \r\n\
   18.  Start Sensor Calibration through Control Point \r\n\
   19.  Update Sensor Location through Control Point \r\n\
   20.  Request Supported Sensor Location through Control Point \r\n\
\r\n\n\
        === Device Information Service === \r\n\n\
   50 - Discover Device Information Service \r\n\
\r\n\
   51 - Read Manufacturer Name \r\n\
   52 - Read Model Number \r\n\
   53 - Read Serial Number \r\n\
   54 - Read Firmware Revision \r\n\
   55 - Read Hardware Revision \r\n\
   56 - Read Software Revision \r\n\
   57 - Read System ID \r\n\
   58 - Read Reg Cert Data List \r\n\
\r\n\n\
        === Battery Service === \r\n\n\
   60 - Discover Battery Service \r\n\
\r\n\
   61 - Read Battery Level \r\n\n\
   Your Option? \0";

#else /* BT_SW_TXP */
const char menu[] = " \r\n\
========= CSCC: M A I N   M E N U ========= \r\n\
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

/* ------------------------------- Functions */

#ifdef APPL_MENU_OPS
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
#endif /* APPL_MENU_OPS */


/**
 *  \brief To poweron the stack.
 *
 *  \par Description:
 *  This routine will turn on the Bluetooth service.
 */
void appl_poweron_stack(void)
{
#ifndef APPL_MENU_OPS
    /* Initialize Connection Complete Count */
    appl_connection_complete_count = 0x00;
#endif /* APPL_MENU_OPS */

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
        "EtherMind-CSCC"
    );

    return;
}


/**
 *  \brief Entry point of the sample application.
 */
#ifndef BT_SW_TXP
int main (int argc, char **argv)
#else
int appl_init(void)
#endif /* BT_SW_TXP */
{
    /* Initialize SNOOP Related Functions */
    em_snoop_init();
    snoop_bt_init();

    /* Step 1. Initialize and power on the stack */
    BT_ethermind_init ();

    #ifdef BT_ANALYZER
        init_analyzer();
    #endif /* BT_ANALYZER */

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

#ifndef APPL_MENU_OPS
    appl_poweron_stack();
#endif /* APPL_MENU_OPS */

#ifdef BT_SW_TXP
#ifdef APPL_MENU_OPS
    /* Initialize console */
    console_init(appl_get_profile_menu());
#endif /* APPL_MENU_OPS */

    /*
     * Note: Stack implementation is non-blocking.
     * Rest of the control flow will be through various
     * registered callback routines and API calls.
     */

#else
    /** Wait-Loop - to avoid exiting from the process */
    while (1)
    {

#ifndef APPL_MENU_OPS
        BT_sleep(1);
#else
        appl_master_menu_handler();
#endif /* APPL_MENU_OPS */
    }
#endif /* BT_SW_TXP */

    return 0;
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

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
    /* Initialize console again - after bluetooth on */
    console_init(appl_get_profile_menu());

    CONSOLE_OUT ("\r\nTurned ON Bluetooth.\r\n");
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

    APPL_TRC (
    "Turned ON Bluetooth.\n");

#ifdef APPL_MENU_OPS
    /* GATT Discovery State Initialization */
    appl_reset_gatt_dscvry_info ();

    APPL_CSCC_INFO_INIT ();
#endif /* APPL_MENU_OPS */


    /* Register application callbacks with stack modules */
    retval = appl_register_callback();

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "Callback Registration Failed. Result = 0x%04X\n", retval);

        return retval;
    }

#if (!defined APPL_MENU_OPS) || (defined BT_SW_TXP)
    /**
     *  Step 2(a). Initiate Scanning.
     *
     *  The operation is performed in two parts.
     *  First set the scan parameters.
     *  On completion of set scan parameter, enable scanning.
     *
     *  Completion of set scan parameter is informed through
     *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback 'appl_hci_callback'.
     *  Look for the handling of opcode 'HCI_LE_SET_SCAN_PARAMETERS_OPCODE'.
     */
    retval = BT_hci_le_set_scan_parameters
             (
                 APPL_GAP_GET_SCAN_TYPE(),
                 APPL_GAP_GET_SCAN_INTERVAL(),
                 APPL_GAP_GET_SCAN_WINDOW(),
                 APPL_GAP_GET_OWN_ADDR_TYPE_IN_SCAN(),
                 APPL_GAP_GET_SCAN_FILTER_POLICY()
             );

    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "Failed to set scan parameters. Result = 0x%04X\n", retval);
    }
#endif /* (!defined APPL_MENU_OPS) || (defined BT_SW_TXP) */
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

void appl_handle_conn_complete
    (
        UCHAR* peer_addr,
        UCHAR peer_addr_type
    )
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
#ifndef APPL_MENU_OPS
    API_RESULT          retval;
#endif /* APPL_MENU_OPS */
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

    /* Num completed packets event, can be ignored by the application */
    if (HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT == event_type)
    {
        return API_SUCCESS;
    }

    APPL_TRC("\n");

    /* Switch based on the Event Code */
    switch (event_type)
    {
    case HCI_DISCONNECTION_COMPLETE_EVENT:
        APPL_TRC (
        "Received HCI_DISCONNECTION_COMPLETE_EVENT.\n");

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

#ifdef APPL_MENU_OPS
       CONSOLE_OUT("Disconnection Complete. Handle: 0x%04X. Reason: 0x%04X\n",
                connection_handle, value_1);
#endif /* APPL_MENU_OPS */

#ifndef APPL_MENU_OPS
        /**
         *  Step 2(g). Initiate Scanning again.
         */
        if (0x01 == appl_connection_complete_count)
        {
            retval = BT_hci_le_set_scan_parameters
                     (
                         APPL_GAP_GET_SCAN_TYPE(),
                         APPL_GAP_GET_SCAN_INTERVAL(),
                         APPL_GAP_GET_SCAN_WINDOW(),
                         APPL_GAP_GET_OWN_ADDR_TYPE_IN_SCAN(),
                         APPL_GAP_GET_SCAN_FILTER_POLICY()
                     );

            if (API_SUCCESS != retval)
            {
                APPL_ERR (
                "Failed to set scan parameters. Result = 0x%04X\n", retval);
            }
        }
        else
        {
            /*
             *  Step 3. Power off the stack
             *          # End of demonstration application
             */
            BT_bluetooth_off();
        }
#endif /* APPL_MENU_OPS */
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

#ifndef APPL_MENU_OPS
                /* Enable Scanning [Step 2(a)] */
                BT_hci_le_set_scan_enable (1, 1);

                /**
                 * Completion of set scan enable is informed through
                 *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback
                 *  'appl_hci_callback'.
                 *
                 *  Look for opcode 'HCI_LE_SET_SCAN_ENABLE_OPCODE'.
                 */
#else
            CONSOLE_OUT("Set Scan Params Complete. Status: 0x%02X\n", status);

#endif /* APPL_MENU_OPS */
            }
        }
        else if (HCI_LE_SET_SCAN_ENABLE_OPCODE == value_2)
        {
#ifdef APPL_MENU_OPS
            CONSOLE_OUT("Set Scan Enable Complete. Status: 0x%02X\n", status);
#endif /* APPL_MENU_OPS */
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

#ifdef APPL_MENU_OPS
        CONSOLE_OUT ("\r\nReceived HCI_HARDWARE_ERROR_EVENT.\r\n");
#endif /* APPL_MENU_OPS */

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
            APPL_TRC (
            "Subevent : HCI_LE_CONNECTION_COMPLETE_SUBEVENT.\n");
            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
            hci_unpack_1_byte_param(&role, event_data +3);
            hci_unpack_1_byte_param(&peer_addr_type, event_data + 4);
            peer_addr = 5 + event_data;
            hci_unpack_2_byte_param(&conn_interval, event_data+11);
            hci_unpack_2_byte_param(&conn_latency, event_data+13);
            hci_unpack_2_byte_param(&supervision_timeout, event_data+15);
            hci_unpack_1_byte_param(&clock_accuracy, event_data + 17);

            /* Save Connection Handle */
            appl_ble_connection_handle = connection_handle;

            /* Print the parameters */
            APPL_TRC (
            "status = 0x%02X\n",status);
            APPL_TRC (
            "connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC (
            "role = 0x%02X\n",role);
            APPL_TRC (
            "peer_addr_type = 0x%02X\n",peer_addr_type);
            APPL_TRC (
            "peer_addr = \n");
            appl_dump_bytes(peer_addr, 6);
            APPL_TRC (
            "conn_interval = 0x%04X\n",conn_interval);
            APPL_TRC (
            "conn_latency = 0x%04X\n",conn_latency);
            APPL_TRC (
            "supervision_timeout = 0x%04X\n",supervision_timeout);
            APPL_TRC (
            "clock_accuracy = 0x%02X\n",clock_accuracy);
#ifdef APPL_MENU_OPS
            CONSOLE_OUT(
            "Connection Complete.\r\nStatus: 0x%02X.\r\nBD Addr: "
            "%02X %02X %02X %02X %02X %02X.\r\nBD Addr Type: 0x%02X.\r\n"
            "Connection Handle: 0x%04X\n", status, peer_addr[0],
            peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4],
            peer_addr[5], peer_addr_type, connection_handle);
#endif /* APPL_MENU_OPS */
#ifndef APPL_MENU_OPS

            /* Save connection complete count */
            appl_connection_complete_count ++;

            /* If connection is successful, initiate bonding [Step 2(c)] */
            if (0x00 == status)
            {
                appl_handle_conn_complete(peer_addr, peer_addr_type);
            }
            /**
             *  Application will receive authentication complete event,
             *  in SMP Callback.
             *
             *  Look for 'SMP_AUTHENTICATION_COMPLETE' event handling in
             *  'appl_smp_callback'.
             */
#endif /* APPL_MENU_OPS */

            break;
#ifdef HCI_LE_PRIVACY_1_2_SUPPORT
        case HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT:
            APPL_TRC (
            "Subevent : HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT.\n");
            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
            hci_unpack_1_byte_param(&role, event_data +3);
            hci_unpack_1_byte_param(&peer_addr_type, event_data + 4);
            /* Check the Identity Address Type */
            peer_addr_type = ((0x01 == peer_addr_type) || (0x03 == peer_addr_type)) ? \
                BT_BD_RANDOM_ADDRESS_TYPE : BT_BD_PUBLIC_ADDRESS_TYPE;

            peer_addr = 5 + event_data;
            local_rpa_addr = 11 + event_data;
            peer_rpa_addr = 17 + event_data;

            hci_unpack_2_byte_param(&conn_interval, event_data+23);
            hci_unpack_2_byte_param(&conn_latency, event_data+25);
            hci_unpack_2_byte_param(&supervision_timeout, event_data+27);
            hci_unpack_1_byte_param(&clock_accuracy, event_data + 29);

            /* Save Connection Handle */
            appl_ble_connection_handle = connection_handle;

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
            "status = 0x%02X\n",status);
            APPL_TRC (
            "connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC (
            "role = 0x%02X\n",role);
            APPL_TRC (
            "peer_addr_type = 0x%02X\n",peer_addr_type);
            APPL_TRC (
            "peer_addr = \n");
            appl_dump_bytes(peer_addr, 6);
            APPL_TRC (
            "conn_interval = 0x%04X\n",conn_interval);
            APPL_TRC (
            "conn_latency = 0x%04X\n",conn_latency);
            APPL_TRC (
            "supervision_timeout = 0x%04X\n",supervision_timeout);
            APPL_TRC (
            "clock_accuracy = 0x%02X\n",clock_accuracy);
#ifdef APPL_MENU_OPS
            CONSOLE_OUT(
            "Connection Complete.\r\nStatus: 0x%02X.\r\nBD Addr: "
            "%02X %02X %02X %02X %02X %02X.\r\nBD Addr Type: 0x%02X.\r\n"
            "Connection Handle: 0x%04X\n", status, peer_addr[0],
            peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4],
            peer_addr[5], peer_addr_type, connection_handle);
#endif /* APPL_MENU_OPS */
#ifndef APPL_MENU_OPS
            /* Save connection complete count */
            appl_connection_complete_count ++;

            /* If connection is successful, initiate bonding [Step 2(c)] */
            if (0x00 == status)
            {
                appl_handle_conn_complete(peer_addr, peer_addr_type);
            }
            /**
             *  Application will receive authentication complete event,
             *  in SMP Callback.
             *
             *  Look for 'SMP_AUTHENTICATION_COMPLETE' event handling in
             *  'appl_smp_callback'.
             */
#endif /* APPL_MENU_OPS */
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
                APPL_TRC("event_type = 0x%02X\n",event_type);
                APPL_TRC("address_type = 0x%02X\n",address_type);
                APPL_TRC("address = \n");
                appl_dump_bytes(address, 6);
                APPL_TRC("length_data = 0x%02X\n",length_data);
                APPL_TRC("data = \n");
                appl_dump_bytes(data, length_data);
                APPL_TRC("rssi = 0x%02X\n",rssi);
#ifdef APPL_MENU_OPS
                CONSOLE_OUT("Advertising Report : BD Addr: "
                "%02X %02X %02X %02X %02X %02X. BD Addr Type: 0x%02X.\r\n",
                address[0],address[1], address[2], address[3], address[4],
                address[5], address_type);
#endif /* APPL_MENU_OPS */
            }

            /* If already connection initiated, do not try to initiate again */
            if (0 != APPL_GET_STATE(CONNECTION_INITIATED))
            {
                break;
            }
#ifndef APPL_MENU_OPS
            /* Stop Scanning */
            BT_hci_le_set_scan_enable (0, 1);

            /* Set state as initiated connection */
            APPL_SET_STATE(CONNECTION_INITIATED);

#endif /* APPL_MENU_OPS */

#ifndef APPL_MENU_OPS
            /* Initiate connection */
            APPL_TRC("Initiating Connection ...\n");

            retval = BT_hci_le_create_connection
                     (
                         APPL_GAP_GET_CONN_SCAN_INTERVAL(),
                         APPL_GAP_GET_CONN_SCAN_WINDOW(),
                         APPL_GAP_GET_CONN_INITIATOR_FILTER_POLICY(),
                         address_type,
                         address,
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
                ":%02X, reason 0x%04X\n", address[5], address[4], address[3],
                address[2], address[1], address[0], retval);

                BT_hci_le_set_scan_enable (1, 1);
            }
#else /* APPL_MENU_OPS */

            BT_COPY_BD_ADDR(g_peer_bd_addr.addr, address);
            BT_COPY_TYPE(g_peer_bd_addr.type, address_type);

#endif /* APPL_MENU_OPS */
            break;

        case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT:
            APPL_TRC (
            "Subevent : HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVENT.\n");
            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
            hci_unpack_2_byte_param(&conn_interval, event_data+3);
            hci_unpack_2_byte_param(&conn_latency, event_data+5);
            hci_unpack_2_byte_param(&supervision_timeout, event_data+7);

#ifdef APPL_MENU_OPS
            CONSOLE_OUT("\r\nReceived Connection Update Complete with "
            "status = 0x%02X  Connection_handle = 0x%04X \r\n",
            status,connection_handle);
#endif /* APPL_MENU_OPS */

            /* Print the parameters */
            APPL_TRC (
            "status = 0x%02X\n",status);
            APPL_TRC (
            "connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC (
            "conn_interval = 0x%04X\n",conn_interval);
            APPL_TRC (
            "conn_latency = 0x%04X\n",conn_latency);
            APPL_TRC (
            "supervision_timeout = 0x%04X\n",supervision_timeout);

#ifndef APPL_MENU_OPS
            /**
             * On Successful Completion, initate Service Discovery [Step 2(e)]
             */
            if (0x00 == status)
            {
                appl_discover_gap_service();
            }

#endif /* APPL_MENU_OPS */
            break;

        case HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT:
            APPL_TRC (
            "Subevent: HCI_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SUBEVENT.\n");

            hci_unpack_1_byte_param(&status, event_data + 0);
            hci_unpack_2_byte_param(&connection_handle, event_data+1);
            le_feature = 3 + event_data;

            /* Print the parameters */
            APPL_TRC (
            "status = 0x%02X\n",status);
            APPL_TRC (
            "connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC (
            "le_feature = \n");
            appl_dump_bytes(le_feature, 8);
            break;

        case HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT:
            APPL_TRC("Subevent : HCI_LE_LONG_TERM_KEY_REQUESTED_SUBEVENT.\n");
            hci_unpack_2_byte_param(&connection_handle, event_data + 0);
            random_number = 2 + event_data;
            hci_unpack_2_byte_param(&encripted_diversifier, event_data+10);

            /* Print the parameters */
            APPL_TRC (
            "connection_handle = 0x%04X\n",connection_handle);
            APPL_TRC (
            "random_number = \n");
            appl_dump_bytes(random_number, 8);
            APPL_TRC (
            "encrypted_diversifier = 0x%04X\n",encripted_diversifier);

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

    device_queue_get_remote_addr (handle,&peer_bd_addr);
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
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n",bd_addr[0],
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

#ifdef APPL_MENU_OPS
        CONSOLE_OUT("L2CAP Connection Update Response. Result: 0x%04X\n",
            result);
#endif /* APPL_MENU_OPS */

        APPL_TRC (
        "Received : L2CAP_CONNECTION_UPDATE\n");
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n",bd_addr[0],
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

#ifdef APPL_MENU_OPS
        CONSOLE_OUT("L2CAP Command Rejected. Reason: 0x%04X\n", reason);
#endif /* APPL_MENU_OPS */

        APPL_TRC (
        "Received : L2CAP_COMMAND_REJ\n");
        APPL_TRC (
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n",bd_addr[0],
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
        "\tBD_ADDR      : %02X:%02X:%02X:%02X:%02X:%02X\n",bd_addr[0],
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
    "[APPL]: Initiated Read Request with result 0x%04X\n",retval);

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
    range->start_handle,range->end_handle);

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
                   (APPL_CHAR_GET_START_HANDLE(APPL_GET_CURRENT_CHAR_INDEX()+1) -1);
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
                if(API_SUCCESS == retval)
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
            if ((APPL_GET_CURRENT_CHAR_INDEX()+1) == APPL_GET_CHAR_COUNT())
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
    uuid,range->start_handle,range->end_handle);

    read_by_type_req_param.range = *range;
    ATT_SET_16_BIT_UUID(&read_by_type_req_param.uuid,uuid);
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
    "[ATT]: Read by Type Request Initiated with result 0x%04X",retval);
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
        BT_UNPACK_LE_2_BYTE(&service_end_handle, (data+2));
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

    if(0 < length)
    {
        if(0x01 == type)
        {
            APPL_TRC ("\n\n16 bit UUIDs : \n");
            for(index = 0; index<length; index=index+4)
            {
                BT_UNPACK_LE_2_BYTE(&handle, (list+ index));
                BT_UNPACK_LE_2_BYTE(&uuid, (list+ index + 2));
                APPL_TRC ("Handle : %04X -> ", handle);
                APPL_TRC ("UUID   : %04X \n", uuid);

                /* Populate Needed CCCDs here */
                if (GATT_CLIENT_CONFIG == uuid)
                {
                    if (GATT_CSC_MSRMT_CHARACTERISTIC == \
                        APPL_CHAR_GET_UUID(APPL_GET_CURRENT_CHAR_INDEX()))
                    {
                        CSCS_CSC_MSRMNT_CCC_HDL = handle;
                    }
                    else if (GATT_SC_CONTROL_POINT_CHARACTERISTIC == \
                        APPL_CHAR_GET_UUID(APPL_GET_CURRENT_CHAR_INDEX()))
                    {
                        CSCS_SC_CP_CCC_HDL = handle;
                    }
                    else
                    {
                        APPL_TRC ("Unknown\n");
                    }
                }
            }
        }
        else if(0x02 == type)
        {
            for(index = 0; index < length; index=index+18)
            {
                APPL_TRC("\n\n128 bit UUIDs : \n");
                BT_UNPACK_LE_2_BYTE(&handle, (list + index));
                ATT_UNPACK_UUID(uuid_arr, (list+index+2), 16);
                APPL_TRC ("Handle : %04X\n", handle);
                APPL_TRC ("UUID   : ");

                appl_dump_bytes(uuid_arr, 16);
            }
        }
        else
        {
            /* TODO: Check if this is correct interpretaion */
            APPL_TRC ("List of handles corrosponding to the Req UUID:\n");

            for(index = 0; index < length; index+=index)
            {

                BT_UNPACK_LE_2_BYTE(&handle, list+ index);
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
           (APPL_CHAR_GET_START_HANDLE(APPL_GET_CURRENT_CHAR_INDEX()+1) -1);
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
            "[APPL]: Failed to initiate follow request, reason 0x%04X\n",retval);
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
    UINT16 att_mtu = ATT_DEFAULT_MTU;

    BT_att_access_mtu(&appl_gatt_client_handle, &att_mtu);

    search_again = 0;

    /**
     * Check if the response received was smaller than max MTU, then no further
     * searching is needed
     */
    diff_in_size = att_mtu - data_len -1;
    if (0 == diff_in_size || ((0 != diff_in_size)
       && (diff_in_size < data_elem_size)))
    {
        /**
         * There could be more requests, as the MTU has been optimally utilized
         */
        search_again = 1;
    }

    for (index = 0; index < (unsigned)(data_len/(data_elem_size)); index++ )
    {
        BT_UNPACK_LE_2_BYTE(&handle, (data + (index * data_elem_size)));
        APPL_TRC ("Handle - 0x%04X\n", handle);
        APPL_TRC ("Handle Value Received - \n");
        value = data + (2+index * data_elem_size);
        appl_dump_bytes(value, data_elem_size-2);

        if (APPL_GD_IN_CHAR_DSCVRY_STATE == APPL_GET_CURRENT_DSCVRY_STATE())
        {
            /* Extract Property */
            c_property = value[0];

            BT_UNPACK_LE_2_BYTE
            (
                &value_handle,
                value+1
            );

            BT_UNPACK_LE_2_BYTE
            (
                &uuid,
                value+3
            );

            APPL_TRC ("----\n");
            APPL_TRC ("Value Handle 0x%04X, Property 0x%02X, UUID 0x%04X\n",
            value_handle, c_property, uuid);

            appl_print_char_property (c_property);

            APPL_TRC ("UUID 0x%04X ", uuid);

#ifdef APPL_MENU_OPS
            switch (uuid)
            {
            case GATT_CSC_MSRMT_CHARACTERISTIC:
                CSCS_CSC_MSRMNT_HDL = value_handle;
                break;

            case GATT_CSC_FEATURE_CHARACTERISTIC:
                CSCS_CSC_FEATURE_HDL = value_handle;
                break;

            case GATT_SENSOR_LOCATION_CHARACTERISTIC:
                CSCS_SNSR_LOC_HDL = value_handle;
                break;

            case GATT_SC_CONTROL_POINT_CHARACTERISTIC:
                CSCS_SC_CP_HDL = value_handle;
                break;

            case GATT_MANUFACTURER_NAME_CHARACTERISTIC:
                DIS_MANUFACTURER_NAME_HDL = value_handle;
                break;

            case GATT_MODEL_NUMBER_CHARACTERISTIC:
                DIS_MODEL_NUMBER_HDL = value_handle;
                break;

            case GATT_SERIAL_NUMBER_CHARACTERISTIC:
                DIS_SERIAL_NUMBER_HDL = value_handle;
                break;

            case GATT_HARDWARE_REV_CHARACTERISTIC:
                DIS_HARDWARE_REVISION_HDL = value_handle;
                break;

            case GATT_FIRMWARE_REV_CHARACTERISTIC:
                DIS_FIRMWARE_REVISION_HDL = value_handle;
                break;

            case GATT_SOFTWARE_REV_CHARACTERISTIC:
                DIS_SOFTWARE_REVISION_HDL = value_handle;
                break;

            case GATT_SYSTEM_ID_CHARACTERISTIC:
                DIS_SYSTEM_ID_HDL = value_handle;
                break;

            case GATT_REG_CERT_DATA_CHARACTERISTIC:
                DIS_REG_CERT_DATA_LIST_HDL = value_handle;
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

    APPL_TRC (
    "[ATT]:[0x%02X]: Received ATT Event 0x%02X with result 0x%04X\n",
    *att_handle, event_type, event_result);

#ifdef APPL_MENU_OPS
    if (event_type != ATT_HANDLE_VALUE_NTF)
    {
        CONSOLE_OUT(
        "ATT Event Type: 0x%02X. Status: 0x%04X\n", event_type,event_result);
    }
#endif /* APPL_MENU_OPS */

    appl_dump_bytes (event_data, event_datalen);

    switch(event_type)
    {
    case ATT_CONNECTION_IND:
        APPL_TRC (
        "[0x%02X]:Received Connection Indtication, Result 0x%04X!\n",
        *att_handle,event_result);
        appl_gatt_client_handle = *att_handle;
        /* appl_att_initiate_service_discovery (); */
        break;

    case ATT_DISCONNECTION_IND:

#ifdef APPL_MENU_OPS
        appl_reset_gatt_dscvry_info ();

        APPL_CSCC_INFO_INIT ();
#endif /* APPL_MENU_OPS */
        APPL_TRC (
        "[0x%02X]:Received Disconnection Indtication, Result 0x%04X!\n",
        *att_handle,event_result);
        break;

    case ATT_ERROR_RSP:
        BT_UNPACK_LE_1_BYTE(&op_code,event_data);
        BT_UNPACK_LE_2_BYTE(&attr_handle, event_data+1);
        BT_UNPACK_LE_1_BYTE(&rsp_code,event_data+3);
        APPL_TRC (
        "Received Error Response, for Op-Code 0x%02X for Handle 0x%04X, Rsp "
        "Code 0x%02X!\n", op_code, attr_handle,rsp_code);
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
        APPL_TRC (
        "Received Exchange MTU Response with Result 0x%04X. MTU Size "
        "= 0x%04X!\n",event_result,mtu);
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
        "Received Read Information Response Opcode!\n");

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

#ifndef APPL_MENU_OPS
        /* Initiate BLE Disconnection - Step 2(f) */
        BT_hci_disconnect (appl_ble_connection_handle, 0x13);
#endif /* APPL_MENU_OPS */
        break;
    case ATT_HANDLE_VALUE_NTF:
#ifndef APPL_MENU_OPS
        APPL_TRC ("Received HVN\n");
        BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
        APPL_TRC ("Handle - 0x%04X\n", attr_handle);
        APPL_TRC ("Handle Value Received - \n");
        appl_dump_bytes(event_data + 2, (event_datalen - 2));
        APPL_TRC ("\n");
#else
        BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
        CONSOLE_OUT("Handle - 0x%04X\r\n", attr_handle);
        CONSOLE_OUT("Handle Value Received - 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
        ,event_data[2],event_data[3],event_data[4],event_data[5]);
#endif /* APPL_MENU_OPS */
        break;
    case ATT_HANDLE_VALUE_IND:
        APPL_TRC ("Received HVI\n");
        BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
        APPL_TRC ("Handle - 0x%04X\n", attr_handle);
        APPL_TRC ("Handle Value Received - \n");
        appl_dump_bytes(event_data + 2, (event_datalen-2));

        retval = BT_att_send_hndl_val_cnf (&appl_gatt_client_handle);
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
#ifndef APPL_MENU_OPS
    UINT16 connection_handle;
#endif /* APPL_MENU_OPS */

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

    /* Get the BD Address from handle */
    BT_smp_get_bd_addr (bd_handle, &bdaddr);

    bd_addr = bdaddr.addr;
    bd_addr_type = bdaddr.type;

    switch(event_type)
    {
    case SMP_AUTHENTICATION_COMPLETE:
        APPL_TRC (
        "\nRecvd SMP_AUTHENTICATION_COMPLETE\n");
#ifdef APPL_MENU_OPS
        CONSOLE_OUT("\r\nRecvd SMP_AUTHENTICATION_COMPLETE.\r\n"
        "Status : %04X\r\n", event_result );
#endif /* APPL_MENU_OPS */
        APPL_TRC (
        "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
        bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
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

#ifndef APPL_MENU_OPS
                /*
                 * If bonding complete on second connection,
                 * initiate disconnection - Step 2(h).
                 */
                if (0x02 == appl_connection_complete_count)
                {
                    BT_hci_disconnect (appl_ble_connection_handle, 0x13);
                }
                else
                {
                    /* Add Bonded Device to White List */
                    retval = BT_hci_le_add_device_to_white_list
                             (
                                 bd_addr_type,
                                 bd_addr
                             );

                    if (API_SUCCESS != retval)
                    {
                        APPL_TRC ("Failed to Add in White List. Error 0x%04X\n",
                        retval);
                    }

                    /*  Initiate Connection Update from Master. [Step 2(d)] */
                    retval = BT_hci_get_le_connection_handle
                             (
                                 &bdaddr,
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
#endif /* APPL_MENU_OPS */
            }
        }
        else
        {
            if (API_SUCCESS == event_result)
            {
                APPL_TRC("\nConfirmed Authentication using Encryption\n");

            }
        }
        break;

    case SMP_AUTHENTICATION_REQUEST:
        APPL_TRC (
        "\nRecvd SMP_AUTHENTICATION_REQUEST\n");
        APPL_TRC (
        "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
        bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
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
        "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
        bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC (
        "BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
        break;

    case SMP_PASSKEY_DISPLAY_REQUEST:
        APPL_TRC (
        "\nEvent   : SMP_PASSKEY_DISPLAY_REQUEST\n");
        APPL_TRC (
        "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
        bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
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
        "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
        bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
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
        "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
        bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
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

            if(API_SUCCESS == retval)
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
        if(API_SUCCESS == event_result);
        {
            bd_addr = (UCHAR *) event_data;

            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
            bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4], bd_addr[5]);
        }
        break;

    case SMP_RESOLVABLE_PVT_ADDR_VERIFY_CNF:
        APPL_TRC (
        "\nEvent   : SMP_RESOLVABLE_PVT_ADDR_VERIFY_CNF\n");
        APPL_TRC (
        "Status : %04X\n", event_result);
        if(API_SUCCESS == event_result);
        {
            bd_addr = (UCHAR *) event_data;

            APPL_TRC (
            "BD Address : %02X %02X %02X %02X %02X %02X\n",bd_addr[0],
            bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        }
        break;

#ifdef SMP_DATA_SIGNING
    case SMP_DATA_SIGNING_COMPLETE:
        APPL_TRC("\nEvent   : SMP_SIGNING_DATA_COMPLETE\n");
        APPL_TRC("Status : %04X\n", event_result);
        if(API_SUCCESS == event_result)
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

    return API_SUCCESS;
}


/* ------------------------------- Utility Functions */
void appl_dump_bytes (UCHAR *buffer, UINT16 length)
{
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

            APPL_TRC("%s   %s\n",
            hex_stream, char_stream);

            BT_mem_set(hex_stream, 0, 49);
            BT_mem_set(char_stream, 0, 17);
        }
    }

    if(offset != 0)
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
API_RESULT appl_discover_service
           (
               /* IN */ ATT_UUID16          * uuid,
               /* IN */ UCHAR                 uuid_frmt
           )
{
    ATT_FIND_BY_TYPE_VAL_REQ_PARAM    find_by_type_val_pram;
    UCHAR                             value[2];
    API_RESULT                        retval;

    find_by_type_val_pram.range.start_handle = ATT_ATTR_HANDLE_START_RANGE;
    find_by_type_val_pram.range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
    find_by_type_val_pram.uuid = GATT_PRIMARY_SERVICE;

    BT_PACK_LE_2_BYTE (value, uuid);

    find_by_type_val_pram.value.val = value;
    find_by_type_val_pram.value.len = 2;

    APPL_GET_CURRENT_DSCVRY_STATE() = APPL_GD_IN_SERV_DSCRVY_STATE;
    APPL_CURRENT_SERVICE_UUID() = (*uuid);

    retval = BT_att_send_find_by_type_val_req
             (
                  &appl_gatt_client_handle,
                  &find_by_type_val_pram
             );

    return retval;
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
    ATT_UUID16             uuid16;

    uuid16 = GATT_GAP_SERVICE;

    /* Discover GAP Service */
    appl_discover_service
    (
        &uuid16,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/**
 *  Function to initiate Service Discovery for
 *  Cycling Speed and Cadence Service.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_cscs_service(void)
{
    ATT_UUID16             uuid16;

    uuid16 = GATT_CYCLING_SPEED_AND_CADENCE_SERVICE;

    /* Discover Cycling Speed & Cadence Service */
    appl_discover_service
    (
        &uuid16,
        ATT_16_BIT_UUID_FORMAT
    );

    return;
}

/**
 *  Function to initiate Service Discovery for
 *  Device Information Service.
 *
 *  \param None.
 *
 *  \return   : None.
 */
void appl_discover_device_info_service(void)
{
    ATT_UUID16             uuid16;

    uuid16 = GATT_DEVICE_INFO_SERVICE;

    /* Discover Device Information Service */
    appl_discover_service
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
#ifdef APPL_MENU_OPS
    host_task_handler(btmsg);
#endif /* APPL_MENU_OPS */

    return;
}
#endif /* BT_SW_TXP */

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP

void appl_profile_operations (void)
{
    UINT32       choice;
    ATT_VALUE    value;
    UINT16       cli_cfg;
    UCHAR        cfg_val[APPL_CLI_CNFG_VAL_LENGTH];
    UCHAR        value_arr[5];
    int          data;
    UINT16       index;

    index = 1;

    while (1)
    {
        CONSOLE_OUT(
        "%s",cscc_client_menu);

        CONSOLE_IN ( "%d",&choice);

        switch (choice)
        {
            case 0:
                /* Exit */
                return;

            case 1:
                /* Refresh*/
                break;

            case 10:
                appl_discover_cscs_service();
                break;

            case 11:
                cli_cfg = GATT_CLI_CNFG_NOTIFICATION;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                value.len = APPL_CLI_CNFG_VAL_LENGTH;
                value.val = cfg_val;
                appl_write_req (CSCS_CSC_MSRMNT_CCC_HDL,&value);
                break;

            case 12:
                cli_cfg = GATT_CLI_CNFG_DEFAULT;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                value.len = APPL_CLI_CNFG_VAL_LENGTH;
                value.val = cfg_val;
                appl_write_req (CSCS_CSC_MSRMNT_CCC_HDL,&value);
                break;

            case 13:
                appl_read_req(CSCS_CSC_FEATURE_HDL);
                break;

            case 14:
                appl_read_req(CSCS_SNSR_LOC_HDL);
                break;

            case 15:
                cli_cfg = GATT_CLI_CNFG_INDICATION;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                value.len = APPL_CLI_CNFG_VAL_LENGTH;
                value.val = cfg_val;
                appl_write_req (CSCS_SC_CP_CCC_HDL,&value);
                break;

            case 16:
                cli_cfg = GATT_CLI_CNFG_DEFAULT;
                BT_PACK_LE_2_BYTE(cfg_val, &cli_cfg);
                value.len = APPL_CLI_CNFG_VAL_LENGTH;
                value.val = cfg_val;
                appl_write_req (CSCS_SC_CP_CCC_HDL,&value);
                break;

            case 17:
                value.val = &value_arr[0];
                value_arr[0] = SET_CUMMULATIVE_VALUE_OPCODE;
                CONSOLE_OUT("Enter the Cummulative Value to be set\n");
                while (index < SC_CP_SET_CUMMULATIVE_VALUE_REQ_LEN)
                {
                    CONSOLE_IN("%x", &data);
                    value_arr[index] = (UCHAR)data;
                    index ++;
                }
                value.len = SC_CP_SET_CUMMULATIVE_VALUE_REQ_LEN;
                if (0 != value.len)
                {
                    appl_write_req (CSCS_SC_CP_HDL,&value);
                }
                break;

            case 18:
                value.val = &value_arr[0];
                value_arr[0] = START_SENSOR_CALIBRATION_OPCODE;
                value.len = SC_CP_START_SENSOR_CALIBRATION_REQ_LEN;
                if (0 != value.len)
                {
                   appl_write_req (CSCS_SC_CP_HDL,&value);
                }
                break;

            case 19:
                value.val = &value_arr[0];
                value_arr[0] = UPDATE_SENSOR_LOCATION_OPCODE;
                CONSOLE_OUT("Enter the Sensor Location to be set[Hex]:\n");
                CONSOLE_IN("%x", &data);
                value_arr[1] = (UCHAR) data;
                value.len = SC_CP_UPDATE_SENSOR_LOCATION_REQ_LEN;
                if (0 != value.len)
                {
                   appl_write_req (CSCS_SC_CP_HDL,&value);
                }
                break;

            case 20:
                value.val = &value_arr[0];
                value_arr[0] = REQUEST_SUPPORTED_SENSOR_LOCATION_OPCODE;
                value.len = SC_CP_REQUEST_SUPPORTED_SENSOR_LOCATION_LEN;
                if (0 != value.len)
                {
                   appl_write_req (CSCS_SC_CP_HDL,&value);
                }
                break;

            case 50:
                appl_discover_device_info_service();
                break;

            case 51:
                appl_read_req (DIS_MANUFACTURER_NAME_HDL);
                break;

            case 52:
                appl_read_req (DIS_MODEL_NUMBER_HDL);
                break;

            case 53:
                appl_read_req (DIS_SERIAL_NUMBER_HDL);
                break;

            case 54:
                appl_read_req (DIS_FIRMWARE_REVISION_HDL);
                break;

            case 55:
                appl_read_req (DIS_HARDWARE_REVISION_HDL);
                break;

            case 56:
                appl_read_req (DIS_SOFTWARE_REVISION_HDL);
                break;

            case 57:
                appl_read_req (DIS_SYSTEM_ID_HDL);
                break;

            case 58:
                appl_read_req (DIS_REG_CERT_DATA_LIST_HDL);
                break;

            case 60:
                appl_discover_battery_service ();
                break;

            case 61:
                appl_read_req (BAS_BATTERY_LEVEL_HDL);
                break;

            default:
                break;
        }
    }
}

void appl_master_menu_handler(void)
{
    API_RESULT retval;
    UINT32 read;
    UINT32   index;
    int choice;

    CONSOLE_OUT("\n");
    CONSOLE_OUT("%s", main_options);
    CONSOLE_IN("%d", &choice);

    switch(choice)
    {
    case 0:
        CONSOLE_OUT ("\nExiting BlueLitE Menu Based MASTER Application ... \n\n");
        exit(0);

    case 1:
        CONSOLE_OUT ("\nRefreshing ...\r\n\n");
        CONSOLE_OUT("%s", main_options);
        break;

    /* ---------- Intialization and Swith ON Bluetooth  --------------- */
    case 2:
        CONSOLE_OUT ("Initializing BlueLitE Stack...\n");
        /* Step 1. Initialize and power on the stack */
        appl_poweron_stack();

        break;

    /* ---------- Swith OFF Bluetooth  --------------- */
    case 3:
        CONSOLE_OUT("BlueLitE: Bluetooth OFF ...\n");
         /**
         *  Step 3. Power off the stack
         *          # End of demonstration application
         */

        BT_bluetooth_off ();

        break;

    case 11:
        {
            UCHAR scan_filter_policy;

            CONSOLE_OUT("Enter the Desired Scanning Mode \r\n");
            CONSOLE_OUT(" 0 - Scan all devices\r\n");
            CONSOLE_OUT(" 1 - Scan only WhiteList-ed Devices\r\n");
            CONSOLE_IN("%u", &read);

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
        break;

    case 12:
            /* Enable Scanning [Step 2(a)] */
            BT_hci_le_set_scan_enable (1, 1);

            /**
             * Completion of set scan enable is informed through
             *  'HCI_COMMAND_COMPLETE_EVENT' in HCI Callback
             *  'appl_hci_callback'.
             *
             *  Look for opcode 'HCI_LE_SET_SCAN_ENABLE_OPCODE'.
             */
        break;
    case 13:
            /* Stop Scanning */
            BT_hci_le_set_scan_enable (0, 1);
        break;

    case 21:
        {
            UCHAR   peer_div_addr[6];
            UCHAR   peer_div_addr_type;

            /* Input Peer Device Address Type */
            CONSOLE_OUT("Enter peer_address_type : \n");
            CONSOLE_IN("%X",&read);
            peer_div_addr_type = (UCHAR) read;

            /* Input Peer Device Address */
            CONSOLE_OUT("Enter peer_address : \n");
            for(index = 0; index<6; index++)
            {
            CONSOLE_IN("%X",&read);
            peer_div_addr[index] = (UCHAR) read;
            }

            BT_COPY_BD_ADDR(g_peer_bd_addr.addr, peer_div_addr);
            BT_COPY_TYPE(g_peer_bd_addr.type, peer_div_addr_type);

            /* Initiate connection */
            CONSOLE_OUT("Initiating Connection ...\n");

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
                g_peer_bd_addr.addr[4], g_peer_bd_addr.addr[3],g_peer_bd_addr.addr[2],
                g_peer_bd_addr.addr[1], g_peer_bd_addr.addr[0], retval);

            }
            }
        break;

    case 22:
        {
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
        break;

    case 23:
        {
            UINT16 connection_handle;

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
        break;

    case 31:
        {
            UCHAR   peer_div_addr[6];
            UCHAR   peer_div_addr_type;

            CONSOLE_OUT("Add desired Peer Device to White List - \n");

            /* Input Peer Device Address Type */
            CONSOLE_OUT("Enter peer_address_type : \n");
            CONSOLE_IN("%X",&read);
            peer_div_addr_type = (UCHAR) read;

            /* Input Peer Device Address */
            CONSOLE_OUT("Enter peer_address : \n");
            for(index = 0; index<6; index++)
            {
            CONSOLE_IN("%X",&read);
            peer_div_addr[index] = (UCHAR) read;
            }
            /* Add Device to White List */
            retval = BT_hci_le_add_device_to_white_list
                     (
                         peer_div_addr_type,
                         peer_div_addr
                     );

            if (API_SUCCESS != retval)
            {
                APPL_TRC ("Failed to Add in White List. Error 0x%04X\n",
                retval);
            }

        }
        break;

    case 41:
        {
            appl_profile_operations ();
        }
        break;

    case 51:
        {
            /* Initiate BLE Disconnection - Step 2(f) */
            BT_hci_disconnect (appl_ble_connection_handle, 0x13);
        }
        break;

    default:
        CONSOLE_OUT("Invalid Option : %d.\n", choice);
        break;
    }
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
        "EtherMind-CSCC"
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
        g_peer_bd_addr.addr[4], g_peer_bd_addr.addr[3],g_peer_bd_addr.addr[2],
        g_peer_bd_addr.addr[1], g_peer_bd_addr.addr[0], retval);

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
#endif /* APPL_MENU_OPS */

#ifdef BT_SW_TXP
#ifdef APPL_MENU_OPS
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
            if (length == 6)
            {
                /* Store BD Address */
                CONSOLE_OUT("BD Address - %02X %02X %02X %02X %02X %02X\n",
                value[0], value[1], value[2],
                value[3], value[4], value[5]);

                BT_COPY_BD_ADDR(peer_addr.addr,value);

                CONSOLE_OUT("\r\nEnter peer BD Address Type:\n");
            }
            else if (length == 1)
            {
                /* Store BD Address type */
                CONSOLE_OUT("BD Address type - %02X\n", value[0]);
                *choice = 1;
                BT_COPY_TYPE(peer_addr.type,value[0]);
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

#endif /* APPL_MENU_OPS */
#endif /* BT_SW_TXP */



