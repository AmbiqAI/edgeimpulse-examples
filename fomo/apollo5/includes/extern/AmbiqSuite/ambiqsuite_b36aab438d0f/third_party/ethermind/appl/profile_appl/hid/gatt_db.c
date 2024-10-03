
/**
 *  \file gatt_db.c
 *  GATT Databse.
 */

/**
 *  GATT DATABASE QUICK REFERENCE TABLE:
 *  Abbreviations used for Permission:
 *      Rd  = Read
 *      WwR = Write Without Response
 *      Wr  = Write
 *      Swr  = Signed Write
 *      Ntf = Notification
 *      Ind = Indication
 *
 *  Handle  |      ATT_Type      |    Permission    |  ATT_VALUE
 *  ========+====================+==================+=====================
 *  0x0001  |       0x2800       | Rd               | 0x1801
 *  --------+--------------------+------------------+---------------------
 *  0x0002  |       0x2803       | Rd               | 0x20, 0x0003, 0x2A05
 *  --------+--------------------+------------------+---------------------
 *  0x0003  |       0x2A05       | Ind              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0004  |       0x2902       | Rd, Wr           | 0x0000
 *  ========+====================+==================+=====================
 *  0x0005  |       0x2800       | Rd               | 0x1800
 *  --------+--------------------+------------------+---------------------
 *  0x0006  |       0x2803       | Rd               | 0x02, 0x0007, 0x2A00
 *  --------+--------------------+------------------+---------------------
 *  0x0007  |       0x2A00       | Rd               | EtherMind
 *  --------+--------------------+------------------+---------------------
 *  0x0008  |       0x2803       | Rd               | 0x02, 0x0009, 0x2A01
 *  --------+--------------------+------------------+---------------------
 *  0x0009  |       0x2A01       | Rd               | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x000A  |       0x2803       | Rd               | 0x02, 0x000B, 0x2A04
 *  --------+--------------------+------------------+---------------------
 *  0x000B  |       0x2A04       | Rd               | 0.0
 *  ========+====================+==================+=====================
 *  0x000C  |       0x2800       | Rd               | 0x180A
 *  --------+--------------------+------------------+---------------------
 *  0x000D  |       0x2803       | Rd               | 0x02, 0x000E, 0x2A29
 *  --------+--------------------+------------------+---------------------
 *  0x000E  |       0x2A29       | Rd               | MindTreeLimited
 *  --------+--------------------+------------------+---------------------
 *  0x000F  |       0x2803       | Rd               | 0x02, 0x0010, 0x2A24
 *  --------+--------------------+------------------+---------------------
 *  0x0010  |       0x2A24       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0011  |       0x2803       | Rd               | 0x02, 0x0012, 0x2A25
 *  --------+--------------------+------------------+---------------------
 *  0x0012  |       0x2A25       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0013  |       0x2803       | Rd               | 0x02, 0x0014, 0x2A27
 *  --------+--------------------+------------------+---------------------
 *  0x0014  |       0x2A27       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0015  |       0x2803       | Rd               | 0x02, 0x0016, 0x2A26
 *  --------+--------------------+------------------+---------------------
 *  0x0016  |       0x2A26       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0017  |       0x2803       | Rd               | 0x02, 0x0018, 0x2A28
 *  --------+--------------------+------------------+---------------------
 *  0x0018  |       0x2A28       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0019  |       0x2803       | Rd               | 0x02, 0x001A, 0x2A23
 *  --------+--------------------+------------------+---------------------
 *  0x001A  |       0x2A23       | Rd               | AABBCCDD
 *  --------+--------------------+------------------+---------------------
 *  0x001B  |       0x2803       | Rd               | 0x02, 0x001C, 0x2A2A
 *  --------+--------------------+------------------+---------------------
 *  0x001C  |       0x2A2A       | Rd               | 0
 *  --------+--------------------+------------------+---------------------
 *  0x001D  |       0x2803       | Rd               | 0x02, 0x001E, 0x2A50
 *  --------+--------------------+------------------+---------------------
 *  0x001E  |       0x2A50       | Rd               | 0
 *  ========+====================+==================+=====================
 *  0x001F  |       0x2800       | Rd               | 0x180F
 *  --------+--------------------+------------------+---------------------
 *  0x0020  |       0x2803       | Rd               | 0x12, 0x0021, 0x2A19
 *  --------+--------------------+------------------+---------------------
 *  0x0021  |       0x2A19       | Rd, Ntf          | 100.0
 *  --------+--------------------+------------------+---------------------
 *  0x0022  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0023  |       0x2904       | Rd               | VALUE
 *  ========+====================+==================+=====================
 *  0x0024  |       0x2800       | Rd               | 0x1812
 *  --------+--------------------+------------------+---------------------
 *  0x0025  |       0x2803       | Rd               | 0x06, 0x0026, 0x2A4E
 *  --------+--------------------+------------------+---------------------
 *  0x0026  |       0x2A4E       | Rd, Wwr          | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0027  |       0x2803       | Rd               | 0x02, 0x0028, 0x2A4B
 *  --------+--------------------+------------------+---------------------
 *  0x0028  |       0x2A4B       | Rd               | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0029  |       0x2907       | Rd               | VALUE
 *  --------+--------------------+------------------+---------------------
 *  0x002A  |       0x2803       | Rd               | 0x1A, 0x002B, 0x2A4D
 *  --------+--------------------+------------------+---------------------
 *  0x002B  |       0x2A4D       | Rd, Wr, Ntf      | 0
 *  --------+--------------------+------------------+---------------------
 *  0x002C  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x002D  |       0x2908       | Rd               | VALUE
 *  --------+--------------------+------------------+---------------------
 *  0x002E  |       0x2803       | Rd               | 0x0E, 0x002F, 0x2A4D
 *  --------+--------------------+------------------+---------------------
 *  0x002F  |       0x2A4D       | Rd, Wr, Wwr      | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0030  |       0x2908       | Rd               | VALUE
 *  --------+--------------------+------------------+---------------------
 *  0x0031  |       0x2803       | Rd               | 0x0A, 0x0032, 0x2A4D
 *  --------+--------------------+------------------+---------------------
 *  0x0032  |       0x2A4D       | Rd, Wr           | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0033  |       0x2908       | Rd               | VALUE
 *  --------+--------------------+------------------+---------------------
 *  0x0034  |       0x2803       | Rd               | 0x04, 0x0035, 0x2A4C
 *  --------+--------------------+------------------+---------------------
 *  0x0035  |       0x2A4C       | Wwr              | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0036  |       0x2803       | Rd               | 0x02, 0x0037, 0x2A4A
 *  --------+--------------------+------------------+---------------------
 *  0x0037  |       0x2A4A       | Rd               | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0038  |       0x2803       | Rd               | 0x1A, 0x0039, 0x2A33
 *  --------+--------------------+------------------+---------------------
 *  0x0039  |       0x2A33       | Rd, Wr, Ntf      | 0
 *  --------+--------------------+------------------+---------------------
 *  0x003A  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x003B  |       0x2803       | Rd               | 0x1A, 0x003C, 0x2A22
 *  --------+--------------------+------------------+---------------------
 *  0x003C  |       0x2A22       | Rd, Wr, Ntf      | 0
 *  --------+--------------------+------------------+---------------------
 *  0x003D  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x003E  |       0x2803       | Rd               | 0x0E, 0x003F, 0x2A32
 *  --------+--------------------+------------------+---------------------
 *  0x003F  |       0x2A32       | Rd, Wr, Wwr      | 0
 *  ========+====================+==================+=====================
 *  0x0040  |       0x2800       | Rd               | 0x1813
 *  --------+--------------------+------------------+---------------------
 *  0x0041  |       0x2803       | Rd               | 0x04, 0x0042, 0x2A4F
 *  --------+--------------------+------------------+---------------------
 *  0x0042  |       0x2A4F       | Wwr              | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0043  |       0x2803       | Rd               | 0x10, 0x0044, 0x2A31
 *  --------+--------------------+------------------+---------------------
 *  0x0044  |       0x2A31       | Ntf              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0045  |       0x2902       | Rd, Wr           | 0x0000
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "gatt_defines.h"

#if ((defined HID) && (defined GATT_DB))

/* ----------------------------------------- Global Variables */
#ifndef GATT_DB_HAVE_STATIC_CONFIGURATION
DECL_CONST UCHAR  gatt_service_count = GATT_SERVICE_COUNT;
DECL_CONST UCHAR  gatt_characteristic_count = GATT_CHARACTERISTIC_COUNT;
DECL_CONST UCHAR  gatt_db_max_type_count = GATT_DB_MAX_TYPE_COUNT;
DECL_CONST UCHAR  gatt_db_peer_val_arr_size = GATT_DB_PEER_VALUE_ARRAY_SIZE;

