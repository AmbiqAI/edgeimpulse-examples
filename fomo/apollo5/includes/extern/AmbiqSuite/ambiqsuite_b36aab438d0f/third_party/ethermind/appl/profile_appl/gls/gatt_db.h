/**
 *  \file gatt_db.h
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_GATT_DB_
#define _H_GATT_DB_

/**
 * addgroup gatt_db_module
 */

/**
 * defgroup gatt_db_tuneable_param Tuneable Parameters
 * {
 * This section defines the Tuneable Constants of Data Base Module.
 */

/** Number of Characteristics in the data base */
#define GATT_CHARACTERISTIC_COUNT            18

/** Number of Services in the data base */
#define GATT_SERVICE_COUNT                   5

/** Number of Characteristics that are configurable by the client */
#define GATT_DB_MAX_CONFIGUREABLE_CHAR       5

/** Maximum Length of any Characteristic Value/Descriptor */
#define GATT_DB_MAX_VAL_LENGTH                32

#define GATT_VALUE_ARRAY_SIZE                 1

#define GATT_CONST_VALUE_ARRAY_SIZE           205

#define GATT_DB_PEER_VALUE_ARRAY_SIZE         10

#define GATT_DB_MAX_ATTRIBUTES                48

#define GATT_UUID_ARRAY_SIZE                  66

#define GATT_DB_MAX_TYPE_COUNT                27

#define GATT_DB_MAX_PEER_CONFIGURATION        \
        (GATT_DB_PEER_VALUE_ARRAY_SIZE * BT_MAX_DEVICE_QUEUE_SIZE)

/** \} */

/** Service Instance Reference */

/** GATT Service */
#define GATT_SER_GATT_INST                 0

/** GAP Service */
#define GATT_SER_GAP_INST                  1

/** DeviceInformation Service */
#define GATT_SER_DEV_INFO_INST             2

/** Battery Service */
#define GATT_SER_BATTERY_INST              3

/** Blood Glucosemeter Service */
#define GATT_SER_GLUCOSE_METER_INST        4

/** Characteristic Instance Reference */

/** Service Changed */
#define GATT_CHAR_SER_CHNGD_INST           0

/** Device Name */
#define GATT_CHAR_DEV_NAME_INST            1

/** Appearance */
#define GATT_CHAR_APPEARANCE_INST          2

/** Peripheral Preferred Connection Paramters */
#define GATT_CHAR_PRFRD_CONNCTN_PARAM_INST 3

/** ManufacturerName */
#define GATT_CHAR_MAN_NAME_INST            4

/** ModelNumber */
#define GATT_CHAR_MODEL_NO_INST            5

/** SerialNumber */
#define GATT_CHAR_SL_NO_INST               6

/** HardwareRevision */
#define GATT_CHAR_HW_REV_INST              7

/** FirmwareRevision */
#define GATT_CHAR_FW_REV_INST              8

/** SoftwareRevision */
#define GATT_CHAR_SW_REV_INST              9

/** SystemId */
#define GATT_CHAR_SYS_ID_INST              10

/** RegCertDataList */
#define GATT_CHAR_REG_CERT_DATA_INST       11

/** PnPID */
#define GATT_CHAR_PNP_ID_INST              12

/** BatteryLevel */
#define GATT_CHAR_BATTERY_LVL_INST         13

/** Glucose Measurement */
#define GATT_CHAR_GL_MSRMNT_INST           14

/** Glucose Context */
#define GATT_CHAR_GL_CNTXT_INST            15

/** Glucose Features */
#define GATT_CHAR_GL_FEATURES_INST         16

/** Record Access Control Point (RACP) */
#define GATT_CHAR_GL_RACP_INST             17

#endif /* _H_GATT_DB_ */

