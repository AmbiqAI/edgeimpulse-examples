
/**
 *  \file appl_gatt.c
 *
 *  This file contains the ATT client application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_att_client.h"
#include "appl.h"
#include "appl_config.h"
#include "appl_config_extern.h"

#ifdef APPL_ATT_CLIENT

#define HID_CP_CCC_IN_BOOT_MAPPING_CHAR
/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
static ATT_HANDLE appl_gatt_client_handle;
static UINT16 appl_mtu_size = ATT_DEFAULT_MTU;
static APPL_SERVICE_INFORMATION service_array[APPL_MAX_SERVICES];
static APPL_CHARACTERISTIC_INFORMATION characteristic_array\
                                       [APPL_MAX_CHARACTERISTICS];
static APPL_PEER_SERVER_INFO peer_info;

static UINT16 appl_service_index;
static UINT16 appl_characteristic_index;
static UINT16 appl_gatt_service_discovery_count = 0;

static UINT16 appl_therm_sindex;
static UINT16 appl_hr_sindex;
static UINT16 appl_bpm_sindex;
static UINT16 appl_hr_cindex;
static UINT16 appl_hr_sensor_loc_cindex;
static UINT16 appl_therm_cindex;
static UINT16 appl_itherm_cindex;
static UINT16 appl_bpm_cindex;
static UINT16 appl_ibpm_cindex;

/** HID Specific Defines - Move to header file */

/* HID Control Point */

/* Command - from Host to Device */
/* Suspend */
#define HID_CP_ENUM_CMD_SUSPEND              0x00

/* Exit Suspend */
#define HID_CP_ENUM_CMD_EXIT_SUSPEND         0x02

/** HID Characteristic related information */
typedef struct
{
    /* HID Protocol Mode */
    ATT_ATTR_HANDLE hid_proto_mode_hdl;

    /* HID Report Descriptor */
    ATT_ATTR_HANDLE hid_report_desc_hdl;

    /* HID Boot Report Mapping */
    ATT_ATTR_HANDLE hid_boot_rpt_mapping_hdl;

    /* HID Report */
    ATT_ATTR_HANDLE hid_report_hdl;

    /* HID Report - CCC */
    ATT_ATTR_HANDLE hid_report_ccc_hdl;

    /* HID Control Point */
    ATT_ATTR_HANDLE hid_cp_hdl;

    /* HID Information */
    ATT_ATTR_HANDLE hid_info_hdl;

    /* HID Boot Protocol Keyboard - Input Report */
    ATT_ATTR_HANDLE hid_kbd_input_hdl;

    /* HID Boot Protocol Keyboard - CCC */
    ATT_ATTR_HANDLE hid_kbd_ccc_hdl;

    /* HID Boot Protocol Keyboard - Output Report */
    ATT_ATTR_HANDLE hid_kbd_output_hdl;

    /* HID Boot Protocol Mouse - Input Report */
    ATT_ATTR_HANDLE hid_mouse_input_hdl;

    /* HID Boot Protocol Mouse - CCC */
    ATT_ATTR_HANDLE hid_mouse_ccc_hdl;

} HID_CHAR_INFO;

/* HID Protocol Mode */
#define HID_PROTO_MODE_HDL (hid_char_info.hid_proto_mode_hdl)

/* HID Report Descriptor */
#define HID_REPORT_DESC_HDL (hid_char_info.hid_report_desc_hdl)

/* HID Boot Report Mapping */
#define HID_BOOT_RPT_MAPPING_HDL (hid_char_info.hid_boot_rpt_mapping_hdl)

/* HID Report */
#define HID_REPORT_HDL (hid_char_info.hid_report_hdl)

/* HID Report - CCC */
#define HID_REPORT_CCC_HDL (hid_char_info.hid_report_ccc_hdl)

/* HID Control Point */
#define HID_CP_HDL (hid_char_info.hid_cp_hdl)

/* HID Information */
#define HID_INFO_HDL (hid_char_info.hid_info_hdl)

/* HID Boot Protocol Keyboard - Input Report */
#define HID_KBD_INPUT_HDL (hid_char_info.hid_kbd_input_hdl)

/* HID Boot Protocol Keyboard - CCC */
#define HID_KBD_CCC_HDL (hid_char_info.hid_kbd_ccc_hdl)

/* HID Boot Protocol Keyboard - Output Report */
#define HID_KBD_OUTPUT_HDL (hid_char_info.hid_kbd_output_hdl)

/* HID Boot Protocol Mouse - Input Report */
#define HID_MOUSE_INPUT_HDL (hid_char_info.hid_mouse_input_hdl)

/* HID Boot Protocol Mouse - CCC */
#define HID_MOUSE_CCC_HDL (hid_char_info.hid_mouse_ccc_hdl)

/**
 * TBD: The HID handle values shall be initialized in some place.
 * It will work today, with the assumption that global variables
 * are initialized with ZERO, and which represents invalid
 * Attribute Handle.
 */
static HID_CHAR_INFO hid_char_info;

/**
 *  Data structure to maintain the current HID related
 *  Read operations
 */
typedef struct
{
    /* Current Handle being read */
    ATT_ATTR_HANDLE hdl;

    /* Read Offset - required to read 'Report Descriptor' */
    UINT16 offset;

} HID_READ_INFO;

static HID_READ_INFO appl_read_info;

#define HID_FLAGS_RECONNECT_INITIATE    0x01
#define HID_FLAGS_REMOTE_WAKE           0x02
#define HID_FLAGS_BOOT_DEVICE           0x04
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x08


/** PUID Profile Specific Defines - Move to header file */

/** PUID Characteristic related information */
typedef struct
{
    /* PAS - Phone Alert Status */
    /* PAS Alert Status  */
    ATT_ATTR_HANDLE pas_alert_status_hdl;

    /* PAS Alert Status - CCC */
    ATT_ATTR_HANDLE pas_alert_status_ccc_hdl;

    /* PAS Ringer Control Point */
    ATT_ATTR_HANDLE pas_ringer_cp_hdl;

    /* PAS Ringer Setting */
    ATT_ATTR_HANDLE pas_ringer_setting_hdl;

    /* PAS Ringer Setting - CCC */
    ATT_ATTR_HANDLE pas_ringer_setting_ccc_hdl;

    /* Find Me and Proximity uses */
    /* IAS - Immediate Alert */

    /* Alert Level */
    ATT_ATTR_HANDLE ias_alert_level_hdl;

    /* TPS - Tx Power Service */

    /* Tx Power Level */
    ATT_ATTR_HANDLE tps_tx_power_level_hdl;

    /* Tx Power Level - CCC */
    ATT_ATTR_HANDLE tps_tx_power_level_ccc_hdl;

    /* LLS - Link Loss Service */

    /* Alert Level */
    ATT_ATTR_HANDLE lls_alert_level_hdl;

    /* CTS - Current Time Service */

    /* Current Time */
    ATT_ATTR_HANDLE cts_current_time_hdl;

    /* Current Time - CCC */
    ATT_ATTR_HANDLE cts_current_time_ccc_hdl;

    /* NAS - Network Availability Service */

    /* Network Availability */
    ATT_ATTR_HANDLE nas_nwa_hdl;

    /* Network Availability - CCC */
    ATT_ATTR_HANDLE nas_nwa_ccc_hdl;

    /* BAS - Battery Service */

    /* Battery Level */
    ATT_ATTR_HANDLE bas_batt_level_hdl;

    /* Battery Power State */
    ATT_ATTR_HANDLE bas_batt_pow_st_hdl;

    /* Service Required */
    ATT_ATTR_HANDLE bas_batt_ser_rqd_hdl;

    /* Service Required - CCC */
    ATT_ATTR_HANDLE bas_batt_ser_rqd_ccc_hdl;

    /* Removable */
    ATT_ATTR_HANDLE bas_removable_hdl;

    /* Battery Level State */
    ATT_ATTR_HANDLE bas_batt_lvl_st_hdl;

    /* Battery Level State - CCC */
    ATT_ATTR_HANDLE bas_batt_lvl_st_ccc_hdl;

} PUID_CHAR_INFO;

/* PAS - Phone Alert Status */
/* PAS Alert Status  */
#define PAS_ALERT_STATUS_HDL (puid_char_info.pas_alert_status_hdl)

/* PAS Alert Status - CCC */
#define PAS_ALERT_STATUS_CCC_HDL (puid_char_info.pas_alert_status_ccc_hdl)

/* PAS Ringer Control Point */
#define PAS_RINGER_CP_HDL (puid_char_info.pas_ringer_cp_hdl)

/* PAS Ringer Setting */
#define PAS_RINGER_SETTING_HDL (puid_char_info.pas_ringer_setting_hdl)

/* PAS Ringer Setting - CCC */
#define PAS_RINGER_SETTING_CCC_HDL (puid_char_info.pas_ringer_setting_ccc_hdl)

/* Find Me and Proximity uses */
/* IAS - Immediate Alert */

/* Alert Level */
#define IAS_ALERT_LEVEL_HDL (puid_char_info.ias_alert_level_hdl)

/* TPS - Tx Power Service */

/* Tx Power Level */
#define TPS_TX_POWER_LEVEL_HDL (puid_char_info.tps_tx_power_level_hdl)

/* Tx Power Level - CCC */
#define TPS_TX_POWER_LEVEL_CCC_HDL (puid_char_info.tps_tx_power_level_ccc_hdl)

/* LLS - Link Loss Service */

/* Alert Level */
#define LLS_ALERT_LEVEL_HDL (puid_char_info.lls_alert_level_hdl)

/* Current Time */
#define CTS_CURRENT_TIME_HDL (puid_char_info.cts_current_time_hdl)

/* Current Time - CCC */
#define CTS_CURRENT_TIME_CCC_HDL (puid_char_info.cts_current_time_ccc_hdl)

/* NAS - Network Availability Service */

/* Network Availability */
#define NAS_NWA_HDL (puid_char_info.nas_nwa_hdl)

/* Network Availability - CCC */
#define NAS_NWA_CCC_HDL (puid_char_info.nas_nwa_ccc_hdl)

/* BAS - Battery Service */

/* Battery Level */
#define BAS_BATT_LEVEL_HDL (puid_char_info.bas_batt_level_hdl)

/* Battery Power State */
#define BAS_BATT_POW_ST_HDL (puid_char_info.bas_batt_pow_st_hdl)

/* Service Required */
#define BAS_BATT_SER_RQD_HDL (puid_char_info.bas_batt_ser_rqd_hdl)

/* Service Required - CCC */
#define BAS_BATT_SER_RQD_CCC_HDL (puid_char_info.bas_batt_ser_rqd_ccc_hdl)

/* Removable */
#define BAS_REMOVABLE_HDL (puid_char_info.bas_removable_hdl)

/* Battery Level State */
#define BAS_BATT_LVL_ST_HDL (puid_char_info.bas_batt_lvl_st_hdl)

/* Battery Level State - CCC */
#define BAS_BATT_LVL_ST_CCC_HDL (puid_char_info.bas_batt_lvl_st_ccc_hdl)

/**
 * TBD: The PUID handle values shall be initialized in some place.
 * It will work today, with the assumption that global variables
 * are initialized with ZERO, and which represents invalid
 * Attribute Handle.
 */
static PUID_CHAR_INFO puid_char_info;

#if 0
/**
 *  Data structure to maintain the current HID related
 *  Read operations
 */
typedef struct
{
    /* Current Handle being read */
    ATT_ATTR_HANDLE hdl;

    /* Read Offset - required to read 'Report Descriptor' */
    UINT16 offset;

} HID_READ_INFO;

static HID_READ_INFO appl_read_info;

#define HID_FLAGS_RECONNECT_INITIATE    0x01
#define HID_FLAGS_REMOTE_WAKE           0x02
#define HID_FLAGS_BOOT_DEVICE           0x04
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x08
#endif /* 0 */

