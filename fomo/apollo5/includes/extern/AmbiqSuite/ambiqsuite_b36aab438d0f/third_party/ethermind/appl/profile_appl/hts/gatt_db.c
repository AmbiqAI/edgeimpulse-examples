
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
 *  0x0001  |       0x2800       | Rd               | 0x1800
 *  --------+--------------------+------------------+---------------------
 *  0x0002  |       0x2803       | Rd               | 0x02, 0x0003, 0x2A00
 *  --------+--------------------+------------------+---------------------
 *  0x0003  |       0x2A00       | Rd               | MindtreeHealthThermometer
 *  --------+--------------------+------------------+---------------------
 *  0x0004  |       0x2803       | Rd               | 0x02, 0x0005, 0x2A01
 *  --------+--------------------+------------------+---------------------
 *  0x0005  |       0x2A01       | Rd               | 768.0
 *  ========+====================+==================+=====================
 *  0x0006  |       0x2800       | Rd               | 0x1801
 *  --------+--------------------+------------------+---------------------
 *  0x0007  |       0x2803       | Rd               | 0x20, 0x0008, 0x2A05
 *  --------+--------------------+------------------+---------------------
 *  0x0008  |       0x2A05       | Ind              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0009  |       0x2902       | Rd, Wr           | 0x0000
 *  ========+====================+==================+=====================
 *  0x000A  |       0x2800       | Rd               | 0x180F
 *  --------+--------------------+------------------+---------------------
 *  0x000B  |       0x2803       | Rd               | 0x12, 0x000C, 0x2A19
 *  --------+--------------------+------------------+---------------------
 *  0x000C  |       0x2A19       | Rd, Ntf          | 100.0
 *  --------+--------------------+------------------+---------------------
 *  0x000D  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x000E  |       0x2904       | Rd               | VALUE
 *  ========+====================+==================+=====================
 *  0x000F  |       0x2800       | Rd               | 0x180A
 *  --------+--------------------+------------------+---------------------
 *  0x0010  |       0x2803       | Rd               | 0x02, 0x0011, 0x2A29
 *  --------+--------------------+------------------+---------------------
 *  0x0011  |       0x2A29       | Rd               | Mindtree Limited.
 *  --------+--------------------+------------------+---------------------
 *  0x0012  |       0x2803       | Rd               | 0x02, 0x0013, 0x2A24
 *  --------+--------------------+------------------+---------------------
 *  0x0013  |       0x2A24       | Rd               | HTS-1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0014  |       0x2803       | Rd               | 0x02, 0x0015, 0x2A25
 *  --------+--------------------+------------------+---------------------
 *  0x0015  |       0x2A25       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0016  |       0x2803       | Rd               | 0x02, 0x0017, 0x2A26
 *  --------+--------------------+------------------+---------------------
 *  0x0017  |       0x2A26       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0018  |       0x2803       | Rd               | 0x02, 0x0019, 0x2A27
 *  --------+--------------------+------------------+---------------------
 *  0x0019  |       0x2A27       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x001A  |       0x2803       | Rd               | 0x02, 0x001B, 0x2A28
 *  --------+--------------------+------------------+---------------------
 *  0x001B  |       0x2A28       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x001C  |       0x2803       | Rd               | 0x02, 0x001D, 0x2A23
 *  --------+--------------------+------------------+---------------------
 *  0x001D  |       0x2A23       | Rd               | 11223344.0
 *  --------+--------------------+------------------+---------------------
 *  0x001E  |       0x2803       | Rd               | 0x02, 0x001F, 0x2A2A
 *  --------+--------------------+------------------+---------------------
 *  0x001F  |       0x2A2A       | Rd               | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0020  |       0x2803       | Rd               | 0x02, 0x0021, 0x2A50
 *  --------+--------------------+------------------+---------------------
 *  0x0021  |       0x2A50       | Rd               | 0
 *  ========+====================+==================+=====================
 *  0x0022  |       0x2800       | Rd               | 0x1809
 *  --------+--------------------+------------------+---------------------
 *  0x0023  |       0x2803       | Rd               | 0x20, 0x0024, 0x2A1C
 *  --------+--------------------+------------------+---------------------
 *  0x0024  |       0x2A1C       | Ind              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0025  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0026  |       0x2803       | Rd               | 0x10, 0x0027, 0x2A1E
 *  --------+--------------------+------------------+---------------------
 *  0x0027  |       0x2A1E       | Ntf              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0028  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0029  |       0x2803       | Rd               | 0x02, 0x002A, 0x2A1D
 *  --------+--------------------+------------------+---------------------
 *  0x002A  |       0x2A1D       | Rd               | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x002B  |       0x2803       | Rd               | 0x2A, 0x002C, 0x2A21
 *  --------+--------------------+------------------+---------------------
 *  0x002C  |       0x2A21       | Rd, Wr, Ind      | 5.0
 *  --------+--------------------+------------------+---------------------
 *  0x002D  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x002E  |       0x2906       | Rd               | VALUE
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "gatt_defines.h"

