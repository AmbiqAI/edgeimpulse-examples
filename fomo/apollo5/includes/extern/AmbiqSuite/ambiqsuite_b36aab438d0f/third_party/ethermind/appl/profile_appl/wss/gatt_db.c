
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
 *  0x0003  |       0x2A00       | Rd               | MindtreeWS
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
 *  0x000C  |       0x2A29       | Rd               | Mindtree Limited
 *  --------+--------------------+------------------+---------------------
 *  0x000D  |       0x2803       | Rd               | 0x02, 0x000E, 0x2A24
 *  --------+--------------------+------------------+---------------------
 *  0x000E  |       0x2A24       | Rd               | WS-1.0.0
 *  ========+====================+==================+=====================
 *  0x000F  |       0x2800       | Rd               | 0x180F
 *  --------+--------------------+------------------+---------------------
 *  0x0010  |       0x2803       | Rd               | 0x12, 0x0011, 0x2A19
 *  --------+--------------------+------------------+---------------------
 *  0x0011  |       0x2A19       | Rd, Ntf          | 100.0
 *  --------+--------------------+------------------+---------------------
 *  0x0012  |       0x2902       | Rd, Wr           | 0x0000
 *  ========+====================+==================+=====================
 *  0x0013  |       0x2800       | Rd               | 0x181D
 *  --------+--------------------+------------------+---------------------
 *  0x0014  |       0x2802       | Rd               | 0x0000, 0x0000, 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0015  |       0x2803       | Rd               | 0x20, 0x0016, 0x2A9D
 *  --------+--------------------+------------------+---------------------
 *  0x0016  |       0x2A9D       | Ind              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x0017  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x0018  |       0x2803       | Rd               | 0x02, 0x0019, 0x2A9E
 *  --------+--------------------+------------------+---------------------
 *  0x0019  |       0x2A9E       | Rd               | 0
 *  ========+====================+==================+=====================
 *  0x001A  |       0x2801       | Rd               | 0x181B
 *  --------+--------------------+------------------+---------------------
 *  0x001B  |       0x2803       | Rd               | 0x20, 0x001C, 0x2A9C
 *  --------+--------------------+------------------+---------------------
 *  0x001C  |       0x2A9C       | Ind              | N/A
 *  --------+--------------------+------------------+---------------------
 *  0x001D  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x001E  |       0x2803       | Rd               | 0x02, 0x001F, 0x2A9B
 *  --------+--------------------+------------------+---------------------
 *  0x001F  |       0x2A9B       | Rd               | 0
 *  ========+====================+==================+=====================
 *  0x0020  |       0x2800       | Rd               | 0x181C
 *  --------+--------------------+------------------+---------------------
 *  0x0021  |       0x2803       | Rd               | 0x0A, 0x0022, 0x2A8A
 *  --------+--------------------+------------------+---------------------
 *  0x0022  |       0x2A8A       | Rd, Wr           | AAAAAA
 *  --------+--------------------+------------------+---------------------
 *  0x0023  |       0x2803       | Rd               | 0x0A, 0x0024, 0x2A90
 *  --------+--------------------+------------------+---------------------
 *  0x0024  |       0x2A90       | Rd, Wr           | BBBBBB
 *  --------+--------------------+------------------+---------------------
 *  0x0025  |       0x2803       | Rd               | 0x0A, 0x0026, 0x2A87
 *  --------+--------------------+------------------+---------------------
 *  0x0026  |       0x2A87       | Rd, Wr           | AAAAAA@Mindtree.com
 *  --------+--------------------+------------------+---------------------
 *  0x0027  |       0x2803       | Rd               | 0x0A, 0x0028, 0x2A80
 *  --------+--------------------+------------------+---------------------
 *  0x0028  |       0x2A80       | Rd, Wr           | 25.0
 *  --------+--------------------+------------------+---------------------
 *  0x0029  |       0x2803       | Rd               | 0x0A, 0x002A, 0x2A8C
 *  --------+--------------------+------------------+---------------------
 *  0x002A  |       0x2A8C       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x002B  |       0x2803       | Rd               | 0x0A, 0x002C, 0x2A85
 *  --------+--------------------+------------------+---------------------
 *  0x002C  |       0x2A85       | Rd, Wr           | 12/19/84 00:00:00
 *  --------+--------------------+------------------+---------------------
 *  0x002D  |       0x2803       | Rd               | 0x0A, 0x002E, 0x2A98
 *  --------+--------------------+------------------+---------------------
 *  0x002E  |       0x2A98       | Rd, Wr           | 55.0
 *  --------+--------------------+------------------+---------------------
 *  0x002F  |       0x2803       | Rd               | 0x0A, 0x0030, 0x2A8E
 *  --------+--------------------+------------------+---------------------
 *  0x0030  |       0x2A8E       | Rd, Wr           | 6.0
 *  --------+--------------------+------------------+---------------------
 *  0x0031  |       0x2803       | Rd               | 0x0A, 0x0032, 0x2A96
 *  --------+--------------------+------------------+---------------------
 *  0x0032  |       0x2A96       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0033  |       0x2803       | Rd               | 0x0A, 0x0034, 0x2A8D
 *  --------+--------------------+------------------+---------------------
 *  0x0034  |       0x2A8D       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x0035  |       0x2803       | Rd               | 0x0A, 0x0036, 0x2A92
 *  --------+--------------------+------------------+---------------------
 *  0x0036  |       0x2A92       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x0037  |       0x2803       | Rd               | 0x0A, 0x0038, 0x2A91
 *  --------+--------------------+------------------+---------------------
 *  0x0038  |       0x2A91       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x0039  |       0x2803       | Rd               | 0x0A, 0x003A, 0x2A7F
 *  --------+--------------------+------------------+---------------------
 *  0x003A  |       0x2A7F       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x003B  |       0x2803       | Rd               | 0x0A, 0x003C, 0x2A83
 *  --------+--------------------+------------------+---------------------
 *  0x003C  |       0x2A83       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x003D  |       0x2803       | Rd               | 0x0A, 0x003E, 0x2A93
 *  --------+--------------------+------------------+---------------------
 *  0x003E  |       0x2A93       | Rd, Wr           | 5.0
 *  --------+--------------------+------------------+---------------------
 *  0x003F  |       0x2803       | Rd               | 0x0A, 0x0040, 0x2A86
 *  --------+--------------------+------------------+---------------------
 *  0x0040  |       0x2A86       | Rd, Wr           | 12/19/14 00:00:00
 *  --------+--------------------+------------------+---------------------
 *  0x0041  |       0x2803       | Rd               | 0x0A, 0x0042, 0x2A97
 *  --------+--------------------+------------------+---------------------
 *  0x0042  |       0x2A97       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0043  |       0x2803       | Rd               | 0x0A, 0x0044, 0x2A8F
 *  --------+--------------------+------------------+---------------------
 *  0x0044  |       0x2A8F       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0045  |       0x2803       | Rd               | 0x0A, 0x0046, 0x2A88
 *  --------+--------------------+------------------+---------------------
 *  0x0046  |       0x2A88       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x0047  |       0x2803       | Rd               | 0x0A, 0x0048, 0x2A89
 *  --------+--------------------+------------------+---------------------
 *  0x0048  |       0x2A89       | Rd, Wr           | 10.0
 *  --------+--------------------+------------------+---------------------
 *  0x0049  |       0x2803       | Rd               | 0x0A, 0x004A, 0x2A7E
 *  --------+--------------------+------------------+---------------------
 *  0x004A  |       0x2A7E       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x004B  |       0x2803       | Rd               | 0x0A, 0x004C, 0x2A84
 *  --------+--------------------+------------------+---------------------
 *  0x004C  |       0x2A84       | Rd, Wr           | 10.0
 *  --------+--------------------+------------------+---------------------
 *  0x004D  |       0x2803       | Rd               | 0x0A, 0x004E, 0x2A81
 *  --------+--------------------+------------------+---------------------
 *  0x004E  |       0x2A81       | Rd, Wr           | 1.0
 *  --------+--------------------+------------------+---------------------
 *  0x004F  |       0x2803       | Rd               | 0x0A, 0x0050, 0x2A82
 *  --------+--------------------+------------------+---------------------
 *  0x0050  |       0x2A82       | Rd, Wr           | 10.0
 *  --------+--------------------+------------------+---------------------
 *  0x0051  |       0x2803       | Rd               | 0x0A, 0x0052, 0x2A8B
 *  --------+--------------------+------------------+---------------------
 *  0x0052  |       0x2A8B       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0053  |       0x2803       | Rd               | 0x0A, 0x0054, 0x2A94
 *  --------+--------------------+------------------+---------------------
 *  0x0054  |       0x2A94       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0055  |       0x2803       | Rd               | 0x0A, 0x0056, 0x2A95
 *  --------+--------------------+------------------+---------------------
 *  0x0056  |       0x2A95       | Rd, Wr           | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0057  |       0x2803       | Rd               | 0x1A, 0x0058, 0x2A99
 *  --------+--------------------+------------------+---------------------
 *  0x0058  |       0x2A99       | Rd, Wr, Ntf      | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0059  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x005A  |       0x2803       | Rd               | 0x02, 0x005B, 0x2A9A
 *  --------+--------------------+------------------+---------------------
 *  0x005B  |       0x2A9A       | Rd               | 255.0
 *  --------+--------------------+------------------+---------------------
 *  0x005C  |       0x2803       | Rd               | 0x28, 0x005D, 0x2A9F
 *  --------+--------------------+------------------+---------------------
 *  0x005D  |       0x2A9F       | Wr, Ind          | 0
 *  --------+--------------------+------------------+---------------------
 *  0x005E  |       0x2902       | Rd, Wr           | 0x0000
 *  --------+--------------------+------------------+---------------------
 *  0x005F  |       0x2803       | Rd               | 0x0A, 0x0060, 0x2AA2
 *  --------+--------------------+------------------+---------------------
 *  0x0060  |       0x2AA2       | Rd, Wr           | English
 *  ========+====================+==================+=====================
 *  0x0061  |       0x2800       | Rd               | 0x1805
 *  --------+--------------------+------------------+---------------------
 *  0x0062  |       0x2803       | Rd               | 0x1A, 0x0063, 0x2A2B
 *  --------+--------------------+------------------+---------------------
 *  0x0063  |       0x2A2B       | Rd, Wr, Ntf      | 0.0
 *  --------+--------------------+------------------+---------------------
 *  0x0064  |       0x2902       | Rd, Wr           | 0x0000
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "gatt_defines.h"

