
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
 *  0x0003  |       0x2A00       | Rd               | Mindtree CSC
 *  --------+--------------------+------------------+---------------------
 *  0x0004  |       0x2803       | Rd               | 0x02, 0x0005, 0x2A01
 *  --------+--------------------+------------------+---------------------
 *  0x0005  |       0x2A01       | Rd               | 0
 *  ========+====================+==================+=====================
 *  0x0006  |       0x2800       | Rd               | 0x1801
 *  --------+--------------------+------------------+---------------------
 *  0x0007  |       0x2803       | Rd               | 0x20, 0x0008, 0x2A05
 *  --------+--------------------+------------------+---------------------
 *  0x0008  |       0x2A05       | Ind              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0009  |       0x2902       | Rd, Wr           | 0x0000
 *  ========+====================+==================+=====================
 *  0x000A  |       0x2800       | Rd               | 0x180A
 *  --------+--------------------+------------------+---------------------
 *  0x000B  |       0x2803       | Rd               | 0x02, 0x000C, 0x2A29
 *  --------+--------------------+------------------+---------------------
 *  0x000C  |       0x2A29       | Rd               | Mindtree Limited.
 *  --------+--------------------+------------------+---------------------
 *  0x000D  |       0x2803       | Rd               | 0x02, 0x000E, 0x2A24
 *  --------+--------------------+------------------+---------------------
 *  0x000E  |       0x2A24       | Rd               | CSC-1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x000F  |       0x2803       | Rd               | 0x02, 0x0010, 0x2A25
 *  --------+--------------------+------------------+---------------------
 *  0x0010  |       0x2A25       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0011  |       0x2803       | Rd               | 0x02, 0x0012, 0x2A26
 *  --------+--------------------+------------------+---------------------
 *  0x0012  |       0x2A26       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0013  |       0x2803       | Rd               | 0x02, 0x0014, 0x2A27
 *  --------+--------------------+------------------+---------------------
 *  0x0014  |       0x2A27       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0015  |       0x2803       | Rd               | 0x02, 0x0016, 0x2A28
 *  --------+--------------------+------------------+---------------------
 *  0x0016  |       0x2A28       | Rd               | 1.0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0017  |       0x2803       | Rd               | 0x02, 0x0018, 0x2A23
 *  --------+--------------------+------------------+---------------------
 *  0x0018  |       0x2A23       | Rd               | 11223344.0
 *  --------+--------------------+------------------+---------------------
 *  0x0019  |       0x2803       | Rd               | 0x02, 0x001A, 0x2A2A
 *  --------+--------------------+------------------+---------------------
 *  0x001A  |       0x2A2A       | Rd               | 0
 *  --------+--------------------+------------------+---------------------
 *  0x001B  |       0x2803       | Rd               | 0x02, 0x001C, 0x2A50
 *  --------+--------------------+------------------+---------------------
 *  0x001C  |       0x2A50       | Rd               | 0
 *  ========+====================+==================+=====================
 *  0x001D  |       0x2800       | Rd               | 0x180F
 *  --------+--------------------+------------------+---------------------
 *  0x001E  |       0x2803       | Rd               | 0x12, 0x001F, 0x2A19
 *  --------+--------------------+------------------+---------------------
 *  0x001F  |       0x2A19       | Rd, Ntf          | 100.0
 *  --------+--------------------+------------------+---------------------
 *  0x0020  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0021  |       0x2904       | Rd               | VALUE
 *  ========+====================+==================+=====================
 *  0x0022  |       0x2800       | Rd               | 0x1816
 *  --------+--------------------+------------------+---------------------
 *  0x0023  |       0x2803       | Rd               | 0x10, 0x0024, 0x2A5B
 *  --------+--------------------+------------------+---------------------
 *  0x0024  |       0x2A5B       | Ntf              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0025  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0026  |       0x2803       | Rd               | 0x02, 0x0027, 0x2A5C
 *  --------+--------------------+------------------+---------------------
 *  0x0027  |       0x2A5C       | Rd               | 0
 *  --------+--------------------+------------------+---------------------
 *  0x0028  |       0x2803       | Rd               | 0x02, 0x0029, 0x2A5D
 *  --------+--------------------+------------------+---------------------
 *  0x0029  |       0x2A5D       | Rd               | 0
 *  --------+--------------------+------------------+---------------------
 *  0x002A  |       0x2803       | Rd               | 0x28, 0x002B, 0x2A55
 *  --------+--------------------+------------------+---------------------
 *  0x002B  |       0x2A55       | Wr, Ind          | 0
 *  --------+--------------------+------------------+---------------------
 *  0x002C  |       0x2902       | Rd, Wr           | 0x0000
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "gatt_defines.h"