#ifdef STORAGE_RETENTION_SUPPORT
DECL_CONST UINT16 gatt_value_array_size = GATT_VALUE_ARRAY_SIZE;
DECL_CONST UINT16 gatt_db_max_peer_config = GATT_DB_MAX_PEER_CONFIGURATION;
#endif /* STORAGE_RETENTION_SUPPORT */
#endif /* GATT_DB_HAVE_STATIC_CONFIGURATION */

DECL_CONST UCHAR gatt_const_uuid_arr[GATT_UUID_ARRAY_SIZE] =
{
    /* 0 - Primary Service UUID */
    0x00, 0x28,

    /* 2 - Secondary Service UUID */
    0x01, 0x28,

    /* 4 - Include UUID */
    0x02, 0x28,

    /* 6 - Characteristic UUID */
    0x03, 0x28,

    /* 8 - Characteristic Extended Properties UUID */
    0x00, 0x29,

    /* 10 - Characteristic User Description UUID */
    0x01, 0x29,

    /* 12 - Client Configuration UUID */
    0x02, 0x29,

    /* 14 - Server Configuration UUID */
    0x03, 0x29,

    /* 16 - Characteristic Format UUID */
    0x04, 0x29,

    /* 18 - Characteristic Aggregate Format UUID */
    0x05, 0x29,

    /* 20 - GATT Service UUID */
    0x01, 0x18,

    /* 22 - Service Changed Characteristic UUID */
    0x05, 0x2A,

    /* 24 - GAP Service UUID */
    0x00, 0x18,

    /* 26 - Device Name Characteristic UUID */
    0x00, 0x2A,

    /* 28 - Appearance Characteristic UUID */
    0x01, 0x2A,

    /* 30 - Peripheral Preferred Connection Paramters Characteristic UUID */
    0x04, 0x2A,

    /* 32 - DeviceInformation Service UUID */
    0x0A, 0x18,

    /* 34 - ManufacturerName Characteristic UUID */
    0x29, 0x2A,

    /* 36 - ModelNumber Characteristic UUID */
    0x24, 0x2A,

    /* 38 - SerialNumber Characteristic UUID */
    0x25, 0x2A,

    /* 40 - HardwareRevision Characteristic UUID */
    0x27, 0x2A,

    /* 42 - FirmwareRevision Characteristic UUID */
    0x26, 0x2A,

    /* 44 - SoftwareRevision Characteristic UUID */
    0x28, 0x2A,

    /* 46 - SystemId Characteristic UUID */
    0x23, 0x2A,

    /* 48 - RegCertDataList Characteristic UUID */
    0x2A, 0x2A,

    /* 50 - PnPID Characteristic UUID */
    0x50, 0x2A,

    /* 52 - Battery Service UUID */
    0x0F, 0x18,

    /* 54 - BatteryLevel Characteristic UUID */
    0x19, 0x2A,

    /* 56 - HumanInterfaceDevice Service UUID */
    0x12, 0x18,

    /* 58 - HIDProtocolMode Characteristic UUID */
    0x4E, 0x2A,

    /* 60 - HIDReportMap Characteristic UUID */
    0x4B, 0x2A,

    /* 62 - HIDReportMap Characteristic HLD UUID */
    0x07, 0x29,

    /* 64 - Input HIDReport Characteristic UUID */
    0x4D, 0x2A,

    /* 66 - Input HIDReport Characteristic HLD UUID */
    0x08, 0x29,

    /* 68 - HIDControlPoint Characteristic UUID */
    0x4C, 0x2A,

    /* 70 - HIDInformation Characteristic UUID */
    0x4A, 0x2A,

    /* 72 - BootMouseInputReport Characteristic UUID */
    0x33, 0x2A,

    /* 74 - BootKeyboardInputReport Characteristic UUID */
    0x22, 0x2A,

    /* 76 - BootKeyboardOutputReport Characteristic UUID */
    0x32, 0x2A,

    /* 78 - ScanParameter Service UUID */
    0x13, 0x18,

    /* 80 - ScanIntervalWindow Characteristic UUID */
    0x4F, 0x2A,

    /* 82 - ScanRefresh Characteristic UUID */
    0x31, 0x2A,

};


