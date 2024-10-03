
/**
 *  \file appl_pxr.h
 *
 *  Application Header File for Health Thermometer Sensor.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_PXR_
#define _H_APPL_PXR_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl_gap.h"

/* --------------------------------------------- Global Definitions */
#define APPL_PXR_NO_ALERT                  0
#define APPL_PXR_MILD_ALERT                1
#define APPL_PXR_HIGH_ALERT                2

#define APPL_PXR_LINK_LOSS                 0
#define APPL_IM_ALERT                      1

#define APPL_PXR_INVALID_ALRT_VALUE   (APPL_ERR_ID | 0x0080)

#define APPL_PXR_VALID_ALERT_LEVEL(lvl)\
    (((lvl) > APPL_PXR_HIGH_ALERT)?BT_FALSE:BT_TRUE)

/* --------------------------------------------- Functions */
void appl_pxr_init (void);
void appl_pxr_alert_user
     (
          DEVICE_HANDLE    * handle,
          UCHAR              alert_reason
     );
char * appl_pxr_get_alert_str (UCHAR  alert_level);
API_RESULT appl_pxr_set_alert_level
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           );

API_RESULT appl_pxr_get_tx_power_level
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           );

API_RESULT appl_pxr_get_tx_power_level_complete
           (
                UCHAR     status,
                UINT16    conn_handle,
                UCHAR     tx_power_level
           );

/* Profile handling */
#define APPL_PROFILE_INIT() appl_pxr_init()
#define APPL_PROFILE_CONNECT(...)
#define APPL_SEND_MEASUREMENT(...)
#define APPL_PROFILE_DISCONNECT_HANDLER(x)\
        appl_pxr_alert_user(x,APPL_PXR_LINK_LOSS)
#define GATT_DB_PROFILE_HANDLER  gatt_db_pxr_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

/* Proximitiy does not use Idle Timer */

#endif /* _H_APPL_PXR_ */

