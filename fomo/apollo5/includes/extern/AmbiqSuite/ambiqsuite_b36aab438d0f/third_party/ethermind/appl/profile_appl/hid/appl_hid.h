
/**
 *  \file appl_hts.h
 *
 *  Human Interface Device Server Application Header.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_HID_
#define _H_APPL_HID_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"

/* --------------------------------------------- Functions */
void appl_hid_init(void);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
API_RESULT appl_manage_wwr (GATT_DB_HANDLE * handle, GATT_DB_PARAMS * params);
void appl_hid_connect(APPL_HANDLE  * appl_handle);
void appl_send_hid_event (APPL_HANDLE  * handle);
void appl_hid_server_reinitialize (void);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_hid_init()
#define APPL_PROFILE_CONNECT(x) appl_hid_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_hid_event(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_hid_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_hid_event_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_HID_ */