UCHAR gatt_value_arr[GATT_VALUE_ARRAY_SIZE] =
{
    /* 0 - BatteryLevel(13); Default: 100.0 */
    0x64,

    /* 1 - HIDProtocolMode(14); Default: 0 */
    0x01,

    /* 2 - HIDReportMap(15); Default: 0 */
    0x3F, 0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7,
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
    0x75, 0x08, 0x81, 0x03, 0x95, 0x05, 0x75, 0x01, 0x05, 0x08, 0x19, 0x01,
    0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x03, 0x95, 0x06,
    0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00, 0x29, 0x65,
    0x81, 0x00, 0xC0,

    /* 66 - HLD for HIDReportMap */
    /* External Report Reference Descriptor  */
    0x00, 0x00,

    /* 68 - HLD for Input HIDReport */
    /* Input Report Reference Descriptor  */
    0x00, 0x01,

    /* 70 - HLD for Output HIDReport */
    /* Output Report Reference Descriptor  */
    0x00, 0x02,

    /* 72 - HLD for Feature HIDReport */
    /* Feature Report Reference Descriptor  */
    0x00, 0x03,

    /* 74 - HIDInformation(20); Default: 0 */
    0x04, 0x13, 0x02, 0x40, 0x01,
};


DECL_CONST UCHAR gatt_const_value_arr[GATT_CONST_VALUE_ARRAY_SIZE] =
{

    /* 0 - Service Changed(0); Property: 0x20, Value Handle: 0x0003, UUID: 0x2A05 */
    0x20, 0x03, 0x00, 0x05, 0x2A,

    /* 5 - Device Name(1); Property: 0x02, Value Handle: 0x0007, UUID: 0x2A00 */
    0x02, 0x07, 0x00, 0x00, 0x2A,

    /* 10 - Device Name(1); Default: EtherMind */
    0x45, 0x74, 0x68, 0x65, 0x72, 0x4d, 0x69, 0x6e, 0x64,

    /* 19 - Appearance(2); Property: 0x02, Value Handle: 0x0009, UUID: 0x2A01 */
    0x02, 0x09, 0x00, 0x01, 0x2A,

    /* 24 - Appearance(2); Default: 0.0 */
    0xC2, 0x03,

    /* 26 - Peripheral Preferred Connection Paramters(3); Property: 0x02, Value Handle: 0x000B, UUID: 0x2A04 */
    0x02, 0x0B, 0x00, 0x04, 0x2A,

    /* 31 - Peripheral Preferred Connection Paramters(3); Default: 0.0 */
    0x0A, 0x00, 0x10, 0x00, 0x64, 0x00, 0xE2, 0x04,

    /* 39 - ManufacturerName(4); Property: 0x02, Value Handle: 0x000E, UUID: 0x2A29 */
    0x02, 0x0E, 0x00, 0x29, 0x2A,

    /* 44 - ManufacturerName(4); Default: MindTreeLimited */
    0x4D, 0x69, 0x6E, 0x64, 0x54, 0x72, 0x65, 0x65, 0x4C, 0x69, 0x6D, 0x69,
    0x74, 0x65, 0x64,

    /* 59 - ModelNumber(5); Property: 0x02, Value Handle: 0x0010, UUID: 0x2A24 */
    0x02, 0x10, 0x00, 0x24, 0x2A,

    /* 64 - ModelNumber(5); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 69 - SerialNumber(6); Property: 0x02, Value Handle: 0x0012, UUID: 0x2A25 */
    0x02, 0x12, 0x00, 0x25, 0x2A,

    /* 74 - SerialNumber(6); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 79 - HardwareRevision(7); Property: 0x02, Value Handle: 0x0014, UUID: 0x2A27 */
    0x02, 0x14, 0x00, 0x27, 0x2A,

    /* 84 - HardwareRevision(7); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 89 - FirmwareRevision(8); Property: 0x02, Value Handle: 0x0016, UUID: 0x2A26 */
    0x02, 0x16, 0x00, 0x26, 0x2A,

    /* 94 - FirmwareRevision(8); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 99 - SoftwareRevision(9); Property: 0x02, Value Handle: 0x0018, UUID: 0x2A28 */
    0x02, 0x18, 0x00, 0x28, 0x2A,

    /* 104 - SoftwareRevision(9); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 109 - SystemId(10); Property: 0x02, Value Handle: 0x001A, UUID: 0x2A23 */
    0x02, 0x1A, 0x00, 0x23, 0x2A,

    /* 114 - SystemId(10); Default: AABBCCDD */
    0x41, 0x41, 0x42, 0x42, 0x43, 0x43, 0x44, 0x44,

    /* 122 - RegCertDataList(11); Property: 0x02, Value Handle: 0x001C, UUID: 0x2A2A */
    0x02, 0x1C, 0x00, 0x2A, 0x2A,

    /* 127 - RegCertDataList(11); Default: 0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* 159 - PnPID(12); Property: 0x02, Value Handle: 0x001E, UUID: 0x2A50 */
    0x02, 0x1E, 0x00, 0x50, 0x2A,

    /* 164 - PnPID(12); Default: 0 */
    0x01, 0x6A, 0x00, 0x4D, 0x01, 0x00, 0x01,

    /* 171 - BatteryLevel(13); Property: 0x12, Value Handle: 0x0021, UUID: 0x2A19 */
    0x12, 0x21, 0x00, 0x19, 0x2A,

    /* 176 - Presentation Fromat for BatteryLevel */
    /**
     * Characteristic Presentation Format for BatteryLevel
     * - Format            (1 octet)
     * - Exponent          (1 octet)
     * - Unit              (2 octet)
     * - Name Space        (1 octet)
     * - Description       (2 octet)
     */
    0x04, 0x00, 0xAD, 0x27, 0x01, 0x00, 0x00,

    /* 183 - HIDProtocolMode(14); Property: 0x06, Value Handle: 0x0026, UUID: 0x2A4E */
    0x06, 0x26, 0x00, 0x4E, 0x2A,

    /* 188 - HIDReportMap(15); Property: 0x02, Value Handle: 0x0028, UUID: 0x2A4B */
    0x02, 0x28, 0x00, 0x4B, 0x2A,

    /* 193 - Input HIDReport(16); Property: 0x1A, Value Handle: 0x002B, UUID: 0x2A4D */
    0x1A, 0x2B, 0x00, 0x4D, 0x2A,

    /* 198 - Output HIDReport(17); Property: 0x0E, Value Handle: 0x002F, UUID: 0x2A4D */
    0x0E, 0x2F, 0x00, 0x4D, 0x2A,

    /* 203 - Feature HIDReport(18); Property: 0x0A, Value Handle: 0x0032, UUID: 0x2A4D */
    0x0A, 0x32, 0x00, 0x4D, 0x2A,

    /* 208 - HIDControlPoint(19); Property: 0x04, Value Handle: 0x0035, UUID: 0x2A4C */
    0x04, 0x35, 0x00, 0x4C, 0x2A,

    /* 213 - HIDInformation(20); Property: 0x02, Value Handle: 0x0037, UUID: 0x2A4A */
    0x02, 0x37, 0x00, 0x4A, 0x2A,

    /* 218 - BootMouseInputReport(21); Property: 0x1A, Value Handle: 0x0039, UUID: 0x2A33 */
    0x1A, 0x39, 0x00, 0x33, 0x2A,

    /* 223 - BootKeyboardInputReport(22); Property: 0x1A, Value Handle: 0x003C, UUID: 0x2A22 */
    0x1A, 0x3C, 0x00, 0x22, 0x2A,

    /* 228 - BootKeyboardOutputReport(23); Property: 0x0E, Value Handle: 0x003F, UUID: 0x2A32 */
    0x0E, 0x3F, 0x00, 0x32, 0x2A,

    /* 233 - ScanIntervalWindow(24); Property: 0x04, Value Handle: 0x0042, UUID: 0x2A4F */
    0x04, 0x42, 0x00, 0x4F, 0x2A,

    /* 238 - ScanRefresh(25); Property: 0x10, Value Handle: 0x0044, UUID: 0x2A31 */
    0x10, 0x44, 0x00, 0x31, 0x2A,
};