static UCHAR *print_str = "\n------------------------------\n"\
                   "        ATT CLINET MENU\n"\
                   "------------------------------\n"\
                   "0.   Exit\n"\
                   "1.   Refresh\n\n"\
                   "2.   Exchange MTU Request\n\n"\
                   "10.  Discover All Services \n"\
                   "11.  Discover Service\n"\
                   "12.  Characteristic Descripotor Discovery\n\n"\
                   "13.  Configure Characteristic\n"\
                   "14.  Read Characteristic Value\n"\
                   "15.  Write Characteristic Value\n"
                   "16.  Write Command\n"
                   "17.  Read Blob\n\n"
                   "20.  Thermometer  Options\n\n"
                   "21.  Heart Rate  Options\n\n"
                   "22.  Blood Pressure Monitor Options\n\n"
                   "30.  Find By Type Value\n"
                   "31.  Read Multiple\n"
                   "32.  Prepare Write\n"
                   "33.  Execute Write\n"
                   "34.  Find Information\n\n"
                   "50.  HID Host Options\n\n"
                   "60.  PUID Client Options\n\n";


static UCHAR collector_menu[] = {
"\n\
    0 - Exit\n\
    1 - Refresh\n\
    2 - Thermometer Menu\n\
    3 - Heart Rate Menu\n\
    4 - Blood Pressure Menu\n\
    \n\
   20 - GATT Menu\n\n\
Your Option:\n"};

static UCHAR thermometer_menu[]={
"\n\
    0 - Exit\n\
    1 - Refresh\n\
    2 - Configure Temperature Measurement Characteristic\n\
    3 - Configure Intermediate Temperature Characteristic\n\
    4 - Configure Periodic Temperature Measurement\n\
    5 - Read Temperature Type\n\n\
\nYour Option:"
};

static UCHAR heart_rate_menu[] = {
"\n\
    0 - Exit\n\
    1 - Refresh\n\
    2 - Configure Heart Rate Characteristic\n\
    3 - Read Sensor Location\n\
    4 - Send Command\n\n\
    5 - Stop Heart Rate Notifications\n\
\nYour Option:"
};

static UCHAR bpm_menu[] = {
"\n\
    0 - Exit\n\
    1 - Refresh\n\
    2 - Configure Blood Pressure Measurement\n\
    3 - Configure Intermediate Blood Pressure Measurement\n\n\
\nYour Option:"
};

static UCHAR hid_host_menu[] =
"\n\
    0 - Exit\n\
    1 - Refresh\n\n\
   10 - Read Protocol Mode\n\
   11 - Read HID Report Descriptor\n\
   12 - Read HID Boot Report Mapping\n\
   13 - Read HID Information\n\n\
   14 - Manually Set - Report CCC\n\n\
   15 - Set Boot Protocol Mode\n\
   16 - Set Report Protocol Mode\n\n\
   20 - HID CP - Suspend\n\
   21 - HID CP - Exit Suspend\n\n\
   30 - Configure CCC - Boot Keyboard\n\
   31 - Configure CCC - Boot Mouse\n\
   32 - Configure CCC - Report\n\
";

static UCHAR puid_client_menu[] =
"\n\
    0 - Exit\n\
    1 - Refresh\n\n\
   --- Link Loss Service ---\n\
   10 - Read Link Loss Service - Alert Level\n\
   11 - Set Link Loss Service - Alert Level - \"No Alert\"\n\
   12 - Set Link Loss Service - Alert Level - \"Mild Alert\"\n\
   13 - Set Link Loss Service - Alert Level - \"High Alert\"\n\n\
   --- Immediate Alert Service ---\n\
   21 - Immediate Alert Service - Alert Level - \"No Alert\"\n\
   22 - Immediate Alert Service - Alert Level - \"Mild Alert\"\n\
   23 - Immediate Alert Service - Alert Level - \"High Alert\"\n\n\
   --- Tx Power Service ---\n\
   30 - Read Tx Power Level\n\n\
   --- Current Time Service ---\n\
   40 - Read Current Time\n\
   41 - Configure CCC - Current Time\n\n\
   --- Network Availability Service ---\n\
   50 - Read Network Availability\n\
   51 - Configure CCC - Network Availability\n\n\
   --- Battery Service ---\n\
   60 - Read Battery Level\n\
   61 - Read Battery Power Staten\n\
   62 - Read Service Required\n\
   63 - Read Removable\n\
   64 - Read Battery Level State\n\n\
   --- Phone Alert Service ---\n\
   70 - Read Alert Status\n\
   71 - Read Ringer Setting\n\
   72 - Set Ringer Control Point\n\
   73 - Configure CCC - Alert Status\n\
   74 - Configure CCC - Ringer Setting\n\n\
";

const APPL_GATT_UUID_DESC uuid_desc_table[] =
{
    {0x9101, "Model Number"},
    {0x9106, "Manufacturer name"},
    {0, NULL}
};


const APPL_UUID_DESC appl_uuid_desc_table[] =
{
    {
        0x1800,
        "GAP Service"
    },
    {
        0x1801,
        "GATT Service"
    },
    {
        0x1802,
        "Immediate Alert Service"
    },
    {
        0x1803,
        "Link Loss Service"
    },
    {
        0x1804,
        "Tx Power Service"
    },
    {
        0x1805,
        "Current Time Service"
    },
    {
        0x1806,
        "Reference Time Update Service"
    },
    {
        0x1807,
        "Next DST Change Service"
    },
    {
        0x180F,
        "Battery Service"
    },
    {
        0x1809,
        "Health Thermometer"
    },
    {
        0x180A,
        "Device Information"
    },
    {
        0x180B,
        "Network Availability"
    },
    {
        0x180C,
        "Watchdog Service"
    },
    {
        0x180E,
        "Phone Alert Status Service"
    },
    {
        0x180D,
        "Heart Rate Service"
    },
    {
        0x2A00,
        "Device Name"
    },
    {
        0x2A01,
        "Appearance"
    },
    {
        0x2A02,
        "Peripheral Privacy Flag"
    },
    {
        0x2A03,
        "Reconnection Address"
    },
    {
        0x2A04,
        "Peripheral Preferred Connection Parameters"
    },
    {
        0x2A05,
        "Service Changed"
    },
    {
        0x2A06,
        "Alert Level"
    },
    {
        0x2A07,
        "Tx Power Level"
    },
    {
        0x2A08,
        "Date Time"
    },
    {
        0x2A09,
        "Day of Week"
    },
    {
        0x2A0A,
        "Time"
    },
    {
        0x2A0B,
        "Exact Time 100"
    },
    {
        0x2A0C,
        "Exact Time 256"
    },
    {
        0x2A0D,
        "Daylight Saving Time"
    },
    {
        0x2A0E,
        "Time Zone"
    },
    {
        0x2A0F,
        "Local Time Information"
    },
    {
        0x2A10,
        "Secondary Time Zone"
    },
    {
        0x2A11,
        "Time with DST"
    },
    {
        0x2A12,
        "Time Accuracy"
    },
    {
        0x2A13,
        "Time Source"
    },
    {
        0x2A14,
        "Reference Time Information"
    },
    {
        0x2A15,
        "Time Broadcast"
    },
    {
        0x2A16,
        "Time Update Control Point"
    },
    {
        0x2A17,
        "Time Update State"
    },
    {
        0x2A18,
        "Boolean"
    },
    {
        0x2A19,
        "Battery Level"
    },
    {
        0x2A1A,
        "Battery Power State"
    },
    {
        0x2A1B,
        "Battery Level State"
    },
    {
        0x2A1C,
        "Temperature Measurement"
    },
    {
        0x2A1D,
        "Temperature Type"
    },
    {
        0x2A1E,
        "Intermediate Temperature"
    },
    {
        0x2A1F,
        "Temperature Celsius"
    },
    {
        0x2A20,
        "Temperature Fahrenheit"
    },
    {
        0x2A21,
        "Measurement Interval"
    },
    {
        0x2A22,
        "HID Boot Report Mapping"
    },
    {
        0x2A23,
        "System ID"
    },
    {
        0x2A24,
        "Model Number String"
    },
    {
        0x2A25,
        "Serial Number String"
    },
    {
        0x2A26,
        "Firmware Revision String"
    },
    {
        0x2A27,
        "Hardware Revision String"
    },
    {
        0x2A28,
        "Software Revision String"
    },
    {
        0x2A29,
        "Manufacturer Name String"
    },
    {
        0x2A2A,
        "IEEE 11073-20601 Regulatory Certification Data List"
    },
    {
        0x2A2B,
        "Current Time"
    },
    {
        0x2A2C,
        "Elevation"
    },
    {
        0x2A2D,
        "Latitude"
    },
    {
        0x2A2E,
        "Longitude"
    },
    {
        0x2A2F,
        "Position 2D"
    },
    {
        0x2A30,
        "Position 3D"
    },
    {
        0x2A31,
        "Vendor ID"
    },
    {
        0x2A32,
        "Product ID"
    },
    {
        0x2A33,
        "HID Version"
    },
    {
        0x2A34,
        "Vendor ID Source"
    },
    {
        0x2A35,
        "Blood Pressure Measurement"
    },
    {
        0x2A36,
        "Intermediate Blood Pressure"
    },
    {
        0x2A37,
        "Heart Rate Measurement"
    },
    {
        0x2A38,
        "Heart Rate Sensor Location"
    },
    {
        0x2A39,
        "Heart Rate Control Point"
    },
    {
        0x1810,
        "Blood Pressure Monitor"
    },
    {
        0x1811,
        "Alert Notification"
    },
    {
        0x2A35,
        "Blood Pressure Measurement"
    },
    {
        0x2A36,
        "Intermediate Cuff Measurement"
    },
    {
        0x2A3E,
        "Network Availability"
    },
    {
        0x2A3F,
        "Alert Status"
    },
    {
        0x2A40,
        "Ringer Control Point"
    },
    {
        0x2A41,
        "Ringer Setting"
    },
    {
        0x2A49,
        "Blood Pressure Feature"
    },
    {
        0x1812,
        "Human Interface Device Service"
    },
    {
        0x2A4A,
        "HID Information"
    },
    {
        0x2A4B,
        "HID Report Descriptor"
    },
    {
        0x2A4C,
        "HID Control Point"
    },
    {
        0x2A4D,
        "HID Report"
    },
    {
        0x2A4E,
        "HID Protocol Mode"
    },
    {
        0x1813,
        "Scan Parameter Service"
    },
    {
        0x2A4F,
        "Scan Interval Window"
    },
    {
        0x0000,
        NULL
    }
};

/* --------------------------------------------- Functions */


void appl_att_client_init (void)
{
    UINT32 index;

    index = APPL_MAX_CHARACTERISTICS;
    do
    {
        index--;
        characteristic_array[index].desc.client_config =
            APPL_INVLAID_CHAR_CONFIGURATION;
        characteristic_array[index].desc.client_config_handle =\
                                    ATT_INVALID_ATTR_HANDLE_VAL;
    } while (index >0);
}


