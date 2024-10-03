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
#define GATT_CHARACTERISTIC_COUNT            6

/** Number of Services in the data base */
#define GATT_SERVICE_COUNT                   4

/** Number of Characteristics that are configurable by the client */
#define GATT_DB_MAX_CONFIGUREABLE_CHAR       2

/** Maximum Length of any Characteristic Value/Descriptor */
#define GATT_DB_MAX_VAL_LENGTH                14

#define GATT_VALUE_ARRAY_SIZE                 1

#define GATT_CONST_VALUE_ARRAY_SIZE           52

#define GATT_DB_PEER_VALUE_ARRAY_SIZE         4

#define GATT_DB_MAX_ATTRIBUTES                20

#define GATT_UUID_ARRAY_SIZE                  40

#define GATT_DB_MAX_TYPE_COUNT                14

#define GATT_DB_MAX_PEER_CONFIGURATION        \
        (GATT_DB_PEER_VALUE_ARRAY_SIZE * BT_MAX_DEVICE_QUEUE_SIZE)

/** \} */

/** Service Instance Reference */

/** GAP Service */
#define GATT_SER_GAP_INST                  0

/** GATT Service */
#define GATT_SER_GATT_INST                 1

/** Battery Service */
#define GATT_SER_BATTERY_INST              2

/** ReferenceTimeUpdate Service */
#define GATT_SER_REF_TIME_UPDATE_INST      3

/** Characteristic Instance Reference */

/** DeviceName */
#define GATT_CHAR_DEV_NAME_INST            0

/** Appearance */
#define GATT_CHAR_APPEARANCE_INST          1

/** Service Changed */
#define GATT_CHAR_SER_CHNGD_INST           2

/** BatteryLevel */
#define GATT_CHAR_BATTERY_LVL_INST         3

/** TimeUpdateControlPoint */
#define GATT_CHAR_TIME_UPDATE_CP_INST      4

/** TimeUpdateState */
#define GATT_CHAR_TIME_UPDATE_STATE_INST   5

#endif /* _H_GATT_DB_ */

