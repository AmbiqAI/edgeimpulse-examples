
/**
 *  \file appl_rtus.h
 *
 *  Application Header File for Reference Time Update Service.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_RTUS_
#define _H_APPL_RTUS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Functions */
void appl_rtus_init(void);
void appl_rtus_server_reinitialize (void);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_rtus_init()
#define APPL_PROFILE_CONNECT(x)
#define APPL_SEND_MEASUREMENT(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_rtus_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_rtus_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30

API_RESULT appl_rtus_config_time_update
           (
               GATT_DB_HANDLE    * handle,
               ATT_VALUE         * value
           );

API_RESULT appl_rtus_get_time_update_state
           (
               GATT_DB_HANDLE    * handle,
               ATT_VALUE         * value
           );
void appl_profile_menu_handler (void);

#endif /* _H_APPL_RTUS_ */