UCHAR gatt_db_peer_specific_val_arr[GATT_DB_MAX_PEER_CONFIGURATION];

DECL_CONST UCHAR gatt_db_const_peer_specific_val_arr[GATT_DB_PEER_VALUE_ARRAY_SIZE] =
{
    /* 0 - Client Configuration for Service Changed */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 2 - Client Configuration for BatteryLevel */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 4 - Input HIDReport(16); Default: 0 */
    /* Client Instance 0 */
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* 14 - Client Configuration for Input HIDReport */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 16 - Output HIDReport(17); Default: 0 */
    /* Client Instance 0 */
    0x03, 0x00, 0x00, 0x00 ,

    /* 20 - Feature HIDReport(18); Default: 0 */
    /* Client Instance 0 */
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  ,

    /* 29 - HIDControlPoint(19); Default: 0 */
    /* Client Instance 0 */
    0x00,

    /* 30 - BootMouseInputReport(21); Default: 0 */
    /* Client Instance 0 */
    0x03, 0x00, 0x00, 0x00,

    /* 34 - Client Configuration for BootMouseInputReport */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 36 - BootKeyboardInputReport(22); Default: 0 */
    /* Client Instance 0 */
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* 45 - Client Configuration for BootKeyboardInputReport */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 47 - BootKeyboardOutputReport(23); Default: 0 */
    /* Client Instance 0 */
    0x03, 0x00, 0x00, 0x00,

    /* 51 - ScanIntervalWindow(24); Default: 0 */
    /* Client Instance 0 */
    0x40, 0x00, 0x10, 0x00,

    /* 55 - Client Configuration for ScanRefresh */
    /* Client Instance 0 */
    0x00, 0x00,
};