API_RESULT att_cb
           (
               ATT_HANDLE    * handle,
               UCHAR         att_event,
               API_RESULT    event_result,
               UCHAR         * event_data,
               UINT16        event_datalen
           )
{
    int i;
    API_RESULT retval;
    UINT16 arg_len;
    UINT16 attr_handle;
    UINT16 offset;
    UINT16 mtu;
    UCHAR op_code;
    UCHAR rsp_code;

    APPL_TRC
    ("[ATT]:[0x%02X]: Received ATT Event 0x%02X with result 0x%04X",
    *handle,att_event,event_result);

    appl_dump_bytes (event_data, event_datalen);

    switch(att_event)
    {
    case ATT_CONNECTION_IND:
        APPL_TRC (
        "[0x%02X]:Received Connection Indtication, Result 0x%04X!\n",
        *handle,event_result);
        appl_gatt_client_handle = *handle;
        break;

    case ATT_DISCONNECTION_IND:
        APPL_TRC (
        "[0x%02X]:Received Disconnection Indtication, Result 0x%04X!\n",
        *handle,event_result);

        appl_handle_disconnection_indication();

        /* Reset any ongoing transaction for HID */
        appl_read_info.hdl = ATT_INVALID_ATTR_HANDLE_VAL;
        appl_read_info.offset = 0;
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
        "= 0x%04X!\n",mtu);

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

    case ATT_FIND_INFO_RSP:
        printf ("Received Find Information Response Opcode!\n");

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
    case ATT_READ_BY_TYPE_RSP:
        printf ("Received Read Type Response Opcode 0x%04X!\n",event_result);
        if (NULL == event_data)
        {
            break;
        }

        arg_len = event_data[0];

        appl_handle_read_by_type_response
        (
            arg_len,
            &event_data[1],
            --event_datalen
        );
        break;
    case ATT_READ_BY_GROUP_RSP:
        if (NULL == event_data)
        {
            break;
        }

        printf ("Received Read Group Type Response Opcode!\n");

        arg_len = event_data[0];

        event_datalen --;
        event_data++;

        appl_handle_read_by_group_response (arg_len,event_data,event_datalen);
        printf ("\n");
        break;
    case ATT_FIND_BY_TYPE_VAL_RSP:
        printf ("Received Find by Type Value Response Opcode!\n");

        if (NULL == event_data)
        {
            break;
        }

        printf ("Found Handle        End Found Handle\n");
        for (i = 0; i < event_datalen; i+=4)
        {
            BT_UNPACK_LE_2_BYTE(&attr_handle, (event_data+i));
            printf ("%04X",attr_handle);
            BT_UNPACK_LE_2_BYTE(&attr_handle, (event_data+i+2));
            printf ("                  %04X\n", attr_handle);
        }
        break;

    case ATT_READ_RSP:
        printf ("Received Read Response Opcode!\n");

        printf ("Handle Value Received - \n");
        appl_dump_bytes(event_data, event_datalen);
        appl_handle_read_response (event_data,event_datalen);
        printf ("\n");
        break;

    case ATT_READ_BLOB_RSP:
        printf ("Received Read Response Opcode!\n");
        printf ("Handle Value Received - \n");
        appl_dump_bytes(event_data, event_datalen);
        printf ("\n");
        appl_handle_read_blob_response (event_data, event_datalen);
        break;

    case ATT_READ_MULTIPLE_RSP:
        printf ("Received read multiple response Opcode!\n");

        printf ("Byte stream of values received [not seperated by handle]");
        appl_dump_bytes(event_data, event_datalen);
        printf ("\n");
        break;
    case ATT_WRITE_RSP:
        printf ("Received Write Response Opcode!\n");
        break;

    case ATT_PREPARE_WRITE_RSP:
        printf ("Prepare Write Response, Event Result 0x%04X\n",event_result);
        if (NULL != event_data)
        {
            BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
            printf ("Handle - 0x%04X\n", attr_handle);
            BT_UNPACK_LE_2_BYTE(&offset, event_data+2);
            printf ("Offset - 0x%04X\n", offset);
            printf ("Handle Value Received - \n");
            appl_dump_bytes(event_data + 4, event_datalen - 4);
        }
        printf ("\n");
        break;

    case ATT_EXECUTE_WRITE_RSP:
        printf ("Received Execute Write Response\n");
        break;

    case ATT_HANDLE_VALUE_NTF:
        printf ("Received HVN\n");
        BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
        printf ("Handle - 0x%04X\n", attr_handle);
        printf ("Handle Value Received - \n");
        appl_dump_bytes(event_data + 2, (event_datalen - 2));
        printf ("\n");
        break;

    case ATT_HANDLE_VALUE_IND:
        printf ("Received HVI\n");
        BT_UNPACK_LE_2_BYTE(&attr_handle, event_data);
        printf ("Handle - 0x%04X\n", attr_handle);
        printf ("Handle Value Received - \n");
        appl_dump_bytes(event_data + 2, (event_datalen-2));

        retval = BT_att_send_hndl_val_cnf (&appl_gatt_client_handle);

        break;
    default:
        break;
    }
    printf("\n\n");

    return API_SUCCESS;
}

void main_gatt_client_operations(void)
{
    int                               data, index;
    INT32                             choice;
    UINT16                            mtu;
    UINT16                            start_handle,end_handle;
    API_RESULT                        retval;
    ATT_READ_REQ_PARAM                read_req_param;
    ATT_READ_BLOB_REQ_PARAM           read_blob_req_param;
    ATT_WRITE_REQ_PARAM               write_req_param;
    ATT_FIND_BY_TYPE_VAL_REQ_PARAM    find_info_param;
    ATT_READ_MULTIPLE_REQ_PARAM       read_multiple_param;
    ATT_PREPARE_WRITE_REQ_PARAM       prepare_write_param;
    ATT_EXECUTE_WRITE_REQ_PARAM       execute_write_param;
    UCHAR value[20];

    while (1)
    {
        printf("%s \n", print_str);
        printf("Enter you choice : ");
        scanf("%d", &choice);
        switch(choice)
        {
        case 0:
            return;
        case 1:
            break;

        case 2: /* Exchange MTU Request */

            printf ("Enter the MTU for the ATT connection\n");
            scanf ("%x", &data);
            mtu = (UINT16)data;

            retval = BT_att_send_mtu_xcnhg_req
                     (
                         &appl_gatt_client_handle,
                         &mtu
                     );

            printf ("Exchange MTU REQ retval - 0x%04X\n", retval);
            break;

        case 10: /* Discover All Services */
            appl_att_initiate_service_discovery ();
            break;
        case 11: /* Discover Service */
            /* TBD: Write a Specific Application */
            appl_att_initiate_service_discovery ();
            break;

        case 12: /* Characteristic Descripotor Discovery */
        {
            printf ("Enter Characteristic Index (in Dec) to be Discovered,"
            "Max Value %d\n",appl_characteristic_index);
            scanf ("%d",&data);

            if (data >= appl_characteristic_index)
            {
                printf ("[***ERR***]: Invalid Characteristic Index!\n");
            }
            else
            {
                /* Characteristic & Its Value are skipped */
                start_handle = characteristic_array[data].desc.value_handle;
                if ((data + 1) == appl_characteristic_index)
                {
                    end_handle = ATT_ATTR_HANDLE_END_RANGE;
                }
                else
                {
                    if (data == service_array[characteristic_array[data].\
                        desc.s_index].char_end_range)
                    {
                        end_handle = service_array[characteristic_array[data].\
                            desc.s_index].service_info.range.end_handle;
                    }
                    else
                    {
                        end_handle =
                        (characteristic_array[data+1].desc.value_handle - 2);
                    }
                }
                appl_discover_characteristic_descriptor
                (
                     start_handle,
                     end_handle
                );
            }


        }
        break;

        case 13: /* Configure Characteristic */
            break;

        case 14: /* Read Characteristic Value */
            printf ("Enter handle[HEX] : ");

            scanf("%x", &data);

            read_req_param = (UINT16) data;

            retval = BT_att_send_read_req
                     (
                          &appl_gatt_client_handle,
                          &read_req_param
                      );

            APPL_TRC (
            "ATT Read Request sent with retval 0x%04X\n", retval);
            break;

        case 15: /* Write Characteristic Value */
            printf ("Enter handle[HEX] : ");

            scanf("%x", &data);
            write_req_param.handle = (UINT16) data;
            printf ("Enter value len[HEX] : ");

            scanf("%x", &write_req_param.value.len);
            printf ("Enter value[HEX] : ");

            for(index = 0; index < write_req_param.value.len; index++)
            {
                scanf("%x", &data);
                value [index] = (UCHAR)data;
            }

            write_req_param.value.val = value;

            retval = BT_att_send_write_req
                     (
                          &appl_gatt_client_handle,
                          &write_req_param
                      );

            APPL_TRC (
            "ATT Write Command Result - 0x%04X, Actual Value Len = 0x%04X\n",
            retval,write_req_param.value.actual_len);
            break;

        case 16: /* Write without Response Characteristic Value */
            printf ("Enter handle[HEX] : ");

            scanf("%x", &data);
            write_req_param.handle = (UINT16) data;
            printf ("Enter value len[HEX] : ");

            scanf("%x", &write_req_param.value.len);
            printf ("Enter value[HEX] : ");

            for(index = 0; index < write_req_param.value.len; index++)
            {
                scanf("%x", &data);
                value [index] = (UCHAR)data;
            }

            write_req_param.value.val = value;

            retval = BT_att_send_write_cmd
                     (
                          &appl_gatt_client_handle,
                          &write_req_param
                      );

            APPL_TRC (
            "ATT Write Command Result - 0x%04X, Actual Value Len = 0x%04X\n",
            retval,write_req_param.value.actual_len);
            break;

        case 17: /* Read Blob */
            printf ("Enter handle[HEX] : ");
            scanf("%x", &data);
            read_blob_req_param.handle = data;

            printf ("Enter Offset[DEC] : ");
            scanf("%d", &data);
            read_blob_req_param.offset = data;

            appl_att_read_blob_request (&read_blob_req_param);
            break;

        case 20:
            appl_thermometer_options ();
            break;
        case 21:
            appl_heart_rate_options ();
            break;
        case 22:
            appl_bpm_options ();
            break;
        case 30:
            {
                printf ("Enter Start handle\n");
                scanf ("%x",&data);
                find_info_param.range.start_handle = data;

                printf ("Enter End handle\n");
                scanf ("%x",&data);
                find_info_param.range.end_handle = data;

                printf ("Enter Type (HEX 2 Octet)\n");
                scanf ("%x",&data);
                find_info_param.uuid = data;

                printf ("Enter Value (HEX 2 octect)\n");
                scanf ("%x",&data);

                BT_PACK_LE_2_BYTE (value,&data);
                find_info_param.value.val = value;
                find_info_param.value.len = 2;

                retval = BT_att_send_find_by_type_val_req
                         (
                              &appl_gatt_client_handle,
                              &find_info_param
                         );

            }
            break;
        case 31: /* Read multiple */
            {
                printf ("Enter number of handles\n");
                scanf ("%x",&data);

                if (0 != data)
                {
                    read_multiple_param.list_count = data;
                    read_multiple_param.handle_list = \
                        BT_alloc_mem ( 2 * read_multiple_param.list_count);
                    if (NULL == read_multiple_param.handle_list)
                    {
                        break;
                    }
                    for (index = 0; index < read_multiple_param.list_count;
                        index++)
                    {
                        printf ("[0x%04X]:Enter handle in HEX:\n",index);
                        scanf ("%x",&data);
                        read_multiple_param.handle_list[index] = data;
                    }
                    appl_att_read_multiple_request (&read_multiple_param);
                    BT_free_mem (read_multiple_param.handle_list);
                }
                else
                {
                    APPL_TRC (
                    "[APPL]: Invalid Entry, try again!");
                }
            }
            break;
        case 32:
            {
                printf ("Enter handle in Hex:\n");
                scanf ("%x",&data);
                prepare_write_param.handle_value.handle = data;

                printf ("Enter Value Offset in Decimal:\n");
                scanf ("%d",&data);
                prepare_write_param.offset = data;

                printf ("Enter Value length in Decimal:\n");
                scanf ("%d",&data);
                prepare_write_param.handle_value.value.len = data;

                if (0 == prepare_write_param.handle_value.value.len)
                {
                    APPL_TRC (
                    "[APPL]: Invalid Zero Length, cannot process request!\n");
                    break;
                }

                prepare_write_param.handle_value.value.val = BT_alloc_mem \
                    (prepare_write_param.handle_value.value.len);

                if (NULL == prepare_write_param.handle_value.value.val)
                {
                    APPL_TRC (
                    "[APPL]: Failed to allocate memory of size 0x%04X\n",
                    prepare_write_param.handle_value.value.len);

                    break;
                }

                printf ("Enter Value in Hex:\n");
                for (index = 0;
                    index < prepare_write_param.handle_value.value.len; index ++)
                {
                    scanf ("%x",&data);
                    prepare_write_param.handle_value.value.val[index] = data;
                }

                appl_att_prepare_write_request (&prepare_write_param);

                BT_free_mem (prepare_write_param.handle_value.value.val);
            }
            break;
        case 33:
            {
                APPL_TRC (
                "[APPL]: Enter := \n\t0 to Cancel\n\t1 to Execute Queued "
                "Requests");
                scanf ("%d",&data);

                if (data > 1)
                {
                    APPL_TRC (
                    "[APPL]:[*** ERR ***]: Invalid Entry, Cannot Process "
                    "Execute Write Request\n");

                    break;
                }

                execute_write_param.flags = data;

                appl_att_execute_write_request (&execute_write_param);
            }
            break;
        case 34:
        {
            ATT_FIND_INFO_REQ_PARAM    find_info_req_param;

            printf ("Enter Start Handle (in HEX)\n");
            scanf ("%04X",&data);
            find_info_req_param.start_handle = (UINT16)data;

            printf ("Enter End Handle (in HEX)\n");
            scanf ("%04X",&data);
            find_info_req_param.end_handle = (UINT16)data;

            retval = BT_att_send_find_info_req
                     (
                          &appl_gatt_client_handle,
                          &find_info_req_param
                     );

            printf (
            "Sent Find Info Request with Result 0x%04X\n",retval);
        }
        break;

        case 50:
            appl_hid_host_options ();
            break;

        case 60:
            appl_puid_client_options ();
            break;

        default:
            break;
        }

    }

}