#if ((defined WSS) && (defined GATT_DB))

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

    /* 28 - ServiceChanged Characteristic UUID */
    0x05, 0x2A,

    /* 30 - DeviceInformation Service UUID */
    0x0A, 0x18,

    /* 32 - ManufacturerName Characteristic UUID */
    0x29, 0x2A,

    /* 34 - ModelNumber Characteristic UUID */
    0x24, 0x2A,

    /* 36 - Battery Service UUID */
    0x0F, 0x18,

    /* 38 - BatteryLevel Characteristic UUID */
    0x19, 0x2A,

    /* 40 - Weight Scale Service UUID */
    0x1D, 0x18,

    /* 42 - WeightMeasurment Characteristic UUID */
    0x9D, 0x2A,

    /* 44 - WeightScaleFeature Characteristic UUID */
    0x9E, 0x2A,

    /* 46 - BodyComposition Service UUID */
    0x1B, 0x18,

    /* 48 - BodyCompositionMeasurement Characteristic UUID */
    0x9C, 0x2A,

    /* 50 - BodyCompositionFeature Characteristic UUID */
    0x9B, 0x2A,

    /* 52 - UserData Service UUID */
    0x1C, 0x18,

    /* 54 - FirstName Characteristic UUID */
    0x8A, 0x2A,

    /* 56 - LastName Characteristic UUID */
    0x90, 0x2A,

    /* 58 - Email Characteristic UUID */
    0x87, 0x2A,

    /* 60 - Age Characteristic UUID */
    0x80, 0x2A,

    /* 62 - Gender Characteristic UUID */
    0x8C, 0x2A,

    /* 64 - DateofBirth Characteristic UUID */
    0x85, 0x2A,

    /* 66 - Weight Characteristic UUID */
    0x98, 0x2A,

    /* 68 - Height Characteristic UUID */
    0x8E, 0x2A,

    /* 70 - VO2Max Characteristic UUID */
    0x96, 0x2A,

    /* 72 - HeartrateMax Characteristic UUID */
    0x8D, 0x2A,

    /* 74 - RestingHeartRate Characteristic UUID */
    0x92, 0x2A,

    /* 76 - MaxRecommHeartRate Characteristic UUID */
    0x91, 0x2A,

    /* 78 - AerobicThreshold Characteristic UUID */
    0x7F, 0x2A,

    /* 80 - AnaerobicThreshold Characteristic UUID */
    0x83, 0x2A,

    /* 82 - SportTypeAerobicAnaerobicThresholds Characteristic UUID */
    0x93, 0x2A,

    /* 84 - DateofThresholdAssessment Characteristic UUID */
    0x86, 0x2A,

    /* 86 - WaistCircumference Characteristic UUID */
    0x97, 0x2A,

    /* 88 - HipCircumference Characteristic UUID */
    0x8F, 0x2A,

    /* 90 - FatBurnHeartRateLowerLimit Characteristic UUID */
    0x88, 0x2A,

    /* 92 - FatBurnHeartRateUpperLimit Characteristic UUID */
    0x89, 0x2A,

    /* 94 - AerobicHeartRateLowerLimit Characteristic UUID */
    0x7E, 0x2A,

    /* 96 - AerobicHeartRateUpperrLimit Characteristic UUID */
    0x84, 0x2A,

    /* 98 - AnaerobicHeartRateLowerLimit Characteristic UUID */
    0x81, 0x2A,

    /* 100 - AnaerobicHeartRateUpperLimit Characteristic UUID */
    0x82, 0x2A,

    /* 102 - FiveZoneHeartRateLimits Characteristic UUID */
    0x8B, 0x2A,

    /* 104 - ThreeZoneHeartRat Limits Characteristic UUID */
    0x94, 0x2A,

    /* 106 - TwoZoneHeartRateLimit Characteristic UUID */
    0x95, 0x2A,

    /* 108 - DatabaseChangeIncrement Characteristic UUID */
    0x99, 0x2A,

    /* 110 - UserIndex Characteristic UUID */
    0x9A, 0x2A,

    /* 112 - UserControlPoint Characteristic UUID */
    0x9F, 0x2A,

    /* 114 - Language Characteristic UUID */
    0xA2, 0x2A,

    /* 116 - CurrentTimeService Service UUID */
    0x05, 0x18,

    /* 118 - CurrentTime Characteristic UUID */
    0x2B, 0x2A,

};