DECL_CONST GATT_ATTR_TYPE gatt_type_table[GATT_DB_MAX_TYPE_COUNT] =
{
    {
        /* UUID Offset */
        0,

        /* First Occurence for Type */
        1,

        /* Last Occurence for Type */
        64,

    },
    {
        /* UUID Offset */
        6,

        /* First Occurence for Type */
        2,

        /* Last Occurence for Type */
        67,

    },
    {
        /* UUID Offset */
        12,

        /* First Occurence for Type */
        4,

        /* Last Occurence for Type */
        69,

    },
    {
        /* UUID Offset */
        16,

        /* First Occurence for Type */
        35,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        20,

        /* First Occurence for Type */
        1,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        22,

        /* First Occurence for Type */
        3,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        24,

        /* First Occurence for Type */
        5,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        26,

        /* First Occurence for Type */
        7,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        28,

        /* First Occurence for Type */
        9,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        30,

        /* First Occurence for Type */
        11,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        32,

        /* First Occurence for Type */
        12,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        34,

        /* First Occurence for Type */
        14,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        36,

        /* First Occurence for Type */
        16,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        38,

        /* First Occurence for Type */
        18,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        40,

        /* First Occurence for Type */
        20,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        42,

        /* First Occurence for Type */
        22,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        44,

        /* First Occurence for Type */
        24,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        46,

        /* First Occurence for Type */
        26,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        48,

        /* First Occurence for Type */
        28,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        50,

        /* First Occurence for Type */
        30,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        52,

        /* First Occurence for Type */
        31,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        54,

        /* First Occurence for Type */
        33,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        56,

        /* First Occurence for Type */
        36,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        58,

        /* First Occurence for Type */
        38,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        60,

        /* First Occurence for Type */
        40,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        62,

        /* First Occurence for Type */
        41,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        64,

        /* First Occurence for Type */
        43,

        /* Last Occurence for Type */
        50,

    },
    {
        /* UUID Offset */
        66,

        /* First Occurence for Type */
        45,

        /* Last Occurence for Type */
        51,

    },
    {
        /* UUID Offset */
        68,

        /* First Occurence for Type */
        53,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        70,

        /* First Occurence for Type */
        55,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        72,

        /* First Occurence for Type */
        57,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        74,

        /* First Occurence for Type */
        60,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        76,

        /* First Occurence for Type */
        63,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        78,

        /* First Occurence for Type */
        64,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        80,

        /* First Occurence for Type */
        66,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        82,

        /* First Occurence for Type */
        68,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
};


DECL_CONST GATT_DB_DESC_DATA gatt_db_attr_table[GATT_DB_MAX_ATTRIBUTES] =
{
    /* Dummy */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* Value */
        NULL,
    },

    /* Handle - 0x0001 */
    /* 0 - GATT - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        5,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 20),
    },

    /* Handle - 0x0002 */
    /* 0 - Service Changed - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        6,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 0),
    },

    /* Handle - 0x0003 */
    /* 0 - Service Changed - Characteristic Value */
    {
        /* Property */
        GATT_DB_CHAR_INDICATE_PROPERTY,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length */
        0,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        22,

        /* Value */
        NULL,
    },

    /* Handle - 0x0004 */
    /* 0 - Service Changed - CCD */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        34,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 0,
    },

    /* Handle - 0x0005 */
    /* 1 - GAP - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        12,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 24),
    },

    /* Handle - 0x0006 */
    /* 1 - Device Name - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        8,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 5),
    },

    /* Handle - 0x0007 */
    /* 1 - Device Name - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        9,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        26,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 10),
    },

    /* Handle - 0x0008 */
    /* 2 - Appearance - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        10,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 19),
    },

    /* Handle - 0x0009 */
    /* 2 - Appearance - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        28,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 24),
    },

    /* Handle - 0x000A */
    /* 3 - Peripheral Preferred Connection Paramters - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        13,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 26),
    },

    /* Handle - 0x000B */
    /* 3 - Peripheral Preferred Connection Paramters - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        8,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        30,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 31),
    },

    /* Handle - 0x000C */
    /* 2 - DeviceInformation - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        31,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 32),
    },

    /* Handle - 0x000D */
    /* 4 - ManufacturerName - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        15,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 39),
    },

    /* Handle - 0x000E */
    /* 4 - ManufacturerName - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        15,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        34,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 44),
    },

    /* Handle - 0x000F */
    /* 5 - ModelNumber - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        17,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 59),
    },

    /* Handle - 0x0010 */
    /* 5 - ModelNumber - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        36,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 64),
    },

    /* Handle - 0x0011 */
    /* 6 - SerialNumber - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        19,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 69),
    },

    /* Handle - 0x0012 */
    /* 6 - SerialNumber - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        38,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 74),
    },

    /* Handle - 0x0013 */
    /* 7 - HardwareRevision - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        21,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 79),
    },

    /* Handle - 0x0014 */
    /* 7 - HardwareRevision - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        40,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 84),
    },

    /* Handle - 0x0015 */
    /* 8 - FirmwareRevision - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        23,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 89),
    },

    /* Handle - 0x0016 */
    /* 8 - FirmwareRevision - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        42,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 94),
    },

    /* Handle - 0x0017 */
    /* 9 - SoftwareRevision - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        25,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 99),
    },

    /* Handle - 0x0018 */
    /* 9 - SoftwareRevision - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        44,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 104),
    },

    /* Handle - 0x0019 */
    /* 10 - SystemId - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        27,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 109),
    },

    /* Handle - 0x001A */
    /* 10 - SystemId - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        8,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        46,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 114),
    },

    /* Handle - 0x001B */
    /* 11 - RegCertDataList - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        29,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 122),
    },

    /* Handle - 0x001C */
    /* 11 - RegCertDataList - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        32,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        48,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 127),
    },

    /* Handle - 0x001D */
    /* 12 - PnPID - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        32,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 159),
    },

    /* Handle - 0x001E */
    /* 12 - PnPID - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        7,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        50,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 164),
    },

    /* Handle - 0x001F */
    /* 3 - Battery - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        36,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 52),
    },

    /* Handle - 0x0020 */
    /* 13 - BatteryLevel - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        37,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 171),
    },

    /* Handle - 0x0021 */
    /* 13 - BatteryLevel - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        54,

        /* Value */
        gatt_value_arr + 0,
    },

    /* Handle - 0x0022 */
    /* 13 - BatteryLevel - CCD */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        44,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 2,
    },

    /* Handle - 0x0023 */
    /* Characteristic 13 - BatteryLevel - Presentation Format */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        7,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        16,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 176),
    },

    /* Handle - 0x0024 */
    /* 4 - HumanInterfaceDevice - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        64,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 56),
    },

    /* Handle - 0x0025 */
    /* 14 - HIDProtocolMode - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        39,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 183),
    },

    /* Handle - 0x0026 */
    /* 14 - HIDProtocolMode - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE_WITHOUT_RSP,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        58,

        /* Value */
        gatt_value_arr + 1,
    },

    /* Handle - 0x0027 */
    /* 15 - HIDReportMap - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        42,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 188),
    },

    /* Handle - 0x0028 */
    /* 15 - HIDReportMap - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length + Length Header Byte */
        (63 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        60,

        /* Value */
        gatt_value_arr + 2,
    },

    /* Handle - 0x0029 */
    /* Characteristic 15 - HIDReportMap - HLD */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        62,

        /* Value */
        gatt_value_arr + 66,
    },

    /* Handle - 0x002A */
    /* 16 - Input HIDReport - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        46,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 193),
    },

    /* Handle - 0x002B */
    /* 16 - Input HIDReport - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length + Length Header Byte */
        (9 + 1),

        /* Next Attribute Type Index */
        47,

        /* UUID Offset */
        64,

        /* Value */
        gatt_db_peer_specific_val_arr + 4,
    },

    /* Handle - 0x002C */
    /* 16 - Input HIDReport - CCD */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        58,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 14,
    },

    /* Handle - 0x002D */
    /* Characteristic 16 - Input HIDReport - HLD */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        48,

        /* UUID Offset */
        66,

        /* Value */
        gatt_value_arr + 68,
    },

    /* Handle - 0x002E */
    /* 17 - Output HIDReport - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        49,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 198),
    },

    /* Handle - 0x002F */
    /* 17 - Output HIDReport - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_WRITE_WITHOUT_RSP,

        /* Auxillary Property */
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length + Length Header Byte */
        (3 + 1),

        /* Next Attribute Type Index */
        50,

        /* UUID Offset */
        64,

        /* Value */
        gatt_db_peer_specific_val_arr + 16,
    },

    /* Handle - 0x0030 */
    /* Characteristic 17 - Output HIDReport - HLD */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        51,

        /* UUID Offset */
        66,

        /* Value */
        gatt_value_arr + 70,
    },

    /* Handle - 0x0031 */
    /* 18 - Feature HIDReport - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        52,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 203),
    },

    /* Handle - 0x0032 */
    /* 18 - Feature HIDReport - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length + Length Header Byte */
        (8 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        64,

        /* Value */
        gatt_db_peer_specific_val_arr + 20,
    },

    /* Handle - 0x0033 */
    /* Characteristic 18 - Feature HIDReport - HLD */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        66,

        /* Value */
        gatt_value_arr + 72,
    },

    /* Handle - 0x0034 */
    /* 19 - HIDControlPoint - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        54,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 208),
    },

    /* Handle - 0x0035 */
    /* 19 - HIDControlPoint - Characteristic Value */
    {
        /* Property */
        GATT_DB_WRITE_WITHOUT_RSP,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        68,

        /* Value */
        gatt_db_peer_specific_val_arr + 29,
    },

    /* Handle - 0x0036 */
    /* 20 - HIDInformation - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        56,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 213),
    },

    /* Handle - 0x0037 */
    /* 20 - HIDInformation - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length + Length Header Byte */
        (4 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        70,

        /* Value */
        gatt_value_arr + 74,
    },

    /* Handle - 0x0038 */
    /* 21 - BootMouseInputReport - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        59,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 218),
    },

    /* Handle - 0x0039 */
    /* 21 - BootMouseInputReport - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length + Length Header Byte */
        (3 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        72,

        /* Value */
        gatt_db_peer_specific_val_arr + 30,
    },

    /* Handle - 0x003A */
    /* 21 - BootMouseInputReport - CCD */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        61,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 34,
    },

    /* Handle - 0x003B */
    /* 22 - BootKeyboardInputReport - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        62,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 223),
    },

    /* Handle - 0x003C */
    /* 22 - BootKeyboardInputReport - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length + Length Header Byte */
        (8 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        74,

        /* Value */
        gatt_db_peer_specific_val_arr + 36,
    },

    /* Handle - 0x003D */
    /* 22 - BootKeyboardInputReport - CCD */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        69,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 45,
    },

    /* Handle - 0x003E */
    /* 23 - BootKeyboardOutputReport - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        65,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 228),
    },

    /* Handle - 0x003F */
    /* 23 - BootKeyboardOutputReport - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_WRITE_WITHOUT_RSP,

        /* Auxillary Property */
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length + Length Header Byte */
        (3 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        76,

        /* Value */
        gatt_db_peer_specific_val_arr + 47,
    },

    /* Handle - 0x0040 */
    /* 5 - ScanParameter - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 78),
    },

    /* Handle - 0x0041 */
    /* 24 - ScanIntervalWindow - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        67,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 233),
    },

    /* Handle - 0x0042 */
    /* 24 - ScanIntervalWindow - Characteristic Value */
    {
        /* Property */
        GATT_DB_WRITE_WITHOUT_RSP,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        4,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        80,

        /* Value */
        gatt_db_peer_specific_val_arr + 51,
    },

    /* Handle - 0x0043 */
    /* 25 - ScanRefresh - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 238),
    },

    /* Handle - 0x0044 */
    /* 25 - ScanRefresh - Characteristic Value */
    {
        /* Property */
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length */
        0,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        82,

        /* Value */
        NULL,
    },

    /* Handle - 0x0045 */
    /* 25 - ScanRefresh - CCD */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY |
        GATT_DB_PEER_SPECIFIC_VAL_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 55,
    },

};