void appl_att_initiate_service_discovery (void)
{
    ATT_HANDLE_RANGE    range;
    printf ("Initiating Service Discovery Procedure..\n");

    /**
     * Search is performed accross all handles to discover all Primary services
     * UUID for Primary Services is 0x2800
     */
    printf ("Discovering Primary Services\n");
    appl_service_index = 0;
    peer_info.pri_start_range = 0;
    appl_characteristic_index = 0;
    range.start_handle = ATT_ATTR_HANDLE_START_RANGE;
    range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
    appl_discover_primary_services
    (
        &range
    );
}

void appl_att_read_by_type
     (
         /* IN */ ATT_HANDLE_RANGE * range,
         /* IN */ UINT16 uuid
     )
{
    ATT_READ_BY_TYPE_REQ_PARAM    find_info_param;
    API_RESULT   retval;

    printf ("Searching for UUID 0x%04X, from handle 0x%04X to 0x%04X\n",
    uuid,range->start_handle,range->end_handle);
    find_info_param.range = *range;
    find_info_param.uuid = uuid;
    find_info_param.uuid_format = ATT_16_BIT_UUID_FORMAT;

    peer_info.state.handle = range->end_handle;
    peer_info.state.uuid = uuid;

    retval = BT_att_send_read_by_type_req
             (
                 &appl_gatt_client_handle,
                 &find_info_param
             );

    APPL_TRC(
    "[ATT]: Read by Type Request Initiated with result 0x%04X",retval);
}

void appl_att_read_by_group_type
     (
         /* IN */ ATT_HANDLE_RANGE * range,
         /* IN */ UINT16             uuid
     )
{
    ATT_READ_BY_GROUP_TYPE_REQ_PARAM    find_info_param;
    API_RESULT   retval;

    printf ("Searching for UUID 0x%04X, from handle 0x%04X to 0x%04X\n",
    uuid,range->start_handle,range->end_handle);
    find_info_param.range = *range;
    find_info_param.group_type = uuid;
    find_info_param.uuid_format = ATT_16_BIT_UUID_FORMAT;

    peer_info.state.handle = range->end_handle;
    peer_info.state.uuid = uuid;

    retval = BT_att_send_read_by_group_req
             (
                 &appl_gatt_client_handle,
                 &find_info_param
             );

    APPL_TRC(
    "[ATT]: Read by Type Request Initiated with result 0x%04X",retval);
}

void appl_handle_read_by_group_response
     (
         UINT16 data_elem_size,
         UCHAR * data,
         UINT16 data_len
     )
{
    ATT_HANDLE_RANGE range;
    UINT32 index;
    UINT16 diff_in_size;
    UCHAR  * value;
    UCHAR  search_again;
    UINT16 att_mtu = ATT_DEFAULT_MTU;

    BT_att_access_mtu(&appl_gatt_client_handle, &att_mtu);

    search_again = 0;


    /**
     * Check if the response received was smaller than max MTU, then no further
     * searching is needed
     */
    /* diff_in_size = appl_mtu_size - data_len - L2CAP_HDR_LEN; */
    diff_in_size = att_mtu - data_len -1;

    /* TBD: To handle short response from Server */
#if 0
    if (0 == diff_in_size || ((0 != diff_in_size)
       && (diff_in_size < data_elem_size)))
#endif /* 0 */
    {
        /**
         * There could be more requests, as the MTU has been optimally utilized
         */
        search_again = 1;
    }
    for (index = 0; index < (unsigned)(data_len/(data_elem_size)); index++ )
    {
        BT_UNPACK_LE_2_BYTE
        (
            &range.start_handle,
            (data + (index * data_elem_size))
        );

        BT_UNPACK_LE_2_BYTE
        (
            &range.end_handle,
            (data + ((index * data_elem_size)+2))
        );
        printf (
        "Handle Range 0x%04X - 0x%04X\n",range.start_handle,range.end_handle);
        printf ("Handle Value Received - \n");
        value = data + (4+index * data_elem_size);
        appl_dump_bytes(value, data_elem_size-4);

        if ((GATT_PRIMARY_SERVICE_UUID == peer_info.state.uuid) ||
           (GATT_SECONDARY_SERVICE_UUID == peer_info.state.uuid ))
        {
            /**
             * Check if the Value is of size 2, as the application handles only
             * 16 bit UUIDs
             */
            if (data_elem_size-4 != 2)
            {
                printf ("[*** ERR ***]: Cannot Handle Service Values of Size"
                "0x%04X\n",data_elem_size-4);
                range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
                break;
            }

            /* Store Primary Service Related information in the peer */
            service_array[appl_service_index].service_info.range = range;
            BT_UNPACK_LE_2_BYTE
            (
                &service_array[appl_service_index].service_info.value,
                value
            );

            switch(service_array[appl_service_index].service_info.value)
            {
            case GATT_THERM_SERVICE_UUID:
                appl_therm_sindex = appl_service_index;
                break;
            case GATT_HR_SERVICE_UUID:
                appl_hr_sindex = appl_service_index;
                break;
            case GATT_BPM_SERVICE_UUID:
                appl_bpm_sindex = appl_service_index;
                break;

            default:
                break;
            }

            appl_service_index ++;
            if (APPL_MAX_SERVICES == appl_service_index)
            {
                /* Cannot process any further Services */
                printf ("[*** Alert***]: Cannot process further requests\n");
                break;
            }

        }
    }

    /**
     * If the last handle is ATT_ATTR_HANDLE_END_RANGE, there is no need to
     * search again, go to the next logical step!
     */
    if (ATT_ATTR_HANDLE_END_RANGE == range.end_handle)
    {
        search_again = 0;
    }

    /**
     * Decide the next step:
     * -If discovery of Primary Services is complete, discover
     * characteristics of each of the services.
     * - If discovery for characteristic in the Primary Service is complete,
     * search for secondary services.
     * If discovery secondary services is complete, discover characteristics of
     * each ssecondary service.
     */
    if (GATT_PRIMARY_SERVICE_UUID == peer_info.state.uuid)
    {
        if (0 == search_again)
        {
            peer_info.pri_end_range = appl_service_index-1;
            appl_handle_primary_service_search_complete (range.end_handle);
        }
        else
        {
            range.start_handle = range.end_handle+1;
            range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            /* There could be more Primary Services Discover them */
            appl_discover_primary_services(&range);
        }
    }
    else if (GATT_SECONDARY_SERVICE_UUID == peer_info.state.uuid)
    {
        if (0 == search_again)
        {
            appl_handle_secondary_service_search_complete (range.end_handle);
        }
        else
        {
            range.start_handle = range.end_handle;
            range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            /* Search Further for Secondary Services */
            appl_discover_secondary_services
            (
                &range
            );
        }
    }
}


