
/**
 *  \file appl_hrs.h
 *
 *  Application Header File for Heart Rate Sensor.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_HRS_
#define _H_APPL_HRS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* ----------------------------------------- Data Types/ Structures */
typedef struct
{
    UCHAR    index;

    UCHAR    length;

}APPL_HRS_OBS_DATA_INFO;

/* --------------------------------------------- Global Definitions */
/** LSB of error code has to be spec defined */
#define APPL_HR_CNTRL_PNT_NOT_SUPPORTED   (APPL_ERR_ID | 0x80)

/* --------------------------------------------- Functions */
void appl_hrs_init(void);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_hrs_connect(APPL_HANDLE  * appl_handle);
void appl_send_heart_rate_measurement (APPL_HANDLE   * handle);
void appl_hrs_server_reinitialize (void);
API_RESULT appl_hr_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           );

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_hrs_init()
#define APPL_PROFILE_CONNECT(x) appl_hrs_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_heart_rate_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_hrs_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_hrs_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_HRS_ */


