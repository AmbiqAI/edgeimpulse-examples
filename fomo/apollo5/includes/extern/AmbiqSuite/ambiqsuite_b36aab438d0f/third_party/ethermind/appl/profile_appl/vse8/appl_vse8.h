
/**
 *  \file appl_vse8.h
 *
 *  Application Header File for Heart Rate Sensor.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_VSE8_
#define _H_APPL_VSE8_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* --------------------------------------------- Functions */
void appl_vse8_init(void);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_vse8_connect(APPL_HANDLE  * appl_handle);
void appl_send_vse8_measurement (APPL_HANDLE   * handle);
void appl_vse8_server_reinitialize (void);
void appl_vse8_handle_ind_complete
     (
         APPL_HANDLE * handle,
         UCHAR * event_data,
         UINT16 datalen
     );
void appl_vse8_handle_ntf_complete
     (
         APPL_HANDLE * handle,
         UCHAR * event_data,
         UINT16 datalen
     );
void appl_vse8_handle_mtu_update_complete
     (
         APPL_HANDLE * handle,
         UINT16 mtu
     );

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_vse8_init()
#define APPL_PROFILE_CONNECT(x) appl_vse8_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_vse8_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) \
                                 appl_vse8_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_vse8_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)\
                                 appl_vse8_handle_ntf_complete((x),(y),(z))
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)\
                                 appl_vse8_handle_ind_complete((x),(y),(z))
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x, y) \
                                 appl_vse8_handle_mtu_update_complete((x),(y))

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_VSE8_ */


