
/**
 *  \file appl_cts.h
 *
 *  Application Header File for Current Time.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_CTS_
#define _H_APPL_CTS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Functions */
void appl_cts_init(void);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_cts_connect(APPL_HANDLE  * appl_handle);
void appl_send_cur_time (APPL_HANDLE   * handle);
void appl_cts_server_reinitialize (void);
API_RESULT appl_cts_get_current_time
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           );
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
API_RESULT appl_update_current_time (ATT_VALUE    * value);
API_RESULT appl_update_local_time (ATT_VALUE    * value);

void appl_profile_menu_handler (void);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_cts_init()
#define APPL_PROFILE_CONNECT(x) appl_cts_connect(x)
#define APPL_SEND_MEASUREMENT(x) appl_send_cur_time(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_cts_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_cts_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_CTS_ */