void appl_handle_read_response (UCHAR * data, UINT16 datalen)
{
    UINT16 att_mtu = ATT_DEFAULT_MTU;

    BT_att_access_mtu(&APPL_GET_ATT_INSTANCE(appl_handle), &att_mtu);

    /* Check if it is HDI Service Characteristics */
    if (GATT_HDI_SERVICE_UUID == peer_info.state.uuid)
    {
        /* Display Device information */
        if (GATT_DIS_SYSTEM_ID_CHAR_UUID ==
            characteristic_array[peer_info.state.handle].desc.uuid)
        {
            BT_assert (8 == datalen);
            printf ("System ID: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
            data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
        }
        else
        {
            printf ("%s: %s\n",
            appl_uuid_desc
            (
                characteristic_array[peer_info.state.handle].desc.uuid),
                data
            );
        }

        if (peer_info.state.handle ==
            service_array[peer_info.state.service_index].char_end_range)
        {
            /* Search is complete */
            peer_info.state.service_index = APPL_INVALID_SERVICE_INDEX;
            peer_info.state.uuid = APPL_INVALID_UUID;
        }
        else
        {
            peer_info.state.handle++;
            appl_attr_read_value
            (
                characteristic_array[peer_info.state.handle].desc.value_handle
            );
        }
    }
    else if (HID_BOOT_RPT_MAPPING_HDL == appl_read_info.hdl)
    {
        appl_read_info.hdl = ATT_INVALID_ATTR_HANDLE_VAL;

        /**
         *  Contains following attribute handles
         *
         *  - HID Control Point Characteristic handle
         *  - Boot Keyboard INPUT HID Report Characteristic handle
         *  - Client Characteristic Configuration Descriptor handle
         *    for the Boot Keyboard INPUT HID Report
         *  - Boot Keyboard OUTPUT HID Report Characteristic handle
         *  - Boot Mouse INPUT HID Report Characteristic handle
         *  - Client Characteristic Configuration Descriptor handle
         *    for the Boot Mouse INPUT HID Report
         */

        /* Validate Length */
        if (12 == datalen)
        {
#define HID_BOOT_MAPPING_KBD_INPUT_OFFSET 2

            /* Extract Handles */
            BT_UNPACK_LE_2_BYTE
            (
                &HID_KBD_INPUT_HDL,
                data + HID_BOOT_MAPPING_KBD_INPUT_OFFSET
            );
            printf ("Keyboard Input Handle: 0x%04X\n", HID_KBD_INPUT_HDL);

            BT_UNPACK_LE_2_BYTE
            (
                &HID_KBD_CCC_HDL,
                data + HID_BOOT_MAPPING_KBD_INPUT_OFFSET + 2
            );
            printf ("Keyboard CCC Handle: 0x%04X\n", HID_KBD_CCC_HDL);

            BT_UNPACK_LE_2_BYTE
            (
                &HID_KBD_OUTPUT_HDL,
                data + HID_BOOT_MAPPING_KBD_INPUT_OFFSET + 4
            );
            printf ("Keyboard Output Handle: 0x%04X\n", HID_KBD_OUTPUT_HDL);

            BT_UNPACK_LE_2_BYTE
            (
                &HID_MOUSE_INPUT_HDL,
                data + HID_BOOT_MAPPING_KBD_INPUT_OFFSET + 6
            );
            printf ("Mouse Input Handle: 0x%04X\n", HID_MOUSE_INPUT_HDL);

            BT_UNPACK_LE_2_BYTE
            (
                &HID_MOUSE_CCC_HDL,
                data + HID_BOOT_MAPPING_KBD_INPUT_OFFSET + 8
            );
            printf ("Mouse CCC Handle: 0x%04X\n", HID_MOUSE_CCC_HDL);
        }
    }
    else if (HID_INFO_HDL == appl_read_info.hdl)
    {
        UINT16 value_2;
        UCHAR  value_1;

        /* HIDParserVersion - 2 Octet */
        BT_UNPACK_LE_2_BYTE
        (
            &value_2,
            data
        );
        printf ("HIDParserVersion: 0x%04X\n", value_2);

        /* HIDDeviceSubClass - 1 Octet */
        BT_UNPACK_LE_1_BYTE
        (
            &value_1,
            data + 2
        );
        printf ("HIDDeviceSubClass: 0x%02X\n", value_1);

        /* HIDCountryCode - 1 Octet */
        BT_UNPACK_LE_1_BYTE
        (
            &value_1,
            data + 3
        );
        printf ("HIDCountryCode: 0x%02X\n", value_1);

        /* Flags */
        BT_UNPACK_LE_1_BYTE
        (
            &value_1,
            data + 4
        );
        printf ("Flags: 0x%02X\n", value_1);

        if (HID_FLAGS_RECONNECT_INITIATE ==
            (value_1 & HID_FLAGS_RECONNECT_INITIATE))
        {
            printf ("Reconnect Initiate Supported\n");
        }
        else
        {
            printf ("Reconnect Initiate Not Supported\n");
        }

        if (HID_FLAGS_REMOTE_WAKE ==
            (value_1 & HID_FLAGS_REMOTE_WAKE))
        {
            printf ("Remote Wake Supported\n");
        }
        else
        {
            printf ("Remote Wake Not Supported\n");
        }

        if (HID_FLAGS_BOOT_DEVICE ==
            (value_1 & HID_FLAGS_BOOT_DEVICE))
        {
            printf ("Boot Protocol Mode Supported\n");
        }
        else
        {
            printf ("Boot Protocol Mode Not Supported\n");
        }

        if (HID_FLAGS_NORMALLY_CONNECTABLE ==
            (value_1 & HID_FLAGS_NORMALLY_CONNECTABLE))
        {
            printf ("Normally Connectable Supported\n");
        }
        else
        {
            printf ("Normally Connectable Not Supported\n");
        }
    }
    else if (HID_REPORT_DESC_HDL == appl_read_info.hdl)
    {
        if ((att_mtu - 1) == datalen)
        {
            ATT_READ_BLOB_REQ_PARAM read_blob_req;

            /* Save the length of data read */
            appl_read_info.offset = datalen;

            read_blob_req.handle = HID_REPORT_DESC_HDL;
            read_blob_req.offset = appl_read_info.offset;

            /* Initiate Read Blob */
            appl_att_read_blob_request (&read_blob_req);
        }
        else
        {
            appl_read_info.hdl = ATT_INVALID_ATTR_HANDLE_VAL;
            appl_read_info.offset = 0;
        }
    }
}

void appl_handle_read_blob_response (UCHAR * data, UINT16 datalen)
{
    UINT16 att_mtu = ATT_DEFAULT_MTU;

    BT_att_access_mtu(&APPL_GET_ATT_INSTANCE(appl_handle), &att_mtu);

    if (HID_REPORT_DESC_HDL == appl_read_info.hdl)
    {
        if ((att_mtu - 1) == datalen)
        {
            ATT_READ_BLOB_REQ_PARAM read_blob_req;

            /* Save the length of data read */
            appl_read_info.offset += datalen;

            read_blob_req.handle = HID_REPORT_DESC_HDL;
            read_blob_req.offset = appl_read_info.offset;

            /* Initiate Read Blob */
            appl_att_read_blob_request (&read_blob_req);
        }
        else
        {
            appl_read_info.hdl = ATT_INVALID_ATTR_HANDLE_VAL;
            appl_read_info.offset = 0;
        }
    }
}

void appl_handle_read_by_type_response
     (
         /* IN */ UINT16   data_elem_size,
         /* IN */ UCHAR   * data,
         /* IN */ UINT16  data_len
     )
{
    ATT_HANDLE_RANGE range;
    UINT32 index;
    UINT16 handle;
    UINT16 diff_in_size;
    UCHAR  * value;
    UCHAR  search_again;
    UINT16 att_mtu = ATT_DEFAULT_MTU;

    BT_att_access_mtu(&appl_gatt_client_handle, &att_mtu);

    search_again = 0;

    /**
     * Check if the response received was smaller than max MTU, then no further
     * searching is needed
     */
    /* diff_in_size = appl_mtu_size - data_len - L2CAP_HDR_LEN; */
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
        printf ("Handle - 0x%04X\n", handle);
        printf ("Handle Value Received - \n");
        value = data + (2+index * data_elem_size);
        appl_dump_bytes(value, data_elem_size-2);

        if ((GATT_PRIMARY_SERVICE_UUID == peer_info.state.uuid) ||
           (GATT_SECONDARY_SERVICE_UUID == peer_info.state.uuid ))
        {
            /**
             * Check if the Value is of size 2, as the application handles only
             * 16 bit UUIDs
             */
            if (data_elem_size-2 != 2)
            {
                printf ("[*** ERR ***]: Cannot Handle Service Values of Size"
                "0x%04X\n",data_elem_size-2);
                handle = ATT_ATTR_HANDLE_END_RANGE;
                break;
            }

            /* Store Primary Service Related information in the peer */
            service_array[appl_service_index].service_info.range.start_handle =\
                handle;
            BT_UNPACK_LE_2_BYTE
            (
                &service_array[appl_service_index].service_info.value,
                value
            );

            switch(service_array[appl_service_index].service_info.value)
            {
            case GATT_THERM_SERVICE_UUID:
                appl_therm_sindex = appl_service_index;
                break;
            case GATT_HR_SERVICE_UUID:
                appl_hr_sindex = appl_service_index;
                break;
            case GATT_BPM_SERVICE_UUID:
                appl_bpm_sindex = appl_service_index;
                break;
            default:
                break;
            }

            appl_service_index ++;
            if (20 == appl_service_index)
            {
                /* Cannot process any further Services */
                printf ("[*** Alert***]: Cannot process further requests\n");
                break;
            }

        }
        else if (GATT_CHARACTERISTIC_UUID == peer_info.state.uuid)
        {
            characteristic_array[appl_characteristic_index].desc.s_index = \
                (UCHAR)peer_info.state.service_index;
            characteristic_array[appl_characteristic_index].handle = handle;
            /* Extract Descriptor */
#if 0
            ATT_UNPACK_UINT8
            (
                characteristic_array[appl_characteristic_index].desc.c_property,
                value[0]
            );
#else /* 0 */
            characteristic_array[appl_characteristic_index].desc.c_property = \
                value[0];
#endif /* 0 */

            BT_UNPACK_LE_2_BYTE
            (
                &characteristic_array[appl_characteristic_index].desc.\
                value_handle,
                value+1
            );

            BT_UNPACK_LE_2_BYTE
            (
                &characteristic_array[appl_characteristic_index].desc.uuid,
                value+3

            );

            switch (characteristic_array[appl_characteristic_index].desc.uuid)
            {
            case GATT_TEMP_CHAR_UUID:
                appl_therm_cindex = appl_characteristic_index;
                break;
            case GATT_INTER_TEMP_CHAR_UUID:
                appl_itherm_cindex = appl_characteristic_index;
                break;
            case GATT_HR_CHAR_UUID:
                appl_hr_cindex = appl_characteristic_index;
                break;
            case GATT_HR_SENSOR_LOC_CHAR_UUID:
                appl_hr_sensor_loc_cindex = appl_characteristic_index;
                break;
            case GATT_BPM_CHAR_UUID:
                appl_bpm_cindex = appl_characteristic_index;
                break;
            case GATT_INTER_BPM_UUID:
                appl_ibpm_cindex = appl_characteristic_index;
                break;

            /* HID Characteristic Handle */
            case GATT_HID_PROTO_MODE_CHARACTERISTIC:
                HID_PROTO_MODE_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_HID_RPT_DESC_CHARACTERISTIC:
                HID_REPORT_DESC_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_HID_BOOT_RPT_MAP_CHARACTERISTIC:
                HID_BOOT_RPT_MAPPING_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_HID_RPT_CHARACTERISTIC:
                HID_REPORT_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_HID_CP_CHARACTERISTIC:
                HID_CP_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_HID_INFO_CHARACTERISTIC:
                HID_INFO_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            /* PUID Characteristic Handles */
            case GATT_ALERT_LEVEL_CHARACTERISTIC:
                if (GATT_LINK_LOSS_SERVICE == service_array[peer_info.state.service_index].\
                    service_info.value)
                {
                    LLS_ALERT_LEVEL_HDL =
                    characteristic_array[appl_characteristic_index].desc.\
                    value_handle;
                }
                else if ((GATT_FIND_ME_SERVICE == service_array[peer_info.state.service_index].\
                    service_info.value) ||
                         (GATT_IMMEDIATE_ALERT_SERVICE == service_array[peer_info.state.service_index].\
                    service_info.value))
                {
                    if (IAS_ALERT_LEVEL_HDL == ATT_INVALID_ATTR_HANDLE_VAL)
                    {
                        IAS_ALERT_LEVEL_HDL =
                        characteristic_array[appl_characteristic_index].desc.\
                        value_handle;
                    }
                }
                break;

            case GATT_ALERT_STATUS_CHARACTERISTIC:
                PAS_ALERT_STATUS_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;

                PAS_ALERT_STATUS_CCC_HDL = PAS_ALERT_STATUS_HDL + 1;
                break;

            case GATT_RINGER_CONTROL_POINT_CHARACTERISTIC:
                PAS_RINGER_CP_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_RINGER_SETTING_CHARACTERISTIC:
                PAS_RINGER_SETTING_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;

                PAS_RINGER_SETTING_CCC_HDL = PAS_RINGER_SETTING_HDL + 1;
                break;

            case GATT_TX_POWER_LEVEL_CHARACTERISTIC:
                TPS_TX_POWER_LEVEL_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_CURRENT_TIME_CHARACTERISTIC:
                CTS_CURRENT_TIME_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;

                /* TBD */
                CTS_CURRENT_TIME_CCC_HDL = CTS_CURRENT_TIME_HDL + 1;
                break;

            case GATT_NWA_CHARACTERISTIC:
                NAS_NWA_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;

                /* TBD */
                NAS_NWA_CCC_HDL = NAS_NWA_HDL + 1;
                break;

            case GATT_BATTERY_LEVEL_CHARACTERISTIC:
                BAS_BATT_LEVEL_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_BATTERY_STATE_CHARACTERISTIC:
                BAS_BATT_POW_ST_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_BATTERY_SERVICE_RQD_CHARACTERISTIC:
                BAS_BATT_SER_RQD_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;

                /* TBD */
                BAS_BATT_SER_RQD_CCC_HDL = BAS_BATT_SER_RQD_HDL + 1;
                break;

            case GATT_BATTERY_REMOVABLE_CHARACTERISTIC:
                BAS_REMOVABLE_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;
                break;

            case GATT_BATTERY_LEVEL_STATE_CHARACTERISTIC:
                BAS_BATT_LVL_ST_HDL =
                characteristic_array[appl_characteristic_index].desc.\
                value_handle;

                /* TBD */
                BAS_BATT_LVL_ST_CCC_HDL = BAS_BATT_LVL_ST_HDL;
                break;

            default:
                break;
            }

            appl_characteristic_index++;
        }
        else if (GATT_CLIENT_CNFG_UUID == peer_info.state.uuid)
        {
            /**
             * Found Client Characteristic Configuration, Initiate Configuration
             * request
             */
            appl_write_attribute_value
            (
                handle,
                (UCHAR *)&peer_info.state.config_property,
                2
            );
            break;
        }
    }

    /**
     * If the last handle is ATT_ATTR_HANDLE_END_RANGE, there is no need to
     * search again, go to the next logical step!
     */
    if (ATT_ATTR_HANDLE_END_RANGE == handle)
    {
        search_again = 0;
    }

    /**
     * Decide the next step:
     * -If discovery of Primary Services is complete, discover
     * characteristics of each of the services.
     * - If discovery for characteristic in the Primary Service is complete,
     * search for secondary services.
     * If discovery secondary services is complete, discover characteristics of
     * each ssecondary service.
     */
    if (GATT_PRIMARY_SERVICE_UUID == peer_info.state.uuid)
    {
        if (0 == search_again)
        {
            peer_info.pri_end_range = appl_service_index-1;
            appl_handle_primary_service_search_complete (handle);
        }
        else
        {
            range.start_handle = handle+1;
            range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            /* There could be more Primary Services Discover them */
            appl_discover_primary_services(&range);
        }
    }
    else if (GATT_CHARACTERISTIC_UUID == peer_info.state.uuid)
    {
        if (handle+2 >= (service_array[peer_info.state.service_index].\
        service_info.range.end_handle))
        {
            search_again = 0;
        }
        /* Check if further Search is required */
        if (0 == search_again)
        {
            appl_handle_characteristics_search_complete (handle);
        }
        else
        {
            /* There are more characteristics, discover them */
            range.start_handle = handle+1;
            if (peer_info.state.service_index == peer_info.pri_end_range)
            {
                range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            }
            else if (peer_info.state.service_index == peer_info.sec_end_range)
            {
                range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            }
            else
            {
                range.end_handle = \
                    service_array[peer_info.state.service_index].\
                    service_info.range.end_handle;
            }
            appl_discover_characteristic(&range);
        }
    }
    else if (GATT_SECONDARY_SERVICE_UUID == peer_info.state.uuid)
    {
        if (0 == search_again)
        {
            appl_handle_secondary_service_search_complete (handle);
        }
        else
        {
            range.start_handle = handle;
            range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            /* Search Further for Secondary Services */
            appl_discover_secondary_services
            (
                &range
            );
        }
    }
}

