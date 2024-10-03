
/**
 *  \file appl_NDCS.h
 *
 *  Sample Profile Application Header.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_NDCS_
#define _H_APPL_NDCS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Functions */
void appl_ndcs_init(void);
void appl_timer_expiry_handler (void *data, UINT16 datalen);

void appl_ndcs_connect(APPL_HANDLE  * appl_handle);
void appl_ndcs_server_reinitialize (void);

API_RESULT appl_ndcs_get_current_time
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           );
void appl_profile_menu_handler (void);

/* Profile handling */
#define APPL_PROFILE_INIT appl_ndcs_init
#define APPL_PROFILE_CONNECT(x) appl_ndcs_connect(x)
#define APPL_SEND_MEASUREMENT(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_ndcs_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_ndcs_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#ifndef USE_BT_PL_TIMER
#define APPL_USE_IDLE_TIMER
#endif /* USE_BT_PL_TIMER */
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_NDCS_ */