#if ((defined HTS) && (defined GATT_DB))

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

    /* 20 - GAP Service UUID */
    0x00, 0x18,

    /* 22 - DeviceName Characteristic UUID */
    0x00, 0x2A,

    /* 24 - Appearance Characteristic UUID */
    0x01, 0x2A,

    /* 26 - GATT Service UUID */
    0x01, 0x18,

    /* 28 - Service Changed Characteristic UUID */
    0x05, 0x2A,

    /* 30 - Battery Service UUID */
    0x0F, 0x18,

    /* 32 - BatteryLevel Characteristic UUID */
    0x19, 0x2A,

    /* 34 - DeviceInformation Service UUID */
    0x0A, 0x18,

    /* 36 - ManufacturerName Characteristic UUID */
    0x29, 0x2A,

    /* 38 - ModelNumber Characteristic UUID */
    0x24, 0x2A,

    /* 40 - SerialNumber Characteristic UUID */
    0x25, 0x2A,

    /* 42 - FirmwareRevision Characteristic UUID */
    0x26, 0x2A,

    /* 44 - HardwareRevision Characteristic UUID */
    0x27, 0x2A,

    /* 46 - SoftwareRevision Characteristic UUID */
    0x28, 0x2A,

    /* 48 - SystemId Characteristic UUID */
    0x23, 0x2A,

    /* 50 - RegCertDataList Characteristic UUID */
    0x2A, 0x2A,

    /* 52 - PnPID Characteristic UUID */
    0x50, 0x2A,

    /* 54 - Health Thermometer Service UUID */
    0x09, 0x18,

    /* 56 - TemperatureMeasurement Characteristic UUID */
    0x1C, 0x2A,

    /* 58 - IntermediateTemperature Characteristic UUID */
    0x1E, 0x2A,

    /* 60 - TemperatureType Characteristic UUID */
    0x1D, 0x2A,

    /* 62 - Measurement Interval Characteristic UUID */
    0x21, 0x2A,

    /* 64 - Measurement Interval Characteristic HLD UUID */
    0x06, 0x29,

};


UCHAR gatt_value_arr[GATT_VALUE_ARRAY_SIZE] =
{
    /* 0 - BatteryLevel(3); Default: 100.0 */
    0x64,

    /* 1 - Measurement Interval(16); Default: 5.0 */
    0x05, 0x00,

    /* 3 - HLD for Measurement Interval */
    /* Valid Range Descriptor   */
    0x01, 0x00, 0x0A, 0x00,
};


