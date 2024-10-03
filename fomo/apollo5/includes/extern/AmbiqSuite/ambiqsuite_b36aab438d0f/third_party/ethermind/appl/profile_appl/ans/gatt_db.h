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
#define GATT_CHARACTERISTIC_COUNT            9

/** Number of Services in the data base */
#define GATT_SERVICE_COUNT                   4

/** Number of Characteristics that are configurable by the client */
#define GATT_DB_MAX_CONFIGUREABLE_CHAR       4

/** Maximum Length of any Characteristic Value/Descriptor */
#define GATT_DB_MAX_VAL_LENGTH                14

#define GATT_VALUE_ARRAY_SIZE                 5

#define GATT_CONST_VALUE_ARRAY_SIZE           67

#define GATT_DB_PEER_VALUE_ARRAY_SIZE         8

#define GATT_DB_MAX_ATTRIBUTES                28

#define GATT_UUID_ARRAY_SIZE                  46

#define GATT_DB_MAX_TYPE_COUNT                17

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

/** AlertNotification Service */
#define GATT_SER_ALRT_NTF_INST             3

/** Characteristic Instance Reference */

/** DeviceName */
#define GATT_CHAR_DEV_NAME_INST            0

/** Appearance */
#define GATT_CHAR_APPEARANCE_INST          1

/** Service Changed */
#define GATT_CHAR_SER_CHNGD_INST           2

/** BatteryLevel */
#define GATT_CHAR_BATTERY_LVL_INST         3

/** SupportedNewAlertCategory */
#define GATT_CHAR_SUP_NEW_ALRT_CAT_INST    4

/** NewAlert */
#define GATT_CHAR_NEW_ALRT_INST            5

/** SupportedUnreadAlertCategory */
#define GATT_CHAR_SUP_UNREAD_ALRT_CAT_INST 6

/** UnreadAlertStatus */
#define GATT_UNREAD_ALRT_STATUS_INST       7

/** AlertNotificationControlPoint */
#define GATT_CHAR_ALRT_NTF_CP_INST         8

#endif /* _H_GATT_DB_ */