void appl_write_attribute_value(UINT16 handle, UCHAR *value, UINT16 length)
{
    ATT_WRITE_REQ_PARAM   find_info_param;
    API_RESULT retval;

    find_info_param.handle = handle;
    find_info_param.value.len = length;
    find_info_param.value.val = value;

    printf ("Write Request on Handle 0x%04X",handle);

    retval = BT_att_send_write_req
             (
                 &appl_gatt_client_handle,
                 &find_info_param
             );

    if (API_SUCCESS != retval)
    {
        printf ("[*** ERR ** ]Failed to issue Write Request, reason 0x%04X\n",
        retval);
    }
    else
    {
        printf ("Successfully written 0x%04X of 0x%04X on Attr handle 0x%04X",
        find_info_param.value.actual_len,find_info_param.value.len,find_info_param.handle);
    }
}

void appl_read_characteristic (UINT16 c_index)
{
    if (c_index >= appl_characteristic_index)
    {
        printf ("[**ERR**]: Cannot perform read, "
        "invalid characteristic index\n");
    }

    appl_attr_read_value (characteristic_array[c_index].desc.value_handle);
}

void appl_attr_read_value(UINT16 handle)
{
    API_RESULT retval;

    retval = BT_att_send_read_req
             (
                 &appl_gatt_client_handle,
                 &handle
             );

    if (API_SUCCESS != retval)
    {
        printf ("Failed to initiate Read Request on handle 0x%04X, reason = "
        "0x%04X",handle,retval);
    }
}

void appl_handle_characteristics_search_complete (UINT16 handle)
{
    ATT_HANDLE_RANGE   range;

    if (appl_characteristic_index ==
       service_array[peer_info.state.service_index].char_start_range)
    {
        printf ("No Characteristics found in Service 0x%04X\n",
        service_array[peer_info.state.service_index].service_info.value);
        service_array[peer_info.state.service_index].char_start_range =
            APPL_INVALID_CHARACTERISTICS_INDEX;
    }
    else
    {
        service_array[peer_info.state.service_index].char_end_range =
        appl_characteristic_index - 1;
    }

    /**
     * Discovery for Characteristic for a particular UUID is complete,
     *
     * Decide the next step:
     * - Discover Characteristics in the Next Service Group.
     * - If All Characteristics of Primary Services are Discovered,
     *   initiate Discovery for Secondary Services.
     * - If All Characteristics in Secondary Services are Discovered,
     *   Discovery is complete.
     */
    if (peer_info.pri_end_range == peer_info.state.service_index)
    {
        /**
         * All characteristics for all Primary Services are discovered,
         * disocver Secondary Services
         * Please note that the Secondary Services may be placed before
         * Primary Services in the record, hence Search os performed
         * across the valid handle range ATT_ATTR_HANDLE_START_RANGE-
         * ATT_ATTR_HANDLE_END_RANGE
         */
        if (APPL_INVALID_SERVICE_INDEX == peer_info.sec_start_range)
        {
            appl_display_peer_service_record();
            return;
        }
        else
        {
            peer_info.state.service_index = peer_info.sec_start_range;
            range.start_handle = service_array[peer_info.sec_start_range].
                service_info.range.start_handle;
            range.end_handle = service_array[peer_info.sec_start_range].
                service_info.range.end_handle;
            service_array[peer_info.state.service_index].char_start_range =\
            appl_characteristic_index;
            appl_discover_characteristic (&range);
        }

    }
    else if ((peer_info.state.service_index < peer_info.pri_end_range) ||
        (peer_info.state.service_index < peer_info.sec_end_range))

    {
        /**
         * Characteristics in the next Primary Service needs to be
         * Discovered
         */
        peer_info.state.service_index++;
        range.start_handle = service_array[peer_info.state.service_index].\
            service_info.range.start_handle;


        if (((peer_info.state.service_index == peer_info.pri_end_range) &&
            (APPL_INVALID_SERVICE_INDEX == peer_info.sec_start_range)) ||
            (peer_info.state.service_index == peer_info.sec_end_range))
        {
            range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
        }
        else
        {
            range.end_handle = service_array[peer_info.state.service_index].\
                service_info.range.end_handle;
        }
        service_array[peer_info.state.service_index].char_start_range =\
            appl_characteristic_index;
        appl_discover_characteristic (&range);
     }
    else
    {
        /**
         * Discovery of Characteristics for each Secondary Service
         * is complete.
         * Read Descriptor for each characteristic
         */
        peer_info.state.uuid = 0x0000;
        appl_display_peer_service_record ();
    }
}


void appl_handle_secondary_service_search_complete (UINT16 handle)
{
    ATT_HANDLE_RANGE   range;

    if ((ATT_INVALID_ATTR_HANDLE_VAL == handle) &&
       (appl_service_index == peer_info.sec_start_range))
    {
        /* No Secondary Services Found, Service Discovery Complete*/
        printf ("No Seoncadry Services Found, Service Discovery Complete\n");
        peer_info.sec_start_range = APPL_INVALID_SERVICE_INDEX;
        peer_info.sec_end_range = APPL_INVALID_SERVICE_INDEX;
        printf ("No Services Found\n");
        range.start_handle = service_array[peer_info.pri_start_range].\
                       service_info.range.start_handle +1;
        peer_info.state.service_index = peer_info.pri_start_range;
        if (peer_info.pri_start_range == peer_info.pri_end_range)
        {
            range.end_handle =ATT_ATTR_HANDLE_END_RANGE;
        }
        else
        {
            range.end_handle = service_array[peer_info.pri_start_range].\
                         service_info.range.end_handle;
        }
        /* appl_display_peer_service_record (); */
    }
    else
    {
        /* Serach is complete, initiate serach for individual characteristics */
        peer_info.sec_end_range = (appl_service_index - 1);
#if 0
        start_handle = service_array[peer_info.sec_start_range].\
            service_info.handle+1;
        if (peer_info.sec_start_range == peer_info.sec_end_range)
        {
            /**
             * Only one service found, hence discover till the end of handle
             * range
             */
            range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
        }
        else
        {
            /* Start of next Service handle indicates the handle value range */
            range.end_handle = service_array[peer_info.sec_start_range+1].\
                service_info.handle;
        }
        peer_info.state.service_index = peer_info.sec_start_range;
        printf ("Discovering Characteristics of Primary Service 0x%04X\n",
        service_array[peer_info.sec_start_range].service_info.value);
        service_array[peer_info.sec_start_range].char_start_range =
            appl_characteristic_index;
#else /* 0 */
        if (APPL_INVALID_SERVICE_INDEX != peer_info.pri_start_range)
        {
            peer_info.state.service_index = peer_info.pri_start_range;
            range.start_handle = service_array[peer_info.pri_start_range].\
            service_info.range.start_handle+1;
            range.end_handle = service_array[peer_info.pri_start_range].\
            service_info.range.end_handle;
        }
        else
        {
            range.start_handle = service_array[peer_info.sec_start_range].\
            service_info.range.start_handle+1;
            if (peer_info.sec_start_range == peer_info.sec_end_range)
            {
                range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
            }
        }
#endif /* 0 */
    }
    appl_discover_characteristic(&range);
}

void appl_handle_primary_service_search_complete(UINT16 handle)
{

    ATT_HANDLE_RANGE  range;

    if (ATT_INVALID_ATTR_HANDLE_VAL == handle)
    {
        if (appl_service_index == peer_info.pri_start_range)
        {
            /**
             *  No Primary Service Records found,
             *  Search for Secondary Services
             */
            printf("No Primary Services found, "
                   "Discovering Secondary Services...\n");

            peer_info.pri_start_range = APPL_INVALID_SERVICE_INDEX;
            peer_info.pri_end_range = APPL_INVALID_SERVICE_INDEX;
        }
        else
        {
            peer_info.pri_end_range = appl_service_index -1;
        }
    }
    peer_info.sec_start_range = appl_service_index;
    peer_info.state.service_index = appl_service_index;
    range.start_handle = ATT_ATTR_HANDLE_START_RANGE;
    range.end_handle = ATT_ATTR_HANDLE_END_RANGE;
    appl_discover_secondary_services
    (
        &range
    );
#if 0
    else
    {
        /* Serach is complete, initiate serach for individual characteristics */
        peer_info.pri_end_range = (appl_service_index -1);
        start_handle = service_array[peer_info.pri_start_range].\
            service_info.handle+1;
        if (peer_info.pri_start_range == peer_info.pri_end_range)
        {
            /**
             * Only one service found, hence discover till the end of handle
             * range
             */
            end_handle = ATT_ATTR_HANDLE_END_RANGE;
        }
        else
        {
            /* Start of next Service handle indicates the handle value range */
            end_handle = service_array[peer_info.pri_start_range+1].\
                service_info.handle;
        }
        peer_info.state.service_index = peer_info.pri_start_range;
        printf ("Discovering Characteristics of Primary Service 0x%04X\n",
        service_array[peer_info.pri_start_range].service_info.value);
        service_array[peer_info.pri_start_range].char_start_range =
            appl_characteristic_index;
        appl_discover_characteristic(start_handle,end_handle);
    }
#endif /* 0 */
}

void appl_handle_find_info_response (UCHAR * list, UINT16 length, UCHAR type)
{
    UCHAR uuid_arr[16];
    UINT32 index;
    UINT16 handle;
    UINT16 uuid;

    if(0 < length)
    {
        if(0x01 == type)
        {
            printf("\n\n16 bit UUIDs : \n");
            for(index = 0; index<length; index=index+4)
            {
                BT_UNPACK_LE_2_BYTE(&handle, (list+ index));
                BT_UNPACK_LE_2_BYTE(&uuid, (list+ index + 2));
                printf ("Handle : %04X -> ", handle);
                printf ("UUID   : %04X \n", uuid);
#ifdef APPL_AUTO_CHAR_CONFIGURATION
                if (GATT_CLIENT_CNFG_UUID == uuid)
                {
                    characteristic_array[peer_info.state.service_index].desc.\
                        client_config_handle = handle;
                    appl_write_attribute_value
                    (
                        handle,
                        (UCHAR *)&characteristic_array[peer_info.state.\
                        service_index].desc.client_config,
                        2
                    );
                }
#endif /* APPL_AUTO_CHAR_CONFIGURATION */
            }
        }
        else if(0x02 == type)
        {
            for(index = 0; index < length; index=index+18)
            {
                printf("\n\n128 bit UUIDs : \n");
                BT_UNPACK_LE_2_BYTE(&handle, (list + index));
                ATT_UNPACK_UUID(uuid_arr, (list+index+2), 16);
                printf ("Handle : %04X\n", handle);
                printf ("UUID   : ");

                appl_dump_bytes(uuid_arr, 16);
            }
        }
        else
        {
            /* TODO: Check if this is correct interpretaion */
            printf ("List of handles corrosponding to the Req UUID:\n");

            for(index = 0; index < length; index+=index)
            {

                BT_UNPACK_LE_2_BYTE(&handle, list+ index);
                printf ("Handle : %04X -> \n", handle);

            }
        }
    }
}

