
/**
 *  \file appl_hts.h
 *
 *  Health Thermometer Server Application Header.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_HTS_
#define _H_APPL_HTS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Functions */
void appl_hts_init(void);
void appl_timer_expiry_handler (void *data_param, UINT16 datalen);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_hts_connect(APPL_HANDLE  * appl_handle);
void appl_send_temperature_measurement (APPL_HANDLE  * handle);
void appl_hts_server_reinitialize (void);
API_RESULT appl_validate_msrmt_interval (GATT_DB_HANDLE * handle, UINT16 data);
void appl_msrmt_intrvl_timer_expiry_handler (void *data_param, UINT16 datalen);

/* Profile handling */
#define APPL_PROFILE_INIT appl_hts_init
#define APPL_PROFILE_CONNECT(x) appl_hts_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_temperature_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_hts_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_temperature_measurment_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#ifndef USE_BT_PL_TIMER
#define APPL_USE_IDLE_TIMER
#endif
#define APPL_IDLE_TIMEOUT           30

#endif /* _H_APPL_HTS_ */

