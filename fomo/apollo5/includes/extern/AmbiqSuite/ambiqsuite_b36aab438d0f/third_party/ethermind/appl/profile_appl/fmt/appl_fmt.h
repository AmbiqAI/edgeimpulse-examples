
/**
 *  \file appl_fmt.h
 *
 *  Application Header File for Find Me Target.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_FMT_
#define _H_APPL_FMT_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl_gap.h"

/* --------------------------------------------- Global Definitions */
#define APPL_FMT_NO_ALERT                  0
#define APPL_FMT_MILD_ALERT                1
#define APPL_FMT_HIGH_ALERT                2

#define APPL_IM_ALERT                      1

#define APPL_FMT_INVALID_ALRT_VALUE   (APPL_ERR_ID | 0x0080)

#define APPL_FMT_VALID_ALERT_LEVEL(lvl)\
    (((lvl) > APPL_FMT_HIGH_ALERT)?BT_FALSE:BT_TRUE)

/* --------------------------------------------- Functions */
void appl_fmt_init (void);
void appl_fmt_alert_user
     (
          DEVICE_HANDLE    * handle
     );
char * appl_fmt_get_alert_str (UCHAR  alert_level);
API_RESULT appl_fmt_set_alert_level
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           );
void appl_fmt_reinitialize (void);

/* Profile handling */
#define APPL_PROFILE_INIT() appl_fmt_init()
#define APPL_PROFILE_CONNECT(...)
#define APPL_SEND_MEASUREMENT(...)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_fmt_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_fmt_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

/* Find Me does not use Idle Timer */

#endif /* _H_APPL_FMT_ */