DECL_CONST GATT_DB_CHARACERISTIC gatt_characteristic[GATT_CHARACTERISTIC_COUNT] =
{

    /* 0 - Service Changed */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0002,

        /* Characteristic Service Index */
        0,
    },

    /* 1 - Device Name */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0006,

        /* Characteristic Service Index */
        1,
    },

    /* 2 - Appearance */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0008,

        /* Characteristic Service Index */
        1,
    },

    /* 3 - Peripheral Preferred Connection Paramters */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x000A,

        /* Characteristic Service Index */
        1,
    },

    /* 4 - ManufacturerName */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x000D,

        /* Characteristic Service Index */
        2,
    },

    /* 5 - ModelNumber */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x000F,

        /* Characteristic Service Index */
        2,
    },

    /* 6 - SerialNumber */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0011,

        /* Characteristic Service Index */
        2,
    },

    /* 7 - HardwareRevision */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0013,

        /* Characteristic Service Index */
        2,
    },

    /* 8 - FirmwareRevision */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0015,

        /* Characteristic Service Index */
        2,
    },

    /* 9 - SoftwareRevision */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0017,

        /* Characteristic Service Index */
        2,
    },

    /* 10 - SystemId */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0019,

        /* Characteristic Service Index */
        2,
    },

    /* 11 - RegCertDataList */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x001B,

        /* Characteristic Service Index */
        2,
    },

    /* 12 - PnPID */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x001D,

        /* Characteristic Service Index */
        2,
    },

    /* 13 - BatteryLevel */
    {
        {
            /* Number of attributes in characteristic including the definition */
            4,
        },

        /* Characteristic Start Handle */
        0x0020,

        /* Characteristic Service Index */
        3,
    },

    /* 14 - HIDProtocolMode */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0025,

        /* Characteristic Service Index */
        4,
    },

    /* 15 - HIDReportMap */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0027,

        /* Characteristic Service Index */
        4,
    },

    /* 16 - Input HIDReport */
    {
        {
            /* Number of attributes in characteristic including the definition */
            4,
        },

        /* Characteristic Start Handle */
        0x002A,

        /* Characteristic Service Index */
        4,
    },

    /* 17 - Output HIDReport */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x002E,

        /* Characteristic Service Index */
        4,
    },

    /* 18 - Feature HIDReport */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0031,

        /* Characteristic Service Index */
        4,
    },

    /* 19 - HIDControlPoint */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0034,

        /* Characteristic Service Index */
        4,
    },

    /* 20 - HIDInformation */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0036,

        /* Characteristic Service Index */
        4,
    },

    /* 21 - BootMouseInputReport */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0038,

        /* Characteristic Service Index */
        4,
    },

    /* 22 - BootKeyboardInputReport */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x003B,

        /* Characteristic Service Index */
        4,
    },

    /* 23 - BootKeyboardOutputReport */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x003E,

        /* Characteristic Service Index */
        4,
    },

    /* 24 - ScanIntervalWindow */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0041,

        /* Characteristic Service Index */
        5,
    },

    /* 25 - ScanRefresh */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0043,

        /* Characteristic Service Index */
        5,
    },
};