#if ((defined CSCS) && (defined GATT_DB))

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

    /* 30 - DeviceInformation Service UUID */
    0x0A, 0x18,

    /* 32 - ManufacturerName Characteristic UUID */
    0x29, 0x2A,

    /* 34 - ModelNumber Characteristic UUID */
    0x24, 0x2A,

    /* 36 - SerialNumber Characteristic UUID */
    0x25, 0x2A,

    /* 38 - FirmwareRevision Characteristic UUID */
    0x26, 0x2A,

    /* 40 - HardwareRevision Characteristic UUID */
    0x27, 0x2A,

    /* 42 - SoftwareRevision Characteristic UUID */
    0x28, 0x2A,

    /* 44 - SystemId Characteristic UUID */
    0x23, 0x2A,

    /* 46 - RegCertDataList Characteristic UUID */
    0x2A, 0x2A,

    /* 48 - PnPID Characteristic UUID */
    0x50, 0x2A,

    /* 50 - Battery Service UUID */
    0x0F, 0x18,

    /* 52 - BatteryLevel Characteristic UUID */
    0x19, 0x2A,

    /* 54 - CSC Service UUID */
    0x16, 0x18,

    /* 56 - CSC Measurement Characteristic UUID */
    0x5B, 0x2A,

    /* 58 - CSC Feature Characteristic UUID */
    0x5C, 0x2A,

    /* 60 - Sensor Location Characteristic UUID */
    0x5D, 0x2A,

    /* 62 - SC Control Point Characteristic UUID */
    0x55, 0x2A,

};


UCHAR gatt_value_arr[GATT_VALUE_ARRAY_SIZE] =
{
    /* 0 - BatteryLevel(12); Default: 100.0 */
    0x64,

    /* 1 - CSC Feature(14); Default: 0 */
    0x07, 0x00,

    /* 3 - Sensor Location(15); Default: 0 */
    0x04,
};