void appl_handle_disconnection_indication(void)
{
    /* Reset any ongoing transaction for HID */
    appl_read_info.hdl = ATT_INVALID_ATTR_HANDLE_VAL;
    appl_read_info.offset = 0;
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
         * Characteristics in the next service or initiae discovery for
         * Secondary service, or discovery complete.
         * - If received for Secondary Service, Discovery Complete
         */
        if (GATT_PRIMARY_SERVICE_UUID == peer_info.state.uuid)
        {
            appl_handle_primary_service_search_complete
            (
                ATT_INVALID_ATTR_HANDLE_VAL
            );
        }
        else if (GATT_SECONDARY_SERVICE_UUID == peer_info.state.uuid)
        {
            appl_handle_secondary_service_search_complete
            (
                ATT_INVALID_ATTR_HANDLE_VAL
            );
        }
        else if (GATT_CHARACTERISTIC_UUID == peer_info.state.uuid)
        {
            appl_handle_characteristics_search_complete
            (
                ATT_INVALID_ATTR_HANDLE_VAL
            );
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

void appl_display_peer_service_record(void)
{
    printf ("\n\nPrimary Services Supported by Peer:\n");
    appl_display_service_info
    (
        peer_info.pri_start_range,
        peer_info.pri_end_range
    );

    printf ("\n\nSecondary Services Supported by Peer:\n");
    appl_display_service_info
    (
        peer_info.sec_start_range,
        peer_info.sec_end_range
    );

    /* TODO: Discover Characteristics Descriptor of each service */

    /* Perform GATT Service Discovery procedure for configured no. of time */
    appl_gatt_service_discovery_count ++;

    if (APPL_CONFIG_GATT_SERVICE_DISCOVERY_COUNT() >
        appl_gatt_service_discovery_count)
    {
        printf ("GATT Discovery Count [%d]\n",
                appl_gatt_service_discovery_count);
        appl_att_initiate_service_discovery();
    }
}

void appl_display_service_info
     (
         /* IN */ UINT16   start_range,
         /* IN */ UINT16   end_range
     )
{
    UINT32 s_index;
    UINT16 no_of_services;

    if (APPL_INVALID_SERVICE_INDEX == start_range)
    {
        /* No services found */
        printf ("No Records Found!\n");
    }
    else
    {

        printf ("Start Range = 0x%04X, End Range 0x%04X\n",
        start_range,end_range);
        no_of_services = (end_range - start_range)+1;
        printf ("\nNumber of Services  = 0x%04X\n",no_of_services);
        for(s_index=0; s_index < no_of_services;s_index++)
        {
            printf ("Service: %s\n",
            appl_display_uuid_label
            (service_array[start_range + s_index].service_info.value));

            printf ("-------\n");
            printf ("\tHandle Range = 0x%04X-0x%04X, UUID = 0x%04X\n",
            service_array[start_range + s_index].service_info.range.\
            start_handle,service_array[start_range + s_index].service_info.\
            range.end_handle,service_array[start_range + s_index].service_info.\
            value);
            appl_print_characteristic_information
            (
                service_array[start_range + s_index].char_start_range,
                service_array[start_range + s_index].char_end_range
            );
        }
        printf ("\n");
    }
}

void appl_print_characteristic_information
     (
         /* IN */ UINT16 start_range,
         /* IN */ UINT16 end_range
     )
{
    UINT32 c_index;
    UINT16 no_of_characteristics;

    if (APPL_INVALID_CHARACTERISTICS_INDEX == start_range)
    {
        printf ("No Characteristics found!\n");
    }
    else
    {
        no_of_characteristics = end_range - start_range +1;

        printf ("\n\tNumber of Characteristic supported = 0x%04X\n\n",
        no_of_characteristics);

        for(c_index = 0; c_index < no_of_characteristics; c_index++)
        {
            printf ("\n  [%02X]:  CHARACTERISTIC: %s\n",
            c_index,appl_display_uuid_label\
            (characteristic_array[start_range+c_index]. desc.uuid));

            printf ("\t-----------------------\n");
            printf ("\tHandle = 0x%04X\n\tProperty = 0x%02X\n\tValue Handle = "
            "0x%04X\n\tUUID 0x%04X\n",
            characteristic_array[start_range+c_index].handle,
            characteristic_array[start_range+c_index].desc.c_property,
            characteristic_array[start_range+c_index].desc.value_handle,
            characteristic_array[start_range+c_index].desc.uuid);
        }
    }
}


void appl_read_peer_hdi_information (void)
{
    int index;
    UCHAR found_hdi_service;

    found_hdi_service = BT_FALSE;

    if ((peer_info.sec_start_range == peer_info.sec_end_range) &&
        (APPL_INVALID_SERVICE_INDEX != peer_info.sec_start_range))
    {

        /* Check if Secondary Service Listed is HDI */
        if (GATT_HDI_SERVICE_UUID ==
            service_array[peer_info.sec_start_range].service_info.value)
        {
            found_hdi_service = BT_TRUE;
            index = peer_info.sec_start_range;

        }
    }
    else
    {
        for (index = peer_info.sec_start_range; index <peer_info.sec_end_range;
            index ++)
        {
            if (GATT_HDI_SERVICE_UUID ==
                service_array[index].service_info.value)
            {
                found_hdi_service = BT_TRUE;
                break;
            }
        }
    }
    if (BT_FALSE != found_hdi_service)
    {
        peer_info.state.service_index = index;
        peer_info.state.handle = service_array[index].char_start_range;
        peer_info.state.uuid = GATT_HDI_SERVICE_UUID;

        /* Issue Read Request for first Handle of HDI Characteristcs */
        appl_attr_read_value
        (
            characteristic_array[service_array[index].char_start_range].
            desc.value_handle\

        );
    }
}


/**
 * Please note the Characteristic here is the Characteristic handle & not the
 * handle of Client Characteristic Configuration
 */
void appl_configure_characteristic_by_index
     (
         /* IN */ UINT16     characteristic_index,
         /* IN */ UINT16    att_property
     )
{

    UINT16 start_handle;
    UINT16 end_handle;

    /* Initiate Procedure to Find Client Characteristic Configuration */
    /* Incrementing by two as handle + 1 contains characteristic value */
    start_handle = characteristic_array[characteristic_index].handle+2;
    end_handle = characteristic_array[characteristic_index+1].handle;

    if (ATT_INVALID_ATTR_HANDLE_VAL == end_handle)
    {
        end_handle = ATT_ATTR_HANDLE_END_RANGE;
    }

    peer_info.state.service_index = characteristic_index;
    peer_info.state.config_property = att_property;
    characteristic_array[characteristic_index].desc.client_config =
                                      att_property;

    /* Read Values of characteristics in the HDI Service */
    appl_discover_characteristic_descriptor (start_handle,end_handle);
}


void appl_search_for_characteristic_uuid
     (
         /* IN */  UINT16 uuid,
         /* OUT */ UCHAR *ch_index
     )
{
    int index;

    for (index = 0; index < APPL_MAX_CHARACTERISTICS; index++)
    {
        if (uuid == characteristic_array[index].desc.uuid)
        {
            *ch_index = index;
        }
    }
}


void appl_configure_characteristic
    (
        /* IN */ UINT16 ch_uuid,
        /* IN */ UINT16  ch_property
    )
{
    UCHAR ch_index = APPL_MAX_CHARACTERISTICS;

    /* Look if the Characteristic is supported by Peer */
    appl_search_for_characteristic_uuid (ch_uuid,&ch_index);

    /* Check if Property Requested is supported by Characteristics */
    if (APPL_CHECK_IF_CH_SUPPORTS_INDICATIONS(ch_index))
    {
        /* Initiate Procedure to Configure Characteristics */
        appl_configure_characteristic_by_index
        (
            ch_index,
            APPL_GATT_CLIENT_CH_INDICATION
        );
    }
}

void appl_profile_client_operations (void)
{
    int val;

    while (1)
    {
        printf ("%s",collector_menu);
        scanf ("%d",&val);
        switch(val)
        {
        case 0:
            return;
            break;
        case 1:
            printf ("%s",collector_menu);
            break;
        case 2:
            appl_thermometer_options ();
            break;
        case 3:
            appl_heart_rate_options ();
            break;
        case 4:
            appl_bpm_options ();
            break;
        default:
            break;
        }
    }
}

UCHAR * appl_uuid_desc (UINT16 uuid)
{
    int index;

    index = 0;
    do
    {
        if (uuid_desc_table[index].uuid == uuid)
        {
            return uuid_desc_table[index].desc;
        }
        index++;
    }while ( 0 != uuid_desc_table[index].uuid);
    return NULL;
}

UCHAR * appl_display_uuid_label(UINT16 uuid)
{
    int index;

    index=0;

    do
    {
        if (uuid == appl_uuid_desc_table[index].uuid)
        {
            return appl_uuid_desc_table[index].desc_str;
        }
        index++;
    }while (0 != appl_uuid_desc_table[index].uuid);

    return "Unknown";
}


API_RESULT appl_discover_characteristic_descriptor
           (
               UINT16 start_handle,
               UINT16 end_handle
           )
{
    ATT_FIND_INFO_REQ_PARAM   find_info_param;
    API_RESULT    retval;

    printf ("Initiating Descriptor Discovery from 0x%04X to 0x%04X",
    start_handle,end_handle);

    if (start_handle == end_handle)
    {
        /* Nothing to discover */
        retval = API_FAILURE;
    }
    find_info_param.start_handle = start_handle;
    find_info_param.end_handle = end_handle;

    retval = BT_att_send_find_info_req
             (&appl_gatt_client_handle, &find_info_param);

    if (API_SUCCESS != retval)
    {
        printf ("[***ERR***]: Failed to to initiate Find Information Request,"
        "reason 0x%04X\n", retval);
    }
    else
    {
        printf ("Successful sent Find Information Request\n");
    }

    return retval;
}


void appl_thermometer_options (void)
{
    int val;

    while (1)
    {
        printf ("%s",thermometer_menu);
        scanf ("%d",&val);

        switch(val)
        {
        case 0:
            return;
            break;
        case 1:
            printf ("%s",thermometer_menu);
            break;
        case 2:
            printf ("Configuring Temperature Characteristic For Indications\n");
            break;
        case 3:
            printf ("Configuring Temperature Characteristic For Indications\n");
            break;
        case 4:
            printf ("Please Enter the Period for Measurement Intervals\n");
            scanf ("%d",&val);
            break;
        case 5:
            printf ("Reading Temperature Type");
            break;
        default:
            break;

        }
    }
}

void appl_heart_rate_options (void)
{
    int val;

    while (1)
    {
        printf ("%s",heart_rate_menu);
        scanf ("%d",&val);

        switch(val)
        {
        case 0:
            return;
            break;
        case 1:
            printf ("%s",heart_rate_menu);
            break;
        case 2:
            printf("Configuring Heart Rate Chraacteristic For Notifications\n");

            if (ATT_INVALID_ATTR_HANDLE_VAL !=
                characteristic_array[appl_hr_cindex].desc.client_config_handle)
            {
                characteristic_array[appl_hr_cindex].desc.client_config = \
                    APPL_GATT_CLIENT_CH_NOTIFICATION;
                appl_write_attribute_value
                (
                    characteristic_array[appl_hr_cindex].\
                    desc.client_config_handle,
                    (UCHAR *)&characteristic_array[appl_hr_cindex].\
                    desc.client_config,
                    2
                );
            }
            else
            {
                appl_configure_characteristic_by_index
                (
                    appl_hr_cindex,
                    APPL_GATT_CLIENT_CH_NOTIFICATION
                );
            }
            break;
        case 3:
            printf ("Reading Sensor Location\n");
            appl_read_characteristic (appl_hr_sensor_loc_cindex);
            break;
        case 4:
            printf ("Please Enter the Period for Measurement Intervals\n");
            scanf ("%d",&val);
            break;
        case 5:
            printf ("Stop Notification of Heart Rate");
            if (ATT_INVALID_ATTR_HANDLE_VAL !=
                characteristic_array[appl_hr_cindex].desc.client_config_handle)
            {
                characteristic_array[appl_hr_cindex].desc.client_config =
                                                                  0x0000;
                appl_write_attribute_value
                (
                    characteristic_array[appl_hr_cindex].\
                    desc.client_config_handle,
                    (UCHAR *)&characteristic_array[appl_hr_cindex].\
                    desc.client_config,
                    2
                );
            }
            break;

        default:
            break;
        }
    }

}

void appl_bpm_options (void)
{
}

void appl_send_read_request (ATT_ATTR_HANDLE hdl)
{
    if (ATT_INVALID_ATTR_HANDLE_VAL != hdl)
    {
        appl_read_info.hdl = hdl;
        appl_read_info.offset = 0;

        /* TBD: Not checking error value */
        BT_att_send_read_req
        (
            &appl_gatt_client_handle,
            &hdl
        );
    }
}

void appl_hid_send_cmd (UCHAR cp_cmd)
{
    ATT_WRITE_CMD_PARAM cp_wr_cmd;

    if (ATT_INVALID_ATTR_HANDLE_VAL != HID_CP_HDL)
    {
        cp_wr_cmd.handle = HID_CP_HDL;
        cp_wr_cmd.value.val = &cp_cmd;
        cp_wr_cmd.value.len = sizeof (cp_cmd);

        BT_att_send_write_cmd
        (
            &appl_gatt_client_handle,
            &cp_wr_cmd
        );
    }
}

void appl_config_ccc_ntf (ATT_ATTR_HANDLE hdl)
{
    ATT_WRITE_REQ_PARAM cp_wr_req;

    /* Notification => Write 0x0001 to CCC */
    UCHAR ntf[] = {0x01, 0x00};

    if (ATT_INVALID_ATTR_HANDLE_VAL != hdl)
    {
        cp_wr_req.handle = hdl;
        cp_wr_req.value.val = ntf;
        cp_wr_req.value.len = sizeof (ntf);

        BT_att_send_write_req
        (
            &appl_gatt_client_handle,
            &cp_wr_req
        );
    }
}

void appl_hid_send_write_cmd (ATT_ATTR_HANDLE hdl, UCHAR mode)
{
    ATT_WRITE_CMD_PARAM cp_wr_cmd;

    if (ATT_INVALID_ATTR_HANDLE_VAL != hdl)
    {
        cp_wr_cmd.handle = hdl;
        cp_wr_cmd.value.val = &mode;
        cp_wr_cmd.value.len = sizeof (mode);

        BT_att_send_write_cmd
        (
            &appl_gatt_client_handle,
            &cp_wr_cmd
        );
    }
}

void appl_hid_host_options (void)
{
    int val;

    while (1)
    {
        printf ("%s", hid_host_menu);
        scanf ("%d",&val);

        switch(val)
        {
        case 0:
            return;

        case 1:
            break;

        case 10:
            printf ("Read HID Protocol Mode\n");
            appl_send_read_request (HID_PROTO_MODE_HDL);
            break;

        case 11:
            printf ("Read HID Report Descriptor\n");
            appl_send_read_request (HID_REPORT_DESC_HDL);
            break;

        case 12:
            printf ("Read HID Boot Report Mapping\n");
            appl_send_read_request (HID_BOOT_RPT_MAPPING_HDL);
            break;

        case 13:
            printf ("Read HID Information\n");
            appl_send_read_request (HID_INFO_HDL);
            break;

        case 14:
            printf ("Manually Enter - Report CCC (in Hex)\n");

            scanf ("%x", &val);

            HID_REPORT_CCC_HDL = (UINT16)val;

            printf ("Set HID_REPORT_CCC_HDL as 0x%04X\n", HID_REPORT_CCC_HDL);
            break;

        case 15:
            printf ("Set HID Protocol Mode - Boot\n");
            appl_hid_send_write_cmd (HID_PROTO_MODE_HDL, 0x00);
            break;

        case 16:
            printf ("Set HID Protocol Mode - Report\n");
            appl_hid_send_write_cmd (HID_PROTO_MODE_HDL, 0x01);
            break;

        case 20:
            printf ("HID CP - Suspend\n");
            appl_hid_send_cmd (HID_CP_ENUM_CMD_SUSPEND);
            break;

        case 21:
            printf ("HID CP - Exit Suspend\n");
            appl_hid_send_cmd (HID_CP_ENUM_CMD_EXIT_SUSPEND);
            break;

        case 30:
            printf ("Configure CCC - Boot Keyboard\n");
            appl_config_ccc_ntf (HID_KBD_CCC_HDL);
            break;

        case 31:
            printf ("Configure CCC - Boot Mouse\n");
            appl_config_ccc_ntf (HID_MOUSE_CCC_HDL);
            break;

        case 32:
            printf ("Configure CCC - Report\n");
            appl_config_ccc_ntf (HID_REPORT_CCC_HDL);
            break;

        default:
            break;
        }
    }
}
void appl_puid_send_write_cmd (ATT_ATTR_HANDLE hdl, UCHAR alert_level)
{
    ATT_WRITE_CMD_PARAM cp_wr_cmd;

    if (ATT_INVALID_ATTR_HANDLE_VAL != hdl)
    {
        cp_wr_cmd.handle = hdl;
        cp_wr_cmd.value.val = &alert_level;
        cp_wr_cmd.value.len = sizeof (alert_level);

        BT_att_send_write_cmd
        (
            &appl_gatt_client_handle,
            &cp_wr_cmd
        );
    }
}


void appl_puid_send_write_request (ATT_ATTR_HANDLE hdl, UCHAR alert_level)
{
    ATT_WRITE_REQ_PARAM cp_wr_req;

    if (ATT_INVALID_ATTR_HANDLE_VAL != hdl)
    {
        cp_wr_req.handle = hdl;
        cp_wr_req.value.val = &alert_level;
        cp_wr_req.value.len = sizeof (alert_level);

        BT_att_send_write_req
        (
            &appl_gatt_client_handle,
            &cp_wr_req
        );
    }
}

void appl_puid_client_options (void)
{
    int val;
    UCHAR alert_level;

    while (1)
    {
        printf ("%s", puid_client_menu);
        scanf ("%d",&val);

        switch(val)
        {
        case 0:
            return;

        case 1:
            break;

        case 10:
            printf ("Read Link Loss Service - Alert Level\n");
            appl_send_read_request (LLS_ALERT_LEVEL_HDL);
            break;

        case 11:
            printf ("Set Link Loss Service - Alert Level\n");
            alert_level = 0x00;
            appl_puid_send_write_request (LLS_ALERT_LEVEL_HDL, alert_level);
            break;

        case 12:
            printf ("Set Link Loss Service - Alert Level - \"Mild Alert\"\n");
            alert_level = 0x01;
            appl_puid_send_write_request (LLS_ALERT_LEVEL_HDL, alert_level);
            break;

        case 13:
            printf ("Set Link Loss Service - Alert Level - \"High Alert\"\n");
            alert_level = 0x02;
            appl_puid_send_write_request (LLS_ALERT_LEVEL_HDL, alert_level);
            break;

        case 21:
            printf ("Immediate Alert Service - Alert Level\n");
            alert_level = 0x00;
            appl_puid_send_write_cmd (IAS_ALERT_LEVEL_HDL, alert_level);
            break;

        case 22:
            printf ("Immediate Alert Service - Alert Level - \"Mild Alert\"\n");
            alert_level = 0x01;
            appl_puid_send_write_cmd (IAS_ALERT_LEVEL_HDL, alert_level);
            break;

        case 23:
            printf ("Immediate Alert Service - Alert Level - \"High Alert\"\n");
            alert_level = 0x02;
            appl_puid_send_write_cmd (IAS_ALERT_LEVEL_HDL, alert_level);
            break;

        case 30:
            printf ("Read Tx Power Level\n");
            appl_send_read_request (TPS_TX_POWER_LEVEL_HDL);
            break;

        case 40:
            printf ("Read Current Time\n");
            appl_send_read_request (CTS_CURRENT_TIME_HDL);
            break;

        case 41:
            printf ("Configure CCC - Current Time\n");
            appl_config_ccc_ntf (CTS_CURRENT_TIME_CCC_HDL);
            break;

        case 50:
            printf ("Read Network Availablity\n");
            appl_send_read_request (NAS_NWA_HDL);
            break;

        case 51:
            printf ("Configure CCC - Network Availablity\n");
            appl_config_ccc_ntf (NAS_NWA_CCC_HDL);
            break;

        case 60:
            printf ("Read Battery Level\n");
            appl_send_read_request (BAS_BATT_LEVEL_HDL);
            break;

        case 61:
            printf ("Read Battery Power Staten\n");
            appl_send_read_request (BAS_BATT_POW_ST_HDL);
            break;

        case 62:
            printf ("Read Service Required\n");
            appl_send_read_request (BAS_BATT_SER_RQD_HDL);
            break;

        case 63:
            printf ("Read Removable\n");
            appl_send_read_request (BAS_REMOVABLE_HDL);
            break;

        case 64:
            printf ("Read Battery Level State\n");
            appl_send_read_request (BAS_BATT_LVL_ST_HDL);
            break;

        case 70:
            printf ("Read Alert Status\n");
            appl_send_read_request (PAS_ALERT_STATUS_HDL);
            break;

        case 71:
            printf ("Read Ringer Setting\n");
            appl_send_read_request (PAS_RINGER_SETTING_HDL);
            break;

        case 72:
            {
                UCHAR ringer_cp;
                printf ("Set Ringer Control Point\n");
                ringer_cp = 0x01;
                appl_puid_send_write_cmd (PAS_RINGER_CP_HDL, ringer_cp);
            }
            break;

        case 73:
            printf ("Configure CCC - Alert Status\n");
            appl_config_ccc_ntf (PAS_ALERT_STATUS_CCC_HDL);
            break;

        case 74:
            printf ("Configure CCC - Ringer Setting\n");
            appl_config_ccc_ntf (PAS_RINGER_SETTING_CCC_HDL);
            break;
        }
    }
}

void appl_att_read_blob_request (ATT_READ_BLOB_REQ_PARAM    * param)
{
    API_RESULT retval;

    retval = BT_att_send_read_blob_req (&appl_gatt_client_handle,param);

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "[APPL]:[*** ERR **]: Failed to send Read Blob Request, reason "
        "0x%04X!", retval);
    }
    else
    {
        APPL_TRC (
        "[APPL]: Successfully Requested Read Blob Request.");
    }
}