DECL_CONST UCHAR gatt_const_value_arr[GATT_CONST_VALUE_ARRAY_SIZE] =
{

    /* 0 - DeviceName(0); Property: 0x02, Value Handle: 0x0003, UUID: 0x2A00 */
    0x02, 0x03, 0x00, 0x00, 0x2A,

    /* 5 - DeviceName(0); Default: MindtreeHealthThermometer */
    0x4D, 0x69, 0x6E, 0x64, 0x74, 0x72, 0x65, 0x65, 0x48, 0x65, 0x61, 0x6C,
    0x74, 0x68, 0x54, 0x68, 0x65, 0x72, 0x6D, 0x6F, 0x6D, 0x65, 0x74, 0x65,
    0x72,

    /* 30 - Appearance(1); Property: 0x02, Value Handle: 0x0005, UUID: 0x2A01 */
    0x02, 0x05, 0x00, 0x01, 0x2A,

    /* 35 - Appearance(1); Default: 768.0 */
    0x03, 0x00,

    /* 37 - Service Changed(2); Property: 0x20, Value Handle: 0x0008, UUID: 0x2A05 */
    0x20, 0x08, 0x00, 0x05, 0x2A,

    /* 42 - BatteryLevel(3); Property: 0x12, Value Handle: 0x000C, UUID: 0x2A19 */
    0x12, 0x0C, 0x00, 0x19, 0x2A,

    /* 47 - Presentation Fromat for BatteryLevel */
    /**
     * Characteristic Presentation Format for BatteryLevel
     * - Format            (1 octet)
     * - Exponent          (1 octet)
     * - Unit              (2 octet)
     * - Name Space        (1 octet)
     * - Description       (2 octet)
     */
    0x04, 0x00, 0xAD, 0x27, 0x01, 0x00, 0x00,

    /* 54 - ManufacturerName(4); Property: 0x02, Value Handle: 0x0011, UUID: 0x2A29 */
    0x02, 0x11, 0x00, 0x29, 0x2A,

    /* 59 - ManufacturerName(4); Default: Mindtree Limited. */
    0x4D, 0x69, 0x6E, 0x64, 0x74, 0x72, 0x65, 0x65, 0x20, 0x4C, 0x69, 0x6D,
    0x69, 0x74, 0x65, 0x64, 0x2E,

    /* 76 - ModelNumber(5); Property: 0x02, Value Handle: 0x0013, UUID: 0x2A24 */
    0x02, 0x13, 0x00, 0x24, 0x2A,

    /* 81 - ModelNumber(5); Default: HTS-1.0.0 */
    0x48, 0x54, 0x53, 0x2D, 0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 90 - SerialNumber(6); Property: 0x02, Value Handle: 0x0015, UUID: 0x2A25 */
    0x02, 0x15, 0x00, 0x25, 0x2A,

    /* 95 - SerialNumber(6); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 100 - FirmwareRevision(7); Property: 0x02, Value Handle: 0x0017, UUID: 0x2A26 */
    0x02, 0x17, 0x00, 0x26, 0x2A,

    /* 105 - FirmwareRevision(7); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 110 - HardwareRevision(8); Property: 0x02, Value Handle: 0x0019, UUID: 0x2A27 */
    0x02, 0x19, 0x00, 0x27, 0x2A,

    /* 115 - HardwareRevision(8); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 120 - SoftwareRevision(9); Property: 0x02, Value Handle: 0x001B, UUID: 0x2A28 */
    0x02, 0x1B, 0x00, 0x28, 0x2A,

    /* 125 - SoftwareRevision(9); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 130 - SystemId(10); Property: 0x02, Value Handle: 0x001D, UUID: 0x2A23 */
    0x02, 0x1D, 0x00, 0x23, 0x2A,

    /* 135 - SystemId(10); Default: 11223344.0 */
    0x31, 0x31, 0x32, 0x32, 0x33, 0x33, 0x34, 0x34,

    /* 143 - RegCertDataList(11); Property: 0x02, Value Handle: 0x001F, UUID: 0x2A2A */
    0x02, 0x1F, 0x00, 0x2A, 0x2A,

    /* 148 - RegCertDataList(11); Default: 0.0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* 180 - PnPID(12); Property: 0x02, Value Handle: 0x0021, UUID: 0x2A50 */
    0x02, 0x21, 0x00, 0x50, 0x2A,

    /* 185 - PnPID(12); Default: 0 */
    0x01, 0x6A, 0x00, 0x4D, 0x01, 0x00, 0x01,

    /* 192 - TemperatureMeasurement(13); Property: 0x20, Value Handle: 0x0024, UUID: 0x2A1C */
    0x20, 0x24, 0x00, 0x1C, 0x2A,

    /* 197 - IntermediateTemperature(14); Property: 0x10, Value Handle: 0x0027, UUID: 0x2A1E */
    0x10, 0x27, 0x00, 0x1E, 0x2A,

    /* 202 - TemperatureType(15); Property: 0x02, Value Handle: 0x002A, UUID: 0x2A1D */
    0x02, 0x2A, 0x00, 0x1D, 0x2A,

    /* 207 - TemperatureType(15); Default: 1.0 */
    0x01,

    /* 208 - Measurement Interval(16); Property: 0x2A, Value Handle: 0x002C, UUID: 0x2A21 */
    0x2A, 0x2C, 0x00, 0x21, 0x2A,
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

    /* 4 - Client Configuration for TemperatureMeasurement */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 6 - Client Configuration for IntermediateTemperature */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 8 - Client Configuration for Measurement Interval */
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
        34,

    },
    {
        /* UUID Offset */
        6,

        /* First Occurence for Type */
        2,

        /* Last Occurence for Type */
        43,

    },
    {
        /* UUID Offset */
        12,

        /* First Occurence for Type */
        9,

        /* Last Occurence for Type */
        45,

    },
    {
        /* UUID Offset */
        16,

        /* First Occurence for Type */
        14,

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
        6,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        28,

        /* First Occurence for Type */
        8,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        30,

        /* First Occurence for Type */
        10,

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
        15,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        36,

        /* First Occurence for Type */
        17,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        38,

        /* First Occurence for Type */
        19,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        40,

        /* First Occurence for Type */
        21,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        42,

        /* First Occurence for Type */
        23,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        44,

        /* First Occurence for Type */
        25,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        46,

        /* First Occurence for Type */
        27,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        48,

        /* First Occurence for Type */
        29,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        50,

        /* First Occurence for Type */
        31,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        52,

        /* First Occurence for Type */
        33,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        54,

        /* First Occurence for Type */
        34,

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
        39,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        60,

        /* First Occurence for Type */
        42,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        62,

        /* First Occurence for Type */
        44,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        64,

        /* First Occurence for Type */
        46,

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
    /* 0 - GAP - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        6,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 20),
    },

    /* Handle - 0x0002 */
    /* 0 - DeviceName - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        4,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 0),
    },

    /* Handle - 0x0003 */
    /* 0 - DeviceName - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        25,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        22,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 5),
    },

    /* Handle - 0x0004 */
    /* 1 - Appearance - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        7,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 30),
    },

    /* Handle - 0x0005 */
    /* 1 - Appearance - Characteristic Value */
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
        24,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 35),
    },

    /* Handle - 0x0006 */
    /* 1 - GATT - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        10,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 26),
    },

    /* Handle - 0x0007 */
    /* 2 - Service Changed - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        11,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 37),
    },

    /* Handle - 0x0008 */
    /* 2 - Service Changed - Characteristic Value */
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
        28,

        /* Value */
        NULL,
    },

    /* Handle - 0x0009 */
    /* 2 - Service Changed - CCD */
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
        13,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 0,
    },

    /* Handle - 0x000A */
    /* 2 - Battery - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        15,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 30),
    },

    /* Handle - 0x000B */
    /* 3 - BatteryLevel - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        16,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 42),
    },

    /* Handle - 0x000C */
    /* 3 - BatteryLevel - Characteristic Value */
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
        32,

        /* Value */
        gatt_value_arr + 0,
    },

    /* Handle - 0x000D */
    /* 3 - BatteryLevel - CCD */
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
        37,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 2,
    },

    /* Handle - 0x000E */
    /* Characteristic 3 - BatteryLevel - Presentation Format */
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
        (UCHAR *)(gatt_const_value_arr + 47),
    },

    /* Handle - 0x000F */
    /* 3 - DeviceInformation - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        34,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 34),
    },

    /* Handle - 0x0010 */
    /* 4 - ManufacturerName - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        18,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 54),
    },

    /* Handle - 0x0011 */
    /* 4 - ManufacturerName - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        17,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        36,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 59),
    },

    /* Handle - 0x0012 */
    /* 5 - ModelNumber - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        20,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 76),
    },

    /* Handle - 0x0013 */
    /* 5 - ModelNumber - Characteristic Value */
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
        38,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 81),
    },

    /* Handle - 0x0014 */
    /* 6 - SerialNumber - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        22,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 90),
    },

    /* Handle - 0x0015 */
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
        40,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 95),
    },

    /* Handle - 0x0016 */
    /* 7 - FirmwareRevision - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        24,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 100),
    },

    /* Handle - 0x0017 */
    /* 7 - FirmwareRevision - Characteristic Value */
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
        (UCHAR *)(gatt_const_value_arr + 105),
    },

    /* Handle - 0x0018 */
    /* 8 - HardwareRevision - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        26,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 110),
    },

    /* Handle - 0x0019 */
    /* 8 - HardwareRevision - Characteristic Value */
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
        (UCHAR *)(gatt_const_value_arr + 115),
    },

    /* Handle - 0x001A */
    /* 9 - SoftwareRevision - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        28,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 120),
    },

    /* Handle - 0x001B */
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
        46,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 125),
    },

    /* Handle - 0x001C */
    /* 10 - SystemId - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        30,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 130),
    },

    /* Handle - 0x001D */
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
        48,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 135),
    },

    /* Handle - 0x001E */
    /* 11 - RegCertDataList - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 143),
    },

    /* Handle - 0x001F */
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
        50,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 148),
    },

    /* Handle - 0x0020 */
    /* 12 - PnPID - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        35,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 180),
    },

    /* Handle - 0x0021 */
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
        52,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 185),
    },

    /* Handle - 0x0022 */
    /* 4 - Health Thermometer - Service Declaration */
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
        (UCHAR *)(gatt_const_uuid_arr + 54),
    },

    /* Handle - 0x0023 */
    /* 13 - TemperatureMeasurement - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        38,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 192),
    },

    /* Handle - 0x0024 */
    /* 13 - TemperatureMeasurement - Characteristic Value */
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
        56,

        /* Value */
        NULL,
    },

    /* Handle - 0x0025 */
    /* 13 - TemperatureMeasurement - CCD */
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
        40,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 4,
    },

    /* Handle - 0x0026 */
    /* 14 - IntermediateTemperature - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        41,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 197),
    },

    /* Handle - 0x0027 */
    /* 14 - IntermediateTemperature - Characteristic Value */
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
        58,

        /* Value */
        NULL,
    },

    /* Handle - 0x0028 */
    /* 14 - IntermediateTemperature - CCD */
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
        45,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 6,
    },

    /* Handle - 0x0029 */
    /* 15 - TemperatureType - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        43,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 202),
    },

    /* Handle - 0x002A */
    /* 15 - TemperatureType - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        60,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 207),
    },

    /* Handle - 0x002B */
    /* 16 - Measurement Interval - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 208),
    },

    /* Handle - 0x002C */
    /* 16 - Measurement Interval - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_CHAR_INDICATE_PROPERTY,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        62,

        /* Value */
        gatt_value_arr + 1,
    },

    /* Handle - 0x002D */
    /* 16 - Measurement Interval - CCD */
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
        gatt_db_peer_specific_val_arr + 8,
    },

    /* Handle - 0x002E */
    /* Characteristic 16 - Measurement Interval - HLD */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        4,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        64,

        /* Value */
        gatt_value_arr + 3,
    },

};