UCHAR gatt_value_arr[GATT_VALUE_ARRAY_SIZE] =
{
    /* 0 - BatteryLevel(5); Default: 100.0 */
    0x64,

    /* 1 - WeightScaleFeature(7); Default: 0 */
    0xB7, 0x01, 0x00, 0x00,

    /* 5 - BodyCompositionFeature(9); Default: 0 */
    0xFF, 0x27, 0x01, 0x00,

    /* 9 - FirstName(10); Default: AAAAAA */
    0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,

    /* 75 - LastName(11); Default: BBBBBB */
    0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,

    /* 141 - Email(12); Default: AAAAAA@Mindtree.com */
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* 209 - Age(13); Default: 25.0 */
    0x19,

    /* 210 - Gender(14); Default: 0.0 */
    0x00,

    /* 211 - DateofBirth(15); Default: 12/19/84 00:00:00 */
    0x19, 0x84, 0x12, 0x19,

    /* 215 - Weight(16); Default: 55.0 */
    0x37, 0x00,

    /* 217 - Height(17); Default: 6.0 */
    0x06, 0x00,

    /* 219 - VO2Max(18); Default: 0.0 */
    0x00,

    /* 220 - HeartrateMax(19); Default: 1.0 */
    0x01,

    /* 221 - RestingHeartRate(20); Default: 1.0 */
    0x01,

    /* 222 - MaxRecommHeartRate(21); Default: 1.0 */
    0x01,

    /* 223 - AerobicThreshold(22); Default: 1.0 */
    0x01,

    /* 224 - AnaerobicThreshold(23); Default: 1.0 */
    0x01,

    /* 225 - SportTypeAerobicAnaerobicThresholds(24); Default: 5.0 */
    0x01,

    /* 226 - DateofThresholdAssessment(25); Default: 12/19/14 00:00:00 */
    0x20, 0x14, 0x19, 0x12,

    /* 230 - WaistCircumference(26); Default: 0.0 */
    0x00, 0x00,

    /* 232 - HipCircumference(27); Default: 0.0 */
    0x00, 0x00,

    /* 234 - FatBurnHeartRateLowerLimit(28); Default: 1.0 */
    0x01,

    /* 235 - FatBurnHeartRateUpperLimit(29); Default: 10.0 */
    0x10,

    /* 236 - AerobicHeartRateLowerLimit(30); Default: 1.0 */
    0x01,

    /* 237 - AerobicHeartRateUpperrLimit(31); Default: 10.0 */
    0x10,

    /* 238 - AnaerobicHeartRateLowerLimit(32); Default: 1.0 */
    0x00,

    /* 239 - AnaerobicHeartRateUpperLimit(33); Default: 10.0 */
    0x10,

    /* 240 - FiveZoneHeartRateLimits(34); Default: 0.0 */
    0x00, 0x00, 0x00, 0x00,

    /* 244 - ThreeZoneHeartRat Limits(35); Default: 0.0 */
    0x00, 0x00,

    /* 246 - TwoZoneHeartRateLimit(36); Default: 0.0 */
    0x00,

    /* 247 - DatabaseChangeIncrement(37); Default: 0.0 */
    0x00, 0x00, 0x00, 0x00,

    /* 251 - UserIndex(38); Default: 255.0 */
    0xFF,

    /* 252 - Language(40); Default: English */
    0x45, 0x6E, 0x67, 0x6C, 0x69, 0x73, 0x68,

    /* 259 - CurrentTime(41); Default: 0.0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


DECL_CONST UCHAR gatt_const_value_arr[GATT_CONST_VALUE_ARRAY_SIZE] =
{

    /* 0 - DeviceName(0); Property: 0x02, Value Handle: 0x0003, UUID: 0x2A00 */
    0x02, 0x03, 0x00, 0x00, 0x2A,

    /* 5 - DeviceName(0); Default: MindtreeWS */
    0x4D, 0x69, 0x6E, 0x64, 0x74, 0x72, 0x65, 0x65, 0x57, 0x53,

    /* 15 - Appearance(1); Property: 0x02, Value Handle: 0x0005, UUID: 0x2A01 */
    0x02, 0x05, 0x00, 0x01, 0x2A,

    /* 20 - Appearance(1); Default: 0 */
    0x00, 0x00,

    /* 22 - ServiceChanged(2); Property: 0x20, Value Handle: 0x0008, UUID: 0x2A05 */
    0x20, 0x08, 0x00, 0x05, 0x2A,

    /* 27 - ManufacturerName(3); Property: 0x02, Value Handle: 0x000C, UUID: 0x2A29 */
    0x02, 0x0C, 0x00, 0x29, 0x2A,

    /* 32 - ManufacturerName(3); Default: Mindtree Limited */
    0x4D, 0x69, 0x6E, 0x64, 0x74, 0x72, 0x65, 0x65, 0x20, 0x4C, 0x69, 0x6D,
    0x69, 0x74, 0x65, 0x64,

    /* 48 - ModelNumber(4); Property: 0x02, Value Handle: 0x000E, UUID: 0x2A24 */
    0x02, 0x0E, 0x00, 0x24, 0x2A,

    /* 53 - ModelNumber(4); Default: WS-1.0.0 */
    0x57, 0x53, 0x2D, 0x31, 0x2E, 0x30, 0x2E, 0x30,

    /* 61 - BatteryLevel(5); Property: 0x12, Value Handle: 0x0011, UUID: 0x2A19 */
    0x12, 0x11, 0x00, 0x19, 0x2A,

    /* 66 - BodyComposition ; Start Handle: 0x001A, End Handle: 0x001F, UUID: 0x181B */
    0x1A, 0x00, 0x1F, 0x00, 0x1B, 0x18,

    /* 72 - WeightMeasurment(6); Property: 0x20, Value Handle: 0x0016, UUID: 0x2A9D */
    0x20, 0x16, 0x00, 0x9D, 0x2A,

    /* 77 - WeightScaleFeature(7); Property: 0x02, Value Handle: 0x0019, UUID: 0x2A9E */
    0x02, 0x19, 0x00, 0x9E, 0x2A,

    /* 82 - BodyCompositionMeasurement(8); Property: 0x20, Value Handle: 0x001C, UUID: 0x2A9C */
    0x20, 0x1C, 0x00, 0x9C, 0x2A,

    /* 87 - BodyCompositionFeature(9); Property: 0x02, Value Handle: 0x001F, UUID: 0x2A9B */
    0x02, 0x1F, 0x00, 0x9B, 0x2A,

    /* 92 - FirstName(10); Property: 0x0A, Value Handle: 0x0022, UUID: 0x2A8A */
    0x0A, 0x22, 0x00, 0x8A, 0x2A,

    /* 97 - LastName(11); Property: 0x0A, Value Handle: 0x0024, UUID: 0x2A90 */
    0x0A, 0x24, 0x00, 0x90, 0x2A,

    /* 102 - Email(12); Property: 0x0A, Value Handle: 0x0026, UUID: 0x2A87 */
    0x0A, 0x26, 0x00, 0x87, 0x2A,

    /* 107 - Age(13); Property: 0x0A, Value Handle: 0x0028, UUID: 0x2A80 */
    0x0A, 0x28, 0x00, 0x80, 0x2A,

    /* 112 - Gender(14); Property: 0x0A, Value Handle: 0x002A, UUID: 0x2A8C */
    0x0A, 0x2A, 0x00, 0x8C, 0x2A,

    /* 117 - DateofBirth(15); Property: 0x0A, Value Handle: 0x002C, UUID: 0x2A85 */
    0x0A, 0x2C, 0x00, 0x85, 0x2A,

    /* 122 - Weight(16); Property: 0x0A, Value Handle: 0x002E, UUID: 0x2A98 */
    0x0A, 0x2E, 0x00, 0x98, 0x2A,

    /* 127 - Height(17); Property: 0x0A, Value Handle: 0x0030, UUID: 0x2A8E */
    0x0A, 0x30, 0x00, 0x8E, 0x2A,

    /* 132 - VO2Max(18); Property: 0x0A, Value Handle: 0x0032, UUID: 0x2A96 */
    0x0A, 0x32, 0x00, 0x96, 0x2A,

    /* 137 - HeartrateMax(19); Property: 0x0A, Value Handle: 0x0034, UUID: 0x2A8D */
    0x0A, 0x34, 0x00, 0x8D, 0x2A,

    /* 142 - RestingHeartRate(20); Property: 0x0A, Value Handle: 0x0036, UUID: 0x2A92 */
    0x0A, 0x36, 0x00, 0x92, 0x2A,

    /* 147 - MaxRecommHeartRate(21); Property: 0x0A, Value Handle: 0x0038, UUID: 0x2A91 */
    0x0A, 0x38, 0x00, 0x91, 0x2A,

    /* 152 - AerobicThreshold(22); Property: 0x0A, Value Handle: 0x003A, UUID: 0x2A7F */
    0x0A, 0x3A, 0x00, 0x7F, 0x2A,

    /* 157 - AnaerobicThreshold(23); Property: 0x0A, Value Handle: 0x003C, UUID: 0x2A83 */
    0x0A, 0x3C, 0x00, 0x83, 0x2A,

    /* 162 - SportTypeAerobicAnaerobicThresholds(24); Property: 0x0A, Value Handle: 0x003E, UUID: 0x2A93 */
    0x0A, 0x3E, 0x00, 0x93, 0x2A,

    /* 167 - DateofThresholdAssessment(25); Property: 0x0A, Value Handle: 0x0040, UUID: 0x2A86 */
    0x0A, 0x40, 0x00, 0x86, 0x2A,

    /* 172 - WaistCircumference(26); Property: 0x0A, Value Handle: 0x0042, UUID: 0x2A97 */
    0x0A, 0x42, 0x00, 0x97, 0x2A,

    /* 177 - HipCircumference(27); Property: 0x0A, Value Handle: 0x0044, UUID: 0x2A8F */
    0x0A, 0x44, 0x00, 0x8F, 0x2A,

    /* 182 - FatBurnHeartRateLowerLimit(28); Property: 0x0A, Value Handle: 0x0046, UUID: 0x2A88 */
    0x0A, 0x46, 0x00, 0x88, 0x2A,

    /* 187 - FatBurnHeartRateUpperLimit(29); Property: 0x0A, Value Handle: 0x0048, UUID: 0x2A89 */
    0x0A, 0x48, 0x00, 0x89, 0x2A,

    /* 192 - AerobicHeartRateLowerLimit(30); Property: 0x0A, Value Handle: 0x004A, UUID: 0x2A7E */
    0x0A, 0x4A, 0x00, 0x7E, 0x2A,

    /* 197 - AerobicHeartRateUpperrLimit(31); Property: 0x0A, Value Handle: 0x004C, UUID: 0x2A84 */
    0x0A, 0x4C, 0x00, 0x84, 0x2A,

    /* 202 - AnaerobicHeartRateLowerLimit(32); Property: 0x0A, Value Handle: 0x004E, UUID: 0x2A81 */
    0x0A, 0x4E, 0x00, 0x81, 0x2A,

    /* 207 - AnaerobicHeartRateUpperLimit(33); Property: 0x0A, Value Handle: 0x0050, UUID: 0x2A82 */
    0x0A, 0x50, 0x00, 0x82, 0x2A,

    /* 212 - FiveZoneHeartRateLimits(34); Property: 0x0A, Value Handle: 0x0052, UUID: 0x2A8B */
    0x0A, 0x52, 0x00, 0x8B, 0x2A,

    /* 217 - ThreeZoneHeartRat Limits(35); Property: 0x0A, Value Handle: 0x0054, UUID: 0x2A94 */
    0x0A, 0x54, 0x00, 0x94, 0x2A,

    /* 222 - TwoZoneHeartRateLimit(36); Property: 0x0A, Value Handle: 0x0056, UUID: 0x2A95 */
    0x0A, 0x56, 0x00, 0x95, 0x2A,

    /* 227 - DatabaseChangeIncrement(37); Property: 0x1A, Value Handle: 0x0058, UUID: 0x2A99 */
    0x1A, 0x58, 0x00, 0x99, 0x2A,

    /* 232 - UserIndex(38); Property: 0x02, Value Handle: 0x005B, UUID: 0x2A9A */
    0x02, 0x5B, 0x00, 0x9A, 0x2A,

    /* 237 - UserControlPoint(39); Property: 0x28, Value Handle: 0x005D, UUID: 0x2A9F */
    0x28, 0x5D, 0x00, 0x9F, 0x2A,

    /* 242 - Language(40); Property: 0x0A, Value Handle: 0x0060, UUID: 0x2AA2 */
    0x0A, 0x60, 0x00, 0xA2, 0x2A,

    /* 247 - CurrentTime(41); Property: 0x1A, Value Handle: 0x0063, UUID: 0x2A2B */
    0x1A, 0x63, 0x00, 0x2B, 0x2A,
};