void appl_att_read_multiple_request (ATT_HANDLE_LIST    * list)
{
    API_RESULT retval;

    retval = BT_att_send_read_multiple_req (&appl_gatt_client_handle,list);

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "[APPL]:[*** ERR **]: Failed to send Read Multiple Request, reason "
        "0x%04X!", retval);
    }
    else
    {
        APPL_TRC (
        "[APPL]: Successfully Requested Read Multiple Request. %04X Handles of "
        "0x%04X Requested",list->actual_count,list->list_count);
    }
}

void appl_att_prepare_write_request (ATT_PREPARE_WRITE_REQ_PARAM * param)
{
    API_RESULT retval;

    retval = BT_att_send_prepare_write_req (&appl_gatt_client_handle,param);

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "[APPL]:[*** ERR **]: Failed to send Prepare Write Request,reason "
        "0x%04X!",retval);
    }
    else
    {
        APPL_TRC (
        "[APPL]: Successfully sent Prepare Write Request");
    }
}


void appl_att_execute_write_request (ATT_EXECUTE_WRITE_REQ_PARAM   * param)
{
    API_RESULT retval;

    retval = BT_att_send_execute_write_req (&appl_gatt_client_handle,param);

    if (API_SUCCESS != retval)
    {
        APPL_TRC (
        "[APPL]:[*** ERR **]: Failed to send Execute Write Request, reason "
        "0x%04X!",retval);
    }
    else
    {
        APPL_TRC (
        "[APPL]: Successfully sent Execute Write Request");
    }
}

#endif /* APPL_ATT_CLIENT */