DECL_CONST GATT_DB_CHARACERISTIC gatt_characteristic[GATT_CHARACTERISTIC_COUNT] =
{

    /* 0 - DeviceName */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0002,

        /* Characteristic Service Index */
        0,
    },

    /* 1 - Appearance */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0004,

        /* Characteristic Service Index */
        0,
    },

    /* 2 - Service Changed */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0007,

        /* Characteristic Service Index */
        1,
    },

    /* 3 - BatteryLevel */
    {
        {
            /* Number of attributes in characteristic including the definition */
            4,
        },

        /* Characteristic Start Handle */
        0x000B,

        /* Characteristic Service Index */
        2,
    },

    /* 4 - ManufacturerName */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0010,

        /* Characteristic Service Index */
        3,
    },

    /* 5 - ModelNumber */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0012,

        /* Characteristic Service Index */
        3,
    },

    /* 6 - SerialNumber */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0014,

        /* Characteristic Service Index */
        3,
    },

    /* 7 - FirmwareRevision */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0016,

        /* Characteristic Service Index */
        3,
    },

    /* 8 - HardwareRevision */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0018,

        /* Characteristic Service Index */
        3,
    },

    /* 9 - SoftwareRevision */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x001A,

        /* Characteristic Service Index */
        3,
    },

    /* 10 - SystemId */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x001C,

        /* Characteristic Service Index */
        3,
    },

    /* 11 - RegCertDataList */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x001E,

        /* Characteristic Service Index */
        3,
    },

    /* 12 - PnPID */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0020,

        /* Characteristic Service Index */
        3,
    },

    /* 13 - TemperatureMeasurement */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0023,

        /* Characteristic Service Index */
        4,
    },

    /* 14 - IntermediateTemperature */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0026,

        /* Characteristic Service Index */
        4,
    },

    /* 15 - TemperatureType */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0029,

        /* Characteristic Service Index */
        4,
    },

    /* 16 - Measurement Interval */
    {
        {
            /* Number of attributes in characteristic including the definition */
            4,
        },

        /* Characteristic Start Handle */
        0x002B,

        /* Characteristic Service Index */
        4,
    },
};


DECL_CONST GATT_DB_SERVICE gatt_service[GATT_SERVICE_COUNT] =
{

    /* 0 - GAP */
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
        0x0005,

        /* Characteristic Start Index */
        0,

        /* Characteristic End Index */
        1
    },

    /* 1 - GATT */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0006,

        /* Service End Handle */
        0x0009,

        /* Characteristic Start Index */
        2,

        /* Characteristic End Index */
        2
    },

    /* 2 - Battery */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x000A,

        /* Service End Handle */
        0x000E,

        /* Characteristic Start Index */
        3,

        /* Characteristic End Index */
        3
    },

    /* 3 - DeviceInformation */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x000F,

        /* Service End Handle */
        0x0021,

        /* Characteristic Start Index */
        4,

        /* Characteristic End Index */
        12
    },

    /* 4 - Health Thermometer */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0022,

        /* Service End Handle */
        0x002E,

        /* Characteristic Start Index */
        13,

        /* Characteristic End Index */
        16
    },
};
#endif /* ((defined HTS) && (defined GATT_DB)) */