DECL_CONST UCHAR gatt_const_value_arr[GATT_CONST_VALUE_ARRAY_SIZE] =
{

    /* 0 - DeviceName(0); Property: 0x02, Value Handle: 0x0003, UUID: 0x2A00 */
    0x02, 0x03, 0x00, 0x00, 0x2A,

    /* 5 - DeviceName(0); Default: Mindtree CSC */
    0x4D, 0x69, 0x6E, 0x64, 0x74, 0x72, 0x65, 0x65, 0x20, 0x43, 0x53, 0x43,

    /* 17 - Appearance(1); Property: 0x02, Value Handle: 0x0005, UUID: 0x2A01 */
    0x02, 0x05, 0x00, 0x01, 0x2A,

    /* 22 - Appearance(1); Default: 0 */
    0x00, 0x00,

    /* 24 - Service Changed(2); Property: 0x20, Value Handle: 0x0008, UUID: 0x2A05 */
    0x20, 0x08, 0x00, 0x05, 0x2A,

    /* 29 - ManufacturerName(3); Property: 0x02, Value Handle: 0x000C, UUID: 0x2A29 */
    0x02, 0x0C, 0x00, 0x29, 0x2A,

    /* 34 - ManufacturerName(3); Default: Mindtree Limited. */
    0x4D, 0x69, 0x6E, 0x64, 0x74, 0x72, 0x65, 0x65, 0x20, 0x4C, 0x69, 0x6D,
    0x69, 0x74, 0x65, 0x64, 0x2E,

    /* 51 - ModelNumber(4); Property: 0x02, Value Handle: 0x000E, UUID: 0x2A24 */
    0x02, 0x0E, 0x00, 0x24, 0x2A,

    /* 56 - ModelNumber(4); Default: CSC-1.0.0 */
    0x43, 0x53, 0x43, 0x2D, 0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 65 - SerialNumber(5); Property: 0x02, Value Handle: 0x0010, UUID: 0x2A25 */
    0x02, 0x10, 0x00, 0x25, 0x2A,

    /* 70 - SerialNumber(5); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 75 - FirmwareRevision(6); Property: 0x02, Value Handle: 0x0012, UUID: 0x2A26 */
    0x02, 0x12, 0x00, 0x26, 0x2A,

    /* 80 - FirmwareRevision(6); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 85 - HardwareRevision(7); Property: 0x02, Value Handle: 0x0014, UUID: 0x2A27 */
    0x02, 0x14, 0x00, 0x27, 0x2A,

    /* 90 - HardwareRevision(7); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 95 - SoftwareRevision(8); Property: 0x02, Value Handle: 0x0016, UUID: 0x2A28 */
    0x02, 0x16, 0x00, 0x28, 0x2A,

    /* 100 - SoftwareRevision(8); Default: 1.0.0 */
    0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 105 - SystemId(9); Property: 0x02, Value Handle: 0x0018, UUID: 0x2A23 */
    0x02, 0x18, 0x00, 0x23, 0x2A,

    /* 110 - SystemId(9); Default: 11223344.0 */
    0x31, 0x31, 0x32, 0x32, 0x33, 0x33, 0x34, 0x34,

    /* 118 - RegCertDataList(10); Property: 0x02, Value Handle: 0x001A, UUID: 0x2A2A */
    0x02, 0x1A, 0x00, 0x2A, 0x2A,

    /* 123 - RegCertDataList(10); Default: 0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* 155 - PnPID(11); Property: 0x02, Value Handle: 0x001C, UUID: 0x2A50 */
    0x02, 0x1C, 0x00, 0x50, 0x2A,

    /* 160 - PnPID(11); Default: 0 */
    0x01, 0x6A, 0x00, 0x4D, 0x01, 0x00, 0x01,

    /* 167 - BatteryLevel(12); Property: 0x12, Value Handle: 0x001F, UUID: 0x2A19 */
    0x12, 0x1F, 0x00, 0x19, 0x2A,

    /* 172 - Presentation Fromat for BatteryLevel */
    /**
     * Characteristic Presentation Format for BatteryLevel
     * - Format            (1 octet)
     * - Exponent          (1 octet)
     * - Unit              (2 octet)
     * - Name Space        (1 octet)
     * - Description       (2 octet)
     */
    0x04, 0x00, 0xAD, 0x27, 0x01, 0x00, 0x00,

    /* 179 - CSC Measurement(13); Property: 0x10, Value Handle: 0x0024, UUID: 0x2A5B */
    0x10, 0x24, 0x00, 0x5B, 0x2A,

    /* 184 - CSC Feature(14); Property: 0x02, Value Handle: 0x0027, UUID: 0x2A5C */
    0x02, 0x27, 0x00, 0x5C, 0x2A,

    /* 189 - Sensor Location(15); Property: 0x02, Value Handle: 0x0029, UUID: 0x2A5D */
    0x02, 0x29, 0x00, 0x5D, 0x2A,

    /* 194 - SC Control Point(16); Property: 0x28, Value Handle: 0x002B, UUID: 0x2A55 */
    0x28, 0x2B, 0x00, 0x55, 0x2A,
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

    /* 4 - Client Configuration for CSC Measurement */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 6 - Client Configuration for SC Control Point */
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
        42,

    },
    {
        /* UUID Offset */
        12,

        /* First Occurence for Type */
        9,

        /* Last Occurence for Type */
        44,

    },
    {
        /* UUID Offset */
        16,

        /* First Occurence for Type */
        33,

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
        29,

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
        41,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        62,

        /* First Occurence for Type */
        43,

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
        12,

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
        (UCHAR *)(gatt_const_value_arr + 17),
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
        (UCHAR *)(gatt_const_value_arr + 22),
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
        (UCHAR *)(gatt_const_value_arr + 24),
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
        32,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 0,
    },

    /* Handle - 0x000A */
    /* 2 - DeviceInformation - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        29,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 30),
    },

    /* Handle - 0x000B */
    /* 3 - ManufacturerName - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 29),
    },

    /* Handle - 0x000C */
    /* 3 - ManufacturerName - Characteristic Value */
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
        32,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 34),
    },

    /* Handle - 0x000D */
    /* 4 - ModelNumber - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 51),
    },

    /* Handle - 0x000E */
    /* 4 - ModelNumber - Characteristic Value */
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
        34,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 56),
    },

    /* Handle - 0x000F */
    /* 5 - SerialNumber - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 65),
    },

    /* Handle - 0x0010 */
    /* 5 - SerialNumber - Characteristic Value */
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
        (UCHAR *)(gatt_const_value_arr + 70),
    },

    /* Handle - 0x0011 */
    /* 6 - FirmwareRevision - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 75),
    },

    /* Handle - 0x0012 */
    /* 6 - FirmwareRevision - Characteristic Value */
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
        (UCHAR *)(gatt_const_value_arr + 80),
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
        (UCHAR *)(gatt_const_value_arr + 85),
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
        (UCHAR *)(gatt_const_value_arr + 90),
    },

    /* Handle - 0x0015 */
    /* 8 - SoftwareRevision - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 95),
    },

    /* Handle - 0x0016 */
    /* 8 - SoftwareRevision - Characteristic Value */
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
        (UCHAR *)(gatt_const_value_arr + 100),
    },

    /* Handle - 0x0017 */
    /* 9 - SystemId - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 105),
    },

    /* Handle - 0x0018 */
    /* 9 - SystemId - Characteristic Value */
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
        44,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 110),
    },

    /* Handle - 0x0019 */
    /* 10 - RegCertDataList - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 118),
    },

    /* Handle - 0x001A */
    /* 10 - RegCertDataList - Characteristic Value */
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
        46,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 123),
    },

    /* Handle - 0x001B */
    /* 11 - PnPID - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 155),
    },

    /* Handle - 0x001C */
    /* 11 - PnPID - Characteristic Value */
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
        48,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 160),
    },

    /* Handle - 0x001D */
    /* 3 - Battery - Service Declaration */
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
        (UCHAR *)(gatt_const_uuid_arr + 50),
    },

    /* Handle - 0x001E */
    /* 12 - BatteryLevel - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 167),
    },

    /* Handle - 0x001F */
    /* 12 - BatteryLevel - Characteristic Value */
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
        52,

        /* Value */
        gatt_value_arr + 0,
    },

    /* Handle - 0x0020 */
    /* 12 - BatteryLevel - CCD */
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

    /* Handle - 0x0021 */
    /* Characteristic 12 - BatteryLevel - Presentation Format */
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
        (UCHAR *)(gatt_const_value_arr + 172),
    },

    /* Handle - 0x0022 */
    /* 4 - CSC - Service Declaration */
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
    /* 13 - CSC Measurement - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 179),
    },

    /* Handle - 0x0024 */
    /* 13 - CSC Measurement - Characteristic Value */
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
        56,

        /* Value */
        NULL,
    },

    /* Handle - 0x0025 */
    /* 13 - CSC Measurement - CCD */
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
        gatt_db_peer_specific_val_arr + 4,
    },

    /* Handle - 0x0026 */
    /* 14 - CSC Feature - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        40,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 184),
    },

    /* Handle - 0x0027 */
    /* 14 - CSC Feature - Characteristic Value */
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
        58,

        /* Value */
        gatt_value_arr + 1,
    },

    /* Handle - 0x0028 */
    /* 15 - Sensor Location - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 189),
    },

    /* Handle - 0x0029 */
    /* 15 - Sensor Location - Characteristic Value */
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
        gatt_value_arr + 3,
    },

    /* Handle - 0x002A */
    /* 16 - SC Control Point - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 194),
    },

    /* Handle - 0x002B */
    /* 16 - SC Control Point - Characteristic Value */
    {
        /* Property */
        GATT_DB_WRITE |
        GATT_DB_CHAR_INDICATE_PROPERTY,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length */
        0,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        62,

        /* Value */
        NULL,
    },

    /* Handle - 0x002C */
    /* 16 - SC Control Point - CCD */
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
        gatt_db_peer_specific_val_arr + 6,
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

    /* 3 - ManufacturerName */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x000B,

        /* Characteristic Service Index */
        2,
    },

    /* 4 - ModelNumber */
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

    /* 5 - SerialNumber */
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

    /* 6 - FirmwareRevision */
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

    /* 8 - SoftwareRevision */
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

    /* 9 - SystemId */
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

    /* 10 - RegCertDataList */
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

    /* 11 - PnPID */
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

    /* 12 - BatteryLevel */
    {
        {
            /* Number of attributes in characteristic including the definition */
            4,
        },

        /* Characteristic Start Handle */
        0x001E,

        /* Characteristic Service Index */
        3,
    },

    /* 13 - CSC Measurement */
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

    /* 14 - CSC Feature */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0026,

        /* Characteristic Service Index */
        4,
    },

    /* 15 - Sensor Location */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0028,

        /* Characteristic Service Index */
        4,
    },

    /* 16 - SC Control Point */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x002A,

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
        0x000A,

        /* Service End Handle */
        0x001C,

        /* Characteristic Start Index */
        3,

        /* Characteristic End Index */
        11
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
        0x001D,

        /* Service End Handle */
        0x0021,

        /* Characteristic Start Index */
        12,

        /* Characteristic End Index */
        12
    },

    /* 4 - CSC */
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
        0x002C,

        /* Characteristic Start Index */
        13,

        /* Characteristic End Index */
        16
    },
};
#endif /* ((defined CSCS) && (defined GATT_DB)) */

