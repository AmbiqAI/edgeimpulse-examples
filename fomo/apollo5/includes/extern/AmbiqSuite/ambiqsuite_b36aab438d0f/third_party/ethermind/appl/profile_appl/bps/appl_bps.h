
/**
 *  \file appl_bps.h
 *
 *  Application Header File for Blood Pressure Monitor.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_BPS_
#define _H_APPL_BPS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* #define APPL_BPS_SINGLE_BOND_SUPPORT */

/* --------------------------------------------- Functions */
void appl_bps_init(void);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_bps_connect(APPL_HANDLE  * appl_handle);
void appl_send_blood_pressure_measurement (APPL_HANDLE * handle);
void appl_bps_server_reinitialize (void);
void appl_bps_start_timer(APPL_HANDLE * hndl, UINT16 hndl_len);

/* Profile handling */
#define APPL_PROFILE_INIT appl_bps_init
#define APPL_PROFILE_CONNECT(x) appl_bps_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_blood_pressure_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_bps_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_bps_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_BPS_ */