UCHAR gatt_db_peer_specific_val_arr[GATT_DB_MAX_PEER_CONFIGURATION];

DECL_CONST UCHAR gatt_db_const_peer_specific_val_arr[GATT_DB_PEER_VALUE_ARRAY_SIZE] =
{
    /* 0 - Client Configuration for ServiceChanged */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 2 - Client Configuration for BatteryLevel */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 4 - Client Configuration for WeightMeasurment */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 6 - Client Configuration for BodyCompositionMeasurement */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 8 - Client Configuration for DatabaseChangeIncrement */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 10 - Client Configuration for UserControlPoint */
    /* Client Instance 0 */
    0x00, 0x00,

    /* 12 - Client Configuration for CurrentTime */
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
        97,

    },
    {
        /* UUID Offset */
        2,

        /* First Occurence for Type */
        26,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        4,

        /* First Occurence for Type */
        20,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        6,

        /* First Occurence for Type */
        2,

        /* Last Occurence for Type */
        98,

    },
    {
        /* UUID Offset */
        12,

        /* First Occurence for Type */
        9,

        /* Last Occurence for Type */
        100,

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
        15,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        38,

        /* First Occurence for Type */
        17,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        40,

        /* First Occurence for Type */
        19,

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
        25,

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
        31,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        52,

        /* First Occurence for Type */
        32,

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
        42,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        64,

        /* First Occurence for Type */
        44,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        66,

        /* First Occurence for Type */
        46,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        68,

        /* First Occurence for Type */
        48,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        70,

        /* First Occurence for Type */
        50,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        72,

        /* First Occurence for Type */
        52,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        74,

        /* First Occurence for Type */
        54,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        76,

        /* First Occurence for Type */
        56,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        78,

        /* First Occurence for Type */
        58,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        80,

        /* First Occurence for Type */
        60,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        82,

        /* First Occurence for Type */
        62,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        84,

        /* First Occurence for Type */
        64,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        86,

        /* First Occurence for Type */
        66,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        88,

        /* First Occurence for Type */
        68,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        90,

        /* First Occurence for Type */
        70,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        92,

        /* First Occurence for Type */
        72,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        94,

        /* First Occurence for Type */
        74,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        96,

        /* First Occurence for Type */
        76,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        98,

        /* First Occurence for Type */
        78,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        100,

        /* First Occurence for Type */
        80,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        102,

        /* First Occurence for Type */
        82,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        104,

        /* First Occurence for Type */
        84,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        106,

        /* First Occurence for Type */
        86,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        108,

        /* First Occurence for Type */
        88,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        110,

        /* First Occurence for Type */
        91,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        112,

        /* First Occurence for Type */
        93,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        114,

        /* First Occurence for Type */
        96,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        116,

        /* First Occurence for Type */
        97,

        /* Last Occurence for Type */
        ATT_INVALID_ATTR_HANDLE_VAL,

    },
    {
        /* UUID Offset */
        118,

        /* First Occurence for Type */
        99,

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
        10,

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
        (UCHAR *)(gatt_const_value_arr + 15),
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
        (UCHAR *)(gatt_const_value_arr + 20),
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
    /* 2 - ServiceChanged - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 22),
    },

    /* Handle - 0x0008 */
    /* 2 - ServiceChanged - Characteristic Value */
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
    /* 2 - ServiceChanged - CCD */
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
        18,

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
        15,

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
        (UCHAR *)(gatt_const_value_arr + 27),
    },

    /* Handle - 0x000C */
    /* 3 - ManufacturerName - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        16,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        32,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 32),
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
        16,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 48),
    },

    /* Handle - 0x000E */
    /* 4 - ModelNumber - Characteristic Value */
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
        34,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 53),
    },

    /* Handle - 0x000F */
    /* 3 - Battery - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        19,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 36),
    },

    /* Handle - 0x0010 */
    /* 5 - BatteryLevel - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 61),
    },

    /* Handle - 0x0011 */
    /* 5 - BatteryLevel - Characteristic Value */
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
        38,

        /* Value */
        gatt_value_arr + 0,
    },

    /* Handle - 0x0012 */
    /* 5 - BatteryLevel - CCD */
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
        23,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 2,
    },

    /* Handle - 0x0013 */
    /* 4 - Weight Scale - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        32,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 40),
    },

    /* Handle - 0x0014 */
    /* 0 - BodyComposition - Include Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        6,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        4,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 66),
    },

    /* Handle - 0x0015 */
    /* 6 - WeightMeasurment - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 72),
    },

    /* Handle - 0x0016 */
    /* 6 - WeightMeasurment - Characteristic Value */
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
        42,

        /* Value */
        NULL,
    },

    /* Handle - 0x0017 */
    /* 6 - WeightMeasurment - CCD */
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
        29,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 4,
    },

    /* Handle - 0x0018 */
    /* 7 - WeightScaleFeature - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 77),
    },

    /* Handle - 0x0019 */
    /* 7 - WeightScaleFeature - Characteristic Value */
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
        44,

        /* Value */
        gatt_value_arr + 1,
    },

    /* Handle - 0x001A */
    /* 5 - BodyComposition - Secondary Service Declaration */
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
        2,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 46),
    },

    /* Handle - 0x001B */
    /* 8 - BodyCompositionMeasurement - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 82),
    },

    /* Handle - 0x001C */
    /* 8 - BodyCompositionMeasurement - Characteristic Value */
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
        48,

        /* Value */
        NULL,
    },

    /* Handle - 0x001D */
    /* 8 - BodyCompositionMeasurement - CCD */
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
        89,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 6,
    },

    /* Handle - 0x001E */
    /* 9 - BodyCompositionFeature - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        33,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 87),
    },

    /* Handle - 0x001F */
    /* 9 - BodyCompositionFeature - Characteristic Value */
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
        50,

        /* Value */
        gatt_value_arr + 5,
    },

    /* Handle - 0x0020 */
    /* 6 - UserData - Service Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        97,

        /* UUID Offset */
        0,

        /* Value */
        (UCHAR *)(gatt_const_uuid_arr + 52),
    },

    /* Handle - 0x0021 */
    /* 10 - FirstName - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 92),
    },

    /* Handle - 0x0022 */
    /* 10 - FirstName - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length + Length Header Byte */
        (65 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        54,

        /* Value */
        gatt_value_arr + 9,
    },

    /* Handle - 0x0023 */
    /* 11 - LastName - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 97),
    },

    /* Handle - 0x0024 */
    /* 11 - LastName - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length + Length Header Byte */
        (65 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        56,

        /* Value */
        gatt_value_arr + 75,
    },

    /* Handle - 0x0025 */
    /* 12 - Email - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 102),
    },

    /* Handle - 0x0026 */
    /* 12 - Email - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_NO_AUXILLARY_PROPERTY,

        /* Value Length + Length Header Byte */
        (67 + 1),

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        58,

        /* Value */
        gatt_value_arr + 141,
    },

    /* Handle - 0x0027 */
    /* 13 - Age - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 107),
    },

    /* Handle - 0x0028 */
    /* 13 - Age - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        60,

        /* Value */
        gatt_value_arr + 209,
    },

    /* Handle - 0x0029 */
    /* 14 - Gender - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 112),
    },

    /* Handle - 0x002A */
    /* 14 - Gender - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        62,

        /* Value */
        gatt_value_arr + 210,
    },

    /* Handle - 0x002B */
    /* 15 - DateofBirth - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        45,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 117),
    },

    /* Handle - 0x002C */
    /* 15 - DateofBirth - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        4,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        64,

        /* Value */
        gatt_value_arr + 211,
    },

    /* Handle - 0x002D */
    /* 16 - Weight - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        47,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 122),
    },

    /* Handle - 0x002E */
    /* 16 - Weight - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        66,

        /* Value */
        gatt_value_arr + 215,
    },

    /* Handle - 0x002F */
    /* 17 - Height - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 127),
    },

    /* Handle - 0x0030 */
    /* 17 - Height - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        68,

        /* Value */
        gatt_value_arr + 217,
    },

    /* Handle - 0x0031 */
    /* 18 - VO2Max - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        51,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 132),
    },

    /* Handle - 0x0032 */
    /* 18 - VO2Max - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        70,

        /* Value */
        gatt_value_arr + 219,
    },

    /* Handle - 0x0033 */
    /* 19 - HeartrateMax - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        53,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 137),
    },

    /* Handle - 0x0034 */
    /* 19 - HeartrateMax - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        72,

        /* Value */
        gatt_value_arr + 220,
    },

    /* Handle - 0x0035 */
    /* 20 - RestingHeartRate - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        55,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 142),
    },

    /* Handle - 0x0036 */
    /* 20 - RestingHeartRate - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        74,

        /* Value */
        gatt_value_arr + 221,
    },

    /* Handle - 0x0037 */
    /* 21 - MaxRecommHeartRate - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        57,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 147),
    },

    /* Handle - 0x0038 */
    /* 21 - MaxRecommHeartRate - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        76,

        /* Value */
        gatt_value_arr + 222,
    },

    /* Handle - 0x0039 */
    /* 22 - AerobicThreshold - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 152),
    },

    /* Handle - 0x003A */
    /* 22 - AerobicThreshold - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        78,

        /* Value */
        gatt_value_arr + 223,
    },

    /* Handle - 0x003B */
    /* 23 - AnaerobicThreshold - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        61,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 157),
    },

    /* Handle - 0x003C */
    /* 23 - AnaerobicThreshold - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        80,

        /* Value */
        gatt_value_arr + 224,
    },

    /* Handle - 0x003D */
    /* 24 - SportTypeAerobicAnaerobicThresholds - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        63,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 162),
    },

    /* Handle - 0x003E */
    /* 24 - SportTypeAerobicAnaerobicThresholds - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        82,

        /* Value */
        gatt_value_arr + 225,
    },

    /* Handle - 0x003F */
    /* 25 - DateofThresholdAssessment - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 167),
    },

    /* Handle - 0x0040 */
    /* 25 - DateofThresholdAssessment - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        4,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        84,

        /* Value */
        gatt_value_arr + 226,
    },

    /* Handle - 0x0041 */
    /* 26 - WaistCircumference - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 172),
    },

    /* Handle - 0x0042 */
    /* 26 - WaistCircumference - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        86,

        /* Value */
        gatt_value_arr + 230,
    },

    /* Handle - 0x0043 */
    /* 27 - HipCircumference - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        69,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 177),
    },

    /* Handle - 0x0044 */
    /* 27 - HipCircumference - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        88,

        /* Value */
        gatt_value_arr + 232,
    },

    /* Handle - 0x0045 */
    /* 28 - FatBurnHeartRateLowerLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        71,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 182),
    },

    /* Handle - 0x0046 */
    /* 28 - FatBurnHeartRateLowerLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        90,

        /* Value */
        gatt_value_arr + 234,
    },

    /* Handle - 0x0047 */
    /* 29 - FatBurnHeartRateUpperLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        73,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 187),
    },

    /* Handle - 0x0048 */
    /* 29 - FatBurnHeartRateUpperLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        92,

        /* Value */
        gatt_value_arr + 235,
    },

    /* Handle - 0x0049 */
    /* 30 - AerobicHeartRateLowerLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        75,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 192),
    },

    /* Handle - 0x004A */
    /* 30 - AerobicHeartRateLowerLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        94,

        /* Value */
        gatt_value_arr + 236,
    },

    /* Handle - 0x004B */
    /* 31 - AerobicHeartRateUpperrLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        77,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 197),
    },

    /* Handle - 0x004C */
    /* 31 - AerobicHeartRateUpperrLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        96,

        /* Value */
        gatt_value_arr + 237,
    },

    /* Handle - 0x004D */
    /* 32 - AnaerobicHeartRateLowerLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        79,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 202),
    },

    /* Handle - 0x004E */
    /* 32 - AnaerobicHeartRateLowerLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        98,

        /* Value */
        gatt_value_arr + 238,
    },

    /* Handle - 0x004F */
    /* 33 - AnaerobicHeartRateUpperLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        81,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 207),
    },

    /* Handle - 0x0050 */
    /* 33 - AnaerobicHeartRateUpperLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        100,

        /* Value */
        gatt_value_arr + 239,
    },

    /* Handle - 0x0051 */
    /* 34 - FiveZoneHeartRateLimits - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        83,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 212),
    },

    /* Handle - 0x0052 */
    /* 34 - FiveZoneHeartRateLimits - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        4,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        102,

        /* Value */
        gatt_value_arr + 240,
    },

    /* Handle - 0x0053 */
    /* 35 - ThreeZoneHeartRat Limits - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        85,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 217),
    },

    /* Handle - 0x0054 */
    /* 35 - ThreeZoneHeartRat Limits - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        2,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        104,

        /* Value */
        gatt_value_arr + 244,
    },

    /* Handle - 0x0055 */
    /* 36 - TwoZoneHeartRateLimit - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        87,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 222),
    },

    /* Handle - 0x0056 */
    /* 36 - TwoZoneHeartRateLimit - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        1,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        106,

        /* Value */
        gatt_value_arr + 246,
    },

    /* Handle - 0x0057 */
    /* 37 - DatabaseChangeIncrement - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        90,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 227),
    },

    /* Handle - 0x0058 */
    /* 37 - DatabaseChangeIncrement - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        4,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        108,

        /* Value */
        gatt_value_arr + 247,
    },

    /* Handle - 0x0059 */
    /* 37 - DatabaseChangeIncrement - CCD */
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
        94,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 8,
    },

    /* Handle - 0x005A */
    /* 38 - UserIndex - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        92,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 232),
    },

    /* Handle - 0x005B */
    /* 38 - UserIndex - Characteristic Value */
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
        110,

        /* Value */
        gatt_value_arr + 251,
    },

    /* Handle - 0x005C */
    /* 39 - UserControlPoint - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        95,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 237),
    },

    /* Handle - 0x005D */
    /* 39 - UserControlPoint - Characteristic Value */
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
        112,

        /* Value */
        NULL,
    },

    /* Handle - 0x005E */
    /* 39 - UserControlPoint - CCD */
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
        100,

        /* UUID Offset */
        12,

        /* Value */
        gatt_db_peer_specific_val_arr + 10,
    },

    /* Handle - 0x005F */
    /* 40 - Language - Characteristic Declaration */
    {
        /* Property */
        GATT_DB_READ,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        5,

        /* Next Attribute Type Index */
        98,

        /* UUID Offset */
        6,

        /* Value */
        (UCHAR *)(gatt_const_value_arr + 242),
    },

    /* Handle - 0x0060 */
    /* 40 - Language - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        7,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        114,

        /* Value */
        gatt_value_arr + 252,
    },

    /* Handle - 0x0061 */
    /* 7 - CurrentTimeService - Service Declaration */
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
        (UCHAR *)(gatt_const_uuid_arr + 116),
    },

    /* Handle - 0x0062 */
    /* 41 - CurrentTime - Characteristic Declaration */
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
        (UCHAR *)(gatt_const_value_arr + 247),
    },

    /* Handle - 0x0063 */
    /* 41 - CurrentTime - Characteristic Value */
    {
        /* Property */
        GATT_DB_READ |
        GATT_DB_WRITE |
        GATT_DB_CHAR_NOTIFY_PROPERTY,

        /* Auxillary Property */
        GATT_DB_FIXED_LENGTH_PROPERTY,

        /* Value Length */
        10,

        /* Next Attribute Type Index */
        ATT_INVALID_ATTR_HANDLE_VAL,

        /* UUID Offset */
        118,

        /* Value */
        gatt_value_arr + 259,
    },

    /* Handle - 0x0064 */
    /* 41 - CurrentTime - CCD */
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
        gatt_db_peer_specific_val_arr + 12,
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

    /* 2 - ServiceChanged */
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

    /* 5 - BatteryLevel */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0010,

        /* Characteristic Service Index */
        3,
    },

    /* 6 - WeightMeasurment */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0015,

        /* Characteristic Service Index */
        4,
    },

    /* 7 - WeightScaleFeature */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0018,

        /* Characteristic Service Index */
        4,
    },

    /* 8 - BodyCompositionMeasurement */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x001B,

        /* Characteristic Service Index */
        5,
    },

    /* 9 - BodyCompositionFeature */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x001E,

        /* Characteristic Service Index */
        5,
    },

    /* 10 - FirstName */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0021,

        /* Characteristic Service Index */
        6,
    },

    /* 11 - LastName */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0023,

        /* Characteristic Service Index */
        6,
    },

    /* 12 - Email */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0025,

        /* Characteristic Service Index */
        6,
    },

    /* 13 - Age */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0027,

        /* Characteristic Service Index */
        6,
    },

    /* 14 - Gender */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0029,

        /* Characteristic Service Index */
        6,
    },

    /* 15 - DateofBirth */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x002B,

        /* Characteristic Service Index */
        6,
    },

    /* 16 - Weight */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x002D,

        /* Characteristic Service Index */
        6,
    },

    /* 17 - Height */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x002F,

        /* Characteristic Service Index */
        6,
    },

    /* 18 - VO2Max */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0031,

        /* Characteristic Service Index */
        6,
    },

    /* 19 - HeartrateMax */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0033,

        /* Characteristic Service Index */
        6,
    },

    /* 20 - RestingHeartRate */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0035,

        /* Characteristic Service Index */
        6,
    },

    /* 21 - MaxRecommHeartRate */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0037,

        /* Characteristic Service Index */
        6,
    },

    /* 22 - AerobicThreshold */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0039,

        /* Characteristic Service Index */
        6,
    },

    /* 23 - AnaerobicThreshold */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x003B,

        /* Characteristic Service Index */
        6,
    },

    /* 24 - SportTypeAerobicAnaerobicThresholds */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x003D,

        /* Characteristic Service Index */
        6,
    },

    /* 25 - DateofThresholdAssessment */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x003F,

        /* Characteristic Service Index */
        6,
    },

    /* 26 - WaistCircumference */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0041,

        /* Characteristic Service Index */
        6,
    },

    /* 27 - HipCircumference */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0043,

        /* Characteristic Service Index */
        6,
    },

    /* 28 - FatBurnHeartRateLowerLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0045,

        /* Characteristic Service Index */
        6,
    },

    /* 29 - FatBurnHeartRateUpperLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0047,

        /* Characteristic Service Index */
        6,
    },

    /* 30 - AerobicHeartRateLowerLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0049,

        /* Characteristic Service Index */
        6,
    },

    /* 31 - AerobicHeartRateUpperrLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x004B,

        /* Characteristic Service Index */
        6,
    },

    /* 32 - AnaerobicHeartRateLowerLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x004D,

        /* Characteristic Service Index */
        6,
    },

    /* 33 - AnaerobicHeartRateUpperLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x004F,

        /* Characteristic Service Index */
        6,
    },

    /* 34 - FiveZoneHeartRateLimits */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0051,

        /* Characteristic Service Index */
        6,
    },

    /* 35 - ThreeZoneHeartRat Limits */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0053,

        /* Characteristic Service Index */
        6,
    },

    /* 36 - TwoZoneHeartRateLimit */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x0055,

        /* Characteristic Service Index */
        6,
    },

    /* 37 - DatabaseChangeIncrement */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0057,

        /* Characteristic Service Index */
        6,
    },

    /* 38 - UserIndex */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x005A,

        /* Characteristic Service Index */
        6,
    },

    /* 39 - UserControlPoint */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x005C,

        /* Characteristic Service Index */
        6,
    },

    /* 40 - Language */
    {
        {
            /* Number of attributes in characteristic including the definition */
            2,
        },

        /* Characteristic Start Handle */
        0x005F,

        /* Characteristic Service Index */
        6,
    },

    /* 41 - CurrentTime */
    {
        {
            /* Number of attributes in characteristic including the definition */
            3,
        },

        /* Characteristic Start Handle */
        0x0062,

        /* Characteristic Service Index */
        7,
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
        0x000E,

        /* Characteristic Start Index */
        3,

        /* Characteristic End Index */
        4
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
        0x000F,

        /* Service End Handle */
        0x0012,

        /* Characteristic Start Index */
        5,

        /* Characteristic End Index */
        5
    },

    /* 4 - Weight Scale */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0013,

        /* Service End Handle */
        0x0019,

        /* Characteristic Start Index */
        6,

        /* Characteristic End Index */
        7
    },

    /* 5 - BodyComposition */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SECONDARY_SERVICE_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x001A,

        /* Service End Handle */
        0x001F,

        /* Characteristic Start Index */
        8,

        /* Characteristic End Index */
        9
    },

    /* 6 - UserData */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0020,

        /* Service End Handle */
        0x0060,

        /* Characteristic Start Index */
        10,

        /* Characteristic End Index */
        40
    },

    /* 7 - CurrentTimeService */
    {
        {
            /* Number of attributes in Service */
            1,
        },

        /* Service Description */
        GATT_DB_SER_NO_SECURITY_PROPERTY |
        GATT_DB_SER_SUPPORT_ANY_LINK_TYPE,

        /* Service Start Handle */
        0x0061,

        /* Service End Handle */
        0x0064,

        /* Characteristic Start Index */
        41,

        /* Characteristic End Index */
        41
    },
};
#endif /* ((defined WSS) && (defined GATT_DB)) */