DECL_CONST GATT_DB_SERVICE gatt_service[GATT_SERVICE_COUNT] =
{

    /* 0 - GATT */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0001,

        /* Service End Handle */
        0x0004,

        /* Characteristic Start Index */
        0,

        /* Characteristic End Index */
        0
    },

    /* 1 - GAP */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0005,

        /* Service End Handle */
        0x000B,

        /* Characteristic Start Index */
        1,

        /* Characteristic End Index */
        3
    },

    /* 2 - DeviceInformation */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x000C,

        /* Service End Handle */
        0x001E,

        /* Characteristic Start Index */
        4,

        /* Characteristic End Index */
        12
    },

    /* 3 - Battery */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x001F,

        /* Service End Handle */
        0x0023,

        /* Characteristic Start Index */
        13,

        /* Characteristic End Index */
        13
    },

    /* 4 - HumanInterfaceDevice */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0024,

        /* Service End Handle */
        0x003F,

        /* Characteristic Start Index */
        14,

        /* Characteristic End Index */
        23
    },

    /* 5 - ScanParameter */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0040,

        /* Service End Handle */
        0x0045,

        /* Characteristic Start Index */
        24,

        /* Characteristic End Index */
        25
    },
};
#endif /* ((defined HID) && (defined GATT_DB)) */

