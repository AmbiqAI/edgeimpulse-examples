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
#define GATT_CHARACTERISTIC_COUNT            7

/** Number of Services in the data base */
#define GATT_SERVICE_COUNT                   6

/** Number of Characteristics that are configurable by the client */
#define GATT_DB_MAX_CONFIGUREABLE_CHAR       3

/** Maximum Length of any Characteristic Value/Descriptor */
#define GATT_DB_MAX_VAL_LENGTH                16

#define GATT_VALUE_ARRAY_SIZE                 2

#define GATT_CONST_VALUE_ARRAY_SIZE           59

#define GATT_DB_PEER_VALUE_ARRAY_SIZE         5

#define GATT_DB_MAX_ATTRIBUTES                24

#define GATT_UUID_ARRAY_SIZE                  44

#define GATT_DB_MAX_TYPE_COUNT                16

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

/** Link Loss Service */
#define GATT_SER_LINK_LOSS_INST            3

/** Immediate Alert Service */
#define GATT_SER_IMMDT_ALRT_INST           4

/** Tx Power Level Service */
#define GATT_SER_TX_POWER_LVL_INST         5

/** Characteristic Instance Reference */

/** DeviceName */
#define GATT_CHAR_DEV_NAME_INST            0

/** Appearance */
#define GATT_CHAR_APPEARANCE_INST          1

/** Service Changed */
#define GATT_CHAR_SER_CHNGD_INST           2

/** BatteryLevel */
#define GATT_CHAR_BATTERY_LVL_INST         3

/** AlertLevel */
#define GATT_CHAR_LL_ALRT_LVL_INST         4

/** AlertLevel */
#define GATT_CHAR_IM_ALRT_LVL_INST         5

/** TxPower */
#define GATT_CHAR_TX_POWER_INST            6

#endif /* _H_GATT_DB_ */

